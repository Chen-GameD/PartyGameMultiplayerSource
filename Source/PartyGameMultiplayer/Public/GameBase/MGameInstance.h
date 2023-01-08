// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class PARTYGAMEMULTIPLAYER_API UMGameInstance : public UGameInstance
{
	GENERATED_BODY()

	// Function
	// ====================================
public:
	UMGameInstance();

	
	UFUNCTION(BlueprintCallable)
	void UI_ShowMainMenu();

	UFUNCTION(BlueprintCallable)
	void JoinServerLobby();

	UFUNCTION(BlueprintCallable)
	void StartServerGame();

protected:
	

	// Members
	// ====================================
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TSubclassOf<UUserWidget> WB_MainMenuClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UUserWidget* WB_MainMenu;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FString PlayerName = "CMY";

private:
};
