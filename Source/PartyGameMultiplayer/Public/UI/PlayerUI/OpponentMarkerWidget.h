// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "OpponentMarkerWidget.generated.h"

/**
 * 
 */
UCLASS()
class PARTYGAMEMULTIPLAYER_API UOpponentMarkerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UOpponentMarkerWidget(const FObjectInitializer& ObjectInitializer);

	void SetMarkerVisibility(int id, bool bVisible);
	void SetAllMarkerVisibility(bool bVisible);
	void SetMarkerTranslation(int id, FVector2D Tran);
	void SetMarkerTranslation(int id, float TranX, float TranY);
	void SetMarkerOffsetAngle(int id, float OffsetAngle_InDegree);
protected:
	void ResetMarkers();

public:
	int TeamID;
protected:
	UPROPERTY(meta = (BindWidget))
		UImage* RedMarker1;
	UPROPERTY(meta = (BindWidget))
		UImage* RedMarker2;
	UPROPERTY(meta = (BindWidget))
		UImage* RedMarker3;
	UPROPERTY(meta = (BindWidget))
		UImage* BlueMarker1;
	UPROPERTY(meta = (BindWidget))
		UImage* BlueMarker2;
	UPROPERTY(meta = (BindWidget))
		UImage* BlueMarker3;
	TArray<UImage*> Markers;
};
