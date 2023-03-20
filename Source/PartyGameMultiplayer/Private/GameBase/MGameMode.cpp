// Fill out your copyright notice in the Description page of Project Settings.


#include "GameBase/MGameMode.h"

#include "EngineUtils.h"
#include "M_PlayerState.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Character/MPlayerController.h"
#include "GameBase/MGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Math/TransformCalculus3D.h"
#include "Character/MCharacter.h"
#include "Matchmaking/EOSGameInstance.h"
#include "Weapon/JsonFactory.h"

void AMGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	/*if (UJsonFactory::InitJsonObject_1())
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, TEXT("GameMode Init JsonObject_1 succeeded"));
	else
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, TEXT("GameMode Init JsonObject_1 failed"));*/

	//CurrentMinigameIndex = FMath::RandRange(0, MinigameDataAsset->MinigameConfigTable.Num() - 1);
}

void AMGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
}

void AMGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (NewPlayer)
	{
		if(Cast<UEOSGameInstance>(GetGameInstance())->GetIsLoggedIn())
		{
			FUniqueNetIdRepl UniqueNetIdRepl;
			if(NewPlayer->IsLocalController())
			{
				ULocalPlayer *LocalPlayer = NewPlayer->GetLocalPlayer();
				if(LocalPlayer)
				{
					UniqueNetIdRepl = LocalPlayer->GetPreferredUniqueNetId();
				}
				else
				{
					UNetConnection *NetConnectionRef = Cast<UNetConnection>(NewPlayer->Player);
					check(IsValid(NetConnectionRef));
					UniqueNetIdRepl = NetConnectionRef->PlayerId;
				}
			}
			else
			{
				UNetConnection *NetConnectionRef = Cast<UNetConnection>(NewPlayer->Player);
				check(IsValid(NetConnectionRef));
				UniqueNetIdRepl = NetConnectionRef->PlayerId;
			}
		
			TSharedPtr<const FUniqueNetId> UniqueNetId = UniqueNetIdRepl.GetUniqueNetId();
			if(UniqueNetId == nullptr)
				return;
			IOnlineSubsystem *OnlineSubsystemRef = Online::GetSubsystem(NewPlayer->GetWorld());
			IOnlineSessionPtr OnlineSessionRef = OnlineSubsystemRef->GetSessionInterface();
			bool bRegistrationSuccess = OnlineSessionRef->RegisterPlayer(FName("MAINSESSION"), *UniqueNetId, false);
			if(bRegistrationSuccess)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, TEXT("Success Registration"));
				UE_LOG(LogTemp, Warning, TEXT("Success registration: %d"), bRegistrationSuccess);
			}
		}
		
		AM_PlayerState* MyPlayerState = NewPlayer->GetPlayerState<AM_PlayerState>();
		MyPlayerState->SetPlayerNameFromUsername();
		CurrentPlayerNum++;

		if (NewPlayer->IsLocalPlayerController())
		{
			AMCharacter* MyPawn = Cast<AMCharacter>(NewPlayer->GetPawn());
			if (MyPawn)
			{
				MyPawn->OnRep_PlayerState();
			}
		}
	}
}

void AMGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
}

void AMGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	if (Exiting)
	{
		APlayerController* NewPlayer = Cast<APlayerController>(Exiting);
		if(Cast<UEOSGameInstance>(GetGameInstance())->GetIsLoggedIn())
		{
			FUniqueNetIdRepl UniqueNetIdRepl;
			if(NewPlayer->IsLocalController())
			{
				ULocalPlayer *LocalPlayer = NewPlayer->GetLocalPlayer();
				if(LocalPlayer)
				{
					UniqueNetIdRepl = LocalPlayer->GetPreferredUniqueNetId();
				}
				else
				{
					UNetConnection *NetConnectionRef = Cast<UNetConnection>(NewPlayer->Player);
					// check(IsValid(NetConnectionRef));
					// UniqueNetIdRepl = NetConnectionRef->PlayerId;
				}
			}
			else
			{
				UNetConnection *NetConnectionRef = Cast<UNetConnection>(NewPlayer->Player);
				// check(IsValid(NetConnectionRef));
				// UniqueNetIdRepl = NetConnectionRef->PlayerId;
			}
		
			TSharedPtr<const FUniqueNetId> UniqueNetId = UniqueNetIdRepl.GetUniqueNetId();
			if(UniqueNetId == nullptr)
				return;
			IOnlineSubsystem *OnlineSubsystemRef = Online::GetSubsystem(NewPlayer->GetWorld());
			IOnlineSessionPtr OnlineSessionRef = OnlineSubsystemRef->GetSessionInterface();
			bool bRegistrationSuccess = OnlineSessionRef->UnregisterPlayer(FName("MAINSESSION"), *UniqueNetId);
			if(bRegistrationSuccess)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, TEXT("Success UN-Registration"));
				UE_LOG(LogTemp, Warning, TEXT("Success UN-registration: %d"), bRegistrationSuccess);
			}
		}

		CurrentPlayerNum--;

		if (CurrentPlayerNum <= 0)
		{
			// Need to restart the server level
			//UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), false);
		}
	}
}

void AMGameMode::Server_RespawnPlayer_Implementation(APlayerController* PlayerController)
{
	if (IsValid(PlayerController))
	{
		FVector spawnLocation;
		FRotator spawnRotation;

		APlayerStart* spawnPlayerStart;
		if (PlayerController->GetPlayerState<AM_PlayerState>()->TeamIndex == 1)
		{
			int index = FMath::RandRange(0, Team_1_SpawnPoints.Num() - 1);
			spawnPlayerStart = Team_1_SpawnPoints[index];
		}
		else
		{
			int index = FMath::RandRange(0, Team_2_SpawnPoints.Num() - 1);
			spawnPlayerStart = Team_2_SpawnPoints[index];
		}
		
		spawnLocation = spawnPlayerStart->GetActorTransform().GetLocation();
		spawnRotation = spawnPlayerStart->GetActorTransform().GetRotation().Rotator();

		// Only Reset the location of the pawn and reset the status;
		AMCharacter* MyCharacter = Cast<AMCharacter>(PlayerController->GetPawn());
		if (MyCharacter)
		{
			MyCharacter->SetActorLocation(spawnLocation);
			MyCharacter->SetActorRotation(spawnRotation);
			MyCharacter->ResetCharacterStatus();

			// Temp for syn mesh every time when player respawn
			// After the customize function is finished
			// Need to delete, let the mesh only update once when the client join an session
			//Cast<AMPlayerController>(PlayerController)->NetMulticast_SynMesh();
		}
	}
}

void AMGameMode::Server_RespawnMinigameObject_Implementation()
{
	if (MinigameDataAsset)
	{
		CurrentMinigameIndex = FMath::RandRange(0, MinigameDataAsset->LevelMinigameConfigTable[LevelIndex].MinigameConfigTable.Num() - 1);
		FVector spawnLocation = MinigameObjectSpawnTransform.GetLocation();
		FRotator spawnRotation = MinigameObjectSpawnTransform.GetRotation().Rotator();
		AMinigameMainObjective* spawnActor = GetWorld()->SpawnActor<AMinigameMainObjective>(MinigameDataAsset->LevelMinigameConfigTable[LevelIndex].MinigameConfigTable[CurrentMinigameIndex].MinigameObject, spawnLocation, spawnRotation);
		if(spawnActor && MinigameDataAsset)
			spawnActor->UpdateScoreCanGet(MinigameDataAsset->LevelMinigameConfigTable[LevelIndex].MinigameConfigTable[CurrentMinigameIndex].ScoreCanGet);
		
		// Update Minigame Hint
		AMGameState* MyGameState = GetGameState<AMGameState>();
		if (MyGameState)
		{
			MyGameState->NetMulticast_UpdateMinigameHint(MinigameDataAsset->LevelMinigameConfigTable[LevelIndex].MinigameConfigTable[CurrentMinigameIndex].MinigameHint);
		}
	}
}

void AMGameMode::CheckGameStart()
{
	int const PlayerNum = UGameplayStatics::GetNumPlayerStates(GetWorld());
	bool CanStart = true;

	// Test
	//CurrentPlayerNum = 2;
	// Test
	
	if (TeamOnePlayerNum + TeamTwoPlayerNum == CurrentPlayerNum && CurrentPlayerNum == PlayerNum)
	{
		// All player join a team
		if (TeamOnePlayerNum == TeamTwoPlayerNum)
		{
			// Team one and team two have the same num of players
			// Check if they are ready
			for (int index = 0; index < CurrentPlayerNum; index++)
			{
				AM_PlayerState* currentPlayerState = Cast<AM_PlayerState>(UGameplayStatics::GetPlayerState(GetWorld(), index));
				if (!currentPlayerState->IsReady)
				{
					CanStart = false;
				}
			}

			if (CanStart)
			{
				// Can start the game
				GetWorldTimerManager().SetTimer(StartGameCountDownTimerHandle, this, &AMGameMode::StartTheGame, 0.5, false);
			}
		}
		else
		{
			CanStart = false;
		}
	}
	else
	{
		CanStart = false;
	}

	if (!CanStart)
	{
		GetWorldTimerManager().ClearTimer(StartGameCountDownTimerHandle);
	}

	NotifyAllClientPlayerControllerUpdateReadyState(CanStart);
}

void AMGameMode::NotifyAllClientPlayerControllerUpdateReadyState(bool IsAllReady)
{
	for (FConstPlayerControllerIterator iter = GetWorld()->GetPlayerControllerIterator(); iter; ++iter)
	{
		AMPlayerController* currentController = Cast<AMPlayerController>(*iter);

		currentController->GetNotifyPlayerControllerUpdateReadyState(IsAllReady);
	}
}

void AMGameMode::StartTheGame()
{
	AMGameState* MyGameState = GetGameState<AMGameState>();

	// Double check if the skin information set correctly.
	// Later probably will use some other way to do this.
	// Like do the Sync after everyone joined the session and then show the game level ( Add an interval level for this )
	// for (FConstPlayerControllerIterator iterator = GetWorld()->GetPlayerControllerIterator(); iterator; ++iterator)
	// {
	// 	AMPlayerController* controller = Cast<AMPlayerController>(*iterator);
	// 	
	// 	if (controller)
	// 	{
	// 		controller->NetMulticast_SynMesh();
	// 	}
	// }

	// In Here, system already created a pawn for all clients and server.
	// It's safe to update the player init information like skin and player name.
	// AMCharacter* MyPawn = Cast<AMCharacter>(NewPlayer->GetPawn());
	// if (MyPawn)
	// {
	// 	MyPawn->NetMulticast_InitNewPlayerInformation();
	// }
	// for (TActorIterator<AMCharacter> PawnItr(GetWorld()); PawnItr; ++PawnItr)
	// {
	// 	AMCharacter* CurrentPawn = *PawnItr;
	// 	if (CurrentPawn)
	// 	{
	// 		CurrentPawn->NetMulticast_InitNewPlayerInformation();
	// 	}
	// }
	
	if (MyGameState)
	{
		// Set the game time
		MyGameState->GameTime = MinigameDataAsset->LevelMinigameConfigTable[LevelIndex].GameTime;
		
		MyGameState->IsGameStart = true;
		MyGameState->Server_StartGame();
	}

	Server_RespawnMinigameObject();
}

void AMGameMode::TestRestartLevel()
{
	UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), false);
}
