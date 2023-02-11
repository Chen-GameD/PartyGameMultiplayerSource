// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MCharacterBuffWidget.h"
#include "Blueprint/UserWidget.h"
#include "Character/MCharacter.h"
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
	UFUNCTION()
	void ShowWidget(bool IsShowing);

	// Player Information
	UFUNCTION()
	void UpdateHealthBar(float percentage);
	UFUNCTION()
	void SetPlayerName(FString i_Name);

	// Buff Information
	// Currently, two functions are used to control the display and disappearance of the two buffs, which can be changed according to subsequent needs.
	UFUNCTION()
	void ShowFireBuff(bool IsShowing);
	UFUNCTION()
	void ShowShockBuff(bool IsShowing);

	// Skill Information
	UFUNCTION()
	void SkillUIUpdate(SkillType UseSkill, float percentage);

protected:
	// Player Information
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerName;

	// Buff Information
	UPROPERTY(meta = (BindWidget))
	UNamedSlot* BuffSlot;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class UMCharacterBuffWidget> BuffInfoClass;
	UPROPERTY()
	UMCharacterBuffWidget* BuffInfo;

	// Skill Information
	UPROPERTY(meta = (BindWidget))
	UNamedSlot* SkillSlot;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class UMCharacterSkillWidget> SkillInfoClass;
	UPROPERTY()
	UMCharacterSkillWidget* SkillInfo;
};
