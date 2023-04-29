// Fill out your copyright notice in the Description page of Project Settings.


#include "Matchmaking/SessionEntry.h"

USessionEntry::USessionEntry()
{
	sessionID = FString();
	maxPlayers = 0;
	minPlayers = 0;
	isPrivateSession = false;
}

void USessionEntry::SetSessionData(FString id, int32 max, int32 min, bool isPrivate)
{
	sessionID = id;
	maxPlayers = max;
	minPlayers = min;
	isPrivateSession = isPrivate;
}

FString USessionEntry::GetSessionID()
{
	return sessionID;
}

int32 USessionEntry::GetMaxPlayers()
{
	return maxPlayers;
}

int32 USessionEntry::GetMinPlayers()
{
	return minPlayers;
}

bool USessionEntry::IsSessionPrivate()
{
	return isPrivateSession;
}
