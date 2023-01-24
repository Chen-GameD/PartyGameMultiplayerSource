// Fill out your copyright notice in the Description page of Project Settings.


#include "M_PlayerState.h"

#include "Character/MPlayerController.h"
#include "GameBase/MGameMode.h"

void AM_PlayerState::UpdatePlayerName_Implementation(const FString& i_Name)
{
	PlayerNameString = i_Name;
	FString PlayerNameMessage = FString::Printf(TEXT("SetPlayerState Player Name to ")) + PlayerNameString;
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, PlayerNameMessage);
}

void AM_PlayerState::UpdateTeamIndex_Implementation(int i_TeamIndex)
{
	AMGameMode* MyGameMode = Cast<AMGameMode>(GetWorld()->GetAuthGameMode());
	// Quit from current team
	if (MyGameMode)
	{
		if (TeamIndex != 0)
		{
			if (TeamIndex == 1)
			{
				MyGameMode->TeamOnePlayerNum--;
			}
			else
			{
				MyGameMode->TeamTwoPlayerNum--;
			}
		}
	}
	
	TeamIndex = i_TeamIndex == 0 ? 1 : i_TeamIndex;
	
	// Add to new team
	if (MyGameMode)
	{
		// on server
		if (TeamIndex == 1)
		{
			if (MyGameMode->TeamOnePlayerNum < MyGameMode->MaxTeamPlayers)
			{
				MyGameMode->TeamOnePlayerNum++;
			}
			else
			{
				TeamIndex = 2;
				MyGameMode->TeamTwoPlayerNum++;
			}
		}
		else
		{
			if (MyGameMode->TeamTwoPlayerNum < MyGameMode->MaxTeamPlayers)
			{
				MyGameMode->TeamTwoPlayerNum++;
			}
			else
			{
				TeamIndex = 1;
				MyGameMode->TeamOnePlayerNum++;
			}
		}
	}

	//UpdateLobbyUIInformation();
}

void AM_PlayerState::UpdateLobbyUIInformation()
{
	//Cast<AMPlayerController>(GetWorld()->GetFirstPlayerController());
	AMPlayerController* MyLocalPlayerController = Cast<AMPlayerController>(GetWorld()->GetFirstPlayerController());
	if (MyLocalPlayerController)
	{
		MyLocalPlayerController->UI_UpdateLobbyMenu();
	}
}

AM_PlayerState::AM_PlayerState()
{
}

// Replicated Properties
// =======================================================
#pragma region Replicated Properties
void AM_PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Replicate current health
	DOREPLIFETIME(AM_PlayerState, kill);
	DOREPLIFETIME(AM_PlayerState, death);
	DOREPLIFETIME(AM_PlayerState, PlayerNameString);
	DOREPLIFETIME(AM_PlayerState, TeamIndex);
	DOREPLIFETIME(AM_PlayerState, IsReady);
}
#pragma endregion Replicated Properties

void AM_PlayerState::addScore(float i_scoreToAdd) {
	SetScore(GetScore() + i_scoreToAdd);
}

void AM_PlayerState::addKill(int i_killToAdd) {
	kill += i_killToAdd;
}

void AM_PlayerState::addDeath(int i_deathToAdd) {
	death += i_deathToAdd;
}