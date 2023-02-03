// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSessionSettings.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "SessionEntry.h"
#include "EOSGameInstance.generated.h"



/**
 * 
 */
UCLASS()
class PARTYGAMEMULTIPLAYER_API UEOSGameInstance : public UGameInstance
{
	GENERATED_BODY()

	bool bIsLoggedIn = false;

public:

	virtual void Init() override;

	UFUNCTION(BlueprintCallable, Category="EOS Functions")
	void Login(FString ID, FString Token, FString LoginType);

	UFUNCTION(BlueprintCallable, Category="EOS Functions")
	FString GetPlayerUsername();

	UFUNCTION(BlueprintCallable, Category="EOS Functions")
	void CreateSession(bool IsDedicatedServer, bool IsLanServer, int32 NumberOfPublicConnections);

	UFUNCTION(BlueprintCallable, Category="EOS Functions")
	void FindSession();

	UFUNCTION(BlueprintCallable, Category="EOS Functions")
	void JoinSession(int32 index);
	
	UFUNCTION(BlueprintCallable)
	void DestroySession();

	UFUNCTION(BlueprintCallable)
	void ShowInviteUI();

	UFUNCTION(BlueprintCallable)
	TArray<USessionEntry*> GetSessionsList();

	TSharedPtr<FOnlineSessionSearch> SearchSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString LevelText;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsSessionsListAvailable = false;

	void OnCreateSessionComplete(FName sessionName, bool bWasSuccessful);
	void OnDestroySessionComplete(FName sessionName, bool bWasSuccessful);
	void OnFindSessionComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName sessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);
};
