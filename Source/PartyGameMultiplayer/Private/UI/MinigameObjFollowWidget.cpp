// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MinigameObjFollowWidget.h"

void UMinigameObjFollowWidget::SetHealthByPercentage(float i_percentage)
{
	if (IsValid(HealthBar))
	{
		HealthBar->SetPercent(i_percentage);
	}
}

void UMinigameObjFollowWidget::SetName(FString i_Name)
{
	if (IsValid(Name))
	{
		Name->SetText(FText::FromString(i_Name));
	}
}