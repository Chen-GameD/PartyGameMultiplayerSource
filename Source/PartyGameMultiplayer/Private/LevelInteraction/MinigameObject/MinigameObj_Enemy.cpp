// Fill out your copyright notice in the Description page of Project Settings.

#include "LevelInteraction/MinigameObject/MinigameObj_Enemy.h"

#include "Components/WidgetComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

#include "M_PlayerState.h"
#include "PartyGameMultiplayer/PartyGameMultiplayerCharacter.h"
#include "Weapon/BaseProjectile.h"
#include "Weapon/BaseWeapon.h"
#include "Weapon/ElementWeapon/WeaponFork.h"
#include "Weapon/ElementWeapon/WeaponLighter.h"
#include "Weapon/ElementWeapon/WeaponBlower.h"
#include "Weapon/ElementWeapon/WeaponAlarm.h"
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

	CollisionMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CollisionMesh"));
	CollisionMesh->SetupAttachment(CrabMesh);
	CollisionMesh->SetCollisionProfileName(TEXT("Custom"));

	BigWeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BigWeaponMesh"));
	BigWeaponMesh->SetupAttachment(CrabMesh);
	BigWeaponMesh->SetCollisionProfileName(TEXT("NoCollision"));

	Explode_NC = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ExplodeVfx"));
	Explode_NC->SetupAttachment(CrabMesh);
	Explode_NC->bAutoActivate = false;
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> DefaultExplodeNS(TEXT("/Game/ArtAssets/Niagara/NS_CrabExplode.NS_CrabExplode"));
	if (DefaultExplodeNS.Succeeded())
		Explode_NC->SetAsset(DefaultExplodeNS.Object);

	Shield_NC = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ShieldVfx"));
	Shield_NC->SetupAttachment(CrabMesh);
	Shield_NC->bAutoActivate = false;
	/*static ConstructorHelpers::FObjectFinder<UNiagaraSystem> DefaultExplodeNS(TEXT("/Game/ArtAssets/Niagara/NS_CrabExplode.NS_CrabExplode"));
	if (DefaultExplodeNS.Succeeded())
		Explode_NC->SetAsset(DefaultExplodeNS.Object);*/

	FollowWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("FollowWidget"));
	FollowWidget->SetupAttachment(RootMesh);

	MaxHealth = 1200.0f;
	CurrentHealth = MaxHealth;

	RisingSpeed = 100.0f;
	RisingTargetHeight = 100.0f;

	DropWeaponDelay = 2.25f;
	RespawnDelay = 5.0f;
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
			SetActorEnableCollision(true);
			IsRisingFromSand = false;

			// Server
			if (GetLocalRole() == ROLE_Authority)
			{
				Server_SpawnBigWeaponLocation = BigWeaponMesh->GetComponentLocation() + FVector(0, -110.0f, 0);
				Server_SpawnBigWeaponRotation = BigWeaponMesh->GetComponentRotation();
			}
		}
		SetActorLocation(TargetLocation);
	}
}


float AMinigameObj_Enemy::TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!EventInstigator || !DamageCauser)
		return 0.0f;
	if (DamageTaken == 0 || CurrentHealth <= 0)
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

	bool beingAttackedByInvalidWeapon = false;
	if (SpecificWeaponClass->IsChildOf(AWeaponFork::StaticClass()))
	{
		if (WeaponType != EnumWeaponType::Fork && WeaponType != EnumWeaponType::Taser && WeaponType != EnumWeaponType::Flamefork && WeaponType != EnumWeaponType::Bomb)
			beingAttackedByInvalidWeapon = true;
	}
	else if (SpecificWeaponClass->IsChildOf(AWeaponLighter::StaticClass()))
	{
		if (WeaponType != EnumWeaponType::Lighter && WeaponType != EnumWeaponType::Flamefork && WeaponType != EnumWeaponType::Flamethrower && WeaponType != EnumWeaponType::Cannon)
			beingAttackedByInvalidWeapon = true;
	}
	else if (SpecificWeaponClass->IsChildOf(AWeaponBlower::StaticClass()))
	{
		if (WeaponType != EnumWeaponType::Blower && WeaponType != EnumWeaponType::Taser && WeaponType != EnumWeaponType::Flamethrower && WeaponType != EnumWeaponType::Alarmgun)
			beingAttackedByInvalidWeapon = true;
	}
	else if (SpecificWeaponClass->IsChildOf(AWeaponAlarm::StaticClass()))
	{
		if (WeaponType != EnumWeaponType::Alarm && WeaponType != EnumWeaponType::Bomb && WeaponType != EnumWeaponType::Cannon && WeaponType != EnumWeaponType::Alarmgun)
			beingAttackedByInvalidWeapon = true;
	}
	if (beingAttackedByInvalidWeapon)
	{
		AController* pController = nullptr;
		if (auto p = Cast<ABaseWeapon>(DamageCauser))
			pController = p->GetHoldingController();
		if (auto p = Cast<ABaseProjectile>(DamageCauser))
			pController = p->Controller;
		NetMulticast_CallShieldVfx(pController);
		return 0.0f;
	}

	FString WeaponName = AWeaponDataHelper::WeaponEnumToString_Map[WeaponType];
	if (IsBigWeapon)
		WeaponName = "Big" + WeaponName;
	if (AWeaponDataHelper::DamageManagerDataAsset->MiniGame_Damage_Map.Contains(WeaponName))
		DamageTaken *= AWeaponDataHelper::DamageManagerDataAsset->MiniGame_Damage_Map[WeaponName];
	else
		return 0.0f;

	CurrentHealth -= DamageTaken;
	CurrentHealth = FMath::Max(CurrentHealth, 0);
	if (GetNetMode() == NM_ListenServer)
		OnRep_CurrentHealth();
	if (CurrentHealth <= 0)
	{
		if (AM_PlayerState* killerPS = EventInstigator->GetPlayerState<AM_PlayerState>())
		{
			killerPS->addScore(ScoreCanGet);
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
			FVector spawnLocation = Server_SpawnBigWeaponLocation;
			FRotator spawnRotation = Server_SpawnBigWeaponRotation;
			FActorSpawnParameters spawnParameters;
			spawnParameters.Instigator = nullptr;
			spawnParameters.Owner = nullptr;
			auto pBigWeapon = GetWorld()->SpawnActor<ABaseWeapon>(SpecificWeaponClass, spawnLocation, spawnRotation, spawnParameters);
		}, DropWeaponDelay, false);

	// TODO: Server or NetMultiCast/OnRep_CurrentHealth: genrate a new little crab and walk into the ocean.

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
	SetActorEnableCollision(false);

	IsRisingFromSand = true; 
}

void AMinigameObj_Enemy::OnRep_CurrentHealth()
{
	Super::OnRep_CurrentHealth();

	if (CurrentHealth <= 0)
	{
		// HideCrab
		FTimerHandle HideCrabTimerHandle;
		GetWorldTimerManager().SetTimer(HideCrabTimerHandle, [this]
			{
				SetActorEnableCollision(false);
				SetActorLocation(GetActorLocation() + FVector(0, 0, -1000.0f));
				FollowWidget->SetVisibility(false);
			}, DropWeaponDelay, false);

		// Vfx
		if (Explode_NC)
			Explode_NC->Activate();	
	}

	// Set UI: Health Bar
	if(UMinigameObjFollowWidget* pFollowWidget = Cast<UMinigameObjFollowWidget>(FollowWidget->GetUserWidgetObject()))
		pFollowWidget->SetHealthByPercentage(CurrentHealth / MaxHealth);
}


void AMinigameObj_Enemy::NetMulticast_CallShieldVfx_Implementation(AController* pController)
{
	if (GetWorld()->GetFirstPlayerController() == pController)
	{
		if (Shield_NC)
			Shield_NC->Activate();
	}	
}