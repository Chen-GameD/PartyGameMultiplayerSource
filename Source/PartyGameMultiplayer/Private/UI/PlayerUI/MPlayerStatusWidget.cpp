// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerUI/MPlayerStatusWidget.h"

#include "Character/MCharacter.h"
#include "UI/PlayerUI/MCharacterSkillWidget.h"

void UMPlayerStatusWidget::ShowWidget(bool IsShowing)
{
	this->SetVisibility(IsShowing ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void UMPlayerStatusWidget::UpdateHealthBar(float percentage)
{
	if(IsValid(HealthBar))
	{
		HealthBar->SetPercent(percentage);
	}
}

void UMPlayerStatusWidget::SetPlayerName(FString i_Name)
{
	PlayerName->SetText(FText::FromString(i_Name));
}

void UMPlayerStatusWidget::ShowFireBuff(bool IsShowing)
{
	if (BuffInfo)
	{
		BuffInfo->ShowFireBuff(IsShowing);
	}
}

void UMPlayerStatusWidget::ShowShockBuff(bool IsShowing)
{
	if (BuffInfo)
	{
		BuffInfo->ShowShockBuff(IsShowing);
	}
}

void UMPlayerStatusWidget::SkillUIUpdate(SkillType UseSkill, float percentage)
{
	if (SkillInfo)
	{
		switch (UseSkill)
		{
		case SkillType::SKILL_DASH:
        		SkillInfo->UpdateDashInfo(percentage);
			break;
		default:
			break;
		}
	}
	
}
