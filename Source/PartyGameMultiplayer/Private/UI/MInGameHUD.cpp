// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MInGameHUD.h"

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
	InGame_ShowPlayerStatusWidget();
	InGame_ShowPlayerWeaponInfoWidget();
	InGame_ShowGameStatusWidget();
}

void AMInGameHUD::InGame_ShowPlayerStatusWidget()
{
	if (InGame_PlayerStatusWidget)
	{
		InGame_PlayerStatusWidget->SetVisibility(ESlateVisibility::Visible);
	}
}

void AMInGameHUD::InGame_UpdatePlayerHealth(float percentage)
{
	if (InGame_PlayerStatusWidget)
	{
		InGame_PlayerStatusWidget->UpdateHealthBar(percentage);
	}
}

void AMInGameHUD::InGame_ShowPlayerWeaponInfoWidget()
{
	if (InGame_PlayerWeaponInfoWidget)
	{
		InGame_PlayerWeaponInfoWidget->SetVisibility(ESlateVisibility::Visible);
	}
}

void AMInGameHUD::InGame_ShowGameStatusWidget()
{
	if (InGame_GameStatusWidget)
	{
		InGame_GameStatusWidget->SetVisibility(ESlateVisibility::Visible);
	}
}
