// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/EnemyCrabFollowWidget.h"
#include "Components/NamedSlot.h"
#include "GameBase/MGameState.h"

void UEnemyCrabFollowWidget::SetHealthByPercentage(float i_percentage)
{
	if (IsValid(HealthBar))
	{
		HealthBar->SetPercent(i_percentage);
	}
}

void UEnemyCrabFollowWidget::SetName(FString i_Name)
{
	Name->SetText(FText::FromString(i_Name));
}