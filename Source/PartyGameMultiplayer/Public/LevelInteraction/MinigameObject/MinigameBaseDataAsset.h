// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LevelInteraction/MinigameMainObjective.h"
#include "MinigameBaseDataAsset.generated.h"

/**
 * 
 */
UENUM()
enum EMinigameTypeEnum { DefaultType, SculptureType };

UENUM()
enum EMinigamePositionTypeEnum { Single, Multiple };

// You can add any type of information you may need;
////////////////////////////////////////////////////
USTRUCT(BlueprintType)
struct FMinigameAdditionalInformation
{
	GENERATED_BODY()
public:
	// Actor
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Minigame Config")
	TSubclassOf<AActor> Additional_ActorClass;
	// Int
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Minigame Config")
	int Additional_Int;
	// Float
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Minigame Config")
	float Additional_Float;
	// Bool
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Minigame Config")
	bool Additional_Bool;
	// Position Type
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Minigame Config")
	TEnumAsByte<EMinigamePositionTypeEnum> Additional_PositionType;
	// Transform (single)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Minigame Config")
	FTransform Additional_Transform;
	// Transform (multiple)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Minigame Config")
	TArray<FTransform> Additional_Transforms;
};
////////////////////////////////////////////////////

USTRUCT(BlueprintType)
struct FMinigameRuleStruct
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Minigame Config")
	int ScoreCanGet;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Minigame Config")
	FString MinigameHint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Minigame Config")
	UTexture2D* MinigameHintImage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Minigame Config")
	TSubclassOf<AMinigameMainObjective> MinigameObject;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Minigame Config")
	FTransform MinigameObjectSpawnTransform;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Minigame Config")
	TEnumAsByte<EMinigameTypeEnum> MinigameType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Minigame Config")
	TArray<FMinigameAdditionalInformation> AdditionalInformation;
};

USTRUCT(BlueprintType)
struct FLevelMinigameRuleTable
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Minigame Config")
	int GameTime;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Minigame Config")
	TArray<FMinigameRuleStruct> MinigameConfigTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Minigame Config")
	int KillScore;
};

UCLASS(BlueprintType)
class PARTYGAMEMULTIPLAYER_API UMinigameBaseDataAsset : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Minigame Config")
	TArray<FLevelMinigameRuleTable> LevelMinigameConfigTable;
};
