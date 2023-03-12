// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "MGameState.generated.h"

/**
 * 
 */
UCLASS()
class PARTYGAMEMULTIPLAYER_API AMGameState : public AGameStateBase
{
	GENERATED_BODY()

public:

	virtual void BeginPlay() override;

	/** Property replication */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(ReplicatedUsing=OnRep_IsGameStart, BlueprintReadWrite)
	bool IsGameStart = false;

	// 300 sec by default
	UPROPERTY(ReplicatedUsing=UpdateGameStartTimerUI, BlueprintReadWrite)
	int GameTime = 300;

	UFUNCTION(Server, Reliable)
	void Server_StartGame();

	UFUNCTION()
	void OnRep_IsGameStart();

	UFUNCTION()
	void UpdateGameStartTimerUI();

	UFUNCTION()
	void UpdateGameTime();

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

	// Minigame Hint Section
	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_UpdateMinigameHint(const FString& i_Hint);
};
