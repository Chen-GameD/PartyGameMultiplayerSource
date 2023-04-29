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

	//Team1 data - name, kills, deaths, score
	TMap<FString, TArray<int32>> playerDataT1;
	int32 teamScore1;

	//Team1 data - name, kills, deaths, score
	TMap<FString, TArray<int32>> playerDataT2;
	int32 teamScore2;
	
public:	
	UReturnGameState();

	UPROPERTY(BlueprintReadWrite)
	bool isReturningFromGame = false;
	UPROPERTY(BlueprintReadWrite)
	bool isReturningFromDisconnect = false;
	
	UFUNCTION(BlueprintCallable)
	int32 GetTeamScore1();
	
	UFUNCTION(BlueprintCallable)
	int32 GetTeamScore2();

	UFUNCTION(BlueprintCallable)
	int32 GetTeam1PlayerCount();
	
	UFUNCTION(BlueprintCallable)
	int32 GetTeam2PlayerCount();

	/**
	 * @brief 
	 * @return
	 * Returns array of strings in order of name, kills, deaths, score, assists
	 */
	UFUNCTION(BlueprintCallable)
	TArray<FString> GetTeam1PlayerData(int32 index);
	/**
	 * @brief 
	 * @return
	 * Returns array of strings in order of name, kills, deaths, score, assists
	 */
	UFUNCTION(BlueprintCallable)
	TArray<FString> GetTeam2PlayerData(int32 index);
	
	void AddPlayerData(bool isTeam1, FString username, int32 kills, int32 deaths, int32 score, int32 assists);
	
	void UpdateTeamScore(bool isTeam1, int32 score);
};
