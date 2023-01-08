// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/MPlayerController.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Camera/CameraComponent.h"
#include "Character/MCharacter.h"
#include "GameBase/MGameInstance.h"
#include "GameBase/MGameMode.h"
#include "GameBase/MGameState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameStateBase.h"

// Constructor
// ===================================================
#pragma region Constructor
AMPlayerController::AMPlayerController()
{
	
}

void AMPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Replicate Action
}

// void AMPlayerController::UpdateLobbyMenu_Implementation()
// {
// 	if (IsLocalPlayerController())
// 	{
// 		UI_UpdateLobbyMenu();
// 	}
// }

void AMPlayerController::JoinATeam_Implementation(int i_TeamIndex, const FString& i_PlayerName)
{
	AMGameMode* MyGameMode = Cast<AMGameMode>(GetWorld()->GetAuthGameMode());
	if (MyGameMode)
	{
		if (i_PlayerName != "")
		{
			GetPlayerState<AM_PlayerState>()->UpdatePlayerName(i_PlayerName);
		}
		GetPlayerState<AM_PlayerState>()->UpdateTeamIndex(i_TeamIndex);
	}
}

void AMPlayerController::GetReadyButtonClick_Implementation()
{
	AM_PlayerState* MyServerPlayerState = GetPlayerState<AM_PlayerState>();
	if (MyServerPlayerState->TeamIndex != 0)
	{
		if (MyServerPlayerState->IsReady == true)
		{
			MyServerPlayerState->IsReady = false;
		}
		else
		{
			MyServerPlayerState->IsReady = true;
		}
	}

	AMGameMode* MyGameMode = Cast<AMGameMode>(GetWorld()->GetAuthGameMode());
	if (MyGameMode)
	{
		MyGameMode->CheckGameStart();
	}
}

void AMPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalPlayerController())
	{
		UI_ShowLobbyMenu();
	}
	
	// if (IsLocalPlayerController())
	// {
	// 	UI_ShowLobbyMenu();
 //        
	// 	// Get Name and update to playerstate
	// 	AM_PlayerState* MyPlayerState = GetPlayerState<AM_PlayerState>();
	// 	UMGameInstance* MyGameInstance = Cast<UMGameInstance>(GetGameInstance());
	// 	if (MyPlayerState && MyGameInstance)
	// 	{
	// 		MyPlayerState->UpdatePlayerName(MyGameInstance->PlayerName);
	// 		MyPlayerState->UpdateTeamIndex();
	// 	}
	// 	//GetPlayerState<AM_PlayerState>()->UpdatePlayerName(Cast<UMGameInstance>(GetGameInstance())->PlayerName);
 //        
	// 	//GetPlayerState<AM_PlayerState>()->UpdateTeamIndex();
	// }
	

	// if (IsLocalPlayerController())
	// {
	// 	//FInputModeGameAndU
	// 	FInputModeGameOnly inputMode;
	// 	inputMode.SetConsumeCaptureMouseDown(false);
	// 	SetInputMode(inputMode);
	// 	// set our turn rate for input
	// 	TurnRateGamePad = 50.f;
	//
	// 	bShowMouseCursor = true;
	// 	DefaultMouseCursor = EMouseCursor::Default;
	// }
}

// Input
// ====================================================
#pragma region Input
void AMPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	InputComponent->BindAxis("Move Forward / Backward", this, &AMPlayerController::MoveForward);
	InputComponent->BindAxis("Move Right / Left", this, &AMPlayerController::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	//InputComponent->BindAxis("Turn Right / Left Mouse", this, &AMPlayerController::AddControllerYawInput);
	//InputComponent->BindAxis("Turn Right / Left Gamepad", this, &AMPlayerController::TurnAtRate);
	//InputComponent->BindAxis("Look Up / Down Mouse", this, &AMPlayerController::AddControllerPitchInput);
	//InputComponent->BindAxis("Look Up / Down Gamepad", this, &AMPlayerController::LookUpAtRate);

	// handle touch devices
	InputComponent->BindTouch(IE_Pressed, this, &AMPlayerController::TouchStarted);
	InputComponent->BindTouch(IE_Released, this, &AMPlayerController::TouchStopped);

	// Test
	//InputComponent->BindAction("TestKey", IE_Released, this, &AMPlayerController::Test);
}

void AMPlayerController::UI_ShowLobbyMenu()
{
	if (WB_LobbyMenuClass)
	{
		if (!WB_LobbyMenu)
		{
			// Create menu on client
			if (IsLocalPlayerController())
			{
				WB_LobbyMenu = CreateWidget<UUserWidget>(this, WB_LobbyMenuClass);
				//CreateWidget(GetFirstLocalPlayerController(), WB_MainMenuClass->StaticClass());
                
				WB_LobbyMenu->AddToViewport();
				//FInputModeUIOnly inputMode;
				FInputModeUIOnly inputMode;
				inputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
				this->SetInputMode(inputMode);
				this->SetShowMouseCursor(true);
			}
		}
	}
}

void AMPlayerController::GetNotifyPlayerControllerUpdateReadyState_Implementation(bool IsAllReady)
{
	if (IsLocalPlayerController())
	{
		UpdateReadyState(IsAllReady);
	}
}

void AMPlayerController::Client_RefreshWeaponUI_Implementation()
{
	AMGameState* MyGameState = Cast<AMGameState>(GetWorld()->GetGameState());
	if (MyGameState->IsGameStart)
	{
		AddWeaponUI();
	}
}

void AMPlayerController::StartTheGame()
{
	// Hide the lobby menu
	if (WB_LobbyMenu)
	{
		if (WB_LobbyMenu->IsVisible())
		{
			WB_LobbyMenu->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	// Remove All Widget
	// TO DO: need to move UI from pawn to controller
	//UWidgetLayoutLibrary::RemoveAllWidgets(GetWorld());

	// Add timer UI
	if (WB_GameTimerUIClass)
	{
		if (!WB_GameTimerUI)
		{
			// Create menu on client
			if (IsLocalPlayerController())
			{
				WB_GameTimerUI = CreateWidget<UUserWidget>(this, WB_GameTimerUIClass);
				//CreateWidget(GetFirstLocalPlayerController(), WB_MainMenuClass->StaticClass());
				
				WB_GameTimerUI->AddToViewport();
				//FInputModeGameAndU
				FInputModeGameOnly inputMode;
				inputMode.SetConsumeCaptureMouseDown(false);
				SetInputMode(inputMode);
				// set our turn rate for input
				//TurnRateGamePad = 50.f;
				//
				bShowMouseCursor = true;
				DefaultMouseCursor = EMouseCursor::Default;
			}
		}
	}
}

void AMPlayerController::Client_SetGameUIVisibility_Implementation(bool isVisible)
{
	// AMCharacter* MyPawn = Cast<AMCharacter>(GetPawn());
	// if (MyPawn)
	// {
	// 	MyPawn->SetGameUIVisibility(isVisible);
	// }
	
	for(FConstPawnIterator iterator = GetWorld()->GetPawnIterator(); iterator; ++iterator)
	{
		AMCharacter* pawn = Cast<AMCharacter>(*iterator);

		if (pawn)
		{
			pawn->SetGameUIVisibility(isVisible);
		}
	}
}

void AMPlayerController::Client_SynMeshWhenJoinSession_Implementation()
{
	for (FConstPawnIterator iterator = GetWorld()->GetPawnIterator(); iterator; ++iterator)
	{
		AMCharacter* pawn = Cast<AMCharacter>(*iterator);
		
		if (pawn)
		{
			AM_PlayerState* MyPlayerState = Cast<AM_PlayerState>(pawn->GetPlayerState());
			if (MyPlayerState)
			{
				pawn->SetThisCharacterMesh(MyPlayerState->TeamIndex);
			}
		}
	}
}

void AMPlayerController::Server_RequestRespawn_Implementation()
{
	// Delete current controlled character
	GetPawn()->Destroy();
	
	AMGameMode* myGameMode = Cast<AMGameMode>(GetWorld()->GetAuthGameMode());

	if (myGameMode)
	{
		myGameMode->Server_RespawnPlayer(this);
		AMGameState* myGameState = Cast<AMGameState>(GetWorld()->GetGameState());
		if (myGameState)
		{
			Client_SetGameUIVisibility(myGameState->IsGameStart);
		}
	}
}


void AMPlayerController::MoveForward(float Value)
{
	AMCharacter* const MyPawn = Cast<AMCharacter>(GetPawn());
	if (MyPawn)
	{
		if (MyPawn->GetIsDead())
			return;
        	
		if (Value != 0.0f)
		{
			// find out which way is forward
			
			const FRotator Rotation = Cast<AMCharacter>(GetPawn())->GetFollowCamera()->GetComponentRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);
        
			// get forward vector
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
			// add movement in that direction
			MyPawn->AddMovementInput(Direction, Value);
		}
	}
	
}

void AMPlayerController::MoveRight(float Value)
{
	AMCharacter* const MyPawn = Cast<AMCharacter>(GetPawn());
	if (MyPawn)
	{
		if (MyPawn->GetIsDead())
			return;
        	
		if (Value != 0.0f)
		{
			// find out which way is right
			const FRotator Rotation = Cast<AMCharacter>(GetPawn())->GetFollowCamera()->GetComponentRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);
        
			// get right vector 
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
			// add movement in that direction
			MyPawn->AddMovementInput(Direction, Value);
		}
	}
	
}

void AMPlayerController::TurnAtRate(float Rate)
{
}

void AMPlayerController::LookUpAtRate(float Rate)
{
}

void AMPlayerController::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
}

void AMPlayerController::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
}

void AMPlayerController::Test_Implementation()
{
	// if (IsLocalPlayerController())
	// {
	// 	UI_ShowLobbyMenu();
 //        
	// 	// Get Name and update to playerstate
	// 	AM_PlayerState* MyPlayerState = GetPlayerState<AM_PlayerState>();
	// 	UMGameInstance* MyGameInstance = Cast<UMGameInstance>(GetGameInstance());
	// 	if (MyPlayerState && MyGameInstance)
	// 	{
	// 		MyPlayerState->UpdatePlayerName(MyGameInstance->PlayerName);
	// 		MyPlayerState->UpdateTeamIndex();
	// 	}
	// 	//GetPlayerState<AM_PlayerState>()->UpdatePlayerName(Cast<UMGameInstance>(GetGameInstance())->PlayerName);
 //        
	// 	//GetPlayerState<AM_PlayerState>()->UpdateTeamIndex();
	// }

	if (GetLocalRole() == ROLE_Authority)
	{
		AMGameMode* MyGameMode = Cast<AMGameMode>(GetWorld()->GetAuthGameMode());
		if (MyGameMode)
		{
			MyGameMode->TestRestartLevel();
		}
	}
}

#pragma endregion Input

void AMPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	
    // Look for the touch location
    FVector HitLocation = FVector::ZeroVector;
    FHitResult Hit;

	GetHitResultUnderCursor(ECC_Visibility, true, Hit);
	HitLocation = Hit.Location;
    
	// Direct the Pawn towards that location
	APawn* const MyPawn = GetPawn();
	AMGameState* const MyGameState = Cast<AMGameState>(GetWorld()->GetGameState());
	if(MyPawn && MyGameState)
	{
		if (MyGameState->IsGameStart)
		{
			FVector WorldDirection = (HitLocation - MyPawn->GetActorLocation()).GetSafeNormal();
			FRotator WorldRotator = WorldDirection.Rotation();
			//MyPawn->AddMovementInput(WorldDirection, 1.f, false);
			SetControlRotation(WorldRotator);
		}
	}
}
#pragma endregion Constructor
