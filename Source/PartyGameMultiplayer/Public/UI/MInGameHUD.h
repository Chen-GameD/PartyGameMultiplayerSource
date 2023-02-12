// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PlayerUI/MPlayerStatusWidget.h"
#include "MInGameHUD.generated.h"

/**
 * Create by CMY
 * This subclass contains all in-game UI elements;
 */
UCLASS()
class PARTYGAMEMULTIPLAYER_API AMInGameHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	AMInGameHUD();

	virtual void DrawHUD() override;

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category="Widgets")
	TSubclassOf<UUserWidget> PlayerStatusWidgetClass;

private:
	UPROPERTY()
	UMPlayerStatusWidget* PlayerStatusWidget;
};
