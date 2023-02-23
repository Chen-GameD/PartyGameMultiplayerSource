// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MCharacterFollowWidget.h"

#include "Character/MPlayerController.h"
#include "Components/NamedSlot.h"
#include "GameBase/MGameState.h"

void UMCharacterFollowWidget::SetLocalControlledUI()
{
	HealthBar->SetVisibility(ESlateVisibility::Hidden);
	InGame_WeaponEnergyCanvasHolder->SetVisibility(ESlateVisibility::Visible);
	PlayerName->SetVisibility(ESlateVisibility::Hidden);
}

void UMCharacterFollowWidget::SetHealthToProgressBar(float percentage)
{
	if(IsValid(HealthBar))
	{
		HealthBar->SetPercent(percentage);
	}
}

void UMCharacterFollowWidget::SetPlayerName(FString i_PlayerName)
{
	PlayerName->SetText(FText::FromString(i_PlayerName));
}


void UMCharacterFollowWidget::ShowTip()
{
	if (Tip_Left && Tip_Right && Tip_Left_Text && Tip_Right_Text)
	{
		Tip_Left->SetVisibility(ESlateVisibility::Visible);
		Tip_Right->SetVisibility(ESlateVisibility::Visible);
		Tip_Left_Text->SetVisibility(ESlateVisibility::Visible);
		Tip_Right_Text->SetVisibility(ESlateVisibility::Visible);
	}
}

void UMCharacterFollowWidget::HideTip()
{
	if (Tip_Left && Tip_Right && Tip_Left_Text && Tip_Right_Text)
	{
		Tip_Left->SetVisibility(ESlateVisibility::Hidden);
		Tip_Right->SetVisibility(ESlateVisibility::Hidden);
		Tip_Left_Text->SetVisibility(ESlateVisibility::Hidden);
		Tip_Right_Text->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UMCharacterFollowWidget::SetWeaponEnergyUIVisibility(bool IsVisible)
{
	if (IsVisible)
	{
		if (InGame_WeaponEnergyCanvas->GetVisibility() == ESlateVisibility::Hidden)
		{
			InGame_WeaponEnergyCanvas->SetVisibility(ESlateVisibility::Visible);
		}
	}
	else
	{
		if (InGame_WeaponEnergyCanvas->GetVisibility() == ESlateVisibility::Visible)
		{
			InGame_WeaponEnergyCanvas->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void UMCharacterFollowWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	AMPlayerController* MyPlayerController = Cast<AMPlayerController>(GetOwningPlayer());
	if (MyPlayerController)
	{
		AMGameState* MyGameState = Cast<AMGameState>(GetWorld()->GetGameState());
		if (MyGameState->IsGameStart)
		{
			// Start to update Weapon Energy
			AMCharacter* MyCharacter = Cast<AMCharacter>(MyPlayerController->GetPawn());
			if (MyCharacter)
			{
				float CurrentPercent = MyCharacter->GetCurrentEnergyWeaponUIUpdatePercent();
				if (CurrentPercent < 0)
				{
					// Do Not Need To Show Current Energy(No Energy Weapon)
					SetWeaponEnergyUIVisibility(false);
				}
				else
				{
					// Need To Show Energy
					SetWeaponEnergyUIVisibility(true);

					//Set Percent
					SetWeaponEnergyProgressBar(CurrentPercent);
				}
			}
		}
	}
}

