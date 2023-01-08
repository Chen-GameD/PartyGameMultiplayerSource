// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerStart.h"
#include "LevelInteraction/MinigameMainObjective.h"
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
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_RespawnPlayer(APlayerController* PlayerController);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_RespawnMinigameObject();

	UFUNCTION()
	void CheckGameStart();

	UFUNCTION()
	void NotifyAllClientPlayerControllerUpdateReadyState(bool IsAllReady);

	UFUNCTION()
	void StartTheGame();

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void Logout(AController* Exiting) override;

	UFUNCTION()
	void TestRestartLevel();

protected:


private:

	
// Members
// ==============================================================

public:
	UPROPERTY()
	int TeamOnePlayerNum = 0;
	UPROPERTY()
	int TeamTwoPlayerNum = 0;

	UPROPERTY()
	int MaxTeamPlayers = 4;

	UPROPERTY()
	int CurrentPlayerNum = 0;

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
	FTransform MinigameObjectSpawnTransform;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<AMinigameMainObjective> MinigameObjectClass;

private:
	
};
