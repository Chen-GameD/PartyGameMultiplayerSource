// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "M_PlayerState.generated.h"

/**
 * 
 */
UCLASS()
class PARTYGAMEMULTIPLAYER_API AM_PlayerState : public APlayerState
{
	GENERATED_BODY()
public:

	UPROPERTY(ReplicatedUsing=OnRep_PlayerNameString, BlueprintReadOnly)
	FString PlayerNameString = "default-name";

	UPROPERTY(ReplicatedUsing=OnRep_UpdateTeamIndex, EditAnywhere, BlueprintReadWrite)
	int TeamIndex = 0;

	UPROPERTY(ReplicatedUsing=OnRep_UpdateReadyInformation, BlueprintReadOnly)
	bool IsReady = false;

	UFUNCTION(Server, Reliable)
	void Server_UpdateTeamIndex(int i_TeamIndex = 1);

	UFUNCTION(Client, Reliable)
	void Client_SetPlayerNameFromGameInstance();
	UFUNCTION(Server, Reliable)
	void Server_UpdatePlayerName(const FString& i_Name);

	UFUNCTION(Client, Reliable)
	void Client_SetPlayerSkinFromGameInstance();
	UFUNCTION(Server, Reliable)
	void Server_UpdatePlayerSkin(FLinearColor i_ColorPicked, int i_CharacterIndex);

	UFUNCTION(Server, Reliable)
	void Server_UpdatePlayerReadyState();

	UFUNCTION()
	void OnRep_PlayerNameString();

	UFUNCTION()
	void OnRep_PlayerSkinInformation();

	UFUNCTION()
	void OnRep_UpdateTeamIndex();

	UFUNCTION()
	void OnRep_UpdateReadyInformation();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Replicated)
	int kill;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Replicated)
	int death;

	AM_PlayerState();
	/** Property replication */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION(BlueprintCallable, Category = "SKD")
	void addScore(float i_scoreToAdd);

	UFUNCTION(BlueprintCallable, Category = "SKD")
	void addKill(int i_killToAdd);

	UFUNCTION(BlueprintCallable, Category = "SKD")
	void addDeath(int i_deathToAdd);

	// Customization
	UPROPERTY(ReplicatedUsing=OnRep_PlayerSkinInformation, EditAnywhere, BlueprintReadWrite)
	FLinearColor colorPicked;

	UPROPERTY(ReplicatedUsing=OnRep_PlayerSkinInformation, EditAnywhere, BlueprintReadWrite)
	int characterIndex;
};
