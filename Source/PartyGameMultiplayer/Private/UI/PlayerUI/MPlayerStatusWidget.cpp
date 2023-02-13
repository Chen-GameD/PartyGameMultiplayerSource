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
		ToggleBuffUI(false);
	}

	if (SkillSlot && SkillInfoClass)
	{
		SkillInfo = CreateWidget<UMCharacterSkillWidget>(this, SkillInfoClass, "SkillInfoWidget");
		SkillSlot->AddChild(SkillInfo);
	}
}

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

void UMPlayerStatusWidget::ToggleBuffUI(bool IsShowing)
{
	ToggleFireBuffUI(IsShowing);
	ToggleShockBuffUI(IsShowing);
}

void UMPlayerStatusWidget::ToggleFireBuffUI(bool IsShowing)
{
	if (BuffInfo)
	{
		BuffInfo->ToggleFireBuff(IsShowing);
	}
}

void UMPlayerStatusWidget::ToggleShockBuffUI(bool IsShowing)
{
	if (BuffInfo)
	{
		BuffInfo->ToggleShockBuff(IsShowing);
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
