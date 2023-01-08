// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PartyGameMultiplayerGameMode.generated.h"

UCLASS(minimalapi)
class APartyGameMultiplayerGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	APartyGameMultiplayerGameMode();
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
};



