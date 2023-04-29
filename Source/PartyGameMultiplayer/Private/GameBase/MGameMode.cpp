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
#include "Kismet/GameplayStatics.h"
#include "Weapon/ElementWeapon/WeaponAlarm.h"
#include "Weapon/ElementWeapon/WeaponBlower.h"
#include "Weapon/ElementWeapon/WeaponFork.h"
#include "Weapon/ElementWeapon/WeaponLighter.h"

void AMGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage); 
	
	/*if (UJsonFactory::InitJsonObject_1())
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, TEXT("GameMode Init JsonObject_1 succeeded"));
	else
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, TEXT("GameMode Init JsonObject_1 failed"));*/

	//CurrentMinigameIndex = FMath::RandRange(0, MinigameDataAsset->MinigameConfigTable.Num() - 1);

	//"UEDPIE_0_FormalLevel_Crabs_Landscape"   "UEDPIE_0_FormalLevel_Museum"
	if( MapName.Contains("FormalLevel_Crabs_Landscape") || MapName.Contains("FormalLevel_Museum") )
		Server_RearrangeWeapons();
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
			bool bRegistrationSuccess = OnlineSessionRef->RegisterPlayer(FName("CBGameSession"), *UniqueNetId, false);
			if(bRegistrationSuccess)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, TEXT("Success Registration"));
				UE_LOG(LogTemp, Warning, TEXT("Success registration: %d"), bRegistrationSuccess);
			}
		}
		
		AM_PlayerState* MyPlayerState = NewPlayer->GetPlayerState<AM_PlayerState>();
		MyPlayerState->Client_SetPlayerNameFromGameInstance();
		MyPlayerState->Client_SetPlayerSkinFromGameInstance();
		CurrentPlayerNum++;
		AMGameState* MyGameState = GetGameState<AMGameState>();
		if (MyGameState)
		{
			Cast<UEOSGameInstance>(GetGameInstance())->UpdateSession(CurrentPlayerNum, !MyGameState->IsGameStart);
		}

		if (NewPlayer->IsLocalPlayerController())
		{
			AMCharacter* MyPawn = Cast<AMCharacter>(NewPlayer->GetPawn());
			if (MyPawn)
			{
				MyPawn->OnRep_PlayerState();
			}

			// Set the host
			MyPlayerState->IsHost = true;
		}
	}
}

void AMGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
}

void AMGameMode::Logout(AController* Exiting)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("CHECKEND : Logout-GameMode-Server"));
	UE_LOG(LogTemp, Warning, TEXT("CHECKEND : Logout-GameMode-Server"));

	CurrentPlayerNum--;
	AMGameState* MyGameState = GetGameState<AMGameState>();
	if (MyGameState)
	{
		Cast<UEOSGameInstance>(GetGameInstance())->UpdateSession(CurrentPlayerNum, !MyGameState->IsGameStart);
	}
	AM_PlayerState* PS = Exiting->GetPlayerState<AM_PlayerState>();
	if (PS)
	{
		switch (PS->TeamIndex)
		{
		case 1:
			TeamOnePlayerNum--;
			break;
		case 2:
			TeamTwoPlayerNum--;
		case 0:
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Log out when Player teamindex is 0!"));
			break;
		default:
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Log out when Player teamindex is invaild!"));
			break;
		}
	}
	
	Super::Logout(Exiting);
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

void AMGameMode::Server_RespawnMinigameObject_Implementation(bool bFirstTimeSpawn)
{
	if (MinigameDataAsset)
	{
		CurrentMinigameIndex = FMath::RandRange(0, MinigameDataAsset->LevelMinigameConfigTable[LevelIndex].MinigameConfigTable.Num() - 1);
		if (bFirstTimeSpawn && LevelIndex != TutorialLevelIndex)
		{
			// SpawnMinigameObject with a certain delay after the game starts
			float DelaySpawnMinigameObjectAtStart = (LevelIndex == 0) ? 1.0f : 4.5f;
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AMGameMode::SpawnMinigameObj, DelaySpawnMinigameObjectAtStart, false);
		}
		else
		{
			// Spawn the minigame object
			SpawnMinigameObj();
		}
		

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

void AMGameMode::SpawnMinigameObj()
{
	// Spawn the minigame object
	FTransform spawnTransform = MinigameDataAsset->LevelMinigameConfigTable[LevelIndex].MinigameConfigTable[CurrentMinigameIndex].MinigameObjectSpawnTransform;
	AMinigameMainObjective* spawnActor = GetWorld()->SpawnActor<AMinigameMainObjective>(MinigameDataAsset->LevelMinigameConfigTable[LevelIndex].MinigameConfigTable[CurrentMinigameIndex].MinigameObject, spawnTransform);
	if (spawnActor && MinigameDataAsset)
	{
		CurrentMinigameObj = spawnActor;
		spawnActor->UpdateScoreCanGet(MinigameDataAsset->LevelMinigameConfigTable[LevelIndex].MinigameConfigTable[CurrentMinigameIndex].ScoreCanGet);
	}
}

void AMGameMode::Server_RearrangeWeapons_Implementation()
{
	TArray<FTransform> Arr_SpawnTransform;
	TArray<EnumWeaponType> Arr_WeaponTypeCanBeSpawned;
	TMap<EnumWeaponType, unsigned int> Map_WeaponTypeCnt;

	TSubclassOf<ABaseWeapon> ActorClass = ABaseWeapon::StaticClass();
	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ActorClass, OutActors);
	for (size_t i = 0; i < OutActors.Num(); i++)
	{
		if (ABaseWeapon* pWeapon = Cast<ABaseWeapon>(OutActors[i]))
		{
			if (pWeapon->WeaponType != EnumWeaponType::Shell)
			{
				Arr_SpawnTransform.Add(pWeapon->GetTransform());
				EnumWeaponType ThisWeaponType = pWeapon->WeaponType;
				pWeapon->Destroy();
				
				if (!Map_WeaponTypeCnt.Contains(ThisWeaponType))
				{
					Arr_WeaponTypeCanBeSpawned.Add(ThisWeaponType);
					Map_WeaponTypeCnt.Add(ThisWeaponType);
					Map_WeaponTypeCnt[ThisWeaponType] = 0;
				}
				Map_WeaponTypeCnt[ThisWeaponType]++;
			}
		}
	}

	for (size_t i = 0; i < Arr_SpawnTransform.Num(); i++)
	{
		int SizeOf_Arr_WeaponTypeCanBeSpawned = Arr_WeaponTypeCanBeSpawned.Num();
		if (SizeOf_Arr_WeaponTypeCanBeSpawned <= 0)
			break;
		
		EnumWeaponType SpawnWeaponType = Arr_WeaponTypeCanBeSpawned[FMath::RandRange(0, SizeOf_Arr_WeaponTypeCanBeSpawned-1)];
		FTransform SpawnTransform = Arr_SpawnTransform[i];
		if (--Map_WeaponTypeCnt[SpawnWeaponType] <= 0)
			Arr_WeaponTypeCanBeSpawned.Remove(SpawnWeaponType);

		ABaseWeapon* NewlySpawnWeapon = nullptr;
		if (SpawnWeaponType == EnumWeaponType::Alarm)
			NewlySpawnWeapon = GetWorld()->SpawnActor<AWeaponAlarm>(WeaponAlarmSubClass, SpawnTransform);
		else if (SpawnWeaponType == EnumWeaponType::Blower)
			NewlySpawnWeapon = GetWorld()->SpawnActor<AWeaponBlower>(WeaponBlowerSubClass, SpawnTransform);
		else if (SpawnWeaponType == EnumWeaponType::Fork)
			NewlySpawnWeapon = GetWorld()->SpawnActor<AWeaponFork>(WeaponForkSubClass, SpawnTransform);
		else if (SpawnWeaponType == EnumWeaponType::Lighter)
			NewlySpawnWeapon = GetWorld()->SpawnActor<AWeaponLighter>(WeaponLighterSubClass, SpawnTransform);

		/*if (NewlySpawnWeapon)
			NewlySpawnWeapon->NetMulticast_CallShowUpVfx();*/
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
				Cast<UEOSGameInstance>(GetGameInstance())->SetSessionStateStarted();
				GetWorldTimerManager().SetTimer(StartGameCountDownTimerHandle, this, &AMGameMode::StartTheGame, 6.5, false);
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

	if (LevelIndex == TutorialLevelIndex)
	{
		CanStart = true;
		GetWorldTimerManager().SetTimer(StartGameCountDownTimerHandle, this, &AMGameMode::StartTheGame, .5, false);
		AMPlayerController* MyPlayerController = Cast<AMPlayerController>(GetWorld()->GetFirstPlayerController());
		if (MyPlayerController)
		{
			MyPlayerController->Tutorial_InitForPlayerController();
		}
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
		MyGameState->KillScore = MinigameDataAsset->LevelMinigameConfigTable[LevelIndex].KillScore;
		if (LevelIndex != TutorialLevelIndex)
		{
			MyGameState->Server_StartGame();
			//Cast<UEOSGameInstance>(GetGameInstance())->SetSessionStateStarted();
		}
		else
		{
			MyGameState->Server_StartTutorialGame();
		}
	}

	//Server_RearrangeWeapons();
	Server_RespawnMinigameObject(true);
}

void AMGameMode::TestRestartLevel()
{
	UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), false);
}

void AMGameMode::OnLevelIndexUpdate(int i_LevelIndex)
{
	if (i_LevelIndex != LevelIndex)
	{
		LevelIndex = i_LevelIndex;
		AMGameState* MyGameState = Cast<AMGameState>(GetWorld()->GetGameState());
		if (MyGameState)
		{
			MyGameState->LevelIndex = LevelIndex;
			MyGameState->TutorialLevelIndex = TutorialLevelIndex;

			if (GetNetMode() == NM_ListenServer)
			{
				MyGameState->OnRep_LevelIndex();
			}
		}
	}
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
