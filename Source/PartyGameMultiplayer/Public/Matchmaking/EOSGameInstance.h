// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSessionSettings.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "ReturnGameState.h"
#include "SessionEntry.h"
#include "EOSGameInstance.generated.h"



/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFindSessionResultsStored);

UCLASS()
class PARTYGAMEMULTIPLAYER_API UEOSGameInstance : public UGameInstance
{
	GENERATED_BODY()

	bool bIsLoggedIn = false;

protected:
	UPROPERTY(BlueprintReadWrite)
	UReturnGameState* ReturnGameState = nullptr;

public:
	UPROPERTY(BlueprintAssignable)
	FOnFindSessionResultsStored OnFindSessionsDelegate;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool isLoading = false;

	virtual void Init() override;

	UFUNCTION(BlueprintCallable, Category="EOS Functions")
	void Login(FString ID, FString Token, FString LoginType);

	UFUNCTION(BlueprintCallable, Category="EOS Functions")
	FString GetPlayerUsername();


	UFUNCTION(BlueprintCallable, Category="EOS Functions")
	void CreateSession(bool IsDedicatedServer, bool IsLanServer, int32 NumberOfPublicConnections, bool IsPrivate, FString RoomName, int MapIndex);

	UFUNCTION(BlueprintCallable, Category="EOS Functions")
	void FindSession();

	UFUNCTION(BlueprintCallable, Category="EOS Functions")
	void JoinSession(int32 index);
	
	UFUNCTION(BlueprintCallable, Category="EOS Functions")
	void DestroySession();

	UFUNCTION(BlueprintCallable)
	UReturnGameState* GetReturnGameStateRef();

	UFUNCTION(BlueprintCallable)
	void ShowInviteUI();

	UFUNCTION(BlueprintCallable)
	TArray<USessionEntry*> GetSessionsList();

	UFUNCTION(BlueprintCallable)
	void UI_ShowMainMenu();

	UFUNCTION(BlueprintCallable)
	bool GetIsLoggedIn();

	TSharedPtr<FOnlineSessionSearch> SearchSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int DebugLevelSelect = -1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> LevelText;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsSessionsListAvailable = false;

	// Customization
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor colorPicked;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int characterIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TSubclassOf<UUserWidget> WB_MainMenuClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UUserWidget* WB_MainMenu;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FString PlayerName = "player-name";

	UPROPERTY(VisibleAnywhere)
	int32 CurrentlyJoiningSessionIndex = -1;

	void OnCreateSessionComplete(FName sessionName, bool bWasSuccessful);
	void OnDestroySessionComplete(FName sessionName, bool bWasSuccessful);
	void OnFindSessionComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName sessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);
};
