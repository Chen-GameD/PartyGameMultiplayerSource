// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LevelInteraction/MinigameMainObjective.h"
#include "MinigameBaseDataAsset.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FMinigameRuleStruct
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Minigame Config")
	float GameTime;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Minigame Config")
	TSubclassOf<AMinigameMainObjective> MinigameObject;
};

UCLASS(BlueprintType)
class PARTYGAMEMULTIPLAYER_API UMinigameBaseDataAsset : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Minigame Config")
	TArray<FMinigameRuleStruct> MinigameConfigTable;
};
