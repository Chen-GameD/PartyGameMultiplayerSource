// Fill out your copyright notice in the Description page of Project Settings.


#include "Matchmaking/ReturnGameState.h"

UReturnGameState::UReturnGameState()
{
	teamScore1 = 0;
	teamScore2 = 0;
	
}

int32 UReturnGameState::GetTeamScore1()
{
	return teamScore1;
}

int32 UReturnGameState::GetTeamScore2()
{
	return teamScore2;
}

int32 UReturnGameState::GetTeam1PlayerCount()
{
	return playerDataT1.Num();
}

int32 UReturnGameState::GetTeam2PlayerCount()
{
	return playerDataT2.Num();
}

TArray<FString> UReturnGameState::GetTeam1PlayerData(int32 index)
{
	TArray<FString> keyList;
	playerDataT1.GenerateKeyArray(keyList);
	return TArray<FString>({keyList[index], FString::FromInt(playerDataT1[keyList[index]][0]), FString::FromInt(playerDataT1[keyList[index]][1]),
		FString::FromInt(playerDataT1[keyList[index]][2]), FString::FromInt(playerDataT1[keyList[index]][3])});
}

TArray<FString> UReturnGameState::GetTeam2PlayerData(int32 index)
{
	TArray<FString> keyList;
	playerDataT2.GenerateKeyArray(keyList);
	return TArray<FString>({keyList[index], FString::FromInt(playerDataT2[keyList[index]][0]), FString::FromInt(playerDataT2[keyList[index]][1]),
		FString::FromInt(playerDataT2[keyList[index]][2]), FString::FromInt(playerDataT2[keyList[index]][3])});
}

void UReturnGameState::AddPlayerData(bool isTeam1, FString username, int32 kills, int32 deaths, int32 score, int32 assists)
{
	if(isTeam1)
	{
		if(playerDataT1.Contains(username))
		{
			playerDataT1[username][0] = kills;
			playerDataT1[username][1] = deaths;
			playerDataT1[username][2] = score;
			playerDataT1[username][3] = assists;
		}
		else
		{
			playerDataT1.Add(username, TArray<int>({kills, deaths, score, assists}));
		}
	}
	else
	{
		if(playerDataT2.Contains(username))
		{
			playerDataT2[username][0] = kills;
			playerDataT2[username][1] = deaths;
			playerDataT2[username][2] = score;
			playerDataT2[username][3] = assists;
		}
		else
		{
			playerDataT2.Add(username, TArray<int>({kills, deaths, score, assists}));
		}
	}
}

void UReturnGameState::UpdateTeamScore(bool isTeam1, int32 score)
{
	if(isTeam1)
	{
		teamScore1 = score;
	}
	else
	{
		teamScore2 = score;
	}
}
