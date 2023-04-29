// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerUI/OpponentMarkerWidget.h"

UOpponentMarkerWidget::UOpponentMarkerWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	TeamID = 0;

	// No use because these images may have not been assinged yet
	//Markers.Add(Marker1);
	//Markers.Add(Marker2);
	//Markers.Add(Marker3);
}


void UOpponentMarkerWidget::ResetMarkers()
{
	Markers.Empty();
	if (TeamID == 2)
	{
		Markers.Add(RedMarker1);
		Markers.Add(RedMarker2);
		Markers.Add(RedMarker3);
	}	
	else if (TeamID == 1)
	{
		Markers.Add(BlueMarker1);
		Markers.Add(BlueMarker2);
		Markers.Add(BlueMarker3);
	}
}

void UOpponentMarkerWidget::SetMarkerVisibility(int id, bool bVisible)
{
	if (Markers.Num() != 3)
		ResetMarkers();
	if (Markers.Num() != 3)
		return;

	if (0 <= id && id < Markers.Num() && Markers[id])
	{
		Markers[id]->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
		
}

void UOpponentMarkerWidget::SetAllMarkerVisibility(bool bVisible)
{
	RedMarker1->SetVisibility(ESlateVisibility::Hidden);
	RedMarker2->SetVisibility(ESlateVisibility::Hidden);
	RedMarker3->SetVisibility(ESlateVisibility::Hidden);
	BlueMarker1->SetVisibility(ESlateVisibility::Hidden);
	BlueMarker2->SetVisibility(ESlateVisibility::Hidden);
	BlueMarker3->SetVisibility(ESlateVisibility::Hidden);
}

void UOpponentMarkerWidget::SetMarkerTranslation(int id, FVector2D Tran)
{
	if (Markers.Num() != 3)
		ResetMarkers();
	if (Markers.Num() != 3)
		return;

	if (0 <= id && id < Markers.Num() && Markers[id])
	{
		FWidgetTransform transform = Markers[id]->RenderTransform;
		transform.Translation = Tran;
		Markers[id]->SetRenderTransform(transform);
	}
}

void UOpponentMarkerWidget::SetMarkerTranslation(int id, float TranX, float TranY)
{
	SetMarkerTranslation(id, FVector2D(TranX, TranY));
}

void UOpponentMarkerWidget::SetMarkerOffsetAngle(int id, float OffsetAngle_InDegree)
{
	if (Markers.Num() != 3)
		ResetMarkers();
	if (Markers.Num() != 3)
		return;

	if (0 <= id && id < Markers.Num() && Markers[id])
	{
		FWidgetTransform transform = Markers[id]->RenderTransform;
		transform.Angle = -90.0f + OffsetAngle_InDegree;
		Markers[id]->SetRenderTransform(transform);
	}
}