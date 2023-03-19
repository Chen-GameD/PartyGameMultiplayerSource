// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/MPlayerController.h"

#include "EngineUtils.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Camera/CameraComponent.h"
#include "Character/MCharacter.h"
#include "Kismet/GameplayStatics.h"

#include "GameBase/MGameInstance.h"
#include "GameBase/MGameMode.h"
#include "GameBase/MGameState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameStateBase.h"
#include "Matchmaking/EOSGameInstance.h"
#include "UI/MInGameHUD.h"


// Constructor
// ===================================================
#pragma region Constructor
AMPlayerController::AMPlayerController()
{
}

void AMPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMPlayerController, CanMove);

	//Replicate Action
}

// void AMPlayerController::NetMulticast_LoginInit_Implementation()
// {
// 	if (GetNetMode() == NM_ListenServer)
// 	{
// 		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::White, TEXT("ListenServer"));
// 	}
// 	else if (GetNetMode() == NM_Client)
// 	{
// 		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::White, TEXT("Client"));
// 	}
// 	if (IsLocalPlayerController())
// 	{
// 		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::White, TEXT("UpdateUI"));
// 		UI_UpdateLobbyMenu();
// 	}
// }

void AMPlayerController::JoinATeam_Implementation(int i_TeamIndex)
{
	AMGameMode* MyGameMode = Cast<AMGameMode>(GetWorld()->GetAuthGameMode());
	if (MyGameMode)
	{
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

void AMPlayerController::UI_InGame_OnUseSkill(SkillType UseSkill, float CoolDownTotalTime)
{
	if (MyInGameHUD)
	{
		MyInGameHUD->InGame_OnSkillUse(UseSkill, CoolDownTotalTime);
	}
}

AMInGameHUD* AMPlayerController::GetInGameHUD()
{
	return MyInGameHUD ? MyInGameHUD : Cast<AMInGameHUD>(GetHUD());
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
		MyInGameHUD->InGame_InitGameStatusAndPlayerStatusWidgetContent();
		
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

	// Update All Pawn's FollowWidget status
	for (TActorIterator<AMCharacter> PawnItr(GetWorld()); PawnItr; ++PawnItr)
	{
		AMCharacter* MyPawn = Cast<AMCharacter>(*PawnItr);
		if (MyPawn)
		{
			MyPawn->SetFollowWidgetVisibility(true);
		}
	}
}

void AMPlayerController::Server_RequestRespawn_Implementation()
{
	// Delete current controlled character
	//GetPawn()->Destroy();
	
	AMGameMode* myGameMode = Cast<AMGameMode>(GetWorld()->GetAuthGameMode());

	if (myGameMode)
	{
		myGameMode->Server_RespawnPlayer(this);
		// AMGameState* myGameState = Cast<AMGameState>(GetWorld()->GetGameState());
		// if (myGameState)
		// {
		// 	Client_SetGameUIVisibility(myGameState->IsGameStart);
		// }
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
	GetHitResultUnderCursor(ECC_GameTraceChannel1, true, Hit);
	HitLocation = Hit.Location;
	{
		FVector Start = HitLocation; // set the start point of the line
		FVector End = HitLocation + -FVector::UpVector*200; // set the end point of the line
		FColor Color = FColor::Red; // set the color of the line to red
		float Duration = 2.0f; // set the duration of the line to 2 seconds
		float Thickness = 5.0f; // set the thickness of the line to 5 units
		DrawDebugLine(GetWorld(), Start, End, Color, false, Duration, 0, Thickness);
	}
	
	FVector RotationVector;
	{
		FVector2D CursorPos;
		GetMousePosition(CursorPos.X, CursorPos.Y);
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("cursor: %f, %f"), CursorPos.X, CursorPos.Y));
	
		UGameViewportClient* GameViewport = GetWorld()->GetGameViewport();
		FVector2D ViewportSize;
		GameViewport->GetViewportSize(ViewportSize);
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("ViewportSize: %f, %f"), ViewportSize.X, ViewportSize.Y));
		FVector2D ScreenCenter = ViewportSize * 0.5;
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("ScreenCenter: %f, %f"), ScreenCenter.X, ScreenCenter.Y));

		FVector2D CursorCoordinate = CursorPos - ScreenCenter;
		CursorCoordinate.Y = -CursorCoordinate.Y;
		float theta = FMath::Atan(CursorCoordinate.Y / CursorCoordinate.X);
		//float theta_InDegrees = FMath::RadiansToDegrees(theta);
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("CursorCoordinate: %f, %f"), CursorCoordinate.X, CursorCoordinate.Y));
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("theta: %f"),  FMath::RadiansToDegrees(theta)));
		
		FVector Axis(0.0f, 0.0f, 1.0f);
		FQuat Rotation = FQuat(Axis, -theta);
		RotationVector = FVector((CursorCoordinate.X < 0 ? 1.0f : -1.0f), 0.0f, 0.0f);
		RotationVector = Rotation.RotateVector(RotationVector);
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Rotation: %f, %f, %f"), Rotation.X, Rotation.Y, Rotation.Z));
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("RotationVector: %f, %f, %f"), RotationVector.X, RotationVector.Y, RotationVector.Z));
		
		/*FVector2D ScreenLocation;
		bool bResult = UGameplayStatics::ProjectWorldToScreen(this, GetPawn()->GetActorLocation(), ScreenLocation);	*/

		UGameViewportClient* ViewportClient = Cast<ULocalPlayer>(this->Player)->ViewportClient;
	}	
    
	// Direct the Pawn towards that location
	APawn* const MyPawn = GetPawn();
	AMGameState* const MyGameState = Cast<AMGameState>(GetWorld()->GetGameState());
	if(MyPawn && MyGameState && CanMove)
	{
		if (MyGameState->IsGameStart)
		{
			FVector WorldDirection = (HitLocation - MyPawn->GetActorLocation()).GetSafeNormal();
			FRotator WorldRotator = WorldDirection.Rotation();
			//FRotator WorldRotator = RotationVector.Rotation();
			//MyPawn->AddMovementInput(WorldDirection, 1.f, false);
			SetControlRotation(WorldRotator);
		}
	}
}
#pragma endregion Constructor
