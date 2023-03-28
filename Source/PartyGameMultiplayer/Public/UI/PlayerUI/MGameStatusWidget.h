// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "MGameStatusWidget.generated.h"

/**
 * Create By CMY
 * This subclass contains game status information: Team Score, Game Timer, Minigame Information;
 */
UCLASS()
class PARTYGAMEMULTIPLAYER_API UMGameStatusWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UMGameStatusWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	UFUNCTION()
	void UpdateTeam_1_Score(int i_Score);
	UFUNCTION()
	void UpdateTeam_2_Score(int i_Score);

	UFUNCTION()
	void UpdateGameTimer(int i_GameTime);

	UFUNCTION()
	void UpdateMinigameInfo(FString i_Info);
	UFUNCTION(BlueprintImplementableEvent)
	void ShowMinigameInfoAnimation();
	UFUNCTION(BlueprintImplementableEvent)
	void ShowTimerAnimation();

protected:
	// Team Score
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Team_1_Score;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Team_2_Score;

	// Game Timer
	UPROPERTY(meta = (BindWidget))
	UTextBlock* GameTimer;

	// Minigame Information
	UPROPERTY(meta = (BindWidget))
	UTextBlock* MinigameInfo;
	
};
