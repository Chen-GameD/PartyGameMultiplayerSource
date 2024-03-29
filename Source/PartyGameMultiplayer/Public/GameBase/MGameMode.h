// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerStart.h"
#include "LevelInteraction/MinigameMainObjective.h"
#include "LevelInteraction/MinigameObject/MinigameBaseDataAsset.h"
#include "MGameMode.generated.h"

/**
 * 
 */
UCLASS()
class PARTYGAMEMULTIPLAYER_API AMGameMode : public AGameModeBase
{
	GENERATED_BODY()

// Function
// ==============================================================
	
public:
	// Override
	// =================================================================================================================
	// The InitGame event is called before any other scripts (including PreInitializeComponents),
	// and is used by AGameModeBase to initialize parameters and spawn its helper classes.
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	// Accepts or rejects a player who is attempting to join the server. Causes the Login function to fail if it sets ErrorMessage to a non-empty string.
	// PreLogin is called before Login, and a significant amount of time may pass before Login is called, especially if the joining player needs to download game content.
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

	// Called after a successful login.
	// This is the first place it is safe to call replicated functions on the PlayerController.
	// OnPostLogin can be implemented in Blueprint to add extra logic.
	virtual void PostLogin(APlayerController* NewPlayer) override;

	// Called after PostLogin or after seamless travel, this can be overridden in Blueprint to change what happens to a new player.
	// By default, it will create a pawn for the player.
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

	// Called when a player leaves the game or is destroyed. OnLogout can be implemented to do Blueprint logic.
	virtual void Logout(AController* Exiting) override;
	// =================================================================================================================

	UFUNCTION(Server, Reliable, BlueprintCallable)
		void Server_RespawnPlayer(APlayerController* PlayerController);

	UFUNCTION(Server, Reliable, BlueprintCallable)
		void Server_RespawnMinigameObject(bool bFirstTimeSpawn = false);

	UFUNCTION(Server, Reliable, BlueprintCallable)
		void Server_RespawnShellObject(int AdditionalInformationIndex);

	UFUNCTION(Server, Reliable, BlueprintCallable)
		void Server_RearrangeWeapons();

	UFUNCTION()
	void CheckGameStart();

	UFUNCTION()
	void NotifyAllClientPlayerControllerUpdateReadyState(bool IsAllReady);

	UFUNCTION(BlueprintCallable)
	void StartTheGame();

	UFUNCTION()
	void TestRestartLevel();

	UFUNCTION(BlueprintCallable)
	void OnLevelIndexUpdate(int i_LevelIndex);

protected:
	UFUNCTION()
	void InitMinigame_ShellObject();

	UFUNCTION()
	void SpawnMinigameObj();

	
// Members
// ==============================================================

public:
	UPROPERTY()
	int TeamOnePlayerNum = 0;
	UPROPERTY()
	int TeamTwoPlayerNum = 0;

	UPROPERTY()
	int MaxTeamPlayers = 3;

	UPROPERTY()
	int CurrentPlayerNum = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int LevelIndex = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int TutorialLevelIndex = 2;

	UPROPERTY()
	AMinigameMainObjective* CurrentMinigameObj;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<APlayerStart*> Team_1_SpawnPoints;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<APlayerStart*> Team_2_SpawnPoints;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<USkeletalMesh*> CharacterBPArray;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<ACharacter> CharaterBPType;
	
	FTimerHandle StartGameCountDownTimerHandle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<AMinigameMainObjective> MinigameObjectClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UMinigameBaseDataAsset* MinigameDataAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int CurrentMinigameIndex;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TSubclassOf<class AWeaponAlarm> WeaponAlarmSubClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TSubclassOf<class AWeaponBlower> WeaponBlowerSubClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TSubclassOf<class AWeaponFork> WeaponForkSubClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TSubclassOf<class AWeaponLighter> WeaponLighterSubClass;

private:
	bool IsGameInitialized = false;
};
