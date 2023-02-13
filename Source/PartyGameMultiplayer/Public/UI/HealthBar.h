// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "HealthBar.generated.h"

/**
 * 
 */
UCLASS()
class PARTYGAMEMULTIPLAYER_API UHealthBar : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetLocalControlledUI();
	void SetHealthToProgressBar(float percentage);
	void ShowTip();
	void HideTip();

protected:
	UPROPERTY(meta = (BindWidget))
	UProgressBar* healthBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* healthText;

	UPROPERTY(meta = (BindWidget))
	UImage* Tip_Left;
	UPROPERTY(meta = (BindWidget))
	UImage* Tip_Right;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Tip_Left_Text;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Tip_Right_Text;
};
