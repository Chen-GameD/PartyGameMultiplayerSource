// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelInteraction/MinigameMainObjective.h"

#include "UObject/ConstructorHelpers.h"
#include "Net/UnrealNetwork.h"
#include "../PartyGameMultiplayerCharacter.h"
//#include "Engine/TriggerBoxDamageTaker.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
//#include "GeometryCacheComponent.h"

#include "UI/MCharacterFollowWidget.h"
#include "Components/WidgetComponent.h"
#include "Character/MCharacter.h"
#include "GameBase/MGameMode.h"
#include "Weapon/BaseWeapon.h"
#include "Weapon/BaseProjectile.h"
#include "Weapon/DamageTypeToCharacter.h"
#include "Weapon/WeaponDataHelper.h"


AMinigameMainObjective::AMinigameMainObjective()
{
	MaxHealth = 0.0f;
	CurrentHealth = MaxHealth;
}


void AMinigameMainObjective::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMinigameMainObjective, CurrentHealth);
}

void AMinigameMainObjective::BeginPlay()
{
	Super::BeginPlay();
}

// client-only
void AMinigameMainObjective::OnRep_CurrentHealth()
{
	
}

void AMinigameMainObjective::StartToRespawnActor()
{
	AMGameMode* MyGameMode = Cast<AMGameMode>(GetWorld()->GetAuthGameMode());
	if (MyGameMode)
	{
		MyGameMode->Server_RespawnMinigameObject();
	}
	AMinigameMainObjective::Destroy(true, true);
}

void AMinigameMainObjective::UpdateScoreCanGet(int n_Score)
{
	ScoreCanGet = n_Score;
}
