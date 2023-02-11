// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerUI/MCharacterBuffWidget.h"


void UMCharacterBuffWidget::ShowFireBuff(bool IsShowing)
{
	FireBuffImage->SetVisibility(IsShowing ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void UMCharacterBuffWidget::ShowShockBuff(bool IsShowing)
{
	ShockBuffImage->SetVisibility(IsShowing ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}
