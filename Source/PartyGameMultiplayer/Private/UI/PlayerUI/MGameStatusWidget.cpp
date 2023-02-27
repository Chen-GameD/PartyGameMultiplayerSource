// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerUI/MGameStatusWidget.h"

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
}

void UMGameStatusWidget::UpdateMinigameInfo(FString i_Info)
{
	MinigameInfo->SetText(FText::FromString(i_Info));

	ShowMinigameInfoAnimation();
}
