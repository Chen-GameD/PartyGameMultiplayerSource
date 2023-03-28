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
	UFUNCTION()
	void InitIsLocalControlledCharacterWidget(bool IsLocalControlled);
	UFUNCTION()
	void SetHealthToProgressBar(float percentage);
	UFUNCTION()
	void ShowTip();
	UFUNCTION()
	void HideTip();
	UFUNCTION()
	void SetPlayerName(FString i_PlayerName);
	UFUNCTION()
	void SetIsEnemyHealthBar(bool IsEnemy);

	UFUNCTION()
	void SetHealthBarRenderOpacity(float percentage);
	UFUNCTION()
	void SetPlayerNameRenderOpacity(float percentage);

	UFUNCTION(BlueprintImplementableEvent)
	void SetWeaponEnergyProgressBar(float Percent);
	UFUNCTION()
	void SetWeaponEnergyUIVisibility(bool IsVisible);

	UFUNCTION()
	void SetLeftWeaponTipUI(UTexture2D* texture);
	UFUNCTION()
	void SetRightWeaponTipUI(UTexture2D* texture);

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	
protected:
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar_Enemy;
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar_Teammate;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerName;

	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* WeaponTipCanvas;
	UPROPERTY(meta = (BindWidget))
	UImage* Tip_Left;
	UPROPERTY(meta = (BindWidget))
	UImage* Tip_Right;
	UPROPERTY(meta = (BindWidget))
	UImage* Tip_Left_Weapon;
	UPROPERTY(meta = (BindWidget))
	UImage* Tip_Right_Weapon;

	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* InGame_WeaponEnergyCanvasHolder;
	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* InGame_WeaponEnergyCanvas;
	UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
	UImage* InGame_WeaponEnergy;

public:
	AMCharacter* OwningPawn;
};
