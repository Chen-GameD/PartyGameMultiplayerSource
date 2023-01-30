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


ABaseWeapon::ABaseWeapon()
{
	bReplicates = true;
	// Can turn this off to improve performance if you don't need it.
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

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	// If make the Collision Type as "BlockAllDynamic" / "PhysicsActor" and attach the weapon onto character's hand, the game would crash.
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

	// Server
	if (GetLocalRole() == ROLE_Authority)
	{
		// Deal with the collision of the display box
		if (!IsPickedUp)
		{
			DisplayCaseLocation = DisplayCase->GetComponentLocation();
			DisplayCaseRotation = DisplayCase->GetComponentRotation();
			DisplayCaseScale = DisplayCase->GetComponentScale();
		}
		else
		{
			// Apply constant damage
			if (AttackType == EnumAttackType::Constant)
			{
				for (auto& Elem : AttackObjectMap)
				{
					Elem.Value += DeltaTime;
					if (Cast<ACharacter>(Elem.Key))
					{
						if (AccumulatedTimeToGenerateDamage < Elem.Value)
						{
							GenerateDamageLike(Elem.Key);
							Elem.Value = 0;
						}
					}
					else if (Cast<AMinigameMainObjective>(Elem.Key))
					{
						if (MiniGameAccumulatedTimeToGenerateDamage < Elem.Value)
						{
							GenerateDamageLike(Elem.Key);
							Elem.Value = 0;
						}
					}
				}

			}
		}		
	}

	// Client(Listen Server)
	if (GetLocalRole() != ROLE_Authority || GetNetMode() == NM_ListenServer)
	{
		if (!IsPickedUp)
		{
			PlayAnimationWhenNotBeingPickedUp(DeltaTime);
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
	DOREPLIFETIME(ABaseWeapon, DisplayCaseLocation);
	DOREPLIFETIME(ABaseWeapon, DisplayCaseRotation);
	DOREPLIFETIME(ABaseWeapon, DisplayCaseScale);
}


void ABaseWeapon::GetPickedUp(ACharacter* pCharacter)
{
	IsPickedUp = true;

	HoldingPlayer = pCharacter;
	check(HoldingPlayer != nullptr);
	SetInstigator(HoldingPlayer);
	SetOwner(HoldingPlayer);

	SetActorEnableCollision(false);
	// Change DisplayCase Collision Type
	DisplayCase->SetCollisionProfileName(TEXT("NoCollision"));
	DisplayCase->SetSimulatePhysics(false);
	DisplayCase->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Listen server
	if (GetNetMode() == NM_ListenServer)
	{
		OnRep_IsPickedUp();
	}
}


void ABaseWeapon::GetThrewAway()
{
	IsPickedUp = false;

	auto OldHoldingPlayer = HoldingPlayer;
	HoldingPlayer = nullptr;
	SetInstigator(HoldingPlayer);
	SetOwner(HoldingPlayer);

	SetActorEnableCollision(true);
	// Change DisplayCase Collision Type
	DisplayCase->SetCollisionProfileName(TEXT("Custom"));
	DisplayCase->SetSimulatePhysics(true);
	DisplayCase->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	DisplayCase->SetCollisionResponseToAllChannels(ECR_Block);
	DisplayCase->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// Listen server
	if (GetNetMode() == NM_ListenServer)
	{
		OnRep_IsPickedUp();
	}
}


void ABaseWeapon::AttackStart()
{
	if (bAttackOn)
		return;

	check(GetOwner() != nullptr);
	bAttackOn = true;

	SetActorEnableCollision(bAttackOn);
	//AttackDetectComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	//AttackDetectComponent->OnActorEnableCollisionChanged();
	
	// Listen server
	if (GetNetMode() == NM_ListenServer)
	{
		OnRep_bAttackOn();
	}	
}


void ABaseWeapon::AttackStop()
{
	if (!bAttackOn)
		return;

	check(GetOwner() != nullptr);
	bAttackOn = false;

	SetActorEnableCollision(bAttackOn);
	//AttackDetectComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Listen server
	if (GetNetMode() == NM_ListenServer)
	{
		OnRep_bAttackOn();
	}
}


void ABaseWeapon::CheckInitilization()
{
	check(WeaponMesh);
	check(DisplayCase);
	check(AttackDetectComponent);
}


void ABaseWeapon::BeginPlay()
{
	Super::BeginPlay();

	CheckInitilization();

	// Server: Register collision callback functions
	if (GetLocalRole() == ROLE_Authority)
	{
		DisplayCase->SetSimulatePhysics(true);
		DisplayCase->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		DisplayCase->SetCollisionResponseToAllChannels(ECR_Block);
		DisplayCase->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		
		AttackDetectComponent->SetCollisionProfileName(TEXT("Trigger"));
		AttackDetectComponent->SetGenerateOverlapEvents(true);
		AttackDetectComponent->OnComponentBeginOverlap.AddDynamic(this, &ABaseWeapon::OnAttackOverlapBegin);
		AttackDetectComponent->OnComponentEndOverlap.AddDynamic(this, &ABaseWeapon::OnAttackOverlapEnd);
		DisplayCase->OnComponentBeginOverlap.AddDynamic(this, &ABaseWeapon::OnDisplayCaseOverlapBegin);
	}
	// Client
	else
	{
		// Disable some high-cost behaviors
		SetActorEnableCollision(false);
	}
}


void ABaseWeapon::Destroyed()
{
	Super::Destroyed();
}


void ABaseWeapon::PlayAnimationWhenNotBeingPickedUp(float DeltaTime)
{
	FVector NewLocation = WeaponMesh->GetComponentLocation();
	FRotator NewRotation = WeaponMesh->GetComponentRotation();
	float RunningTime = GetGameTimeSinceCreation();
	float DeltaHeight = (FMath::Sin(RunningTime + DeltaTime) - FMath::Sin(RunningTime));
	// Scale our height by a factor of 20
	NewLocation.Z += DeltaHeight * 20.0f;   
	// Rotate by 20 degrees per second
	float DeltaRotation = DeltaTime * 60.0f;    
	NewRotation.Yaw += DeltaRotation;
	WeaponMesh->SetWorldLocation(NewLocation);
	WeaponMesh->SetWorldRotation(NewRotation);
}


void ABaseWeapon::GenerateAttackHitEffect()
{
	FVector spawnLocation = AttackDetectComponent->GetComponentLocation();
	UGameplayStatics::SpawnEmitterAtLocation(this, AttackHitEffect, spawnLocation, FRotator::ZeroRotator, true, EPSCPoolMethod::AutoRelease);	
}


void ABaseWeapon::GenerateDamageLike(class AActor* DamagedActor)
{
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

	bool success = DamageManager::DealDamageAndBuffBetweenActors(this, DamagedActor);
	if (success)
	{
		DamageGenerationCounter = (DamageGenerationCounter + 1) % 1000;
		// Listen Server
		if (GetNetMode() == NM_ListenServer)
		{
			OnRep_DamageGenerationCounter();
		}
	}
		
}

void ABaseWeapon::OnRep_DisplayCaseTransform()
{
	if (!IsPickedUp)
	{
		//SetActorTransform(Transform);
		DisplayCase->SetWorldLocation(DisplayCaseLocation);
		DisplayCase->SetWorldRotation(DisplayCaseRotation);
		DisplayCase->SetWorldScale3D(DisplayCaseScale);
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
}


void ABaseWeapon::OnRep_DamageGenerationCounter()
{
	GenerateAttackHitEffect();
}


void ABaseWeapon::OnAttackOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, 
	class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IsPickedUp && GetOwner())
	{
		// What will happen if the weapon hit another player
		if( (Cast<ACharacter>(OtherActor) && OtherActor != GetOwner()) ||
			Cast<AMinigameMainObjective>(OtherActor) )
		{
			if (!AttackObjectMap.Contains(OtherActor))
				AttackObjectMap.Add(OtherActor);
			AttackObjectMap[OtherActor] = 0.0f;
			bAttackOverlap = true;

			if (AttackType == EnumAttackType::OneHit)
			{
				GenerateDamageLike(OtherActor);
			}
			// Listen server
			if (GetNetMode() == NM_ListenServer)
			{
				OnRep_bAttackOverlap();
			}		
		}
	}
}


void ABaseWeapon::OnAttackOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, 
	class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (IsPickedUp && GetOwner())
	{
		if ((Cast<ACharacter>(OtherActor) && OtherActor != GetOwner()) ||
			Cast<AMinigameMainObjective>(OtherActor))
		{
			if (AttackObjectMap.Contains(OtherActor))
			{
				//AttackObjectMap.Remove(OtherActor);
				// change should be faster than remove
				AttackObjectMap[OtherActor] = 0.0f;
			}
			bAttackOverlap = false;
		}
	}
}


void ABaseWeapon::OnDisplayCaseOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, 
	class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsPickedUp)
	{
		if (auto pCharacter = Cast<APartyGameMultiplayerCharacter>(OtherActor))
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
