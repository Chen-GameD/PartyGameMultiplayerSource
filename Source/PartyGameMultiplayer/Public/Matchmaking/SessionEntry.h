// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SessionEntry.generated.h"

/**
 * 
 */
UCLASS()
class PARTYGAMEMULTIPLAYER_API USessionEntry : public UObject
{
	GENERATED_BODY()
	
	FString sessionID;
	int32 maxPlayers;
	int32 minPlayers;

public:
	USessionEntry();

	void SetSessionData(FString id, int32 max, int32 min);

	UFUNCTION(BlueprintCallable)
	FString GetSessionID();
	UFUNCTION(BlueprintCallable)
	int32 GetMaxPlayers();
	UFUNCTION(BlueprintCallable)
	int32 GetMinPlayers();
};
