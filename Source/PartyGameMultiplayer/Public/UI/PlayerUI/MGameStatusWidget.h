// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/Image.h"
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
	void UpdateMinigameInfo(FString i_Info, UTexture2D* i_InfoImage);
	UFUNCTION(BlueprintImplementableEvent)
	void ShowMinigameInfoAnimation();
	UFUNCTION(BlueprintImplementableEvent)
	void ShowTimerAnimation();

	// Broadcasting System
	UFUNCTION()
	void UpdateAndShowBroadcastingInformation(int KillerTeamIndex, int DeceasedTeamIndex, FString i_KillerName, FString i_DeceasedName, UTexture2D* i_WeaponImage);
	UFUNCTION(BlueprintImplementableEvent)
	void BroadcastingAnimationEvent();
	

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
	UPROPERTY(meta = (BindWidget))
	UImage* MinigameInfoImage;

	// Broadcasting System Information
	UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
	UCanvasPanel* BroadcastCanvas;
	UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
	UTextBlock* KillerName;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* DeceasedName;
	UPROPERTY(meta = (BindWidget))
	UImage* WeaponImage;
	UPROPERTY(BlueprintReadWrite)
	FLinearColor Team1Color;
	UPROPERTY(BlueprintReadWrite)
	FLinearColor Team2Color;
	UPROPERTY(BlueprintReadWrite)
	float BroadcastTotalTime = 3;
	UPROPERTY(BlueprintReadWrite)
	float BroadcastTimer = 0;
	UPROPERTY(BlueprintReadWrite)
	bool IsNeedShowBroadcast = false;
	
};
