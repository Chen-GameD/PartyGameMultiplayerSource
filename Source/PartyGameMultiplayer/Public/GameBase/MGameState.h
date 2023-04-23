// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Matchmaking/ReturnGameState.h"
#include "UI/PlayerUI/MLobbyWidget.h"
#include "MGameState.generated.h"

/**
 * 
 */
UCLASS()
class PARTYGAMEMULTIPLAYER_API AMGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	
	virtual bool HasBegunPlay() const override;
	
	virtual void BeginPlay() override;

	virtual void RemovePlayerState(APlayerState* PlayerState) override;

	UFUNCTION()
	void GameHasBeenPlayed();
	
	UFUNCTION(Server, Reliable)
	void Server_StartSyncForNewPlayer();

	/** Property replication */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(ReplicatedUsing=OnRep_IsGameStart, BlueprintReadWrite)
	bool IsGameStart = false;

	// 300 sec by default
	UPROPERTY(ReplicatedUsing=UpdateGameStartTimerUI, BlueprintReadWrite)
	int GameTime = 300;
	UPROPERTY(BlueprintReadWrite)
	int CountdownTime = 10;

	UPROPERTY(Replicated, BlueprintReadOnly)
	int LevelIndex = -1;
	UPROPERTY(Replicated, BlueprintReadOnly)
	int TutorialLevelIndex;

	UPROPERTY(Replicated, BlueprintReadOnly)
	int KillScore = -1;

	UFUNCTION(Server, Reliable)
	void Server_StartGame();

	UFUNCTION(Server, Reliable)
	void Server_StartTutorialGame();

	UFUNCTION(BlueprintImplementableEvent)
	void BPF_GameStartBGM(bool isBroadcast);
	UFUNCTION(BlueprintImplementableEvent)
	void BPF_CountdownSFX();

	UFUNCTION()
	void OnRep_IsGameStart();

	UFUNCTION()
	void UpdateGameStartTimerUI();

	UFUNCTION()
	void UpdateGameTime();

	UFUNCTION()
	void UpdateTutorialGameTimer();

	// UFUNCTION()
	// void OnRep_UpdateTeam1Array();
	// UFUNCTION()
	// void OnRep_UpdateTeam2Array();
	// UFUNCTION()
	// void OnRep_UpdateUndecidedArray();

	FTimerHandle GameStartTimerHandle;

	// Team Score Section
	UFUNCTION()
	void OnRep_Team_1_ScoreUpdate();
	UFUNCTION()
	void OnRep_Team_2_ScoreUpdate();
	UPROPERTY(ReplicatedUsing=OnRep_Team_1_ScoreUpdate)
	int Team_1_Score;
	UPROPERTY(ReplicatedUsing=OnRep_Team_2_ScoreUpdate)
	int Team_2_Score;

	// Teams information
	// UPROPERTY(ReplicatedUsing=OnRep_UpdateTeam1Array)
	// TArray<FLobbyInformationStruct> Team1Array;
	// UPROPERTY(ReplicatedUsing=OnRep_UpdateTeam2Array)
	// TArray<FLobbyInformationStruct> Team2Array;
	// UPROPERTY(ReplicatedUsing=OnRep_UpdateUndecidedArray)
	// TArray<FLobbyInformationStruct> UndecidedArray;

	// Minigame Hint Section
	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_UpdateMinigameHint(const FString& i_Hint, UTexture2D* i_HintImage);

	// Has Been Played Timer Handler
	FTimerHandle HasBeenPlayedTimerHandle;

	// Countdown SFX
	bool IsStartBroadcastCountdown;
};
