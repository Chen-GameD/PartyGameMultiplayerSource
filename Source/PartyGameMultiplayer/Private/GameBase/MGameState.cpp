// Fill out your copyright notice in the Description page of Project Settings.


#include "GameBase/MGameState.h"
#include "../../Public/M_PlayerState.h"
#include "Character/MPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "../../Public/Character/MCharacter.h"
#include "../../../../../Engine/Source/Runtime/Engine/Classes/Components/PrimitiveComponent.h"

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
	
#pragma region Siloutte_Config

	// Get local player state and team id
	AM_PlayerState* MyPlayerState = Cast<AM_PlayerState>(MyLocalPlayerController->
		GetPawn()->GetPlayerState());
	auto myTeamID = MyPlayerState->TeamIndex;

	for (int i = 0; i < PlayerArray.Num(); i++) {
		// Cast to custom ps
		auto ps = Cast<AM_PlayerState>(PlayerArray[i]);
		auto character = Cast<AMCharacter>(ps->GetPawn());
		// On the same team as the local player
		if (ps->TeamIndex == myTeamID) {
			character->GetMesh()->SetCustomDepthStencilValue(252);
		}
		else {
			character->GetMesh()->SetCustomDepthStencilValue(0);
		}
	}

#pragma endregion Siloutte_Config

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
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TipInformation);

	if (GameTime <= 0)
	{
		// Game end
		IsGameStart = false;
		GetWorldTimerManager().ClearTimer(GameStartTimerHandle);
	}
	
	if (GetNetMode() == NM_ListenServer)
	{
		UpdateGameStartTimerUI();
	}
}

void AMGameState::StartGame_Implementation()
{
	FString TipInformation = FString::Printf(TEXT("Start game timer!"));
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TipInformation);
	
	GetWorldTimerManager().SetTimer(GameStartTimerHandle, this, &AMGameState::UpdateGameTime, 1, true);
	SetClientStartGame();
}
