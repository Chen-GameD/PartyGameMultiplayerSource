// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelInteraction/MinigameObject/MinigameObj_Balloon.h"

#include "M_PlayerState.h"
#include "NiagaraComponent.h"
#include "PartyGameMultiplayer/PartyGameMultiplayerCharacter.h"
#include "Weapon/BaseProjectile.h"
#include "Weapon/BaseWeapon.h"
#include "Weapon/WeaponDataHelper.h"

AMinigameObj_Balloon::AMinigameObj_Balloon()
{
	RootMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RootMesh"));
	RootMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	RootMesh->SetupAttachment(RootComponent);

	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(RootMesh);

	BlowUpEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("BlowUpEffect"));
	BlowUpEffect->SetupAttachment(RootMesh);
	BlowUpEffect->bAutoActivate = false;

	MaxHealth = 1200.0f;
	CurrentHealth = MaxHealth;
}

float AMinigameObj_Balloon::TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!EventInstigator || !DamageCauser)
		return 0.0f;
	if (CurrentHealth <= 0)
		return 0.0f;

	// Adjust the damage according to the minigame damage ratio
	EnumWeaponType WeaponType = EnumWeaponType::None;
	if (auto p = Cast<ABaseWeapon>(DamageCauser))
		WeaponType = p->WeaponType;
	if (auto p = Cast<ABaseProjectile>(DamageCauser))
		WeaponType = p->WeaponType;
	if (WeaponType == EnumWeaponType::None)
		return 0.0f;
	FString WeaponName = AWeaponDataHelper::WeaponEnumToString_Map[WeaponType];
	if (AWeaponDataHelper::DamageManagerDataAsset->MiniGame_Damage_Map.Contains(WeaponName))
		DamageTaken *= AWeaponDataHelper::DamageManagerDataAsset->MiniGame_Damage_Map[WeaponName];
	else
		return 0.0f;

	CurrentHealth -= DamageTaken;
	if (CurrentHealth <= 0)
	{
		CurrentHealth = 0.0f;	
		if (GetNetMode() == NM_ListenServer)
		{
			OnRep_CurrentHealth();
		}
		if (SkeletalMesh)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Destroying MiniGameObjective's SkeletalMesh on Server"));
			SkeletalMesh->DestroyComponent();
		}
		if (AM_PlayerState* killerPS = EventInstigator->GetPlayerState<AM_PlayerState>())
		{
			killerPS->addScore(ScoreCanGet);
		}

		// Set timer and respawn this actor
		FTimerHandle RespawnMinigameObjectTimerHandle;
		GetWorldTimerManager().SetTimer(RespawnMinigameObjectTimerHandle, this, &AMinigameObj_Balloon::StartToRespawnActor, 5, false);
	}		
	
	return 0.0f;
}

void AMinigameObj_Balloon::BeginPlay()
{
	Super::BeginPlay();
	BlowUpEffect->Deactivate();
}

void AMinigameObj_Balloon::OnRep_CurrentHealth()
{
	Super::OnRep_CurrentHealth();

	if (CurrentHealth <= 0)
	{
		if (SkeletalMesh)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Destroying MiniGameObjective's SkeletalMesh on Client"));
			SkeletalMesh->DestroyComponent();
		}
		if (BlowUpEffect)
		{
			BlowUpEffect->Activate();
		}
		EnableBlowUpGeometryCacheComponent();
	}
}
