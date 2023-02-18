// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DamageManagerDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class PARTYGAMEMULTIPLAYER_API UDamageManagerDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UDamageManagerDataAsset()
	{
	}

	UPROPERTY(EditAnywhere, Category = "Damage Manager")
	TMap<FString, float> Character_Damage_Map;
	
	UPROPERTY(EditAnywhere, Category = "Damage Manager")
		TMap<FString, float> AccumulatedTimeToGenerateDamage_Map;

	UPROPERTY(EditAnywhere, Category = "Damage Manager")
		TMap<FString, float> CoolDown_Map;

	UPROPERTY(EditAnywhere, Category = "Damage Manager")
	TMap<FString, float> Character_Buff_Map;

	UPROPERTY(EditAnywhere, Category = "Damage Manager")
	TMap<FString, float> MiniGame_Damage_Map;

	
};
