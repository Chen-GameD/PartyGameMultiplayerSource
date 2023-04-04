// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerUI/MCharacterSkillWidget.h"

void UMCharacterSkillWidget::OnUseDashSkill(float CoolDownTotalTime)
{
	UpdateDashOpacity(0.5);
	// Set Timer to update the progress bar
	Dash_Timer = 0;
	Dash_CoolDown = CoolDownTotalTime;
	GetWorld()->GetTimerManager().SetTimer(Dash_TimerHandle, this, &UMCharacterSkillWidget::DashTimerHandleFunction, 0.05, true);
}

void UMCharacterSkillWidget::UpdateDashInfo(float percentage)
{
	if(IsValid(Dash_INFO))
	{
		Dash_INFO->SetPercent(percentage);
	}
}

void UMCharacterSkillWidget::UpdateDashOpacity(float percentage)
{
	if (IsValid(Dash_INFO))
	{
		Dash_INFO->SetRenderOpacity(percentage);
	}
}

void UMCharacterSkillWidget::DashTimerHandleFunction()
{
	Dash_Timer += 0.05;
	if (Dash_Timer >= Dash_CoolDown)
	{
		Dash_Timer = Dash_CoolDown;
		GetWorld()->GetTimerManager().ClearTimer(Dash_TimerHandle);
		UpdateDashOpacity(1);
	}
	UpdateDashInfo(Dash_Timer / Dash_CoolDown);
}
