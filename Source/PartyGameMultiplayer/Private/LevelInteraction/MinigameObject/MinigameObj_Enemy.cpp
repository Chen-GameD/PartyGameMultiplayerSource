// Fill out your copyright notice in the Description page of Project Settings.

#include "LevelInteraction/MinigameObject/MinigameObj_Enemy.h"

#include "Components/WidgetComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

#include "M_PlayerState.h"
#include "Character/MPlayerController.h"
#include "PartyGameMultiplayer/PartyGameMultiplayerCharacter.h"
#include "Weapon/BaseProjectile.h"
#include "Weapon/BaseWeapon.h"
#include "Weapon/ElementWeapon/WeaponFork.h"
#include "Weapon/ElementWeapon/WeaponLighter.h"
#include "Weapon/ElementWeapon/WeaponBlower.h"
#include "Weapon/ElementWeapon/WeaponAlarm.h"
#include "Weapon/DamageManager.h"
#include "Weapon/WeaponDataHelper.h"
#include "UI/MinigameObjFollowWidget.h"

AMinigameObj_Enemy::AMinigameObj_Enemy()
{
	RootMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RootMesh"));	
	RootMesh->SetupAttachment(RootComponent);
	RootMesh->SetCollisionProfileName(TEXT("NoCollision"));

	CrabMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CrabMesh"));
	CrabMesh->SetupAttachment(RootMesh);
	CrabMesh->SetCollisionProfileName(TEXT("Custom"));

	CrabCenterMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CrabCenterMesh"));
	CrabCenterMesh->SetupAttachment(CrabMesh);
	CrabCenterMesh->SetCollisionProfileName(TEXT("NoCollision"));

	CollisionMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CollisionMesh"));
	CollisionMesh->SetupAttachment(CrabMesh);
	CollisionMesh->SetCollisionProfileName(TEXT("Custom"));

	BigWeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BigWeaponMesh"));
	BigWeaponMesh->SetupAttachment(CrabMesh);
	BigWeaponMesh->SetCollisionProfileName(TEXT("NoCollision"));

	LittleCrabMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LittleCrabMesh"));
	LittleCrabMesh->SetupAttachment(CrabMesh);
	LittleCrabMesh->SetCollisionProfileName(TEXT("NoCollision"));
	

	Explode_NC = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ExplodeVfx"));
	Explode_NC->SetupAttachment(CrabMesh);
	Explode_NC->bAutoActivate = false;
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> DefaultExplodeNS(TEXT("/Game/ArtAssets/Niagara/NS_CrabExplode.NS_CrabExplode"));
	if (DefaultExplodeNS.Succeeded())
		Explode_NC->SetAsset(DefaultExplodeNS.Object);

	Rising_NC = CreateDefaultSubobject<UNiagaraComponent>(TEXT("RisingVfx"));
	Rising_NC->SetupAttachment(CrabMesh);
	Rising_NC->bAutoActivate = true;

	FollowWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("FollowWidget"));
	FollowWidget->SetupAttachment(RootMesh);

	MaxHealth = 1200.0f;
	CurrentHealth = MaxHealth;

	RisingSpeed = 100.0f;
	RisingTargetHeight = 100.0f;

	DropWeaponDelay = 2.25f;
	RespawnDelay = 6.5f;

	Local_ShowNoDamageHint_Interval = 0.25f;
	Local_ShowNoDamageHint_LastTime = -1.0f;

	Server_CallGetHitEffects_MinInterval = 0.5f;
	Server_LastTime_CallGetHitEffects = -1.0f;

	GetHitAnimMinLastingTime = 0.25f;
}


void AMinigameObj_Enemy::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMinigameObj_Enemy, isAttacked);
}


void AMinigameObj_Enemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsRisingFromSand && GetActorLocation().Z < RisingTargetHeight)
	{
		FVector TargetLocation = GetActorLocation() + FVector::UpVector * RisingSpeed * DeltaTime;
		// The End
		if (RisingTargetHeight <= TargetLocation.Z)
		{
			TargetLocation.Z = RisingTargetHeight;			
			FollowWidget->SetVisibility(true);
			if (Rising_NC && Rising_NC->IsActive())
				Rising_NC->Deactivate();
			IsRisingFromSand = false;

			// Server
			if (GetLocalRole() == ROLE_Authority)
			{
				Server_SpawnBigWeaponLocation = BigWeaponMesh->GetComponentLocation() + FVector(0, -110.0f, 0);
				Server_SpawnBigWeaponRotation = BigWeaponMesh->GetComponentRotation();
			}
		}
		// Detach in Beginplay won't work somehow, so do the following instead
		if (Rising_NC && Rising_NC->IsActive())
		{
			Rising_NC->SetWorldLocation(FVector(0, -90.0, 130.0));
			Rising_NC->SetWorldRotation(FRotator::ZeroRotator);
			Rising_NC->SetWorldScale3D(FVector::OneVector);
		}
		SetActorLocation(TargetLocation);
	}

	// Server
	if (GetLocalRole() == ROLE_Authority)
	{
		// Reset crab's animation state to idle after not getting hit for GetHitAnimMinLastingTime seconds
		if (isAttacked)
		{
			if (0 < Server_LastTime_CallGetHitEffects && GetHitAnimMinLastingTime < GetWorld()->TimeSeconds - Server_LastTime_CallGetHitEffects)
			{
				//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("CrabHit isAttacked = false"));
				isAttacked = false;
			}
		}
	}
}


float AMinigameObj_Enemy::TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!EventInstigator || !DamageCauser)
		return 0.0f;
	if (DamageTaken == 0 || CurrentHealth <= 0 || IsRisingFromSand)
		return 0.0f;

	// Adjust the damage according to the minigame damage ratio
	EnumWeaponType WeaponType = EnumWeaponType::None;
	bool IsBigWeapon = false;
	if (auto p = Cast<ABaseWeapon>(DamageCauser))
	{
		WeaponType = p->WeaponType;
		IsBigWeapon = p->IsBigWeapon;
	}
	if (auto p = Cast<ABaseProjectile>(DamageCauser))
	{
		WeaponType = p->WeaponType;
		IsBigWeapon = p->IsBigWeapon;
	}
	if (WeaponType == EnumWeaponType::None)
		return 0.0f;
	
	if (!ADamageManager::CanApplyDamageToEnemyCrab(SpecificWeaponClass, WeaponType))
		return 0.0f;

	FString WeaponName = AWeaponDataHelper::WeaponEnumToString_Map[WeaponType];
	if (IsBigWeapon)
		WeaponName = "Big" + WeaponName;
	if (AWeaponDataHelper::DamageManagerDataAsset->MiniGame_Damage_Map.Contains(WeaponName))
		DamageTaken *= AWeaponDataHelper::DamageManagerDataAsset->MiniGame_Damage_Map[WeaponName];
	else
		return 0.0f;

	// Call GetHit vfx & sfx (cannot call in OnHealthUpdate since health can be increased or decreased)
	if (Server_LastTime_CallGetHitEffects < 0 || Server_CallGetHitEffects_MinInterval < GetWorld()->TimeSeconds - Server_LastTime_CallGetHitEffects)
	{
		NetMulticast_CallGetCorrectHitSfx();
		
		// Call crab get hit animation
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("CrabHit isAttacked = true"));
		isAttacked = true;

		Server_LastTime_CallGetHitEffects = GetWorld()->TimeSeconds;
	}

	CurrentHealth = FMath::Max(CurrentHealth - DamageTaken, 0);
	if (GetNetMode() == NM_ListenServer)
		OnRep_CurrentHealth();
	if (CurrentHealth <= 0)
	{
		if (AM_PlayerState* killerPS = EventInstigator->GetPlayerState<AM_PlayerState>())
		{
			killerPS->addScore(ScoreCanGet);
			
			// Broadcast information to all clients
			for (FConstPlayerControllerIterator iter = GetWorld()->GetPlayerControllerIterator(); iter; ++iter)
			{
				AMPlayerController* currentController = Cast<AMPlayerController>(*iter);
				currentController->UI_InGame_BroadcastMiniInformation(killerPS->TeamIndex, killerPS->PlayerNameString, MinigameInformation);
			}
		}
		
		Server_WhenDead();
	}

	return 0.0f;
}


void AMinigameObj_Enemy::Server_WhenDead()
{
	// Drop the Big Weapon
	FTimerHandle DropWeaponTimerHandle;
	GetWorldTimerManager().SetTimer(DropWeaponTimerHandle, [this] 
		{
			if (this)
			{
				FVector spawnLocation = Server_SpawnBigWeaponLocation;
				if (this)
				{
					FRotator spawnRotation = Server_SpawnBigWeaponRotation;
					FActorSpawnParameters spawnParameters;
					spawnParameters.Instigator = nullptr;
					spawnParameters.Owner = nullptr;
					if (this)
					{
						auto pBigWeapon = GetWorld()->SpawnActor<ABaseWeapon>(SpecificWeaponClass, spawnLocation, spawnRotation, spawnParameters);
					}
				}
			}
		
		}, DropWeaponDelay, false);

	//// Spawn the little crab that walks into the ocean.
	//FTimerHandle SpawnCrabTimerHandle;
	//GetWorldTimerManager().SetTimer(SpawnCrabTimerHandle, [this]
	//	{
	//		FVector spawnLocation = GetActorLocation();
	//		FRotator spawnRotation = GetActorRotation();
	//		auto pLittleCrab = GetWorld()->SpawnActor<AActor>(SpecificLittleCrabClass, spawnLocation, spawnRotation);
	//	}, DropWeaponDelay * 0.5f, false);

	// Respawn(Destroy)
	FTimerHandle RespawnMinigameObjectTimerHandle;
	GetWorldTimerManager().SetTimer(RespawnMinigameObjectTimerHandle, this, &AMinigameObj_Enemy::StartToRespawnActor, RespawnDelay, false);
}

void AMinigameObj_Enemy::BeginPlay()
{
	Super::BeginPlay();

	if (UMinigameObjFollowWidget* pFollowWidget = Cast<UMinigameObjFollowWidget>(FollowWidget->GetUserWidgetObject()))
	{
		pFollowWidget->SetHealthByPercentage(1);
		FollowWidget->SetVisibility(false);
	}	

	FName SocketName;
	FString SocketName_str = "";
	
	if (SpecificWeaponClass->IsChildOf(AWeaponFork::StaticClass()))
		SocketName_str = "Fork";
	else if (SpecificWeaponClass->IsChildOf(AWeaponLighter::StaticClass()))
		SocketName_str = "Lighter";
	else if (SpecificWeaponClass->IsChildOf(AWeaponBlower::StaticClass()))
		SocketName_str = "Blower";
	else if (SpecificWeaponClass->IsChildOf(AWeaponAlarm::StaticClass()))
		SocketName_str = "Alarm";
	SocketName = FName(*SocketName_str);
	BigWeaponMesh->AttachToComponent(CrabMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, SocketName);
	SocketName_str = "LittleCrab";
	SocketName = FName(*SocketName_str);
	LittleCrabMesh->AttachToComponent(CrabMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, SocketName);

	IsRisingFromSand = true; 
	if (Rising_NC && !Rising_NC->IsActive())
	{
		Rising_NC->Activate();		
	}
	// Shake localcontrolled character's camera
	if (auto pController = GetWorld()->GetFirstPlayerController())
	{
		//if (auto pMCharacter = Cast<AMCharacter>(pController->GetPawn()))
		//{
		//	pMCharacter->CameraShakingTime = 0.7f;
		//	pMCharacter->CameraShakingIntensity = 10.0f;
		//}
		if (auto pCamMg = pController->PlayerCameraManager)
			pCamMg->StartCameraShake(CameraShakeTriggered);
	}	

	// Sfx
	CallStartSfx();
}

void AMinigameObj_Enemy::OnRep_CurrentHealth()
{
	Super::OnRep_CurrentHealth();

	if (CurrentHealth <= 0)
	{
		// Hide Crab
		FTimerHandle HideCrabTimerHandle;
		GetWorldTimerManager().SetTimer(HideCrabTimerHandle, [this]
			{
				if (this)
				{
					SetActorEnableCollision(false);
					if (this)
					{
						SetActorLocation(GetActorLocation() + FVector(0, 0, -1000.0f));
						if (FollowWidget)
							FollowWidget->SetVisibility(false);
						// little crab
						if (this)
						{
							BPF_BroadcastCrabAnimation();
							if (this)
							{
								CallLittleCrabFleeSfx();
							}
						}
					}
				}
			}, DropWeaponDelay, false);

		// Vfx
		if (Explode_NC)
			Explode_NC->Activate();
		// Sfx
		CallDeathSfx();
	}

	// Set UI: Health Bar
	if(UMinigameObjFollowWidget* pFollowWidget = Cast<UMinigameObjFollowWidget>(FollowWidget->GetUserWidgetObject()))
		pFollowWidget->SetHealthByPercentage(CurrentHealth / MaxHealth);
}


void AMinigameObj_Enemy::NetMulticast_ShowNoDamageHint_Implementation(AController* pController, FVector HitLocation)
{
	if (0 < CurrentHealth && GetWorld()->GetFirstPlayerController() == pController)
	{
		if (Local_ShowNoDamageHint_LastTime < 0 || Local_ShowNoDamageHint_Interval < GetWorld()->TimeSeconds - Local_ShowNoDamageHint_LastTime)
		{
			FVector spawnLocation = HitLocation;
			FRotator spawnRotation = FRotator::ZeroRotator;
			AActor* NoDamageHintActor = GetWorld()->SpawnActor<AActor>(SpecificNoDamageHintActorClass, spawnLocation, spawnRotation);
			FTimerHandle TimerHandle;
			float NoDamageHintActor_LifeTime = Local_ShowNoDamageHint_Interval;
			float NoDamageHintAnimTime = 1.0f;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, [NoDamageHintActor]()
				{
					if (NoDamageHintActor)
						NoDamageHintActor->Destroy();
				}, NoDamageHintAnimTime, false);
			Local_ShowNoDamageHint_LastTime = GetWorld()->TimeSeconds;
			CallGetIncorrectHitSfx();
		}		
	}	
}

void AMinigameObj_Enemy::NetMulticast_CallGetCorrectHitSfx_Implementation()
{
	CallGetCorrectHitSfx();
}