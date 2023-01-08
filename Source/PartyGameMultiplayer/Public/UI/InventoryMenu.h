// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "InventoryMenu.generated.h"

UENUM()
enum InventoryUISlot
{
	Left,
	Right,
	Main
};

/**
 * 
 */
UCLASS()
class PARTYGAMEMULTIPLAYER_API UInventoryMenu : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget))
	UImage* leftItem;

	UPROPERTY(meta = (BindWidget))
	UImage* rightItem;

	UPROPERTY(meta = (BindWidget))
	UImage* weapon;
	
public:
	void SetLeftItemUI(UTexture2D* texture);
	void SetRightItemUI(UTexture2D* texture);
	void SetWeaponUI(UTexture2D* texture);
};
