// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "EnemyCrabFollowWidget.generated.h"


UCLASS()
class PARTYGAMEMULTIPLAYER_API UEnemyCrabFollowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetHealthByPercentage(float i_percentage);
	void SetName(FString i_Name);
protected:
private:


public:
protected:
	UPROPERTY(meta = (BindWidget))
		UProgressBar* HealthBar;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* Name;
private:

};
