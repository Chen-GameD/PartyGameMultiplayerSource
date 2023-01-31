// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SystemHelper/GlobalMacro.h"
#include "MGameState.generated.h"

/**
 * 
 */
UCLASS()
class PARTYGAMEMULTIPLAYER_API AMGameState : public AGameStateBase
{
	GENERATED_BODY()

public:

	/** Property replication */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(ReplicatedUsing=SetClientStartGame, BlueprintReadWrite)
	bool IsGameStart = false;

	// 300 sec by default
	UPROPERTY(ReplicatedUsing=UpdateGameStartTimerUI, BlueprintReadWrite)
	int GameTime = 300;

	UFUNCTION(Server, Reliable)
	void StartGame();

	UFUNCTION(Client, Reliable)
	void SetClientStartGame();

	UFUNCTION()
	void UpdateGameStartTimerUI();

	UFUNCTION()
	void UpdateGameTime();

	FTimerHandle GameStartTimerHandle;
};
