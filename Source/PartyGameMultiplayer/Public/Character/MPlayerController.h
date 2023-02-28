// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UI/MCharacterFollowWidget.h"
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
	
	/** Property replication */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintImplementableEvent)
	void UI_UpdateLobbyMenu();

	UFUNCTION(BlueprintImplementableEvent)
	void UI_UpdateGameTimer();

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateReadyState(bool IsAllReady);

	UFUNCTION(BlueprintImplementableEvent)
	void EndTheGame();

	UFUNCTION(BlueprintImplementableEvent)
	void AddWeaponUI();

	UFUNCTION(Client, Reliable)
	void Client_RefreshWeaponUI();

	//UFUNCTION(Client, Reliable)
	//void UpdateLobbyMenu();

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void JoinATeam(int i_TeamIndex = 1, const FString& i_PlayerName = "");

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void GetReadyButtonClick();

	UFUNCTION(Client, Reliable, BlueprintCallable)
	void GetNotifyPlayerControllerUpdateReadyState(bool IsAllReady);

	UFUNCTION()
	void StartTheGame();

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void NetMulticast_InitPlayerFollowWidget(bool isVisible, bool needReset = false);

	UFUNCTION(Client, Reliable)
	void Client_SynMeshWhenJoinSession();

	UFUNCTION(Server, Reliable)
	void Server_RequestRespawn();

	UFUNCTION(Server, Reliable)
	void SetCanMove(bool i_CanMove);

	// UI Update
	// InGame UI
	UFUNCTION()
	void UI_InGame_UpdateHealth(float percentage);

	// HUD getter
	UFUNCTION(BlueprintCallable)
	AMInGameHUD* GetInGameHUD();
	
	// FollowWidget Relate
	//=========================================================
	// // This function should only call when this controller is locally controlled controller.
	// // Set the health bar hidden, set the pick up hint and weapon energy bar visible.
	// UFUNCTION()
	// void LocallyControlled_SetFollowWidgetUIVisibility(ESlateVisibility newVisibility);
	// // This function should only call when this controller is not locally controlled controller.
	// // Set the health bar visible, set the pick up hint and weapon energy bar hidden.
	// UFUNCTION()
	// void NotLocallyControlled_SetFollowWidgetUIVisibility(ESlateVisibility newVisibility);
	UFUNCTION()
	void SetFollowWidgetUIVisibility(ESlateVisibility newVisibility);
	UFUNCTION()
	void UpdateFollowWidgetHealthBar(float percent);
	UFUNCTION()
	void SetFollowWidgetPlayerName();
	UFUNCTION()
	void SetFollowWidgetWeaponHintVisibility(ESlateVisibility newVisibility);
	UFUNCTION()
	void UpdateFollowWidgetWeaponHint(UTexture2D* LeftTextureUI, UTexture2D* RightTextureUI);
	
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

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

	// Test
	UFUNCTION(Server, Reliable)
	void Test();

	// Begin PlayerController interface
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;
	// End PlayerController interface

	UFUNCTION(BlueprintCallable)
	void UI_ShowLobbyMenu();
	
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

	/**	Health Bar UI widget */
	// UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	// TSubclassOf<UUserWidget> WB_PlayerFollowWidgetClass;
	// UPROPERTY()
	// UMCharacterFollowWidget* PlayerFollowWidget;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TSubclassOf<UMCharacterFollowWidget> PlayerFollowWidgetClass;
	// Follow Widget
	UMCharacterFollowWidget* CharacterFollowWidget;

// Members
// ==============================================================
private:
	//InGame HUD
	AMInGameHUD* MyInGameHUD;

	// // Follow Widget
	// UMCharacterFollowWidget* CharacterFollowWidget;
	
};
