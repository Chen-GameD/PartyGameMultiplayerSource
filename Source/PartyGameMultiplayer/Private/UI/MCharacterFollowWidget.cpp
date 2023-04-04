// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MCharacterFollowWidget.h"

#include "Character/MPlayerController.h"
#include "Components/NamedSlot.h"
#include "GameBase/MGameState.h"

void UMCharacterFollowWidget::InitIsLocalControlledCharacterWidget(bool IsLocalControlled)
{
	if (IsLocalControlled)
	{
		HealthBar_Enemy->SetVisibility(ESlateVisibility::Hidden);
		HealthBar_Teammate->SetVisibility(ESlateVisibility::Hidden);
		PlayerName->SetVisibility(ESlateVisibility::Hidden);
		InGame_WeaponEnergyCanvasHolder->SetVisibility(ESlateVisibility::Visible);
		WeaponTipCanvas->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		HealthBar_Enemy->SetVisibility(ESlateVisibility::Visible);
		HealthBar_Teammate->SetVisibility(ESlateVisibility::Visible);
		PlayerName->SetVisibility(ESlateVisibility::Visible);
		InGame_WeaponEnergyCanvasHolder->SetVisibility(ESlateVisibility::Hidden);
		WeaponTipCanvas->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UMCharacterFollowWidget::SetHealthToProgressBar(float percentage)
{
	if(IsValid(HealthBar_Enemy) && IsValid(HealthBar_Teammate))
	{
		HealthBar_Enemy->SetPercent(percentage);
		HealthBar_Teammate->SetPercent(percentage);
	}
}

void UMCharacterFollowWidget::SetPlayerName(FString i_PlayerName)
{
	PlayerName->SetText(FText::FromString(i_PlayerName));
}

void UMCharacterFollowWidget::SetIsEnemyHealthBar(bool IsEnemy)
{
	HealthBar_Teammate->SetVisibility(IsEnemy ? ESlateVisibility::Hidden : ESlateVisibility::Visible);
	HealthBar_Enemy->SetVisibility(IsEnemy ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void UMCharacterFollowWidget::SetHealthBarRenderOpacity(float percentage)
{
	HealthBar_Enemy->SetRenderOpacity(percentage);
	HealthBar_Teammate->SetRenderOpacity(percentage);
}

void UMCharacterFollowWidget::SetPlayerNameRenderOpacity(float percentage)
{
	PlayerName->SetRenderOpacity(percentage);
}


void UMCharacterFollowWidget::ShowTip()
{
	if (Tip_Left && Tip_Right && /*Tip_Left_Text && Tip_Right_Text &&*/ Tip_Left_Weapon && Tip_Right_Weapon)
	{
		Tip_Left->SetVisibility(ESlateVisibility::Visible);
		Tip_Right->SetVisibility(ESlateVisibility::Visible);
		Tip_Left_Weapon->SetVisibility(ESlateVisibility::Visible);
		Tip_Right_Weapon->SetVisibility(ESlateVisibility::Visible);
	}
}

void UMCharacterFollowWidget::HideTip()
{
	if (Tip_Left && Tip_Right && /*Tip_Left_Text && Tip_Right_Text &&*/ Tip_Left_Weapon && Tip_Right_Weapon)
	{
		Tip_Left->SetVisibility(ESlateVisibility::Hidden);
		Tip_Right->SetVisibility(ESlateVisibility::Hidden);
		Tip_Left_Weapon->SetVisibility(ESlateVisibility::Hidden);
		Tip_Right_Weapon->SetVisibility(ESlateVisibility::Hidden);
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

void UMCharacterFollowWidget::SetLeftWeaponTipUI(UTexture2D* texture)
{
	Tip_Left_Weapon->SetBrushFromTexture(texture);
	FLinearColor LColorAndOpacity = Tip_Left_Weapon->ColorAndOpacity;
	if(texture != nullptr)
	{
		LColorAndOpacity.A = 1;
		Tip_Left_Weapon->SetColorAndOpacity(LColorAndOpacity);
	}
	else
	{
		LColorAndOpacity.A = 0;
		Tip_Left_Weapon->SetColorAndOpacity(LColorAndOpacity);
	}
}

void UMCharacterFollowWidget::SetRightWeaponTipUI(UTexture2D* texture)
{
	Tip_Right_Weapon->SetBrushFromTexture(texture);
	FLinearColor LColorAndOpacity = Tip_Right_Weapon->ColorAndOpacity;
	if(texture != nullptr)
	{
		LColorAndOpacity.A = 1;
		Tip_Right_Weapon->SetColorAndOpacity(LColorAndOpacity);
	}
	else
	{
		LColorAndOpacity.A = 0;
		Tip_Right_Weapon->SetColorAndOpacity(LColorAndOpacity);
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

