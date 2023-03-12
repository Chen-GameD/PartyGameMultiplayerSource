// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "MCharacterSkillWidget.generated.h"

/**
 * Create By CMY
 * This subclass contains player's skill cd information.
 */
UCLASS()
class PARTYGAMEMULTIPLAYER_API UMCharacterSkillWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void OnUseDashSkill(float CoolDownTotalTime);
	UFUNCTION()
	void UpdateDashInfo(float percentage);
	UFUNCTION()
	void UpdateDashOpacity(float percentage);

private:
	UFUNCTION()
	void DashTimerHandleFunction();

protected:
	// *** For every skill, you need to create all the variables below ***
	//////////////////////////////////////////////////////////////////////
	// Dash Skill
	UPROPERTY(meta = (BindWidget))
	UProgressBar* Dash_INFO;
	// Dash timer handler
	UPROPERTY()
	FTimerHandle Dash_TimerHandle;
	// For dash calculate what percentage should show on the UI
	UPROPERTY()
	float Dash_Timer;
	// Save total cool down time
	UPROPERTY()
	float Dash_CoolDown;
	//////////////////////////////////////////////////////////////////////
};
