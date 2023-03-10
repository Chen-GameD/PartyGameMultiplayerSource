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
	// Show or Hide InGame_PlayerStatusWidget
	UFUNCTION()
	void InGame_SetVisibilityPlayerStatusWidget(ESlateVisibility n_Visibility);
	// Update Player Health UI
	UFUNCTION()
	void InGame_UpdatePlayerHealth(float percentage);

	// Show or Hide InGame_PlayerWeaponInfoWidget
	UFUNCTION()
	void InGame_SetVisibilityPlayerWeaponInfoWidget(ESlateVisibility n_Visibility);

	// Show or Hide InGame_GameStatusWidget
	UFUNCTION()
	void InGame_SetVisibilityGameStatusWidget(ESlateVisibility n_Visibility);
	// Update Team Score in total
	UFUNCTION()
	void InGame_UpdateTeamScore(int TeamIndex, int CurrentScore);
	// Update Timer
	UFUNCTION()
	void InGame_UpdateTimer(int CurrentTimer);
	// Show and update minigame hint ( with animation )
	UFUNCTION()
	void InGame_UpdateMinigameHint(FString i_Hint);
	// Initialize the game status UI content
	UFUNCTION()
	void InGame_InitGameStatusWidgetContent();

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
