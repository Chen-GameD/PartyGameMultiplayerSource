// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MInGameHUD.h"

#include "GameBase/MGameState.h"

AMInGameHUD::AMInGameHUD()
{
}

void AMInGameHUD::DrawHUD()
{
	Super::DrawHUD();
}

void AMInGameHUD::BeginPlay()
{
	Super::BeginPlay();

	// Set all ref, and add them to the viewport but without displaying;
	if (InGame_PlayerStatusWidgetClass)
	{
		if (!InGame_PlayerStatusWidget)
		{
			InGame_PlayerStatusWidget = CreateWidget<UMPlayerStatusWidget>(GetWorld(), InGame_PlayerStatusWidgetClass, "PlayerStatusWidget");
			if (InGame_PlayerStatusWidget)
			{
				InGame_PlayerStatusWidget->AddToViewport();
				InGame_PlayerStatusWidget->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}
	if (InGame_PlayerWeaponInfoWidgetClass)
	{
		if (!InGame_PlayerWeaponInfoWidget)
		{
			InGame_PlayerWeaponInfoWidget = CreateWidget<UMPlayerWeaponInfoWidget>(GetWorld(), InGame_PlayerWeaponInfoWidgetClass, "PlayerWeaponInfoWidget");
			if (InGame_PlayerWeaponInfoWidget)
			{
				InGame_PlayerWeaponInfoWidget->AddToViewport();
				InGame_PlayerWeaponInfoWidget->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}
	if (InGame_GameStatusWidgetClass)
	{
		if (!InGame_GameStatusWidget)
		{
			InGame_GameStatusWidget = CreateWidget<UMGameStatusWidget>(GetWorld(), InGame_GameStatusWidgetClass, "GameStatusWidget");
			if (InGame_GameStatusWidget)
			{
				InGame_GameStatusWidget->AddToViewport();
				InGame_GameStatusWidget->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}
	if (InGame_LobbyWidgetClass)
	{
		if (!InGame_LobbyWidget)
		{
			InGame_LobbyWidget = CreateWidget<UMLobbyWidget>(GetWorld(), InGame_LobbyWidgetClass, "LobbyWidget");
			if (InGame_LobbyWidget)
			{
				InGame_LobbyWidget->AddToViewport();
				InGame_LobbyWidget->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}

	IsFinishedInit = true;
}

void AMInGameHUD::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AMInGameHUD::StartGameUI(FString& userName)
{
	InGame_SetVisibilityPlayerStatusWidget(ESlateVisibility::Visible);
	InGame_UpdatePlayerNameUI(userName);
	InGame_SetVisibilityPlayerWeaponInfoWidget(ESlateVisibility::Visible);
	InGame_SetVisibilityGameStatusWidget(ESlateVisibility::Visible);
}

void AMInGameHUD::InGame_SetVisibilityPlayerStatusWidget(ESlateVisibility n_Visibility)
{
	if (InGame_PlayerStatusWidget)
	{
		InGame_PlayerStatusWidget->SetVisibility(n_Visibility);
	}
}

void AMInGameHUD::InGame_UpdatePlayerNameUI(FString& userName)
{
	if (!InGame_PlayerStatusWidget)
	{
		InGame_PlayerStatusWidget = CreateWidget<UMPlayerStatusWidget>(GetWorld(), InGame_PlayerStatusWidgetClass, "PlayerStatusWidget");
		if (InGame_PlayerStatusWidget)
		{
    		InGame_PlayerStatusWidget->AddToViewport();
    		InGame_PlayerStatusWidget->SetVisibility(ESlateVisibility::Hidden);
    	}
	}
	
	if (InGame_PlayerStatusWidget)
	{
		InGame_PlayerStatusWidget->SetPlayerName(userName);
	}
}

void AMInGameHUD::InGame_UpdatePlayerHealth(float percentage)
{
	if (InGame_PlayerStatusWidget)
	{
		InGame_PlayerStatusWidget->UpdateHealthBar(percentage);
	}
}

void AMInGameHUD::InGame_ToggleInvincibleUI(bool isShowing)
{
	if (InGame_PlayerStatusWidget)
	{
		InGame_PlayerStatusWidget->ToggleInvincibleUI(isShowing);
	}
}

void AMInGameHUD::InGame_OnSkillUse(SkillType UseSkill, float CoolDownTotalTime)
{
	if (InGame_PlayerStatusWidget)
	{
		InGame_PlayerStatusWidget->OnSkillUse(UseSkill, CoolDownTotalTime);
	}
}

void AMInGameHUD::InGame_SkillUIOpacityUpdate(SkillType UseSkill, float percentage)
{
	if (InGame_PlayerStatusWidget)
	{
		InGame_PlayerStatusWidget->SkillUIOpacityUpdate(UseSkill, percentage);
	}
}

void AMInGameHUD::InGame_ToggleFireBuffWidget(bool isShowing)
{
	if (InGame_PlayerStatusWidget)
	{
		InGame_PlayerStatusWidget->ToggleFireBuffUI(isShowing);
	}
}

void AMInGameHUD::InGame_ToggleShockBuffWidget(bool isShowing)
{
	if (InGame_PlayerStatusWidget)
	{
		InGame_PlayerStatusWidget->ToggleShockBuffUI(isShowing);
	}
}

void AMInGameHUD::InGame_SetVisibilityPlayerWeaponInfoWidget(ESlateVisibility n_Visibility)
{
	if (InGame_PlayerWeaponInfoWidget)
	{
		InGame_PlayerWeaponInfoWidget->SetVisibility(n_Visibility);
	}
}

void AMInGameHUD::InGame_SetVisibilityGameStatusWidget(ESlateVisibility n_Visibility)
{
	if (InGame_GameStatusWidget)
	{
		InGame_GameStatusWidget->SetVisibility(n_Visibility);
	}
}

void AMInGameHUD::InGame_UpdateTeamScore(int TeamIndex, int CurrentScore)
{
	if (InGame_GameStatusWidget)
	{
		if (TeamIndex == 1)
		{
			InGame_GameStatusWidget->UpdateTeam_1_Score(CurrentScore);
		}
		else if (TeamIndex == 2)
		{
			InGame_GameStatusWidget->UpdateTeam_2_Score(CurrentScore);
		}
		else
		{
			// Invalid TeamIndex
			// TODO
		}
	}
}

void AMInGameHUD::InGame_UpdateTimer(int CurrentTimer)
{
	if (InGame_GameStatusWidget)
	{
		InGame_GameStatusWidget->UpdateGameTimer(CurrentTimer);
	}
}

void AMInGameHUD::InGame_UpdateMinigameHint(FString i_Hint, UTexture2D* i_HintImage)
{
	if (InGame_GameStatusWidget)
	{
		InGame_GameStatusWidget->UpdateMinigameInfo(i_Hint, i_HintImage);
		
	}
}

void AMInGameHUD::InGame_InitGameStatusAndPlayerStatusWidgetContent()
{
	AMGameState* MyGameState = Cast<AMGameState>(GetWorld()->GetGameState());
	if (MyGameState)
	{
		InGame_SetVisibilityGameStatusWidget(ESlateVisibility::Visible);
		InGame_UpdateTimer(MyGameState->GameTime);
		InGame_UpdateTeamScore(1, 0);
		InGame_UpdateTeamScore(2, 0);
		InGame_SetVisibilityPlayerStatusWidget(ESlateVisibility::Visible);
		InGame_UpdatePlayerHealth(1);
		// Update Minigame Tip
		// TODO
	}
	
}

void AMInGameHUD::InGame_BroadcastInformation(int KillerTeamIndex, int DeceasedTeamIndex, FString i_KillerName, FString i_DeceasedName, UTexture2D* i_WeaponImage)
{
	if (InGame_GameStatusWidget)
	{
		InGame_GameStatusWidget->UpdateAndShowBroadcastingInformation(KillerTeamIndex, DeceasedTeamIndex, i_KillerName, i_DeceasedName, i_WeaponImage);
	}
}

void AMInGameHUD::InGame_BroadcastMinigameInformation(int KillerTeamIndex, FString i_KillerName, FString i_MinigameInformation)
{
	if (InGame_GameStatusWidget)
	{
		InGame_GameStatusWidget->UpdateAndShowMiniBroadcastingInformation(KillerTeamIndex, i_KillerName, i_MinigameInformation);
	}
}

void AMInGameHUD::InGame_GameEnd()
{
	if (InGame_GameStatusWidget)
	{
		InGame_GameStatusWidget->BPF_GameEnd();
	}
}

void AMInGameHUD::InGame_HideMinigameInfo()
{
	if (InGame_GameStatusWidget)
	{
		InGame_GameStatusWidget->HideMinigameInfo();
	}
}

void AMInGameHUD::InGame_CountdownAnimation()
{
	if (InGame_GameStatusWidget)
	{
		InGame_GameStatusWidget->BPF_CountdownAnimation();
	}
}

void AMInGameHUD::InGame_SetVisibilityLobbyWidget(ESlateVisibility n_Visibility)
{
	if (InGame_LobbyWidget)
	{
		InGame_LobbyWidget->SetVisibility(n_Visibility);
	}
}

void AMInGameHUD::InGame_UpdateLobbyInformation(TArray<FLobbyInformationStruct> i_Team1Arr, TArray<FLobbyInformationStruct> i_Team2Arr, TArray<FLobbyInformationStruct> i_UndecidedArr)
{
	if (InGame_LobbyWidget)
	{
		InGame_LobbyWidget->UpdateLobbyInformation(i_Team1Arr, i_Team2Arr, i_UndecidedArr);
	}
	else
	{
		if (InGame_LobbyWidgetClass)
		{
			if (!InGame_LobbyWidget)
			{
				InGame_LobbyWidget = CreateWidget<UMLobbyWidget>(GetWorld(), InGame_LobbyWidgetClass, "LobbyWidget");
				if (InGame_LobbyWidget)
				{
					InGame_LobbyWidget->AddToViewport();
					InGame_LobbyWidget->SetVisibility(ESlateVisibility::Visible);
					InGame_LobbyWidget->UpdateLobbyInformation(i_Team1Arr, i_Team2Arr, i_UndecidedArr);
				}
			}
		}
	}
}

void AMInGameHUD::InGame_UpdateTeam1LobbyInformation(TArray<FLobbyInformationStruct> i_TeamArr)
{
	if (InGame_LobbyWidget)
	{
		InGame_LobbyWidget->UpdateTeam1LobbyInformation(i_TeamArr);
	}
	else
	{
		if (InGame_LobbyWidgetClass)
		{
			if (!InGame_LobbyWidget)
			{
				InGame_LobbyWidget = CreateWidget<UMLobbyWidget>(GetWorld(), InGame_LobbyWidgetClass, "LobbyWidget");
				if (InGame_LobbyWidget)
				{
					InGame_LobbyWidget->AddToViewport();
					InGame_LobbyWidget->SetVisibility(ESlateVisibility::Visible);
					InGame_LobbyWidget->UpdateTeam1LobbyInformation(i_TeamArr);
				}
			}
		}
	}
}

void AMInGameHUD::InGame_UpdateTeam2LobbyInformation(TArray<FLobbyInformationStruct> i_TeamArr)
{
	if (InGame_LobbyWidget)
	{
		InGame_LobbyWidget->UpdateTeam2LobbyInformation(i_TeamArr);
	}
	else
	{
		if (InGame_LobbyWidgetClass)
		{
			if (!InGame_LobbyWidget)
			{
				InGame_LobbyWidget = CreateWidget<UMLobbyWidget>(GetWorld(), InGame_LobbyWidgetClass, "LobbyWidget");
				if (InGame_LobbyWidget)
				{
					InGame_LobbyWidget->AddToViewport();
					InGame_LobbyWidget->SetVisibility(ESlateVisibility::Visible);
					InGame_LobbyWidget->UpdateTeam2LobbyInformation(i_TeamArr);
				}
			}
		}
	}
}

void AMInGameHUD::InGame_UpdateUndecidedLobbyInformation(TArray<FLobbyInformationStruct> i_TeamArr)
{
	if (InGame_LobbyWidget)
	{
		InGame_LobbyWidget->UpdateUndecidedLobbyInformation(i_TeamArr);
	}
	else
	{
		if (InGame_LobbyWidgetClass)
		{
			if (!InGame_LobbyWidget)
			{
				InGame_LobbyWidget = CreateWidget<UMLobbyWidget>(GetWorld(), InGame_LobbyWidgetClass, "LobbyWidget");
				if (InGame_LobbyWidget)
				{
					InGame_LobbyWidget->AddToViewport();
					InGame_LobbyWidget->SetVisibility(ESlateVisibility::Visible);
					InGame_LobbyWidget->UpdateUndecidedLobbyInformation(i_TeamArr);
				}
			}
		}
	}
}

void AMInGameHUD::InGame_UpdateReadyButtonState(bool isReady)
{
	if (InGame_LobbyWidget)
	{
		InGame_LobbyWidget->UpdateReadyButtonState(isReady);
	}
}

void AMInGameHUD::InGame_UpdateEqualConditionState(bool isEqual)
{
	if (InGame_LobbyWidget)
	{
		InGame_LobbyWidget->UpdateEqualConditionState(isEqual);
	}
}

void AMInGameHUD::InGame_UpdateReadyConditionState(bool isReady)
{
	if (InGame_LobbyWidget)
	{
		InGame_LobbyWidget->UpdateReadyConditionState(isReady);
	}
}

void AMInGameHUD::InGame_UpdateHintPageInformation(int levelIndex)
{
	if (InGame_LobbyWidget)
	{
		InGame_LobbyWidget->UpdateHintPageInformation(levelIndex);
	}
}

UMLobbyWidget* AMInGameHUD::InGame_GetLobbyWidget()
{
	if (InGame_LobbyWidget)
	{
		return InGame_LobbyWidget;
	}

	return nullptr;
}
