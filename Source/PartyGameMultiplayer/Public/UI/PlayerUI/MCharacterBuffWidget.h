// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "MCharacterBuffWidget.generated.h"

/**
 * Create By CMY
 * This subclass contains all kinds of Buff that the Player could have.
 */
UCLASS()
class PARTYGAMEMULTIPLAYER_API UMCharacterBuffWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION()
	void ToggleFireBuff(bool IsShowing);
	UFUNCTION()
	void ToggleShockBuff(bool IsShowing);

protected:
	UPROPERTY(meta = (BindWidget))
	UImage* FireBuffImage;

	UPROPERTY(meta = (BindWidget))
	UImage* ShockBuffImage;
};
