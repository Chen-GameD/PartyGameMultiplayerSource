// Fill out your copyright notice in the Description page of Project Settings.


#include "GameBase/MGameState.h"
#include "../../Public/M_PlayerState.h"
#include "Character/MPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "../../Public/Character/MCharacter.h"
#include "../../../../../Engine/Source/Runtime/Engine/Classes/Components/PrimitiveComponent.h"
#include "EngineUtils.h"

bool AMGameState::HasBegunPlay() const
{
	return Super::HasBegunPlay();
}

void AMGameState::BeginPlay()
{
	Super::BeginPlay();

	Team_1_Score = 0;
	Team_2_Score = 0;
	
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUObject(this, &AMGameState::GameHasBeenPlayed);
	GetWorldTimerManager().SetTimer(HasBeenPlayedTimerHandle, TimerDelegate, 1, true);
}

void AMGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);

	for (TActorIterator<AMPlayerController> ControllerItr(GetWorld()); ControllerItr; ++ControllerItr)
	{
		if (*ControllerItr && ControllerItr->IsLocalPlayerController())
		{
			ControllerItr->Client_SyncLobbyInformation_Implementation();
		}
	}
}

void AMGameState::GameHasBeenPlayed()
{
	if (HasBegunPlay())
	{
		GetWorldTimerManager().ClearTimer(HasBeenPlayedTimerHandle);
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::White, TEXT("HasBeenPlayed!"));
		// Start Sync
		Server_StartSyncForNewPlayer();
	}
}

void AMGameState::Server_StartSyncForNewPlayer_Implementation()
{
	for (TActorIterator<AMPlayerController> ControllerItr(GetWorld()); ControllerItr; ++ControllerItr)
	{
		AMPlayerController* MyController = Cast<AMPlayerController>(*ControllerItr);
		if (MyController)
		{
			MyController->Client_SyncLobbyInformation();
			MyController->Client_SyncCharacters();
		}
	}
}

void AMGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate
	DOREPLIFETIME(AMGameState, IsGameStart);
	DOREPLIFETIME(AMGameState, GameTime);
	DOREPLIFETIME(AMGameState, Team_1_Score);
	DOREPLIFETIME(AMGameState, Team_2_Score);
	DOREPLIFETIME(AMGameState, LevelIndex);
}

void AMGameState::OnRep_IsGameStart()
{
	AMPlayerController* MyLocalPlayerController = Cast<AMPlayerController>(GetWorld()->GetFirstPlayerController());
	
	// Assign character's Teammates and Opponents
	TSubclassOf<AMCharacter> ActorClass = AMCharacter::StaticClass();
	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ActorClass, OutActors);
	for (size_t i = 0; i < OutActors.Num(); i++)
	{
		auto Chr = Cast<AMCharacter>(OutActors[i]);
		Chr->Teammates.Empty();
		Chr->Opponents.Empty();
	}
	for (size_t i = 0; i < OutActors.Num(); i++)
	{
		for (size_t j = i+1; j < OutActors.Num(); j++)
		{
			auto Chr1 = Cast<AMCharacter>(OutActors[i]);
			auto Chr2 = Cast<AMCharacter>(OutActors[j]);
			if (!Chr1 || !Chr2)
				continue;
			auto Ps1 = Chr1->GetPlayerState<AM_PlayerState>();
			auto Ps2 = Chr2->GetPlayerState<AM_PlayerState>();
			if (!Ps1 || !Ps2)
				continue;
			if (Ps1->TeamIndex == Ps2->TeamIndex)
			{
				Chr1->Teammates.Add(Chr2);
				Chr2->Teammates.Add(Chr1);
			}
			else
			{
				Chr1->Opponents.Add(Chr2);
				Chr2->Opponents.Add(Chr1);
			}
		}
	}	

#pragma region Siloutte_Config

	//// Get local player state and team id
	//AM_PlayerState* MyPlayerState = Cast<AM_PlayerState>(MyLocalPlayerController->
	//	GetPawn()->GetPlayerState());
	//auto myTeamID = MyPlayerState->TeamIndex;
	//for (int i = 0; i < PlayerArray.Num(); i++) {
	//	// Cast to custom ps
	//	auto ps = Cast<AM_PlayerState>(PlayerArray[i]);
	//	auto character = Cast<AMCharacter>(ps->GetPawn());
	//	// On the same team as the local player
	//	if (ps->TeamIndex == myTeamID) 
	//	{
	//		character->GetMesh()->SetCustomDepthStencilValue(252);
	//	}
	//	else 
	//	{
	//		character->GetMesh()->SetCustomDepthStencilValue(0);
	//	}

	//	if (ps->PlayerNameString == MyPlayerState->PlayerNameString)
	//		character->GetMesh()->SetRenderCustomDepth(false);
	//}

#pragma endregion Siloutte_Config

	if (IsGameStart)
	{
		if (MyLocalPlayerController)
		{
			if(const auto gameInstance = Cast<UEOSGameInstance>(GetGameInstance()))
			{
				gameInstance->SaveEndGameState();
			}
			MyLocalPlayerController->StartTheGame();
			MyLocalPlayerController->AddWeaponUI();
			BPF_GameStartBGM(true);
		}
	}
	else
	{
		if (MyLocalPlayerController)
		{
			if(const auto gameInstance = Cast<UEOSGameInstance>(GetGameInstance()))
			{
				gameInstance->SaveEndGameState();
			}
			MyLocalPlayerController->EndTheGame();
			BPF_GameStartBGM(false);
		}
	}
}

void AMGameState::UpdateGameStartTimerUI()
{
	AMPlayerController* MyLocalPlayerController = Cast<AMPlayerController>(GetWorld()->GetFirstPlayerController());
	if (MyLocalPlayerController)
	{
		MyLocalPlayerController->GetInGameHUD()->InGame_UpdateTimer(GameTime);
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
		if (GetNetMode() == NM_ListenServer)
		{
			OnRep_IsGameStart();
		}
		GetWorldTimerManager().ClearTimer(GameStartTimerHandle);
	}
	
	if (GetNetMode() == NM_ListenServer)
	{
		UpdateGameStartTimerUI();
	}
}

void AMGameState::UpdateTutorialGameTimer()
{
	GameTime++;

	FString TipInformation = FString::Printf(TEXT("Game time : %d"), GameTime);
	
	if (GetNetMode() == NM_ListenServer)
	{
		UpdateGameStartTimerUI();
	}
}

void AMGameState::OnRep_Team_1_ScoreUpdate()
{
	for (FConstPlayerControllerIterator iter = GetWorld()->GetPlayerControllerIterator(); iter; ++iter)
	{
		AMPlayerController* currentController = Cast<AMPlayerController>(*iter);

		if (currentController->IsLocalPlayerController())
		{
			AMInGameHUD* MyHUD = currentController->GetInGameHUD();
			if (MyHUD)
			{
				MyHUD->InGame_UpdateTeamScore(1, Team_1_Score);
			}
		}
	}
	if(const auto gameInstance = Cast<UEOSGameInstance>(GetGameInstance()))
	{
		gameInstance->SaveEndGameState();
	}
}

void AMGameState::OnRep_Team_2_ScoreUpdate()
{
	for (FConstPlayerControllerIterator iter = GetWorld()->GetPlayerControllerIterator(); iter; ++iter)
	{
		AMPlayerController* currentController = Cast<AMPlayerController>(*iter);

		if (currentController->IsLocalPlayerController())
		{
			AMInGameHUD* MyHUD = currentController->GetInGameHUD();
			if (MyHUD)
			{
				MyHUD->InGame_UpdateTeamScore(2, Team_2_Score);
			}
		}
	}
	if(const auto gameInstance = Cast<UEOSGameInstance>(GetGameInstance()))
	{
		gameInstance->SaveEndGameState();
	}
}

void AMGameState::NetMulticast_UpdateMinigameHint_Implementation(const FString& i_Hint, UTexture2D* i_HintImage)
{
	for (FConstPlayerControllerIterator iter = GetWorld()->GetPlayerControllerIterator(); iter; ++iter)
	{
		AMPlayerController* currentController = Cast<AMPlayerController>(*iter);

		if (currentController->IsLocalPlayerController())
		{
			AMInGameHUD* MyHUD = currentController->GetInGameHUD();
			if (MyHUD)
			{
				MyHUD->InGame_UpdateMinigameHint(i_Hint, i_HintImage);
			}
		}
	}
}

void AMGameState::Server_StartGame_Implementation()
{
	FString TipInformation = FString::Printf(TEXT("Start game timer!"));
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TipInformation);
	
	GetWorldTimerManager().SetTimer(GameStartTimerHandle, this, &AMGameState::UpdateGameTime, 1, true);
	OnRep_IsGameStart();
}


void AMGameState::Server_StartTutorialGame_Implementation()
{
	FString TipInformation = FString::Printf(TEXT("Start Tutorial timer!"));
	
	GetWorldTimerManager().SetTimer(GameStartTimerHandle, this, &AMGameState::UpdateTutorialGameTimer, 1, true);
	OnRep_IsGameStart();
}
