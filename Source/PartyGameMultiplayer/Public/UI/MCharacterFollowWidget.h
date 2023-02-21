// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Character/MCharacter.h"
#include "Components/CanvasPanel.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "MCharacterFollowWidget.generated.h"

/**
 * 
 */
UCLASS()
class PARTYGAMEMULTIPLAYER_API UMCharacterFollowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetLocalControlledUI();
	void SetHealthToProgressBar(float percentage);
	void ShowTip();
	void HideTip();
	UFUNCTION()
	void SetPlayerName(FString i_PlayerName);

	UFUNCTION(BlueprintImplementableEvent)
	void SetWeaponEnergyProgressBar(float Percent);
	UFUNCTION()
	void SetWeaponEnergyUIVisibility(bool IsVisible);

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	
protected:
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerName;

	UPROPERTY(meta = (BindWidget))
	UImage* Tip_Left;
	UPROPERTY(meta = (BindWidget))
	UImage* Tip_Right;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Tip_Left_Text;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Tip_Right_Text;

	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* InGame_WeaponEnergyCanvasHolder;
	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* InGame_WeaponEnergyCanvas;
	UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
	UImage* InGame_WeaponEnergy;

public:
	AMCharacter* OwningPawn;
};
