// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/MPlayerController.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Camera/CameraComponent.h"
#include "Character/MCharacter.h"
#include "Components/WidgetComponent.h"
#include "GameBase/MGameInstance.h"
#include "GameBase/MGameMode.h"
#include "GameBase/MGameState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameStateBase.h"
#include "Matchmaking/EOSGameInstance.h"
#include "UI/MCharacterFollowWidget.h"
#include "UI/MInGameHUD.h"

// Constructor
// ===================================================
#pragma region Constructor
AMPlayerController::AMPlayerController()
{
	//Create HealthBar UI Widget
	//PlayerFollowWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("FollowWidget"));
	//PlayerFollowWidget->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	//check(PlayerFollowWidget);
	
	//CharacterFollowWidget = Cast<UMCharacterFollowWidget>(PlayerFollowWidget->GetUserWidgetObject());
}

void AMPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMPlayerController, CanMove);

	//Replicate Action
}

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

	MyServerPlayerState->UpdatePlayerReadyState();

	AMGameMode* MyGameMode = Cast<AMGameMode>(GetWorld()->GetAuthGameMode());
	if (MyGameMode)
	{
		MyGameMode->CheckGameStart();
	}
}

void AMPlayerController::SetCanMove_Implementation(bool i_CanMove)
{
	CanMove = i_CanMove;
}

void AMPlayerController::UI_InGame_UpdateHealth(float percentage)
{
	if (MyInGameHUD)
	{
		MyInGameHUD->InGame_UpdatePlayerHealth(percentage);
	}
}

AMInGameHUD* AMPlayerController::GetInGameHUD()
{
	return MyInGameHUD;
}

void AMPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalPlayerController())
	{
		UI_ShowLobbyMenu();
		MyInGameHUD = Cast<AMInGameHUD>(GetHUD());
		check(MyInGameHUD);
	}

	// if (GetLocalRole() != ROLE_Authority || GetNetMode() == NM_ListenServer)
	// {
	// 	CharacterFollowWidget = CreateWidget<UMCharacterFollowWidget>(this, PlayerFollowWidgetClass);
	// 	CharacterFollowWidget->AddToViewport();
	//
	// 	if (IsLocalPlayerController())
	// 	{
	// 		// Only show Hint UI and Weapon Energy UI
	// 		if (CharacterFollowWidget)
	// 		{
	// 			CharacterFollowWidget->SetIsLocalControlledUI(true);
	// 		}
	// 	}
	// 	else
	// 	{
	// 		if (CharacterFollowWidget)
	// 		{
	// 			CharacterFollowWidget->SetIsLocalControlledUI(false);
	// 		}
	// 	}
	// 	
	// 	SetFollowWidgetUIVisibility(ESlateVisibility::Hidden);
	// }
}

void AMPlayerController::OnNetCleanup(UNetConnection* Connection)
{
	UEOSGameInstance* GameInstanceRef = Cast<UEOSGameInstance>(GetWorld()->GetGameInstance());
	if(GameInstanceRef)
	{
		GameInstanceRef->DestroySession();
	}
	Super::OnNetCleanup(Connection);
}

// Input
// ====================================================
#pragma region Input
void AMPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	InputComponent->BindAxis("Move Forward / Backward", this, &AMPlayerController::MoveForward);
	InputComponent->BindAxis("Move Right / Left", this, &AMPlayerController::MoveRight);

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

void AMPlayerController::SetFollowWidgetUIVisibility(ESlateVisibility newVisibility)
{
	// if (IsLocalPlayerController())
	// {
	// 	// Only show Hint UI and Weapon Energy UI
	// 	if (CharacterFollowWidget)
	// 	{
	// 		CharacterFollowWidget->SetIsLocalControlledUI(true);
	// 	}
	// }
	// else
	// {
	// 	if (CharacterFollowWidget)
	// 	{
	// 		CharacterFollowWidget->SetIsLocalControlledUI(false);
	// 	}
	// }

	if (CharacterFollowWidget)
	{
		CharacterFollowWidget->SetVisibility(newVisibility);
	}
}

void AMPlayerController::UpdateFollowWidgetHealthBar(float percent)
{
	if (CharacterFollowWidget)
	{
		CharacterFollowWidget->UpdateHealthToProgressBar(percent);
	}
}

void AMPlayerController::SetFollowWidgetPlayerName()
{
	if (CharacterFollowWidget)
	{
		AM_PlayerState* MyPlayerstate = GetPlayerState<AM_PlayerState>();
		if (MyPlayerstate)
		{
			CharacterFollowWidget->SetPlayerName(MyPlayerstate->PlayerNameString);
		}
	}
}

void AMPlayerController::SetFollowWidgetWeaponHintVisibility(ESlateVisibility newVisibility)
{
	if (CharacterFollowWidget)
	{
		CharacterFollowWidget->SetHintUIVisibility(newVisibility);
	}
}

void AMPlayerController::UpdateFollowWidgetWeaponHint(UTexture2D* LeftTextureUI, UTexture2D* RightTextureUI)
{
	if (LeftTextureUI && RightTextureUI && CharacterFollowWidget)
	{
		CharacterFollowWidget->SetLeftWeaponTipUI(LeftTextureUI);
		CharacterFollowWidget->SetRightWeaponTipUI(RightTextureUI);
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
	
	if (MyInGameHUD)
	{
		// Show and Init Game Status UI
		MyInGameHUD->InGame_InitGameStatusWidgetContent();
		
		// Set the input mode
		FInputModeGameOnly inputMode;
		inputMode.SetConsumeCaptureMouseDown(false);
		SetInputMode(inputMode);
		bShowMouseCursor = true;
		DefaultMouseCursor = EMouseCursor::Default;
	}
	else
	{
		// No HUD
		// TODO
	}

	// Do Init about all the controller's follow widget
	for (FConstPlayerControllerIterator iter = GetWorld()->GetPlayerControllerIterator(); iter; ++iter)
	{
		AMPlayerController* currentController = Cast<AMPlayerController>(*iter);
		
		currentController->CharacterFollowWidget = CreateWidget<UMCharacterFollowWidget>(currentController, currentController->PlayerFollowWidgetClass);
		currentController->CharacterFollowWidget->AddToViewport();

		if (currentController->IsLocalPlayerController())
		{
			// Only show Hint UI and Weapon Energy UI
			if (currentController->CharacterFollowWidget)
			{
				currentController->CharacterFollowWidget->SetIsLocalControlledUI(true);
			}
		}
		else
		{
			if (currentController->CharacterFollowWidget)
			{
				currentController->CharacterFollowWidget->SetIsLocalControlledUI(false);
			}
		}
		
		currentController->SetFollowWidgetUIVisibility(ESlateVisibility::Visible);
	}
}

void AMPlayerController::NetMulticast_InitPlayerFollowWidget_Implementation(bool isVisible, bool needReset)
{
	// for(FConstPawnIterator iterator = GetWorld()->GetPawnIterator(); iterator; ++iterator)
	// {
	// 	AMCharacter* pawn = Cast<AMCharacter>(*iterator);
	//
	// 	if (pawn && !pawn->IsLocallyControlled())
	// 	{
	// 		pawn->SetGameUIVisibility(isVisible);
	// 	}
	// 	else if (pawn && pawn->IsLocallyControlled())
	// 	{
	// 		pawn->SetLocallyControlledGameUI(isVisible);
	// 	}
	// }

	if (GetLocalRole() != ROLE_Authority || GetNetMode() == NM_ListenServer)
	{
		SetFollowWidgetUIVisibility(isVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
        
		if (needReset)
		{
			// Reset the health bar
			UpdateFollowWidgetHealthBar(1);
		}
		
		// Open Current Player Status Widget
		if (MyInGameHUD && IsLocalPlayerController() && isVisible)
		{
			MyInGameHUD->StartGameUI();
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
			NetMulticast_InitPlayerFollowWidget(myGameState->IsGameStart, true);
		}
	}
}


void AMPlayerController::MoveForward(float Value)
{
	AMCharacter* const MyPawn = Cast<AMCharacter>(GetPawn());
	if (MyPawn && CanMove)
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
	if (MyPawn && CanMove)
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
	if(MyPawn && MyGameState && CanMove)
	{
		if (MyGameState->IsGameStart)
		{
			FVector WorldDirection = (HitLocation - MyPawn->GetActorLocation()).GetSafeNormal();
			FRotator WorldRotator = WorldDirection.Rotation();
			//MyPawn->AddMovementInput(WorldDirection, 1.f, false);
			SetControlRotation(WorldRotator);
		}
	}

	if (MyPawn && CharacterFollowWidget)
	{
		FVector2D ScreenLocation;
		FVector WorldLocation = MyPawn->GetActorLocation();
		APlayerController* PC = Cast<APlayerController>(this);
		if (PC != nullptr && PC->ProjectWorldLocationToScreen(WorldLocation, ScreenLocation))
		{
			CharacterFollowWidget->SetPositionInViewport(ScreenLocation);
		}
	}
}
#pragma endregion Constructor
