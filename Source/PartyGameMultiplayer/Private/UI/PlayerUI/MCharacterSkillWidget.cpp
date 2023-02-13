// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerUI/MCharacterSkillWidget.h"

void UMCharacterSkillWidget::UpdateDashInfo(float percentage)
{
	if(IsValid(Dash_INFO))
	{
		Dash_INFO->SetPercent(percentage);
	}
}
