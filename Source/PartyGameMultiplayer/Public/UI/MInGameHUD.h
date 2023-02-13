// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PlayerUI/MGameStatusWidget.h"
#include "PlayerUI/MPlayerStatusWidget.h"
#include "PlayerUI/MPlayerWeaponInfoWidget.h"
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
	
////////////////////////////////////////////////////
/////Gameplay
////////////////////////////////////////////////////
	UFUNCTION()
	void StartGameUI();

////////////////////////////////////////////////////
/////Interface
////////////////////////////////////////////////////	
	// Show InGame_PlayerStatusWidget
	UFUNCTION()
	void InGame_ShowPlayerStatusWidget();
	// Update Player Health UI
	UFUNCTION()
	void InGame_UpdatePlayerHealth(float percentage);

	// Show InGame_PlayerWeaponInfoWidget
	UFUNCTION()
	void InGame_ShowPlayerWeaponInfoWidget();

	//Show InGame_GameStatusWidget
	UFUNCTION()
	void InGame_ShowGameStatusWidget();

protected:
	// In Game UI Class Ref
	UPROPERTY(EditDefaultsOnly, Category="Widgets")
	TSubclassOf<UUserWidget> InGame_PlayerStatusWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category="Widgets")
	TSubclassOf<UUserWidget> InGame_PlayerWeaponInfoWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category="Widgets")
	TSubclassOf<UUserWidget> InGame_GameStatusWidgetClass;

private:
	// In Game UI Widget
	UPROPERTY()
	UMPlayerStatusWidget* InGame_PlayerStatusWidget;

	UPROPERTY()
	UMPlayerWeaponInfoWidget* InGame_PlayerWeaponInfoWidget;

	UPROPERTY()
	UMGameStatusWidget* InGame_GameStatusWidget;
};
