// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UI/MInGameHUD.h"
#include "MPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class PARTYGAMEMULTIPLAYER_API AMPlayerController : public APlayerController
{
	GENERATED_BODY()
	
// Function
// ==============================================================
	
public:
	AMPlayerController();

	virtual void Destroyed() override;
	
	/** Property replication */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// UFUNCTION(NetMulticast, Reliable)
	// void NetMulticast_LoginInit();

	UFUNCTION(BlueprintImplementableEvent)
	void UI_UpdateLobbyMenu();

	UFUNCTION(BlueprintImplementableEvent)
	void UI_UpdateGameTimer();

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateReadyState(bool IsAllReady);

	UFUNCTION()
	void UI_UpdateLobbyInformation();
	UFUNCTION()
	void Timer_CheckUpdateLobby(TArray<FLobbyInformationStruct> arrTeam1, TArray<FLobbyInformationStruct> arrTeam2, TArray<FLobbyInformationStruct> arrUndecided);
	UFUNCTION()
	void Timer_CheckPlayerState();

	UFUNCTION(Client, Reliable)
	void Client_SyncLobbyInformation();

	UFUNCTION(Client, Reliable)
	void Client_SyncCharacters();

	UFUNCTION(BlueprintImplementableEvent)
		void CheckIfSaveFileExist();

	UFUNCTION(BlueprintImplementableEvent)
	void EndTheGame();

	UFUNCTION(BlueprintImplementableEvent)
	void AddWeaponUI();

	UFUNCTION(Client, Reliable)
	void Client_RefreshWeaponUI();

	//UFUNCTION(Client, Reliable)
	//void UpdateLobbyMenu();

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void JoinATeam(int i_TeamIndex);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_ReadyButtonClick();

	UFUNCTION(Client, Reliable, BlueprintCallable)
	void GetNotifyPlayerControllerUpdateReadyState(bool IsAllReady);

	UFUNCTION()
	void StartTheGame();
	
	UFUNCTION(Server, Reliable)
	void Server_RequestRespawn();

	UFUNCTION(Server, Reliable)
	void Server_SetCanMove(bool i_CanMove);

	// UI Update
	// InGame UI
	UFUNCTION()
	void UI_InGame_UpdateHealth(float percentage);
	UFUNCTION()
	void UI_InGame_OnUseSkill(SkillType UseSkill, float CoolDownTotalTime);
	UFUNCTION(Client, Reliable)
	void UI_InGame_BroadcastInformation(int KillerTeamIndex, int DeceasedTeamIndex, const FString& i_KillerName, const FString& i_DeceasedName, UTexture2D* i_WeaponImage);
	UFUNCTION(Client, Reliable)
	void UI_InGame_BroadcastMiniInformation(int KillerTeamIndex, const FString& i_KillerName, const FString& i_MinigameInformation);

	// HUD getter
	UFUNCTION(BlueprintCallable)
	AMInGameHUD* GetInGameHUD();
	
protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void OnNetCleanup(UNetConnection* Connection) override;
	
	// Movement
	// ==========================
	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	// Begin PlayerController interface
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;
	// End PlayerController interface
	
public:
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Input)
	float TurnRateGamePad;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TSubclassOf<UUserWidget> WB_LobbyMenuClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UUserWidget* WB_LobbyMenu;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TSubclassOf<UUserWidget> WB_GameTimerUIClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UUserWidget* WB_GameTimerUI;

	UPROPERTY(BlueprintReadOnly, Replicated)
	bool CanMove = true;

// Members
// ==============================================================
private:
	//InGame HUD
	UPROPERTY()
	AMInGameHUD* MyInGameHUD;
	
	FTimerHandle UpdateLobbyTimerHandle;
	FTimerHandle UpdatePlayerStateHandle;
	
};
