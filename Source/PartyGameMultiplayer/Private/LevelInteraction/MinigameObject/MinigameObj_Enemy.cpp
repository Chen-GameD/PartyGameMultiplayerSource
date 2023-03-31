// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelInteraction/MinigameObject/MinigameObj_Enemy.h"

#include "M_PlayerState.h"
#include "NiagaraComponent.h"
#include "PartyGameMultiplayer/PartyGameMultiplayerCharacter.h"
#include "Weapon/BaseProjectile.h"
#include "Weapon/BaseWeapon.h"
#include "Weapon/WeaponDataHelper.h"

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

	MaxHealth = 1200.0f;
	CurrentHealth = MaxHealth;
}

float AMinigameObj_Enemy::TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!EventInstigator || !DamageCauser)
		return 0.0f;
	if (CurrentHealth <= 0)
		return 0.0f;

	//// Adjust the damage according to the minigame damage ratio
	//EnumWeaponType WeaponType = EnumWeaponType::None;
	//if (auto p = Cast<ABaseWeapon>(DamageCauser))
	//	WeaponType = p->WeaponType;
	//if (auto p = Cast<ABaseProjectile>(DamageCauser))
	//	WeaponType = p->WeaponType;
	//if (WeaponType == EnumWeaponType::None)
	//	return 0.0f;
	//FString WeaponName = AWeaponDataHelper::WeaponEnumToString_Map[WeaponType];
	//if (AWeaponDataHelper::DamageManagerDataAsset->MiniGame_Damage_Map.Contains(WeaponName))
	//	DamageTaken *= AWeaponDataHelper::DamageManagerDataAsset->MiniGame_Damage_Map[WeaponName];
	//else
	//	return 0.0f;

	CurrentHealth -= DamageTaken;
	if (CurrentHealth <= 0)
	{
		CurrentHealth = 0.0f;
		if (GetNetMode() == NM_ListenServer)
		{
			OnRep_CurrentHealth();
		}
		/*if (SkeletalMesh)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Destroying MiniGameObjective's SkeletalMesh on Server"));
			SkeletalMesh->DestroyComponent();
		}*/
		if (AM_PlayerState* killerPS = EventInstigator->GetPlayerState<AM_PlayerState>())
		{
			killerPS->addScore(ScoreCanGet);
		}

		// Set timer and respawn this actor
		//FTimerHandle RespawnMinigameObjectTimerHandle;
		//GetWorldTimerManager().SetTimer(RespawnMinigameObjectTimerHandle, this, &AMinigameObj_Enemy::StartToRespawnActor, 5, false);

		Destroy();

		FVector spawnLocation = BigWeaponMesh->GetComponentLocation();
		FRotator spawnRotation = BigWeaponMesh->GetComponentRotation();
		FActorSpawnParameters spawnParameters;
		spawnParameters.Instigator = nullptr;
		spawnParameters.Owner = nullptr;
		auto pBigWeapon = GetWorld()->SpawnActor<ABaseWeapon>(SpecificWeaponClass, spawnLocation, spawnRotation, spawnParameters);
	}

	return 0.0f;
}

void AMinigameObj_Enemy::BeginPlay()
{
	Super::BeginPlay();
}

void AMinigameObj_Enemy::OnRep_CurrentHealth()
{
	Super::OnRep_CurrentHealth();

	if (CurrentHealth <= 0)
	{
	}
}

