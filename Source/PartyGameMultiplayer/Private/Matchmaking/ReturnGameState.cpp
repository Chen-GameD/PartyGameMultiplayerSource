// Fill out your copyright notice in the Description page of Project Settings.


#include "Matchmaking/ReturnGameState.h"

UReturnGameState::UReturnGameState()
{
	teamScore1 = 0;
	teamScore2 = 0;
	
}

void UReturnGameState::AddPlayerData(bool isTeam1, FString username, int32 kills, int32 deaths)
{
	if(isTeam1)
	{
		playerDataT1.Add(username, TArray<int>({kills, deaths}));
	}
	else
	{
		playerDataT2.Add(username, TArray<int>({kills, deaths}));
	}
}

void UReturnGameState::UpdatePlayerKills(bool isTeam1, FString username, int32 kills)
{
	if(isTeam1)
	{
		if(const auto seekVal = playerDataT1.Find(username))
		{
			(*seekVal)[0] = kills;
		}
		else
		{
			AddPlayerData(true, username, kills, 0);
		}
	}
	else
	{
		if(const auto seekVal = playerDataT2.Find(username))
		{
			(*seekVal)[0] = kills;
		}
		else
		{
			AddPlayerData(false, username, kills, 0);
		}
	}
}

void UReturnGameState::UpdatePlayerDeaths(bool isTeam1, FString username, int32 deaths)
{
	if(isTeam1)
	{
		if(const auto seekVal = playerDataT1.Find(username))
		{
			(*seekVal)[1] = deaths;
		}
		else
		{
			AddPlayerData(true, username, 0, deaths);
		}
	}
	else
	{
		if(const auto seekVal = playerDataT2.Find(username))
		{
			(*seekVal)[1] = deaths;
		}
		else
		{
			AddPlayerData(false, username, 0, deaths);
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
