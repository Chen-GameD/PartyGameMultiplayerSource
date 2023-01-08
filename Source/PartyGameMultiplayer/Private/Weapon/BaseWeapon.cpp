// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/BaseWeapon.h"
#include "Weapon/DamageTypeToCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
//#include "Components/CapsuleComponent.h"
//#include "Particles/ParticleSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
//#include "UObject/ConstructorHelpers.h"
#include "Components/PrimitiveComponent.h"
#include "Net/UnrealNetwork.h"
//#include "Character/MCharacter.h"
#include "GameFramework/Character.h"
#include "Character/MCharacter.h"
#include "../PartyGameMultiplayerCharacter.h"
#include "LevelInteraction/MinigameMainObjective.h"
#include "Weapon/DamageManager.h"


// Sets default values
ABaseWeapon::ABaseWeapon()
{
	/*
	What it means to set bReplicates to true ?
		(1) If there is any member variables that is set to Replicated [by us], then we must set bReplicates to true in the constructor
		(2) No matter if there is ... or not, by setting bReplicates to true, we will get some variables replicated [automatically].
			Not exactly sure what these variables will be, but at least include:
				Construction(When creating an instance on server, the instance would also be created on all clients.)
				Destroyed(When the instance is destroyed on server, the instance on all clients would also be destroyed)
				Movement(i.e. Transform Info of every parts of the Actor)
				Velocity
			What should not be replicated:
				Mesh([After construction, each instance will have the mesh generated] as the visual representation so that all clients could see it,
					It will change according to the change of Movement)
				Animation(It will change according to the change of Movement/Velocity)

		The Pawns and Characters in most templates have replication enabled by default.
		Regularly, anything interacts with more than one player should have replication enabled.
	*/
	bReplicates = true;
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	IsPickedUp = false;
	WeaponType = EnumWeaponType::None;
	AttackType = EnumAttackType::OneHit;
	bAttackOverlap = false;

	DisplayCase = CreateDefaultSubobject<UBoxComponent>(TEXT("Box_DisplayCase"));
	DisplayCase->SetCollisionProfileName(TEXT("Custom"));
	/*DisplayCase->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));*/
	DisplayCase->SetBoxExtent(FVector3d(100.0f, 100.0f, 100.0f));
	DisplayCase->SetupAttachment(RootComponent);

	//Definition for the Mesh that will serve as our visual representation.
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	/*
		If make the Collision Type as "BlockAllDynamic" / "PhysicsActor" and attach the weapon onto character's hand, the game would crash!
		Right now the weapon's collision type is set to Trigger by default, this could be changed in the future. But I guess Trigger would be
		the most convenient solution
	*/
	// TODO: Not necessarily be Trigger
	WeaponMesh->SetCollisionProfileName(TEXT("Trigger"));
	WeaponMesh->SetupAttachment(DisplayCase);	
	WeaponMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	WeaponMesh->SetRelativeScale3D(FVector(0.75f, 0.75f, 0.75f));

	AttackOnEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("AttackOnNiagaraEffect"));
	AttackOnEffect->SetupAttachment(WeaponMesh);

	DamageType = UDamageType::StaticClass();
	Damage = 0.0f;
	AccumulatedTimeToGenerateDamage = TNumericLimits<float>::Max();
	DamageGenerationCounter = 0;

	MiniGameDamageType = UDamageTypeToCharacter::StaticClass();
	
	MiniGameDamage = 0.0f;
	MiniGameAccumulatedTimeToGenerateDamage = TNumericLimits<float>::Max();
}


void ABaseWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Client duty
	if (GetLocalRole() != ROLE_Authority) 
	{
		if (!IsPickedUp)
		{
			PlayAnimationWhenNotBeingPickedUp(DeltaTime);
			// The following 2 settings can be commented because the OnRep_Transform() should already have done the work. Just a safety measure.
			DisplayCase->SetWorldLocation(RootLocation);
			DisplayCase->SetWorldRotation(RootRotation);
			DisplayCase->SetWorldScale3D(RootScale);
		}
	}

	// Server duty
	if (GetLocalRole() == ROLE_Authority)
	{
		// Deal with the collision of the display box
		if (!IsPickedUp)
		{
			RootLocation = DisplayCase->GetComponentLocation();
			RootRotation = DisplayCase->GetComponentRotation();
			RootScale = DisplayCase->GetComponentScale();
		}

		// Apply constant damage
		if (AttackType == EnumAttackType::Constant)
		{
			if (IsPickedUp)
			{
				for (auto& Elem : AttackObjectMap)
				{
					Elem.Value += DeltaTime;
					if (dynamic_cast<ACharacter*>(Elem.Key))
					{						
						if (AccumulatedTimeToGenerateDamage < Elem.Value)
						{
							GenerateDamage(Elem.Key);
							Elem.Value = 0;
						}
					}
					else if (dynamic_cast<AMinigameMainObjective*>(Elem.Key))
					{
						if (MiniGameAccumulatedTimeToGenerateDamage < Elem.Value)
						{
							GenerateDamage(Elem.Key);
							Elem.Value = 0;
						}
					}					
				}
			}
		}
	}
}


void ABaseWeapon::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Replicate specific variables
	//DOREPLIFETIME(ABaseWeapon, HoldingPlayer);
	DOREPLIFETIME(ABaseWeapon, bAttackOn);
	DOREPLIFETIME(ABaseWeapon, bAttackOverlap);
	DOREPLIFETIME(ABaseWeapon, IsPickedUp);
	DOREPLIFETIME(ABaseWeapon, MovementComponent);
	DOREPLIFETIME(ABaseWeapon, DamageGenerationCounter);
	DOREPLIFETIME(ABaseWeapon, RootLocation);
	DOREPLIFETIME(ABaseWeapon, RootRotation);
	DOREPLIFETIME(ABaseWeapon, RootScale);
}

// should only be called on server
void ABaseWeapon::AttackStart()
{
	if (bAttackOn)
		return;

	bAttackOn = true;
	check(GetOwner() != nullptr);
	// Server duty
	SetActorEnableCollision(bAttackOn);
	//AttackDetectComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	//AttackDetectComponent->OnActorEnableCollisionChanged();
	// if listen server
	check(GetOwner() != nullptr);
	if (auto owner = dynamic_cast<APawn*>(GetOwner()))
	{		
		if (owner->IsLocallyControlled() && owner->GetLocalRole() == ROLE_Authority)
		{
			OnRep_bAttackOn();
		}
	}	
}

// should only be called on server
void ABaseWeapon::AttackStop()
{
	if (!bAttackOn)
		return;

	bAttackOn = false;
	// Server duty
	SetActorEnableCollision(bAttackOn);
	//AttackDetectComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// if listen server
	check(GetOwner() != nullptr);
	if (auto owner = dynamic_cast<APawn*>(GetOwner()))
	{
		if (owner->IsLocallyControlled() && owner->GetLocalRole() == ROLE_Authority)
		{
			OnRep_bAttackOn();
		}
	}
}

// should only be called on server
void ABaseWeapon::GetPickedUp(ACharacter* pCharacter)
{ 
	IsPickedUp = true;

	HoldingPlayer = pCharacter;
	check(HoldingPlayer != nullptr);
	SetInstigator(HoldingPlayer);
	SetOwner(HoldingPlayer);
	
	WeaponMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	WeaponMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));

	SetActorEnableCollision(false);
	DisplayCase->SetCollisionProfileName(TEXT("NoCollision"));
	DisplayCase->SetSimulatePhysics(false);
	DisplayCase->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
	// if listen server
	if (pCharacter->IsLocallyControlled() && pCharacter->GetLocalRole() == ROLE_Authority)
	{
		OnRep_IsPickedUp();
	}
}

// should only be called on server
void ABaseWeapon::GetThrewAway()
{	
	IsPickedUp = false;

	auto OldHoldingPlayer = HoldingPlayer;
	HoldingPlayer = nullptr;
	SetInstigator(HoldingPlayer);
	SetOwner(HoldingPlayer);
	
	SetActorEnableCollision(true);
	DisplayCase->SetCollisionProfileName(TEXT("Custom"));
	DisplayCase->SetSimulatePhysics(true);
	DisplayCase->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	DisplayCase->SetCollisionResponseToAllChannels(ECR_Block);
	DisplayCase->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		
	// if listen server
	if (OldHoldingPlayer->IsLocallyControlled() && OldHoldingPlayer->GetLocalRole() == ROLE_Authority)
	{
		OnRep_IsPickedUp();
	}
}


void ABaseWeapon::CheckInitilization()
{
	check(WeaponMesh);
	check(DisplayCase);
	check(AttackDetectComponent);
}


// Called when the game starts or when spawned
void ABaseWeapon::BeginPlay()
{
	Super::BeginPlay();

	CheckInitilization();

	// Register "What would happen when the weapon hit/un-hit something(other players/item/env...)" on server
	if (GetLocalRole() == ROLE_Authority)
	{
		DisplayCase->SetSimulatePhysics(true);
		DisplayCase->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		DisplayCase->SetCollisionResponseToAllChannels(ECR_Block);
		DisplayCase->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		
		AttackDetectComponent->SetCollisionProfileName(TEXT("Trigger"));
		AttackDetectComponent->SetGenerateOverlapEvents(true);
		// Note: 
		// (1) AddDynamic won't be auto-filled by Intellisense, but it does exist and can be used!
		// (2) The function used here has different parameters than the one used in OnComponentHit
		// (3) Though it has the "ABaseWeapon" class name, the callback function [can be] the OnWeaponOverlapBegin
		//		of [the inherited class](for instance, AWeaponFork) if OnWeaponOverlapBegin is virtual and gets overridden. 
		AttackDetectComponent->OnComponentBeginOverlap.AddDynamic(this, &ABaseWeapon::OnAttackOverlapBegin);
		AttackDetectComponent->OnComponentEndOverlap.AddDynamic(this, &ABaseWeapon::OnAttackOverlapEnd);
		DisplayCase->OnComponentBeginOverlap.AddDynamic(this, &ABaseWeapon::OnDisplayCaseOverlapBegin);

		// OnComponentHit doesn't work with "Trigger" Collision Type
		//StaticMesh->OnComponentHit.AddDynamic(this, &ABaseWeapon::OnWeaponImpact);

	}
	else
	{
		// If not a server, turn off some high-cost behaviors
		SetActorEnableCollision(false);
	}

	/*if(AttackOnEffect)
		AttackOnEffect->Deactivate();*/
}


//void ABaseWeapon::Destroyed()
//{
//	
//}


void ABaseWeapon::PlayAnimationWhenNotBeingPickedUp(float DeltaTime)
{
	FVector NewLocation = WeaponMesh->GetComponentLocation();
	FRotator NewRotation = WeaponMesh->GetComponentRotation();
	float RunningTime = GetGameTimeSinceCreation();
	float DeltaHeight = (FMath::Sin(RunningTime + DeltaTime) - FMath::Sin(RunningTime));
	//Scale our height by a factor of 20
	NewLocation.Z += DeltaHeight * 20.0f;   
	//Rotate by 20 degrees per second
	float DeltaRotation = DeltaTime * 60.0f;    
	NewRotation.Yaw += DeltaRotation;
	WeaponMesh->SetWorldLocation(NewLocation);
	WeaponMesh->SetWorldRotation(NewRotation);
}

// client-only duty
void ABaseWeapon::GenerateAttackHitEffect()
{
	//FVector spawnLocation = GetActorLocation();
	FVector spawnLocation = AttackDetectComponent->GetComponentLocation();
	UGameplayStatics::SpawnEmitterAtLocation(this, AttackHitEffect, spawnLocation, FRotator::ZeroRotator, true, EPSCPoolMethod::AutoRelease);	
}


void ABaseWeapon::GenerateDamage(class AActor* DamagedActor)
{
	// Note: Be careful if the weapon has an instigator or not.
	// if it doesn't and the 3rd parameter of ApplyDamage() is set to GetInstigator()->Controller, the game would crash when overlap happens
	// (The projectile in the demo has an instigator, because the instigator parameter is assigned when the the character spawns it in HandleFire function)
	
	check(GetInstigator()->Controller);
	check(DamagedActor);

	//if (auto pCharacter = Cast<AMCharacter>(DamagedActor))
	//{		
	//	//UGameplayStatics::ApplyDamage(DamagedActor, Damage, GetInstigator()->Controller, this, DamageType);
	//	pCharacter->TakeDamageRe(Damage, WeaponType, GetInstigator()->Controller, this);
	//	DamageGenerationCounter = (DamageGenerationCounter + 1) % 1000;

	//	// To transfer
	//	// knockback
	//	/*check(HoldingPlayer);
	//	FRotator AttackerControlRotation = HoldingPlayer->GetControlRotation();
	//	FVector3d AttackerControlDir = AttackerControlRotation.RotateVector(FVector3d::ForwardVector);
	//	pCharacter->AccumulateAttackedBuff(EnumAttackBuff::Knockback, 1.0f, AttackerControlDir, GetInstigator()->Controller, this);*/
	//	// paralysis
	//	//pCharacter->AccumulateAttackedBuff(EnumAttackBuff::Paralysis, 1.0f, FVector3d::Zero(), GetInstigator()->Controller, this);
	//	// burning
	//	//pCharacter->AccumulateAttackedBuff(EnumAttackBuff::Burning, 0.5f, FVector3d::Zero(), GetInstigator()->Controller, this);

	//}
	//else if(dynamic_cast<AMinigameMainObjective*>(DamagedActor))
	//{
	//	UGameplayStatics::ApplyDamage(DamagedActor, MiniGameDamage, GetInstigator()->Controller, this, MiniGameDamageType);
	//	//DamagedActor->TakeDamageRe(Damage, GetInstigator()->Controller, this);
	//	DamageGenerationCounter = (DamageGenerationCounter + 1) % 1000;
	//}

	bool result = DamageManager::DealDamageAndBuffBetweenActors(this, DamagedActor);
	if(result)
		DamageGenerationCounter = (DamageGenerationCounter + 1) % 1000;
}

void ABaseWeapon::OnRep_Transform()
{
	if (!IsPickedUp)
	{
		/*GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("location: %f, %f, %f"), 
			RootLocation.X, RootLocation.Y, RootLocation.Z));*/

		//SetActorTransform(Transform);
		DisplayCase->SetWorldLocation(RootLocation);
		DisplayCase->SetWorldRotation(RootRotation);
		DisplayCase->SetWorldScale3D(RootScale);
	}	
}

void ABaseWeapon::OnRep_bAttackOn()
{
	if (bAttackOn)
	{		
		AttackOnEffect->Activate();
	}
	else
	{
		AttackOnEffect->Deactivate();
	}
}

void ABaseWeapon::OnRep_bAttackOverlap()
{
	/*if (bAttackOverlap)
	{
		GenerateAttackHitEffect();		
	}*/
}

void ABaseWeapon::OnRep_IsPickedUp()
{
	if (AttackOnEffect)
		AttackOnEffect->Deactivate();

	WeaponMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	WeaponMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));

	/*if (IsPickedUp)
	{
		check(HoldingPlayer != nullptr);
		SetInstigator(HoldingPlayer);
		SetOwner(HoldingPlayer);		
	}
	else
	{
		SetInstigator(nullptr);
		SetOwner(nullptr);
	}*/
}


void ABaseWeapon::OnRep_DamageGenerationCounter()
{
	GenerateAttackHitEffect();
}


// only is called on server
// "What would happen when the weapon hit something(other players/item/env...)"
void ABaseWeapon::OnAttackOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, 
	class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	/*if(dynamic_cast<APartyGameMultiplayerCharacter*>(OtherActor))
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("AttackDetectComponent overlaps a PartyGameMultiplayerCharacter"));*/

	if (IsPickedUp && GetOwner())
	{
		// What will happen if the weapon hit another player
		if (dynamic_cast<ACharacter*>(OtherActor) && OtherActor != GetInstigator())
		{
			if (!AttackObjectMap.Contains(OtherActor))
			{
				AttackObjectMap.Add(OtherActor);
			}
			AttackObjectMap[OtherActor] = 0.0f;
			bAttackOverlap = true;

			if (AttackType == EnumAttackType::OneHit)
			{
				GenerateDamage(OtherActor);
			}
			// if listen server
			if (auto owner = dynamic_cast<APawn*>(GetOwner()))
			{
				if (owner->IsLocallyControlled() && owner->GetLocalRole() == ROLE_Authority)
				{
					OnRep_bAttackOverlap();
				}
			}
			/*	If a weapon needs to be destroyed when overlapping with another character, [do it here, inside the if statement instead of outside].
				Reason: There is a possiblity that the weapon is overlapping with the controlling character when being generated(like generated in hand).
				In that case, it is very likely that the weapon would be spawned firstly and attached secondly. If accidentally putting the destroy
				function outside this statement, the weapon reference/pointer would become invalid before attaching and cause a crash.
			*/			
		}
		// What will happen if the weapon hit AMinigameMainObjective
		else if (dynamic_cast<AMinigameMainObjective*>(OtherActor))
		{
			if (!AttackObjectMap.Contains(OtherActor))
			{
				AttackObjectMap.Add(OtherActor);
			}
			AttackObjectMap[OtherActor] = 0.0f;
			bAttackOverlap = true;

			if (AttackType == EnumAttackType::OneHit)
			{
				GenerateDamage(OtherActor);
			}
		}
	}
}


// only is called on server
// "What would happen when the weapon un-hit something(other players/item/env...)"
void ABaseWeapon::OnAttackOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, 
	class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (IsPickedUp)
	{
		if (dynamic_cast<ACharacter*>(OtherActor) && OtherActor != GetInstigator())
		{
			if (AttackObjectMap.Contains(OtherActor))
			{
				AttackObjectMap.Remove(OtherActor);
			}
			bAttackOverlap = false;
		}
		else if (dynamic_cast<AMinigameMainObjective*>(OtherActor))
		{
			if (AttackObjectMap.Contains(OtherActor))
			{
				AttackObjectMap.Remove(OtherActor);
			}
			bAttackOverlap = false;
		}
	}
}


// only is called on server
void ABaseWeapon::OnDisplayCaseOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, 
	class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsPickedUp)
	{
		if (auto pCharacter = dynamic_cast<APartyGameMultiplayerCharacter*>(OtherActor))
		{
			GetPickedUp(pCharacter);
			pCharacter->TouchWeapon(this);
		}
	}
	else
	{
		
	}
}

FString ABaseWeapon::GetWeaponName() const
{
	return WeaponName;
}

ACharacter* ABaseWeapon::GetHoldingPlayer() const
{
	return HoldingPlayer;
}
