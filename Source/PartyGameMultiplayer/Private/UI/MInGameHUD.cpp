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
		InGame_PlayerStatusWidget = CreateWidget<UMPlayerStatusWidget>(GetWorld(), InGame_PlayerStatusWidgetClass, "PlayerStatusWidget");
		if (InGame_PlayerStatusWidget)
		{
			InGame_PlayerStatusWidget->AddToViewport();
			InGame_PlayerStatusWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
	if (InGame_PlayerWeaponInfoWidgetClass)
	{
		InGame_PlayerWeaponInfoWidget = CreateWidget<UMPlayerWeaponInfoWidget>(GetWorld(), InGame_PlayerWeaponInfoWidgetClass, "PlayerWeaponInfoWidget");
		if (InGame_PlayerWeaponInfoWidget)
		{
			InGame_PlayerWeaponInfoWidget->AddToViewport();
			InGame_PlayerWeaponInfoWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
	if (InGame_GameStatusWidgetClass)
	{
		InGame_GameStatusWidget = CreateWidget<UMGameStatusWidget>(GetWorld(), InGame_GameStatusWidgetClass, "GameStatusWidget");
		if (InGame_GameStatusWidget)
		{
			InGame_GameStatusWidget->AddToViewport();
			InGame_GameStatusWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void AMInGameHUD::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AMInGameHUD::StartGameUI()
{
	InGame_SetVisibilityPlayerStatusWidget(ESlateVisibility::Visible);
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

void AMInGameHUD::InGame_UpdatePlayerHealth(float percentage)
{
	if (InGame_PlayerStatusWidget)
	{
		InGame_PlayerStatusWidget->UpdateHealthBar(percentage);
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

void AMInGameHUD::InGame_UpdateMinigameHint(FString i_Hint)
{
	if (InGame_GameStatusWidget)
	{
		InGame_GameStatusWidget->UpdateMinigameInfo(i_Hint);
	}
}

void AMInGameHUD::InGame_InitGameStatusWidgetContent()
{
	AMGameState* MyGameState = Cast<AMGameState>(GetWorld()->GetGameState());
	if (MyGameState)
	{
		InGame_SetVisibilityGameStatusWidget(ESlateVisibility::Visible);
		InGame_UpdateTimer(MyGameState->GameTime);
		InGame_UpdateTeamScore(1, 0);
		InGame_UpdateTeamScore(2, 0);
		// Update Minigame Tip
		// TODO
	}
	
}
