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

void AMPlayerController::UI_InGame_BroadcastInformation_Implementation(int KillerTeamIndex, int DeceasedTeamIndex, const FString& i_KillerName, const FString& i_DeceasedName, UTexture2D* i_WeaponImage)
{
	if (MyInGameHUD)
	{
		MyInGameHUD->InGame_BroadcastInformation(KillerTeamIndex, DeceasedTeamIndex, i_KillerName, i_DeceasedName, i_WeaponImage);
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

	AM_PlayerState* MyPlayerState = GetPlayerState<AM_PlayerState>();
	if (MyPlayerState)
	{
		// Update All Pawn's FollowWidget status
		for (TActorIterator<AMCharacter> PawnItr(GetWorld()); PawnItr; ++PawnItr)
		{
			AMCharacter* MyPawn = Cast<AMCharacter>(*PawnItr);
			AM_PlayerState* CurrentPawnPlayerState = Cast<AM_PlayerState>(MyPawn->GetPlayerState());
			if (MyPawn && CurrentPawnPlayerState)
			{
				MyPawn->SetFollowWidgetVisibility(true);
				MyPawn->SetFollowWidgetHealthBarIsEnemy(MyPlayerState->TeamIndex != CurrentPawnPlayerState->TeamIndex);
			}
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
	
    // Rotate the character by mouse
    FHitResult Hit;	
	bool successHit = GetHitResultUnderCursor(ECC_GameTraceChannel1, false, Hit); // ECC_GameTraceChannel1 is Cursor; Only Character's CursorHitPlance would block this channel
	if (successHit)
	{
		FVector HitLocation = Hit.Location;
		//// Draw Debug Line
		//{
		//	FVector Start = HitLocation;
		//	FVector End = HitLocation + -FVector::UpVector*200;
		//	FColor Color = FColor::Red;
		//	float Duration = 2.0f;
		//	float Thickness = 5.0f;
		//	DrawDebugLine(GetWorld(), Start, End, Color, false, Duration, 0, Thickness);
		//}	
		APawn* const MyPawn = GetPawn();
		AMGameState* const MyGameState = Cast<AMGameState>(GetWorld()->GetGameState());
		if (MyPawn && MyGameState && CanMove)
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
	//else if(GetNetMode() != NM_ListenServer)
	//{
	//	GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Red, TEXT("GetHitResult Failed in AMPlayerController::PlayerTick!"));
	//}
}
#pragma endregion Constructor
