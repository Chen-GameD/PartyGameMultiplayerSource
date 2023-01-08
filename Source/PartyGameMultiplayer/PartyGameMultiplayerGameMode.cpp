// Copyright Epic Games, Inc. All Rights Reserved.

#include "PartyGameMultiplayerGameMode.h"
#include "PartyGameMultiplayerCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "TestInfo.h"

APartyGameMultiplayerGameMode::APartyGameMultiplayerGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

void APartyGameMultiplayerGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	TestInfo::CntPlayerCreated = 0;
	TestInfo::SomeoneHasWeapon = false;
}
