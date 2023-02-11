// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
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

protected:
	UPROPERTY(meta = (BindWidget))
	UImage* Dash_INFO;
	
};
