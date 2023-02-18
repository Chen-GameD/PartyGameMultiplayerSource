// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/BaseWeapon.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Components/PrimitiveComponent.h"
//#include "Components/CapsuleComponent.h"
//#include "Particles/ParticleSystem.h"
//#include "UObject/ConstructorHelpers.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"

#include "Weapon/DamageTypeToCharacter.h"
#include "Weapon/DamageManager.h"
#include "Weapon/BaseProjectile.h"
#include "Weapon/WeaponDataHelper.h"
#include "Character/MCharacter.h"
#include "../PartyGameMultiplayerCharacter.h"
#include "LevelInteraction/MinigameMainObjective.h"

TMap<EnumWeaponType, FString> ABaseWeapon::WeaponEnumToString_Map = 
{
	{EnumWeaponType::None, "None"},
	{EnumWeaponType::Fork, "Fork"},
	{EnumWeaponType::Blower, "Blower"},
	{EnumWeaponType::Lighter, "Lighter"},
	{EnumWeaponType::Alarm, "Alarm"},
	{EnumWeaponType::Flamethrower, "Flamethrower"},
	{EnumWeaponType::Flamefork, "Flamefork"},
	{EnumWeaponType::Taser, "Taser"},
	{EnumWeaponType::Alarmgun, "Alarmgun"},
	{EnumWeaponType::Bomb, "Bomb"},
	{EnumWeaponType::Cannon, "Cannon"},
};

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
	/*DisplayCase->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));*/
	DisplayCase->SetBoxExtent(FVector3d(100.0f, 100.0f, 100.0f));
	DisplayCase->SetupAttachment(RootComponent);

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	// If make the Collision Type as "BlockAllDynamic" / "PhysicsActor" and attach the weapon onto character's hand, the game would crash.
	WeaponMesh->SetCollisionProfileName(TEXT("Trigger"));
	WeaponMesh->SetupAttachment(DisplayCase);	
	WeaponMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	WeaponMesh->SetRelativeScale3D(FVector(0.75f, 0.75f, 0.75f));

	SpawnProjectilePointMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpawnProjectilePointMesh"));
	SpawnProjectilePointMesh->SetCollisionProfileName(TEXT("Trigger"));
	SpawnProjectilePointMesh->SetupAttachment(WeaponMesh);

	AttackOnEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("AttackOnNiagaraEffect"));
	AttackOnEffect->SetupAttachment(WeaponMesh);

	DamageType = UDamageType::StaticClass();
	Damage = 0.0f;
	AccumulatedTimeToGenerateDamage = TNumericLimits<float>::Max();
	DamageGenerationCounter = 0;

	CD_MaxEnergy = CD_LeftEnergy = CD_DropSpeed = CD_RecoverSpeed = 0.0f;

	MiniGameDamageType = UDamageTypeToCharacter::StaticClass();
	MiniGameDamage = 0.0f;
	//MiniGameAccumulatedTimeToGenerateDamage = TNumericLimits<float>::Max();
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
			
			// If the weapon has cd
			if (0 < CD_MaxEnergy)
			{
				if (bAttackOn)
				{
					if (0 < CD_LeftEnergy)
					{
						CD_LeftEnergy -= CD_DropSpeed * DeltaTime;
						CD_LeftEnergy = FMath::Max(CD_LeftEnergy, 0.0f);
						GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Blue, FString::Printf(TEXT("CD_LeftEnergy: %f"), CD_LeftEnergy));
					}					
				}
				else
				{
					if (CD_LeftEnergy < CD_MaxEnergy)
					{
						CD_LeftEnergy += CD_RecoverSpeed * DeltaTime;
						CD_LeftEnergy = FMath::Min(CD_LeftEnergy, CD_MaxEnergy);
						GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Blue, FString::Printf(TEXT("CD_LeftEnergy: %f"), CD_LeftEnergy));
					}					
				}	
				if (CD_LeftEnergy < CD_MinEnergyToAttak)
					AttackStop();
			}
			// Apply constant damage
			if (AttackType == EnumAttackType::Constant && bAttackOn && CD_MinEnergyToAttak <= CD_LeftEnergy)
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
						//if (MiniGameAccumulatedTixmeToGenerateDamage < Elem.Value)
						if (AccumulatedTimeToGenerateDamage < Elem.Value)
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
	DOREPLIFETIME(ABaseWeapon, CD_LeftEnergy);
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
	//  Set DisplayCaseCollision to inactive
	DisplayCaseCollisionSetActive(false);

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
	//  Set DisplayCaseCollision to active
	DisplayCaseCollisionSetActive(true);

	// Listen server
	if (GetNetMode() == NM_ListenServer)
	{
		OnRep_IsPickedUp();
	}
}


void ABaseWeapon::AttackStart()
{
	if (bAttackOn || !GetOwner())
		return;
	if (AttackType == EnumAttackType::Constant && CD_LeftEnergy <= 0)
		return;

	bAttackOn = true;
	// Listen server
	if (GetNetMode() == NM_ListenServer)
	{
		OnRep_bAttackOn();
	}
	ApplyDamageCounter = 0;

	SetActorEnableCollision(bAttackOn);
	//AttackDetectComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	//AttackDetectComponent->OnActorEnableCollisionChanged();
}


void ABaseWeapon::AttackStop()
{
	if (!bAttackOn || !GetOwner())
		return;

	check(GetOwner() != nullptr);

	bAttackOn = false;
	// Listen server
	if (GetNetMode() == NM_ListenServer)
	{
		OnRep_bAttackOn();
	}
	ApplyDamageCounter = 0;

	AttackObjectMap.Empty();
	SetActorEnableCollision(bAttackOn);
	//AttackDetectComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}


void ABaseWeapon::CheckInitilization()
{
	check(WeaponMesh);
	check(DisplayCase);
	//check(AttackDetectComponent);
}


void ABaseWeapon::BeginPlay()
{
	Super::BeginPlay();

	CheckInitilization();

	// Server: Register collision callback functions
	if (GetLocalRole() == ROLE_Authority)
	{
		// Assign some member variables
		if (AWeaponDataHelper::DamageManagerDataAsset)
		{
			// AccumulatedTimeToGenerateDamage
			FString ParName = WeaponName;
			if (AWeaponDataHelper::DamageManagerDataAsset->AccumulatedTimeToGenerateDamage_Map.Contains(ParName))
				AccumulatedTimeToGenerateDamage = AWeaponDataHelper::DamageManagerDataAsset->AccumulatedTimeToGenerateDamage_Map[ParName];
			// CoolDown
			ParName = WeaponName + "_" + "CD_MaxEnergy";
			if (AWeaponDataHelper::DamageManagerDataAsset->CoolDown_Map.Contains(ParName))
				CD_LeftEnergy = CD_MaxEnergy = AWeaponDataHelper::DamageManagerDataAsset->CoolDown_Map[ParName];
			ParName = WeaponName + "_" + "CD_DropSpeed";
			if (AWeaponDataHelper::DamageManagerDataAsset->CoolDown_Map.Contains(ParName))
				CD_DropSpeed = AWeaponDataHelper::DamageManagerDataAsset->CoolDown_Map[ParName];
			ParName = WeaponName + "_" + "CD_RecoverSpeed";
			if (AWeaponDataHelper::DamageManagerDataAsset->CoolDown_Map.Contains(ParName))
				CD_RecoverSpeed = AWeaponDataHelper::DamageManagerDataAsset->CoolDown_Map[ParName];
			ParName = WeaponName + "_" + "CD_MinEnergyToAttak";
			if (AWeaponDataHelper::DamageManagerDataAsset->CoolDown_Map.Contains(ParName))
				CD_MinEnergyToAttak = AWeaponDataHelper::DamageManagerDataAsset->CoolDown_Map[ParName];
		}
		//  Set DisplayCaseCollision to active
		DisplayCaseCollisionSetActive(true);
		DisplayCase->OnComponentBeginOverlap.AddDynamic(this, &ABaseWeapon::OnDisplayCaseOverlapBegin);
		
		if (AttackDetectComponent)
		{
			AttackDetectComponent->SetCollisionProfileName(TEXT("Trigger"));
			AttackDetectComponent->SetGenerateOverlapEvents(true);

			AttackDetectComponent->OnComponentBeginOverlap.AddDynamic(this, &ABaseWeapon::OnAttackOverlapBegin);
			AttackDetectComponent->OnComponentEndOverlap.AddDynamic(this, &ABaseWeapon::OnAttackOverlapEnd);
		}
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


void ABaseWeapon::DisplayCaseCollisionSetActive(bool IsActive)
{
	if (IsActive)
	{
		DisplayCase->SetCollisionProfileName(TEXT("Custom"));
		DisplayCase->SetSimulatePhysics(true);
		DisplayCase->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		DisplayCase->SetCollisionResponseToAllChannels(ECR_Block);
		DisplayCase->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	}
	else
	{
		DisplayCase->SetCollisionProfileName(TEXT("NoCollision"));
		DisplayCase->SetSimulatePhysics(false);
		DisplayCase->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void ABaseWeapon::GenerateAttackHitEffect()
{
	// TODO: assign a new variable(decided by AttackDetectComponent or Projectile) to spawnLocation
	if (AttackDetectComponent)
	{
		FVector spawnLocation = AttackDetectComponent->GetComponentLocation();
		UGameplayStatics::SpawnEmitterAtLocation(this, AttackHitEffect, spawnLocation, FRotator::ZeroRotator, true, EPSCPoolMethod::AutoRelease);
	}	
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

	//bool success = DamageManager::DealDamageAndBuffBetweenActors(this, DamagedActor);
	bool success = ADamageManager::DealDamageAndBuffBetweenActors(this, DamagedActor);
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

			if (AttackType == EnumAttackType::OneHit && ApplyDamageCounter == 0)
			{
				GenerateDamageLike(OtherActor);
				ApplyDamageCounter++;
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
				AttackObjectMap.Remove(OtherActor);
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


void ABaseWeapon::SpawnProjectile()
{
	//auto pCharacter = GetOwner();
	//if (pCharacter && SpecificProjectileClass)
	//{
	//	FVector spawnLocation = GetActorLocation() + (GetActorRotation().Vector() * 100.0f) + (GetActorUpVector() * 50.0f);
	//	FRotator spawnRotation = (pCharacter->GetActorRotation().Vector() + pCharacter->GetActorUpVector()).Rotation();  // character up 45 degree

	//	FActorSpawnParameters spawnParameters;
	//	spawnParameters.Instigator = GetInstigator();
	//	spawnParameters.Owner = this;

	//	//ABaseProjectile* spawnedProjectile = NewObject<ABaseProjectile>(this, SpecificProjectileClass);
	//	ABaseProjectile* spawnedProjectile = GetWorld()->SpawnActor<ABaseProjectile>(SpecificProjectileClass, spawnLocation, spawnRotation, spawnParameters);
	//}
}
