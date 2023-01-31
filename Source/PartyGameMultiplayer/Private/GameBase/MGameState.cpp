// Fill out your copyright notice in the Description page of Project Settings.


#include "GameBase/MGameState.h"

#include "Character/MPlayerController.h"
#include "Net/UnrealNetwork.h"

void AMGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate
	DOREPLIFETIME(AMGameState, IsGameStart);
	DOREPLIFETIME(AMGameState, GameTime);
}

void AMGameState::SetClientStartGame_Implementation()
{
	AMPlayerController* MyLocalPlayerController = Cast<AMPlayerController>(GetWorld()->GetFirstPlayerController());
	if (IsGameStart)
	{
		if (MyLocalPlayerController)
		{
			MyLocalPlayerController->StartTheGame();
			MyLocalPlayerController->Client_SetGameUIVisibility(IsGameStart);
			MyLocalPlayerController->AddWeaponUI();
		}
	}
	else
	{
		if (MyLocalPlayerController)
		{
			MyLocalPlayerController->EndTheGame();
			MyLocalPlayerController->Client_SetGameUIVisibility(IsGameStart);
		}
	}
}

void AMGameState::UpdateGameStartTimerUI()
{
	AMPlayerController* MyLocalPlayerController = Cast<AMPlayerController>(GetWorld()->GetFirstPlayerController());
	if (MyLocalPlayerController)
	{
		MyLocalPlayerController->UI_UpdateGameTimer();
	}
}

void AMGameState::UpdateGameTime()
{
	GameTime--;

	FString TipInformation = FString::Printf(TEXT("Game time : %d"), GameTime);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TipInformation);

	if (GameTime <= 0)
	{
		// Game end
		IsGameStart = false;
		GetWorldTimerManager().ClearTimer(GameStartTimerHandle);
	}

#ifdef IS_LISTEN_SERVER
	UpdateGameStartTimerUI();
#endif
}

void AMGameState::StartGame_Implementation()
{
	FString TipInformation = FString::Printf(TEXT("Start game timer!"));
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TipInformation);
	
	GetWorldTimerManager().SetTimer(GameStartTimerHandle, this, &AMGameState::UpdateGameTime, 1, true);
	SetClientStartGame();
}
