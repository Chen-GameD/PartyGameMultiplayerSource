// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InventoryMenu.h"

void UInventoryMenu::SetLeftItemUI(UTexture2D* texture)
{
	leftItem->SetBrushFromTexture(texture);
	FLinearColor LColorAndOpacity = leftItem->ColorAndOpacity;
	if(texture != nullptr)
	{
		LColorAndOpacity.A = 1;
		leftItem->SetColorAndOpacity(LColorAndOpacity);
	}
	else
	{
		LColorAndOpacity.A = 0;
		leftItem->SetColorAndOpacity(LColorAndOpacity);
	}
}

void UInventoryMenu::SetRightItemUI(UTexture2D* texture)
{
	rightItem->SetBrushFromTexture(texture);
	FLinearColor LColorAndOpacity = rightItem->ColorAndOpacity;
	if(texture != nullptr)
	{
		LColorAndOpacity.A = 1;
		rightItem->SetColorAndOpacity(LColorAndOpacity);
	}
	else
	{
		LColorAndOpacity.A = 0;
		rightItem->SetColorAndOpacity(LColorAndOpacity);
	}
}

void UInventoryMenu::SetWeaponUI(UTexture2D* texture)
{
	weapon->SetBrushFromTexture(texture);
	FLinearColor LColorAndOpacity = weapon->ColorAndOpacity;
	if(texture != nullptr)
	{
		LColorAndOpacity.A = 1;
		weapon->SetColorAndOpacity(LColorAndOpacity);
	}
	else
	{
		LColorAndOpacity.A = 0;
		weapon->SetColorAndOpacity(LColorAndOpacity);
	}
}