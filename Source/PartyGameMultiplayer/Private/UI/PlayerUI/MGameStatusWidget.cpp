// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerUI/MGameStatusWidget.h"
#include "SlateCore.h"


#include <string>

UMGameStatusWidget::UMGameStatusWidget(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{
}

void UMGameStatusWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UMGameStatusWidget::UpdateTeam_1_Score(int i_Score)
{
	Team_1_Score->SetText(FText::FromString(FString::FromInt(i_Score)));
}

void UMGameStatusWidget::UpdateTeam_2_Score(int i_Score)
{
	Team_2_Score->SetText(FText::FromString(FString::FromInt(i_Score)));
}

void UMGameStatusWidget::UpdateGameTimer(int i_GameTime)
{
	int Min = i_GameTime / 60;
	int Sec = i_GameTime % 60;

	FString SMin = FString::FromInt(Min);
	FString SSec = FString::FromInt(Sec);

	if (Min < 10)
	{
		SMin = FString::FromInt(0) + SMin;
	}

	if (Sec < 10)
	{
		SSec = FString::FromInt(0) + SSec;
	}

	GameTimer->SetText(FText::FromString(SMin + ":" + SSec));
	ShowTimerAnimation();
}

void UMGameStatusWidget::UpdateMinigameInfo(FString i_Info, UTexture2D* i_InfoImage)
{
	MinigameInfo->SetText(FText::FromString(i_Info));
	MinigameInfoImage->SetBrushFromTexture(i_InfoImage);

	ShowMinigameInfoAnimation();
}

void UMGameStatusWidget::UpdateAndShowBroadcastingInformation(int KillerTeamIndex, int DeceasedTeamIndex, FString i_KillerName, FString i_DeceasedName, UTexture2D* i_WeaponImage)
{
	KillerName->SetColorAndOpacity(FSlateColor(KillerTeamIndex == 1 ? Team1Color : Team2Color));
	DeceasedName->SetColorAndOpacity(FSlateColor(DeceasedTeamIndex == 1 ? Team1Color : Team2Color));
	KillerName->SetText(FText::FromString(i_KillerName));
	DeceasedName->SetText(FText::FromString(i_DeceasedName));
	WeaponImage->SetBrushFromTexture(i_WeaponImage);
	BroadcastTimer = 0;
	IsNeedShowBroadcast = true;
	BroadcastCanvas->SetVisibility(ESlateVisibility::Visible);
	BroadcastingAnimationEvent();
	UpdateKillBoardInformation(KillerTeamIndex, DeceasedTeamIndex, i_KillerName, i_DeceasedName, i_WeaponImage);
}
