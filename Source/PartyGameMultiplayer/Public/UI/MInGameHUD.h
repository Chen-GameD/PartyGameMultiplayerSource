// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PlayerUI/MGameStatusWidget.h"
#include "PlayerUI/MLobbyWidget.h"
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
	void StartGameUI(FString& userName);

////////////////////////////////////////////////////
/////Interface
////////////////////////////////////////////////////	
	// Show or Hide InGame_PlayerStatusWidget
	UFUNCTION()
	void InGame_SetVisibilityPlayerStatusWidget(ESlateVisibility n_Visibility);
	// Update PlayerName UI
	UFUNCTION()
	void InGame_UpdatePlayerNameUI(FString& userName);
	// Update Player Health UI
	UFUNCTION()
	void InGame_UpdatePlayerHealth(float percentage);
	UFUNCTION()
	void InGame_ToggleInvincibleUI(bool isShowing);
	// Update Player Skill UI
	void InGame_OnSkillUse(SkillType UseSkill, float CoolDownTotalTime);
	// Update Player Skill Opacity
	void InGame_SkillUIOpacityUpdate(SkillType UseSkill, float percentage);
	// Toggle Player Buff Widget
	void InGame_ToggleFireBuffWidget(bool isShowing);
	void InGame_ToggleShockBuffWidget(bool isShowing);
	

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
	void InGame_UpdateMinigameHint(FString i_Hint, UTexture2D* i_HintImage);
	// Initialize the game status UI content
	UFUNCTION()
	void InGame_InitGameStatusAndPlayerStatusWidgetContent();
	// Broadcasting system
	UFUNCTION()
	void InGame_BroadcastInformation(int KillerTeamIndex, int DeceasedTeamIndex, FString i_KillerName, FString i_DeceasedName, UTexture2D* i_WeaponImage);

	// Show or Hide InGame_LobbyWidget
	UFUNCTION()
	void InGame_SetVisibilityLobbyWidget(ESlateVisibility n_Visibility);
	UFUNCTION()
	void InGame_UpdateLobbyInformation(TArray<FLobbyInformationStruct> i_Team1Arr, TArray<FLobbyInformationStruct> i_Team2Arr, TArray<FLobbyInformationStruct> i_UndecidedArr);
	UFUNCTION()
	void InGame_UpdateTeam1LobbyInformation(TArray<FLobbyInformationStruct> i_TeamArr);
	UFUNCTION()
	void InGame_UpdateTeam2LobbyInformation(TArray<FLobbyInformationStruct> i_TeamArr);
	UFUNCTION()
	void InGame_UpdateUndecidedLobbyInformation(TArray<FLobbyInformationStruct> i_TeamArr);

protected:
	// In Game UI Class Ref
	UPROPERTY(EditDefaultsOnly, Category="Widgets")
	TSubclassOf<UUserWidget> InGame_PlayerStatusWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category="Widgets")
	TSubclassOf<UUserWidget> InGame_PlayerWeaponInfoWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category="Widgets")
	TSubclassOf<UUserWidget> InGame_GameStatusWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category="Widgets")
	TSubclassOf<UUserWidget> InGame_LobbyWidgetClass;

private:
	// In Game UI Widget
	UPROPERTY()
	UMPlayerStatusWidget* InGame_PlayerStatusWidget;

	UPROPERTY()
	UMPlayerWeaponInfoWidget* InGame_PlayerWeaponInfoWidget;

	UPROPERTY()
	UMGameStatusWidget* InGame_GameStatusWidget;

	UPROPERTY()
	UMLobbyWidget* InGame_LobbyWidget;
};
