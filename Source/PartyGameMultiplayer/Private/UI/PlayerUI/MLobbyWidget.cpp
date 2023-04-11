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
	// Team2
	Team2TextContainer.Add(0, Team_2_Player1);
	Team2TextContainer.Add(1, Team_2_Player2);
	Team2TextContainer.Add(2, Team_2_Player3);
	// Undecided
	UndecidedTextContainer.Add(0, Undecided_Player1);
	UndecidedTextContainer.Add(1, Undecided_Player2);
	UndecidedTextContainer.Add(2, Undecided_Player3);
	UndecidedTextContainer.Add(3, Undecided_Player4);
	UndecidedTextContainer.Add(4, Undecided_Player5);
	UndecidedTextContainer.Add(5, Undecided_Player6);
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
		UTextBlock* CurrentTextBlock = *Team1TextContainer.Find(index);
		if (CurrentTextBlock)
		{
			FLobbyInformationStruct CurrentInfo = i_TeamArr[index];
			FString TextInfo = CurrentInfo.PlayerName + (CurrentInfo.IsReady ? " is Ready!" : " is Not Ready");
			CurrentTextBlock->SetText(FText::FromString(TextInfo));
		}
	}
	// Set the rest
	for (int rest = index; rest < Team1TextContainer.Num(); rest++)
	{
		UTextBlock* CurrentTextBlock = *Team1TextContainer.Find(rest);
		if (CurrentTextBlock)
		{
			FString TextInfo = "Waiting for player...";
			CurrentTextBlock->SetText(FText::FromString(TextInfo));
		}
	}
}

void UMLobbyWidget::UpdateTeam2LobbyInformation(TArray<FLobbyInformationStruct> i_TeamArr)
{
	int index;
	// Set information for each player in this team;
	for (index = 0; index < i_TeamArr.Num(); index++)
	{
		UTextBlock* CurrentTextBlock = *Team2TextContainer.Find(index);
		if (CurrentTextBlock)
		{
			FLobbyInformationStruct CurrentInfo = i_TeamArr[index];
			FString TextInfo = CurrentInfo.PlayerName + (CurrentInfo.IsReady ? " is Ready!" : " is Not Ready");
			CurrentTextBlock->SetText(FText::FromString(TextInfo));
		}
	}
	// Set the rest
	for (int rest = index; rest < Team2TextContainer.Num(); rest++)
	{
		UTextBlock* CurrentTextBlock = *Team2TextContainer.Find(rest);
		if (CurrentTextBlock)
		{
			FString TextInfo = "Waiting for player...";
			CurrentTextBlock->SetText(FText::FromString(TextInfo));
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
		if (CurrentTextBlock)
		{
			FLobbyInformationStruct CurrentInfo = i_TeamArr[index];
			FString TextInfo = CurrentInfo.PlayerName + " boarded!";
			CurrentTextBlock->SetText(FText::FromString(TextInfo));
		}
	}
	// Set the rest
	for (int rest = index; rest < UndecidedTextContainer.Num(); rest++)
	{
		UTextBlock* CurrentTextBlock = *UndecidedTextContainer.Find(rest);
		if (CurrentTextBlock)
		{
			FString TextInfo = "";
			CurrentTextBlock->SetText(FText::FromString(TextInfo));
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
