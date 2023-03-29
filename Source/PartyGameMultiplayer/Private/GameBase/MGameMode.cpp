// Fill out your copyright notice in the Description page of Project Settings.


#include "GameBase/MGameMode.h"

#include <process.h>

#include "EngineUtils.h"
#include "M_PlayerState.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Character/MPlayerController.h"
#include "GameBase/MGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Math/TransformCalculus3D.h"
#include "Character/MCharacter.h"
#include "LevelInteraction/MinigameObject/MinigameChild/MinigameChild_Statue_Shell.h"
#include "Matchmaking/EOSGameInstance.h"
#include "Weapon/JsonFactory.h"
#include "Weapon/ElementWeapon/WeaponShell.h"

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
		// Spawn the minigame object
		CurrentMinigameIndex = FMath::RandRange(0, MinigameDataAsset->LevelMinigameConfigTable[LevelIndex].MinigameConfigTable.Num() - 1);
		FTransform spawnTransform = MinigameDataAsset->LevelMinigameConfigTable[LevelIndex].MinigameConfigTable[CurrentMinigameIndex].MinigameObjectSpawnTransform;
		AMinigameMainObjective* spawnActor = GetWorld()->SpawnActor<AMinigameMainObjective>(MinigameDataAsset->LevelMinigameConfigTable[LevelIndex].MinigameConfigTable[CurrentMinigameIndex].MinigameObject, spawnTransform);
		if(spawnActor && MinigameDataAsset)
			spawnActor->UpdateScoreCanGet(MinigameDataAsset->LevelMinigameConfigTable[LevelIndex].MinigameConfigTable[CurrentMinigameIndex].ScoreCanGet);

		// Special rules
		switch (MinigameDataAsset->LevelMinigameConfigTable[LevelIndex].MinigameConfigTable[CurrentMinigameIndex].MinigameType)
		{
		case EMinigameTypeEnum::SculptureType:
			InitMinigame_ShellObject();
			break;
		case EMinigameTypeEnum::DefaultType:
		default:
			break;
		}
		
		// Update Minigame Hint
		AMGameState* MyGameState = GetGameState<AMGameState>();
		if (MyGameState)
		{
			MyGameState->NetMulticast_UpdateMinigameHint(MinigameDataAsset->LevelMinigameConfigTable[LevelIndex].MinigameConfigTable[CurrentMinigameIndex].MinigameHint, MinigameDataAsset->LevelMinigameConfigTable[LevelIndex].MinigameConfigTable[CurrentMinigameIndex].MinigameHintImage);
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

void AMGameMode::InitMinigame_ShellObject()
{
	// Delete all MinigameChild_Statue_Shell actor
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMinigameChild_Statue_Shell::StaticClass(), FoundActors);
	for(int i = 0; i < FoundActors.Num(); i++)
	{
		FoundActors[i]->Destroy();
	}
	
	if (!IsGameInitialized)
	{
		// Start initialize
		int SpawnShellNum = MinigameDataAsset->LevelMinigameConfigTable[LevelIndex].MinigameConfigTable[CurrentMinigameIndex].AdditionalInformation.Num();
		for (int i = 0; i < SpawnShellNum; i++)
		{
			Server_RespawnShellObject(i);
		}

		IsGameInitialized = true;
	}
}

void AMGameMode::Server_RespawnShellObject_Implementation(int AdditionalInformationIndex)
{
	FTransform spawnTransform;
	if (MinigameDataAsset->LevelMinigameConfigTable[LevelIndex].MinigameConfigTable[CurrentMinigameIndex].AdditionalInformation[AdditionalInformationIndex].Additional_PositionType == EMinigamePositionTypeEnum::Single)
	{
		spawnTransform = MinigameDataAsset->LevelMinigameConfigTable[LevelIndex].MinigameConfigTable[CurrentMinigameIndex].AdditionalInformation[AdditionalInformationIndex].Additional_Transform;
	}
	else if (MinigameDataAsset->LevelMinigameConfigTable[LevelIndex].MinigameConfigTable[CurrentMinigameIndex].AdditionalInformation[AdditionalInformationIndex].Additional_PositionType == EMinigamePositionTypeEnum::Multiple)
	{
		int CurrentSpawnTransformIndex = FMath::RandRange(0, MinigameDataAsset->LevelMinigameConfigTable[LevelIndex].MinigameConfigTable[CurrentMinigameIndex].AdditionalInformation[AdditionalInformationIndex].Additional_Transforms.Num() - 1);
		spawnTransform = MinigameDataAsset->LevelMinigameConfigTable[LevelIndex].MinigameConfigTable[CurrentMinigameIndex].AdditionalInformation[AdditionalInformationIndex].Additional_Transforms[CurrentSpawnTransformIndex];
	}

	AWeaponShell* spawnActor = GetWorld()->SpawnActor<AWeaponShell>(MinigameDataAsset->LevelMinigameConfigTable[LevelIndex].MinigameConfigTable[CurrentMinigameIndex].AdditionalInformation[AdditionalInformationIndex].Additional_ActorClass, spawnTransform);
	if(spawnActor)
	{
		spawnActor->UpdateScoreCanGet(MinigameDataAsset->LevelMinigameConfigTable[LevelIndex].MinigameConfigTable[CurrentMinigameIndex].AdditionalInformation[AdditionalInformationIndex].Additional_Int);
		spawnActor->UpdateConfigIndex(AdditionalInformationIndex);
	}
}
