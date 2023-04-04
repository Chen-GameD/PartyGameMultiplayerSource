// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerUI/MCharacterBuffWidget.h"

void UMCharacterBuffWidget::ToggleFireBuff(bool IsShowing)
{
	FireBuffImage->SetVisibility(IsShowing ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void UMCharacterBuffWidget::ToggleShockBuff(bool IsShowing)
{
	ShockBuffImage->SetVisibility(IsShowing ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void UMCharacterBuffWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// if (FireBuffImage && ShockBuffImage)
	// {
	// 	// FireBuffImage Opacity Animation
	// 	if (IsFireBuffNeedReduceOpacity)
	// 	{
	// 		FireBuffImage->SetOpacity(FireBuffImage->GetRenderOpacity() - InDeltaTime / 1000);
	// 		if (FireBuffImage->GetRenderOpacity() <= 0.5)
	// 			IsFireBuffNeedReduceOpacity = false;
	// 	}
	// 	else
	// 	{
	// 		FireBuffImage->SetOpacity(FireBuffImage->GetRenderOpacity() + InDeltaTime / 1000);
	// 		if (FireBuffImage->GetRenderOpacity() >= 1)
	// 			IsFireBuffNeedReduceOpacity = true;
	// 	}
	//
	// 	// ShockBuffImage Opacity Animation
	// 	if (IsShockBuffNeedReduceOpacity)
	// 	{
	// 		ShockBuffImage->SetOpacity(ShockBuffImage->GetRenderOpacity() - InDeltaTime / 1000);
	// 		if (ShockBuffImage->GetRenderOpacity() <= 0.5)
	// 			IsShockBuffNeedReduceOpacity = false;
	// 	}
	// 	else
	// 	{
	// 		ShockBuffImage->SetOpacity(ShockBuffImage->GetRenderOpacity() + InDeltaTime / 1000);
	// 		if (ShockBuffImage->GetRenderOpacity() >= 1)
	// 			IsShockBuffNeedReduceOpacity = true;
	// 	}
	// }
}