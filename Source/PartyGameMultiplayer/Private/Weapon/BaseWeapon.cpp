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
#include "Weapon/DamageType/MeleeDamageType.h"
#include "Weapon/DamageManager.h"
#include "Weapon/BaseProjectile.h"
#include "Weapon/WeaponDataHelper.h"
#include "Character/MCharacter.h"
#include "../PartyGameMultiplayerCharacter.h"
#include "LevelInteraction/MinigameObject/MinigameObj_Enemy.h"

ABaseWeapon::ABaseWeapon()
{
	bReplicates = true;	
	PrimaryActorTick.bCanEverTick = true;  // Can turn this off to improve performance if you don't need it.

	IsPickedUp = false;
	Server_BigWeaponShouldSink = false;
	HasBeenCombined = false;
	IsBigWeapon = false;
	WeaponType = EnumWeaponType::None;
	WeaponName = "";
	AttackType = EnumAttackType::OneHit;  // default is one-hit
	bAttackOn = false;
	bAttackOverlap = false;
	HoldingController = nullptr;

	DisplayCase = CreateDefaultSubobject<UBoxComponent>(TEXT("Box_DisplayCase"));
	DisplayCase->SetupAttachment(RootComponent);
	DisplayCase->SetBoxExtent(FVector3d(90.0f, 90.0f, 90.0f));
	
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(DisplayCase);
	WeaponMesh->SetCollisionProfileName(TEXT("Trigger"));	
	WeaponMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	WeaponMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));

	SpawnProjectilePointMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpawnProjectilePointMesh"));
	SpawnProjectilePointMesh->SetupAttachment(WeaponMesh);
	SpawnProjectilePointMesh->SetCollisionProfileName(TEXT("Trigger"));	

	AttackOnEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("AttackOnNiagaraEffect"));
	AttackOnEffect->SetupAttachment(WeaponMesh);
	AttackOnEffect->bAutoActivate = false;

	HaloEffect_NSComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("HaloEffect"));
	HaloEffect_NSComponent->SetupAttachment(WeaponMesh);
	HaloEffect_NSComponent->bAutoActivate = true;
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> DefaultHaloEffect(TEXT("/Game/ArtAssets/Niagara/NS_WeaponHalo.NS_WeaponHalo"));
	if (DefaultHaloEffect.Succeeded())
		HaloEffect_NSComponent->SetAsset(DefaultHaloEffect.Object);

	ShowUpEffect_NC = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ShowUpVfx"));
	ShowUpEffect_NC->SetupAttachment(WeaponMesh);
	ShowUpEffect_NC->bAutoActivate = false;
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> DefaultShowUpEffect(TEXT("/Game/ArtAssets/Niagara/NS_Confetti.NS_Confetti"));
	if (DefaultShowUpEffect.Succeeded())
		ShowUpEffect_NC->SetAsset(DefaultShowUpEffect.Object);

	DamageType = UDamageType::StaticClass();
	//Damage = 0.0f;
	//DamageGenerationCounter = 0;
	CD_MaxEnergy = CD_LeftEnergy = CD_DropSpeed = CD_RecoverSpeed = CD_RecoverDelay = 0.0f;
	CD_CanRecover = true;
	TimePassed_SinceAttackStop = 0.0f;
	LastTime_DisplayCaseTransformBeenReplicated = -1.0f;

	MiniGameDamageType = UDamageTypeToCharacter::StaticClass();
	//MiniGameDamage = 0.0f;

	if (WeaponMesh)
	{
		WeaponMeshDefaultRelativeLocation = WeaponMesh->GetRelativeLocation();
		WeaponMeshDefaultRelativeRotation = WeaponMesh->GetRelativeRotation();
		WeaponMeshDefaultRelativeScale = WeaponMesh->GetRelativeScale3D();
	}
}


void ABaseWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CurDeltaTime = DeltaTime;

	// Server
	if (GetLocalRole() == ROLE_Authority)
	{
		// Not being picked up
		if (!IsPickedUp)
		{
			if (DisplayCaseLocation != DisplayCase->GetComponentLocation()
				|| DisplayCaseRotation != DisplayCase->GetComponentRotation()
				|| DisplayCaseScale != DisplayCase->GetComponentScale())
				LastTime_DisplayCaseTransformBeenReplicated = GetWorld()->TimeSeconds;
			DisplayCaseLocation = DisplayCase->GetComponentLocation();
			DisplayCaseRotation = DisplayCase->GetComponentRotation();
			DisplayCaseScale = DisplayCase->GetComponentScale();

			if (CD_LeftEnergy < CD_MaxEnergy && CD_CanRecover)
			{
				CD_LeftEnergy += CD_RecoverSpeed * DeltaTime;
				CD_LeftEnergy = FMath::Min(CD_LeftEnergy, CD_MaxEnergy);
			}

			if (IsBigWeapon && Server_BigWeaponShouldSink)
			{
				float SinkSpeed = 75.0f;
				SetActorLocation(GetActorLocation() + FVector::DownVector * DeltaTime * SinkSpeed);
				if (GetActorLocation().Z < -250.0f)
					Destroy();
			}
		}
		// being picked up
		else
		{		
			if (!HasBeenCombined)
			{
				// If the weapon has CD
				if (0 < CD_MaxEnergy)
				{
					// if the attack type is constant 
					if (AttackType == EnumAttackType::Constant)
					{
						if (bAttackOn)
						{
							if (0 < CD_LeftEnergy)
							{
								CD_LeftEnergy -= CD_DropSpeed * DeltaTime;
								CD_LeftEnergy = FMath::Max(CD_LeftEnergy, 0.0f);
								if (CD_LeftEnergy < CD_MinEnergyToAttak)
									AttackStop();
							}
						}
						else
						{
							if (CD_LeftEnergy < CD_MaxEnergy && CD_CanRecover)
							{
								CD_LeftEnergy += CD_RecoverSpeed * DeltaTime;
								CD_LeftEnergy = FMath::Min(CD_LeftEnergy, CD_MaxEnergy);
							}
						}
					}
					// if the attack type is not constant 
					else
					{
						if (CD_LeftEnergy < CD_MaxEnergy && CD_CanRecover)
						{
							CD_LeftEnergy += CD_RecoverSpeed * DeltaTime;
							CD_LeftEnergy = FMath::Min(CD_LeftEnergy, CD_MaxEnergy);
						}
					}
					if (!CD_CanRecover)
					{
						if (CD_RecoverDelay < TimePassed_SinceAttackStop)
							CD_CanRecover = true;
					}
				}
				// Apply constant damage
				if (AttackType == EnumAttackType::Constant && bAttackOn && CD_MinEnergyToAttak <= CD_LeftEnergy)
				{
					// Prevent AttackObjectMap from getting written by the overlap functions during the iteration
					FScopeLock Lock(&DataGuard);
					auto CopiedAttackObjectMap = AttackObjectMap;					
					for (auto Elem : CopiedAttackObjectMap)
					{			
						AActor* DamagedActor = Elem.Key;						
						if (DamagedActor)
						{
							bool IsDamagedActorDead = false;
							if (auto pMCharacter = Cast<AMCharacter>(DamagedActor))
							{
								if (pMCharacter->GetCurrentHealth() <= 0)
									IsDamagedActorDead = true;
							}
							else if (auto pMinigameObj_Enemy = Cast<AMinigameObj_Enemy>(DamagedActor))
							{
								if (pMinigameObj_Enemy->GetCurrentHealth() <= 0)
									IsDamagedActorDead = true;
							}
							if(!IsDamagedActorDead)
								ADamageManager::TryApplyDamageToAnActor(this, HoldingController, UDamageType::StaticClass(), DamagedActor, DeltaTime);
						}
					}
				}				
			}			
		}		
	}

	// Client(Listen Server)
	if (GetLocalRole() != ROLE_Authority || GetNetMode() == NM_ListenServer)
	{
		if (!IsPickedUp && !IsBigWeapon)
		{
			PlayAnimationWhenNotBeingPickedUp(DeltaTime);
		}
	}

	// Assign TimePassed_SinceAttackStop(We will use this value on both clients and server)
	if (!bAttackOn)
	{
		double tmpVal = TimePassed_SinceAttackStop + DeltaTime;
		if (TNumericLimits<float>::Max() < tmpVal)
			TimePassed_SinceAttackStop = TNumericLimits<float>::Max();
		else
			TimePassed_SinceAttackStop = tmpVal;
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
	DOREPLIFETIME(ABaseWeapon, HasBeenCombined);	
	DOREPLIFETIME(ABaseWeapon, MovementComponent);
	DOREPLIFETIME(ABaseWeapon, DisplayCaseLocation);
	DOREPLIFETIME(ABaseWeapon, DisplayCaseRotation);
	DOREPLIFETIME(ABaseWeapon, DisplayCaseScale);
}


void ABaseWeapon::GetPickedUp(ACharacter* pCharacter)
{
	if (IsPickedUp)
		return;

	if (!pCharacter)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Unexpected situation in ABaseWeapon::GetPickedUp"));
		return;
	}
	HoldingController = pCharacter->GetController();
	if (!HoldingController)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Unexpected situation in ABaseWeapon::GetPickedUp"));
		return;
	}
	SetInstigator(pCharacter);
	SetOwner(pCharacter);
	PreHoldingController = pCharacter->GetController();

	SetActorEnableCollision(false);
	DisplayCaseCollisionSetActive(false);

	IsPickedUp = true;
	if (GetNetMode() == NM_ListenServer)
		OnRep_IsPickedUp();
}


// only will be called on element weapons
void ABaseWeapon::GetThrewAway()
{
	if (!IsPickedUp)
		return;

	AttackStop();

	HasBeenCombined = false;
	HoldingController = nullptr;
	SetInstigator(nullptr);
	SetOwner(nullptr);

	SetActorEnableCollision(true);
	if (!IsBigWeapon)
	{
		//  Set DisplayCaseCollision to active
		DisplayCaseCollisionSetActive(true);
	}
	else
	{
		DisplayCase->SetCollisionProfileName(TEXT("Custom"));
		DisplayCase->SetSimulatePhysics(true);
		DisplayCase->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		DisplayCase->SetCollisionResponseToAllChannels(ECR_Block);
		DisplayCase->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		DisplayCase->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore);
		FTimerHandle TimerHandle;
		float SinkDelay = 3.0f;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
			{
				DisplayCase->SetSimulatePhysics(false);
				DisplayCase->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				DisplayCase->SetCollisionProfileName(TEXT("NoCollision"));			
				Server_BigWeaponShouldSink = true;
			}, SinkDelay, false);		
	}	
	IsPickedUp = false;
	if (GetNetMode() == NM_ListenServer)
		OnRep_IsPickedUp();
}

void ABaseWeapon::SafeDestroyWhenGetThrew()
{
	AttackStop();
	SetActorEnableCollision(false);
	SetActorHiddenInGame(true);

	FTimerHandle TimerHandle;
	float Delay = 1.0f;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
		{
			Destroy();
		}, Delay, false);
}

void ABaseWeapon::AttackStart(float AttackTargetDistance)
{
	if (bAttackOn || !GetOwner())
		return;	

	// If the weapon has cd
	if (0 < CD_MaxEnergy)
	{
		if (AttackType == EnumAttackType::Constant)
		{
			if (CD_LeftEnergy <= 0)
				return;
		}
		else
		{
			if (CD_MinEnergyToAttak <= CD_LeftEnergy)
				CD_LeftEnergy -= CD_MinEnergyToAttak;
			else
				return;
		}		
	}

	bAttackOn = true;
	// Listen server
	if (GetNetMode() == NM_ListenServer)
		OnRep_bAttackOn();
	ApplyDamageCounter = 0;
	
	if (AttackType != EnumAttackType::SpawnProjectile)
	{
		SetActorEnableCollision(bAttackOn);
		//AttackDetectComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		//AttackDetectComponent->OnActorEnableCollisionChanged();
	}
	else
	{
		SpawnProjectile(AttackTargetDistance);
	}
}


void ABaseWeapon::AttackStop()
{
	if (!bAttackOn || !GetOwner())
		return;

	bAttackOn = false;
	// Listen server
	if (GetNetMode() == NM_ListenServer)
	{
		OnRep_bAttackOn();
	}
	ApplyDamageCounter = 0;
	AttackObjectMap.Empty();

	if (AttackType == EnumAttackType::Constant)
		CD_CanRecover = false;
	TimePassed_SinceAttackStop = 0.0f;

	SetActorEnableCollision(bAttackOn);
	//AttackDetectComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}


void ABaseWeapon::BeginPlay()
{
	Super::BeginPlay();

	GetWeaponName(); // to update weapon name

	// Assign some member variables(we want both the server and client have these values)
	if (AWeaponDataHelper::DamageManagerDataAsset)
	{
		// CoolDown	
		FString ParName = "";
		FString ThisWeaponName = WeaponName;
		if (IsBigWeapon)
			ThisWeaponName = "Big" + WeaponName;
		ParName = ThisWeaponName + "_" + "CD_MaxEnergy";
		if (AWeaponDataHelper::DamageManagerDataAsset->CoolDown_Map.Contains(ParName))
			CD_LeftEnergy = CD_MaxEnergy = AWeaponDataHelper::DamageManagerDataAsset->CoolDown_Map[ParName];
		ParName = ThisWeaponName + "_" + "CD_DropSpeed";
		if (AWeaponDataHelper::DamageManagerDataAsset->CoolDown_Map.Contains(ParName))
			CD_DropSpeed = AWeaponDataHelper::DamageManagerDataAsset->CoolDown_Map[ParName];
		ParName = ThisWeaponName + "_" + "CD_RecoverSpeed";
		if (AWeaponDataHelper::DamageManagerDataAsset->CoolDown_Map.Contains(ParName))
			CD_RecoverSpeed = AWeaponDataHelper::DamageManagerDataAsset->CoolDown_Map[ParName];
		ParName = ThisWeaponName + "_" + "CD_RecoverDelay";
		if (AWeaponDataHelper::DamageManagerDataAsset->CoolDown_Map.Contains(ParName))
			CD_RecoverDelay = AWeaponDataHelper::DamageManagerDataAsset->CoolDown_Map[ParName];
		ParName = ThisWeaponName + "_" + "CD_MinEnergyToAttak";
		if (AWeaponDataHelper::DamageManagerDataAsset->CoolDown_Map.Contains(ParName))
		{
			// Even when it is constant attack type, do not set CD_MinEnergyToAttak as 0 in the table! 
			// Set it as a number slightly bigger, like 0.01f
			CD_MinEnergyToAttak = AWeaponDataHelper::DamageManagerDataAsset->CoolDown_Map[ParName];
			if (CD_MinEnergyToAttak <= 0.0f)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Unexpected situation in ABaseWeapon::BeginPlay"));
				CD_MinEnergyToAttak = 0.01f;
			}				
		}
	}

	// Server: Register collision callback functions
	if (GetLocalRole() == ROLE_Authority)
	{
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

	//AttackStop();
}


void ABaseWeapon::PlayAnimationWhenNotBeingPickedUp(float DeltaTime)
{
	FVector NewWorldLocation = WeaponMesh->GetComponentLocation();
	FRotator NewWorldRotation = WeaponMesh->GetComponentRotation();
	float RunningTime = GetGameTimeSinceCreation();
	float DeltaHeight = (FMath::Sin(RunningTime + DeltaTime) - FMath::Sin(RunningTime));
	// Scale our height by a factor of 20
	NewWorldLocation.Z += DeltaHeight * 20.0f;
	// Rotate by 20 degrees per second
	float DeltaRotation = DeltaTime * 60.0f;    
	NewWorldRotation.Yaw += DeltaRotation;
	WeaponMesh->SetWorldLocation(NewWorldLocation);
	WeaponMesh->SetWorldRotation(NewWorldRotation);
	
	TimePassed_SinceGetThrewAway += DeltaTime;
	if (HaloEffect_NSComponent && !HaloEffect_NSComponent->IsActive())
	{
		if (0.2f < TimePassed_SinceGetThrewAway && 0 < LastTime_DisplayCaseTransformBeenReplicated && 0.2f < GetWorld()->TimeSeconds - LastTime_DisplayCaseTransformBeenReplicated)
		{
			HaloEffect_NSComponent->Activate();
			HaloEffect_NSComponent->SetVisibility(true);
			HaloEffect_NSComponent->SetWorldRotation(FRotator::ZeroRotator);
		}
	}		
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
		DisplayCase->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore);
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
	//// TODO: assign a new variable(decided by AttackDetectComponent or Projectile) to spawnLocation
	//if (AttackDetectComponent)
	//{
	//	FVector spawnLocation = AttackDetectComponent->GetComponentLocation();
	//	UGameplayStatics::SpawnEmitterAtLocation(this, AttackHitEffect, spawnLocation, FRotator::ZeroRotator, true, EPSCPoolMethod::AutoRelease);
	//}	
}


void ABaseWeapon::OnRep_DisplayCaseTransform()
{
	if (!IsPickedUp)
	{
		//SetActorTransform(Transform);
		DisplayCase->SetWorldLocation(DisplayCaseLocation);
		DisplayCase->SetWorldRotation(DisplayCaseRotation);
		DisplayCase->SetWorldScale3D(DisplayCaseScale);

		LastTime_DisplayCaseTransformBeenReplicated = GetWorld()->TimeSeconds;
	}	
}

void ABaseWeapon::OnRep_bAttackOn()
{
	if (bAttackOn)
	{		
		// Vfx
		if(AttackOnEffect)
			AttackOnEffect->Activate();
		// Sfx
		CallAttackStartSfx();
	}
	else
	{
		// Vfx
		if (AttackOnEffect)
			AttackOnEffect->Deactivate();
		// Sfx
		CallAttackStopSfx();
	}
}

void ABaseWeapon::OnRep_bAttackOverlap()
{
}

void ABaseWeapon::OnRep_IsPickedUp()
{
	if (IsPickedUp)
	{
		// Vfx
		if (HaloEffect_NSComponent && HaloEffect_NSComponent->IsActive())
		{
			HaloEffect_NSComponent->Deactivate();
			HaloEffect_NSComponent->SetVisibility(false);
		}

		WeaponMesh->SetRelativeLocation(FVector::ZeroVector);
		WeaponMesh->SetRelativeRotation(FRotator::ZeroRotator);

		// Show weapon silouette on teammates' end
		int TeammateCheckResult = ADamageManager::IsTeammate(GetOwner(), GetWorld()->GetFirstPlayerController());
		if (TeammateCheckResult == 1)
		{
			// Exclude self
			if (auto pMCharacter = Cast<AMCharacter>(GetOwner()))
			{
				if (pMCharacter->GetController() != GetWorld()->GetFirstPlayerController())
				{
					WeaponMesh->SetRenderCustomDepth(true);
					WeaponMesh->SetCustomDepthStencilValue(252);
				}				
			}	
		}			
	}
	else
	{		
		TimePassed_SinceGetThrewAway = 0;	
		WeaponMesh->SetRelativeLocation(WeaponMeshDefaultRelativeLocation);
		WeaponMesh->SetRelativeRotation(WeaponMeshDefaultRelativeRotation);

		WeaponMesh->SetRenderCustomDepth(false);
	}	

	if (AttackOnEffect && AttackOnEffect->IsActive())
	{
		AttackOnEffect->Deactivate();
	}
}


//void ABaseWeapon::OnRep_DamageGenerationCounter()
//{
//	GenerateAttackHitEffect();
//}


void ABaseWeapon::OnAttackOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, 
	class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IsPickedUp && HoldingController && GetOwner())
	{
		if( (Cast<AMCharacter>(OtherActor) && OtherActor != GetOwner()) ||
			Cast<AMinigameObj_Enemy>(OtherActor) )
		{
			if (!AttackObjectMap.Contains(OtherActor))
				AttackObjectMap.Add(OtherActor);
			AttackObjectMap[OtherActor] = 0.0f;
			bAttackOverlap = true;
			if (GetNetMode() == NM_ListenServer)
				OnRep_bAttackOverlap();

			if (AttackType != EnumAttackType::Constant)
			{
				if ((!IsBigWeapon && ApplyDamageCounter == 0) || IsBigWeapon)
				{
					ADamageManager::TryApplyDamageToAnActor(this, HoldingController, UMeleeDamageType::StaticClass(), OtherActor, 0);
					ADamageManager::ApplyOneTimeBuff(WeaponType, EnumAttackBuff::Knockback, HoldingController, Cast<AMCharacter>(OtherActor), 0);
					ApplyDamageCounter++;
				}				
			}				
		}
	}
}


void ABaseWeapon::OnAttackOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, 
	class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	//if (IsPickedUp && GetOwner())
	//{
	//	if ((Cast<AMCharacter>(OtherActor) && OtherActor != GetOwner()) ||
	//		Cast<AMinigameObj_Enemy>(OtherActor))
	//	{
	//		if (AttackObjectMap.Contains(OtherActor))
	//		{
	//			AttackObjectMap.Remove(OtherActor);
	//		}
	//		bAttackOverlap = false;
	//	}
	//}

	if (AttackObjectMap.Contains(OtherActor))
	{
		AttackObjectMap.Remove(OtherActor);
	}
	bAttackOverlap = false;
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

FString ABaseWeapon::GetWeaponName()
{
	if (WeaponName == "")
		WeaponName = AWeaponDataHelper::WeaponEnumToString_Map[WeaponType];
	return WeaponName;
}

AController* ABaseWeapon::GetHoldingController() const
{
	return HoldingController;
}


void ABaseWeapon::SpawnProjectile(float AttackTargetDistance)
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


#pragma region Effects
void ABaseWeapon::NetMulticast_CallPickedUpSfx_Implementation()
{
	CallPickedUpSfx();
}

void ABaseWeapon::NetMulticast_CallThrewAwaySfx_Implementation()
{
	CallThrewAwaySfx();
}

void ABaseWeapon::NetMulticast_CallShowUpVfx_Implementation()
{
	if (ShowUpEffect_NC && !ShowUpEffect_NC->IsActive())
		ShowUpEffect_NC->Activate();
}

void ABaseWeapon::CallShowUpVfx()
{
	if (ShowUpEffect_NC && !ShowUpEffect_NC->IsActive())
		ShowUpEffect_NC->Activate();
}

#pragma endregion Effects
