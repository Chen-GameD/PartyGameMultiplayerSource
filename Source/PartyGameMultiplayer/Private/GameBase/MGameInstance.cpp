// Fill out your copyright notice in the Description page of Project Settings.


#include "GameBase/MGameInstance.h"

#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

UMGameInstance::UMGameInstance()
{
	// static ConstructorHelpers::FClassFinder<UUserWidget> UI_MainMenu(TEXT("/Game/Blueprint/UI/MenuUI/UI_Menu"));
	// if (UI_MainMenu.Succeeded())
	// {
	// 	WB_MainMenuClass = UI_MainMenu.Class;
	// }
}

void UMGameInstance::UI_ShowMainMenu()
{
	if (WB_MainMenuClass)
	{
		if (!WB_MainMenu)
		{
			// Create menu
			APlayerController* currentPlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
			if (currentPlayerController != nullptr)
			{
				WB_MainMenu = CreateWidget<UUserWidget>(currentPlayerController, WB_MainMenuClass);
				//CreateWidget(GetFirstLocalPlayerController(), WB_MainMenuClass->StaticClass());
                
				WB_MainMenu->AddToViewport();
				FInputModeUIOnly inputMode;
				inputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
				currentPlayerController->SetInputMode(inputMode);
				currentPlayerController->SetShowMouseCursor(true);
			}
			
		}
	}
}

void UMGameInstance::JoinServerLobby()
{
	
}

void UMGameInstance::StartServerGame()
{
	
}




