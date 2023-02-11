// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MCharacterBuffWidget.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "MPlayerStatusWidget.generated.h"

/**
 * Create By CMY
 * This subclass contains the player's name, HP, Buff and skill CD information;
 */
UCLASS()
class PARTYGAMEMULTIPLAYER_API UMPlayerStatusWidget : public UUserWidget
{
	GENERATED_BODY()
public:

protected:
	// Player Information
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerName;

	// Buff information
	UPROPERTY(meta = (BindWidget))
	UNamedSlot* BuffSlot;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class UMCharacterBuffWidget> BuffInfoClass;
	UPROPERTY()
	UMCharacterBuffWidget* BuffInfo;

	// Skill information
	UPROPERTY(meta = (BindWidget))
	UNamedSlot* SkillSlot;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class UMCharacterSkillWidget> SkillInfoClass;
	UPROPERTY()
	UMCharacterSkillWidget* SkillInfo;
};
