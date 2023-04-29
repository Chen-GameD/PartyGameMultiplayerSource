// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "MinigameObjFollowWidget.generated.h"

/**
 * 
 */
UCLASS()
class PARTYGAMEMULTIPLAYER_API UMinigameObjFollowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetHealthByPercentage(float i_percentage);
	void SetName(FString i_Name);
	UFUNCTION(BlueprintImplementableEvent)
	void BPF_SetHealthProgressBar(float Percent);

protected:
	// UPROPERTY(meta = (BindWidget))
	// 	UProgressBar* HealthBar;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* Name;
	UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
	UImage* HealthBar;
};
