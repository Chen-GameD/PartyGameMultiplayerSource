// Fill out your copyright notice in the Description page of Project Settings.


#include "M_PlayerState.h"

#include "Character/MPlayerController.h"
#include "GameBase/MGameMode.h"
#include "GameBase/MGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Matchmaking/EOSGameInstance.h"
#include "EngineUtils.h"

void AM_PlayerState::Server_UpdatePlayerName_Implementation(const FString& i_Name)
{
	PlayerNameString = i_Name;
	FString PlayerNameMessage = FString::Printf(TEXT("SetPlayerState Player Name to ")) + PlayerNameString;
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, PlayerNameMessage);

	if (GetNetMode() == NM_ListenServer)
	{
		OnRep_PlayerNameString();
	}
}

void AM_PlayerState::Server_UpdatePlayerReadyState_Implementation()
{
	if (TeamIndex != 0)
	{
		if (IsReady == true)
		{
			IsReady = false;
		}
		else
		{
			IsReady = true;
		}
	}

	if (GetNetMode() == NM_ListenServer)
	{
		OnRep_UpdateReadyInformation();
	}
}


void AM_PlayerState::Server_UpdateTeamIndex_Implementation(int i_TeamIndex)
{
	AMGameMode* MyGameMode = Cast<AMGameMode>(GetWorld()->GetAuthGameMode());
	int PreTeamIndex = TeamIndex;

	if (PreTeamIndex == i_TeamIndex)
		return;
	
	// Add to new team
	if (MyGameMode)
	{
		// on server
		if (i_TeamIndex == 1)
		{
			if (MyGameMode->TeamOnePlayerNum < MyGameMode->MaxTeamPlayers)
			{
				TeamIndex = i_TeamIndex;
				MyGameMode->TeamOnePlayerNum++;

				if (PreTeamIndex == 2)
				{
					MyGameMode->TeamTwoPlayerNum--;
				}
			}
		}
		else if (i_TeamIndex == 2)
		{
			if (MyGameMode->TeamTwoPlayerNum < MyGameMode->MaxTeamPlayers)
			{
				TeamIndex = i_TeamIndex;
				MyGameMode->TeamTwoPlayerNum++;
				
				if (PreTeamIndex == 1)
				{
					MyGameMode->TeamOnePlayerNum--;
				}
			}
		}
		else
		{
			// i_TeamIndex == 0
			TeamIndex = i_TeamIndex;
			PreTeamIndex == 1 ? MyGameMode->TeamOnePlayerNum-- : MyGameMode->TeamTwoPlayerNum--;
		}
	}
	
	if (GetNetMode() == NM_ListenServer && TeamIndex != PreTeamIndex)
	{
		OnRep_UpdateTeamIndex();
	}
}

void AM_PlayerState::Client_SetPlayerNameFromGameInstance_Implementation()
{
	FString TempPlayerName = Cast<UEOSGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()))->GetPlayerUsername();
	//PlayerNameString = Cast<UEOSGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()))->GetPlayerUsername();
	Server_UpdatePlayerName(TempPlayerName);
}

void AM_PlayerState::Client_SetPlayerSkinFromGameInstance_Implementation()
{
	UEOSGameInstance* MyGameInstance = Cast<UEOSGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	if (MyGameInstance)
	{
		Server_UpdatePlayerSkin(MyGameInstance->colorPicked, MyGameInstance->characterIndex);
	}
}

void AM_PlayerState::Server_UpdatePlayerSkin_Implementation(FLinearColor i_ColorPicked, int i_CharacterIndex)
{
	colorPicked = i_ColorPicked;
	characterIndex = i_CharacterIndex;

	if (GetNetMode() == NM_ListenServer)
	{
		OnRep_PlayerSkinInformation();
	}
}

void AM_PlayerState::OnRep_PlayerNameString()
{
	if (GetNetMode() == NM_ListenServer)
	{
		FString Message = FString::Printf(TEXT("OnRep_PlayerNameString: ListenServer"));
		GEngine->AddOnScreenDebugMessage(-1, 60.f, FColor::Yellow, Message);
	}
	else if (GetNetMode() == NM_Client)
	{
		FString Message = FString::Printf(TEXT("OnRep_PlayerNameString: Client"));
		GEngine->AddOnScreenDebugMessage(-1, 60.f, FColor::Yellow, Message);
	}
	
	AMCharacter* MyPawn = Cast<AMCharacter>(GetPawn());
	if (MyPawn)
	{
		MyPawn->SetPlayerNameUIInformation();
	}

	for (TActorIterator<AMPlayerController> ControllerItr(GetWorld()); ControllerItr; ++ControllerItr)
	{
		AMPlayerController* MyController = Cast<AMPlayerController>(*ControllerItr);
		if (MyController && MyController->IsLocalPlayerController())
		{
			MyController->Client_SyncLobbyInformation_Implementation();
		}
	}
}

void AM_PlayerState::OnRep_PlayerSkinInformation()
{
	AMCharacter* MyPawn = Cast<AMCharacter>(GetPawn());
	if (MyPawn)
	{
		MyPawn->SetPlayerSkin();
	}
}

void AM_PlayerState::OnRep_UpdateTeamIndex()
{
	AMPlayerController* MyLocalPlayerController = Cast<AMPlayerController>(GetWorld()->GetFirstPlayerController());
	if (MyLocalPlayerController)
	{
		MyLocalPlayerController->UI_UpdateLobbyInformation();
	}
}

void AM_PlayerState::OnRep_UpdateReadyInformation()
{
	AMPlayerController* MyLocalPlayerController = Cast<AMPlayerController>(GetWorld()->GetFirstPlayerController());
	if (MyLocalPlayerController)
	{
		MyLocalPlayerController->UI_UpdateLobbyInformation();
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
	DOREPLIFETIME(AM_PlayerState, colorPicked);
	DOREPLIFETIME(AM_PlayerState, characterIndex);
}
#pragma endregion Replicated Properties

void AM_PlayerState::addScore(float i_scoreToAdd) {
	SetScore(GetScore() + i_scoreToAdd);

	// This function only call on server
	AMGameState* MyGameState = Cast<AMGameState>(GetWorld()->GetGameState());
	if (MyGameState)
	{
		TeamIndex == 1 ? MyGameState->Team_1_Score += i_scoreToAdd : MyGameState->Team_2_Score += i_scoreToAdd;
		
		if (GetNetMode() == NM_ListenServer)
		{
			TeamIndex == 1 ? MyGameState->OnRep_Team_1_ScoreUpdate() : MyGameState->OnRep_Team_2_ScoreUpdate();
		}
	}
}

void AM_PlayerState::addKill(int i_killToAdd) {
	kill += i_killToAdd;
}

void AM_PlayerState::addDeath(int i_deathToAdd) {
	death += i_deathToAdd;
}
