// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerUI/MPlayerStatusWidget.h"

#include "Character/MCharacter.h"
#include "Components/NamedSlot.h"
#include "Kismet/GameplayStatics.h"
#include "UI/PlayerUI/MCharacterSkillWidget.h"

UMPlayerStatusWidget::UMPlayerStatusWidget(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{
}

void UMPlayerStatusWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (BuffSlot && BuffInfoClass)
	{
		BuffInfo = CreateWidget<UMCharacterBuffWidget>(this, BuffInfoClass, "BuffWidget");
		BuffSlot->AddChild(BuffInfo);
		ToggleBuffUI(true);
		//ToggleFireBuffUI(false);
		//ToggleShockBuffUI(false);
	}

	if (SkillSlot && SkillInfoClass)
	{
		SkillInfo = CreateWidget<UMCharacterSkillWidget>(this, SkillInfoClass, "SkillInfoWidget");
		SkillSlot->AddChild(SkillInfo);
	}
}

void UMPlayerStatusWidget::ShowWidget(bool isShowing)
{
	this->SetVisibility(isShowing ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void UMPlayerStatusWidget::UpdateHealthBar(float percentage)
{
	if(IsValid(HealthBar))
	{
		HealthBar->SetPercent(percentage);
	}
}

void UMPlayerStatusWidget::ToggleInvicibleUI(bool isShowing)
{
	if (InvincibleUI)
	{
		InvincibleUI->SetVisibility(isShowing ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void UMPlayerStatusWidget::SetPlayerName(FString i_Name)
{
	PlayerName->SetText(FText::FromString(i_Name));
}

void UMPlayerStatusWidget::ToggleBuffUI(bool isShowing)
{
	ToggleFireBuffUI(isShowing);
	ToggleShockBuffUI(isShowing);
}

void UMPlayerStatusWidget::ToggleFireBuffUI(bool isShowing)
{
	if (BuffInfo)
	{
		BuffInfo->ToggleFireBuff(isShowing);
	}
}

void UMPlayerStatusWidget::ToggleShockBuffUI(bool isShowing)
{
	if (BuffInfo)
	{
		BuffInfo->ToggleShockBuff(isShowing);
	}
}

void UMPlayerStatusWidget::OnSkillUse(SkillType UseSkill, float CoolDownTotalTime)
{
	if (SkillInfo)
	{
		switch (UseSkill)
		{
		case SkillType::SKILL_DASH:
        		SkillInfo->OnUseDashSkill(CoolDownTotalTime);
			break;
		default:
			break;
		}
	}
}

void UMPlayerStatusWidget::SkillUIOpacityUpdate(SkillType UseSkill, float percentage)
{
	if (SkillInfo)
	{
		switch (UseSkill)
		{
		case SkillType::SKILL_DASH:
			SkillInfo->UpdateDashOpacity(percentage);
			break;
		default:
			break;
		}
	}
}
