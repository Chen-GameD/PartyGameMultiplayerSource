// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ReturnGameState.generated.h"

/**
 * 
 */
UCLASS()
class PARTYGAMEMULTIPLAYER_API UReturnGameState : public UObject
{
	GENERATED_BODY()

	//Team1 data - name, kills, deaths
	TMap<FString, TArray<int32>> playerDataT1;
	int32 teamScore1;

	//Team1 data - name, kills, deaths
	TMap<FString, TArray<int32>> playerDataT2;
	int32 teamScore2;
	
public:	
	UReturnGameState();

	UPROPERTY(BlueprintReadWrite)
	bool isReturningFromGame = false;
	UPROPERTY(BlueprintReadWrite)
	bool isReturningFromDisconnect = false;
	
	UFUNCTION(BlueprintCallable)
	void AddPlayerData(bool isTeam1, FString username, int32 kills, int32 deaths);
	
	void UpdatePlayerKills(bool isTeam1, FString username, int32 kills);
	void UpdatePlayerDeaths(bool isTeam1, FString username, int32 deaths);
	void UpdateTeamScore(bool isTeam1, int32 score);
};
