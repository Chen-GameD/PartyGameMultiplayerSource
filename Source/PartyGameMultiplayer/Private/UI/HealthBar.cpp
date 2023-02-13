// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HealthBar.h"

void UHealthBar::SetLocalControlledUI()
{
	healthBar->SetVisibility(ESlateVisibility::Hidden);
	healthText->SetVisibility(ESlateVisibility::Hidden);
}

void UHealthBar::SetHealthToProgressBar(float percentage)
{
	if(!IsValid(healthBar))
	{
		return;
	}
	else
	{
		healthBar->SetPercent(percentage);
		healthText->SetText(FText::AsNumber(percentage*100));
	}
}


void UHealthBar::ShowTip()
{
	if (Tip_Left && Tip_Right && Tip_Left_Text && Tip_Right_Text)
	{
		Tip_Left->SetVisibility(ESlateVisibility::Visible);
		Tip_Right->SetVisibility(ESlateVisibility::Visible);
		Tip_Left_Text->SetVisibility(ESlateVisibility::Visible);
		Tip_Right_Text->SetVisibility(ESlateVisibility::Visible);
	}
}

void UHealthBar::HideTip()
{
	if (Tip_Left && Tip_Right && Tip_Left_Text && Tip_Right_Text)
	{
		Tip_Left->SetVisibility(ESlateVisibility::Hidden);
		Tip_Right->SetVisibility(ESlateVisibility::Hidden);
		Tip_Left_Text->SetVisibility(ESlateVisibility::Hidden);
		Tip_Right_Text->SetVisibility(ESlateVisibility::Hidden);
	}
}

