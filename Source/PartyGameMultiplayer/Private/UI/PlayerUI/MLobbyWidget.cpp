// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerUI/MLobbyWidget.h"

UMLobbyWidget::UMLobbyWidget(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{
}

void UMLobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Initialize container
	// Team1
	Team1TextContainer.Add(0, Team_1_Player1);
	Team1TextContainer.Add(1, Team_1_Player2);
	Team1TextContainer.Add(2, Team_1_Player3);
	Team1HostImageContainer.Add(0, Team_1_HostImage1);
	Team1HostImageContainer.Add(1, Team_1_HostImage2);
	Team1HostImageContainer.Add(2, Team_1_HostImage3);
	// Team2
	Team2TextContainer.Add(0, Team_2_Player1);
	Team2TextContainer.Add(1, Team_2_Player2);
	Team2TextContainer.Add(2, Team_2_Player3);
	Team2HostImageContainer.Add(0, Team_2_HostImage1);
	Team2HostImageContainer.Add(1, Team_2_HostImage2);
	Team2HostImageContainer.Add(2, Team_2_HostImage3);
	// Undecided
	UndecidedTextContainer.Add(0, Undecided_Player1);
	UndecidedTextContainer.Add(1, Undecided_Player2);
	UndecidedTextContainer.Add(2, Undecided_Player3);
	UndecidedTextContainer.Add(3, Undecided_Player4);
	UndecidedTextContainer.Add(4, Undecided_Player5);
	UndecidedTextContainer.Add(5, Undecided_Player6);
	UndecidedHostImageContainer.Add(0, Undecided_HostImage1);
	UndecidedHostImageContainer.Add(1, Undecided_HostImage2);
	UndecidedHostImageContainer.Add(2, Undecided_HostImage3);
	UndecidedHostImageContainer.Add(3, Undecided_HostImage4);
	UndecidedHostImageContainer.Add(4, Undecided_HostImage5);
	UndecidedHostImageContainer.Add(5, Undecided_HostImage6);
}

void UMLobbyWidget::UpdateLobbyInformation(TArray<FLobbyInformationStruct> i_Team1Arr, TArray<FLobbyInformationStruct> i_Team2Arr, TArray<FLobbyInformationStruct> i_UndecidedArr)
{
	UpdateTeam1LobbyInformation(i_Team1Arr);
	UpdateTeam2LobbyInformation(i_Team2Arr);
	UpdateUndecidedLobbyInformation(i_UndecidedArr);
}

void UMLobbyWidget::UpdateTeam1LobbyInformation(TArray<FLobbyInformationStruct> i_TeamArr)
{
	int index;
	// Set information for each player in this team;
	for (index = 0; index < i_TeamArr.Num(); index++)
	{
		URichTextBlock* CurrentTextBlock = *Team1TextContainer.Find(index);
		UImage* CurrentHostImage = *Team1HostImageContainer.Find(index);
		if (CurrentTextBlock && CurrentHostImage)
		{
			FLobbyInformationStruct CurrentInfo = i_TeamArr[index];
			FString TextInfo = CurrentInfo.PlayerName + (CurrentInfo.IsReady ? " is <Ready>Ready!</>" : " is <NotReady>Not Ready</>");
			CurrentTextBlock->SetText(FText::FromString(TextInfo));
			if (CurrentInfo.IsHost)
			{
				CurrentHostImage->SetVisibility(ESlateVisibility::Visible);
			}
			else
			{
				CurrentHostImage->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}
	// Set the rest
	for (int rest = index; rest < Team1TextContainer.Num(); rest++)
	{
		URichTextBlock* CurrentTextBlock = *Team1TextContainer.Find(rest);
		UImage* CurrentHostImage = *Team1HostImageContainer.Find(index);
		if (CurrentTextBlock && CurrentHostImage)
		{
			FString TextInfo = "Waiting for player...";
			CurrentTextBlock->SetText(FText::FromString(TextInfo));
			CurrentHostImage->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void UMLobbyWidget::UpdateTeam2LobbyInformation(TArray<FLobbyInformationStruct> i_TeamArr)
{
	int index;
	// Set information for each player in this team;
	for (index = 0; index < i_TeamArr.Num(); index++)
	{
		URichTextBlock* CurrentTextBlock = *Team2TextContainer.Find(index);
		UImage* CurrentHostImage = *Team2HostImageContainer.Find(index);
		if (CurrentTextBlock && CurrentHostImage)
		{
			FLobbyInformationStruct CurrentInfo = i_TeamArr[index];
			FString TextInfo = CurrentInfo.PlayerName + (CurrentInfo.IsReady ? " is <Ready>Ready!</>" : " is <NotReady>Not Ready</>");
			CurrentTextBlock->SetText(FText::FromString(TextInfo));
			if (CurrentInfo.IsHost)
			{
				CurrentHostImage->SetVisibility(ESlateVisibility::Visible);
			}
			else
			{
				CurrentHostImage->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}
	// Set the rest
	for (int rest = index; rest < Team2TextContainer.Num(); rest++)
	{
		URichTextBlock* CurrentTextBlock = *Team2TextContainer.Find(rest);
		UImage* CurrentHostImage = *Team2HostImageContainer.Find(index);
		if (CurrentTextBlock && CurrentHostImage)
		{
			FString TextInfo = "Waiting for player...";
			CurrentTextBlock->SetText(FText::FromString(TextInfo));
			CurrentHostImage->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void UMLobbyWidget::UpdateUndecidedLobbyInformation(TArray<FLobbyInformationStruct> i_TeamArr)
{
	int index;
	// Set information for each player in this team;
	for (index = 0; index < i_TeamArr.Num(); index++)
	{
		UTextBlock* CurrentTextBlock = *UndecidedTextContainer.Find(index);
		UImage* CurrentHostImage = *UndecidedHostImageContainer.Find(index);
		if (CurrentTextBlock && CurrentHostImage)
		{
			FLobbyInformationStruct CurrentInfo = i_TeamArr[index];
			FString TextInfo = CurrentInfo.PlayerName + " boarded!";
			CurrentTextBlock->SetText(FText::FromString(TextInfo));
			if (CurrentInfo.IsHost && CurrentHostImage)
			{
				CurrentHostImage->SetVisibility(ESlateVisibility::Visible);
			}
			else
			{
				CurrentHostImage->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}
	// Set the rest
	for (int rest = index; rest < UndecidedTextContainer.Num(); rest++)
	{
		UTextBlock* CurrentTextBlock = *UndecidedTextContainer.Find(rest);
		UImage* CurrentHostImage = *UndecidedHostImageContainer.Find(index);
		if (CurrentTextBlock)
		{
			FString TextInfo = "";
			CurrentTextBlock->SetText(FText::FromString(TextInfo));
			CurrentHostImage->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void UMLobbyWidget::UpdateReadyButtonState(bool isReady)
{
	if (isReady)
	{
		Btn_Cancel->SetVisibility(ESlateVisibility::Visible);
		Btn_Ready->SetVisibility(ESlateVisibility::Hidden);
	}
	else
	{
		Btn_Cancel->SetVisibility(ESlateVisibility::Hidden);
		Btn_Ready->SetVisibility(ESlateVisibility::Visible);
	}
}

void UMLobbyWidget::UpdateEqualConditionState(bool isEqual)
{
	IM_EqualCondition->SetVisibility(isEqual ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void UMLobbyWidget::UpdateReadyConditionState(bool isReady)
{
	IM_ReadyCondition->SetVisibility(isReady ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}
