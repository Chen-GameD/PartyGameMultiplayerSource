// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MCharacterFollowWidget.h"

#include "Character/MPlayerController.h"
#include "GameBase/MGameState.h"

void UMCharacterFollowWidget::SetLocalControlledUI()
{
	HealthBar->SetVisibility(ESlateVisibility::Hidden);
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
				// Is Combine Weapon
				SetWeaponEnergyProgressBar(MyCharacter->GetCurrentEnergyWeaponUIUpdatePercent());
			}
			
		}
	}
}

