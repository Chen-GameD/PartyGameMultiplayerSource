// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/MPlayerController.h"

#include "EngineUtils.h"
#include "M_PlayerState.h"
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

void AMPlayerController::Destroyed()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("CHECKEND : Destroyed-PlayerController"));
	UE_LOG(LogTemp, Warning, TEXT("CHECKEND : DestroyedPlayerController"));
	Super::Destroyed();
}

void AMPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMPlayerController, CanMove);

	//Replicate Action
}

void AMPlayerController::UI_UpdateLobbyInformation()
{
	TArray<FLobbyInformationStruct> arrTeam1;
	TArray<FLobbyInformationStruct> arrTeam2;
	TArray<FLobbyInformationStruct> arrUndecided;

	AMGameState* MyGameState = Cast<AMGameState>(GetWorld()->GetGameState());
	if (MyGameState)
	{
		TArray<TObjectPtr<APlayerState>> PlayerArray = MyGameState->PlayerArray;
		for (TObjectPtr<APlayerState> CurrentPlayer : PlayerArray)
		{
			AM_PlayerState* CurrentPlayerState = Cast<AM_PlayerState>(CurrentPlayer);
			if (CurrentPlayerState)
			{
				FLobbyInformationStruct newStruct;
				newStruct.PlayerName = CurrentPlayerState->PlayerNameString;
				newStruct.TeamIndex = CurrentPlayerState->TeamIndex;
				newStruct.IsReady = CurrentPlayerState->IsReady;
				switch (newStruct.TeamIndex)
				{
				case 0:
					arrUndecided.Add(newStruct);
					break;
				case 1:
					arrTeam1.Add(newStruct);
					break;
				case 2:
					arrTeam2.Add(newStruct);
					break;
				default:
					break;
				}
			}
		}
	}
	
	GetWorldTimerManager().ClearTimer(UpdateLobbyTimerHandle);
	FTimerDelegate UpdateLobbyTimerDelegate;
	UpdateLobbyTimerDelegate.BindUObject(this, &AMPlayerController::Timer_CheckUpdateLobby, arrTeam1, arrTeam2, arrUndecided);
	GetWorldTimerManager().SetTimer(UpdateLobbyTimerHandle, UpdateLobbyTimerDelegate, 0.5, true);
}

void AMPlayerController::Timer_CheckUpdateLobby(TArray<FLobbyInformationStruct> arrTeam1, TArray<FLobbyInformationStruct> arrTeam2, TArray<FLobbyInformationStruct> arrUndecided)
{
	if (IsValid(MyInGameHUD))
	{
		MyInGameHUD->InGame_UpdateLobbyInformation(arrTeam1, arrTeam2, arrUndecided);
		GetWorldTimerManager().ClearTimer(UpdateLobbyTimerHandle);

		MyInGameHUD->InGame_UpdateReadyButtonState(GetPlayerState<AM_PlayerState>()->IsReady);
		AMGameState* MyGameState = Cast<AMGameState>(GetWorld()->GetGameState());
		if (MyGameState)
		{
			MyInGameHUD->InGame_UpdateHintPageInformation(MyGameState->LevelIndex);
		}

		// Update Equal State
		if (arrTeam1.Num() == arrTeam2.Num() && arrTeam1.Num() != 0)
		{
			MyInGameHUD->InGame_UpdateEqualConditionState(true);
		}
		else
		{
			MyInGameHUD->InGame_UpdateEqualConditionState(false);
		}

		// Update Ready State
		if (arrUndecided.Num() > 0)
		{
			MyInGameHUD->InGame_UpdateReadyConditionState(false);
		}
		else
		{
			bool isReady = true;
			for (int i = 0; i < arrTeam1.Num(); i++)
			{
				if (!arrTeam1[i].IsReady)
				{
					isReady = false;
					break;
				}
			}
			if (isReady)
			{
				for (int i = 0; i < arrTeam2.Num(); i++)
				{
					if (!arrTeam2[i].IsReady)
					{
						isReady = false;
						break;
					}
				}
			}
			MyInGameHUD->InGame_UpdateReadyConditionState(isReady);
		}
	}
}

void AMPlayerController::Timer_CheckPlayerState()
{
	AM_PlayerState* MyPlayerstate = GetPlayerState<AM_PlayerState>();
	if (MyPlayerstate)
	{
		UI_UpdateLobbyInformation();
		GetWorldTimerManager().ClearTimer(UpdatePlayerStateHandle);
	}
}

void AMPlayerController::Client_SyncCharacters_Implementation()
{
	for (TActorIterator<AMCharacter> PawnItr(GetWorld()); PawnItr; ++PawnItr)
	{
		AMCharacter* CurrentPawn = *PawnItr;
		if (CurrentPawn)
		{
			CurrentPawn->SetPlayerNameUIInformation();
			CurrentPawn->SetPlayerSkin();
			CurrentPawn->InitFollowWidget();
		}
	}
}

void AMPlayerController::Client_SyncLobbyInformation_Implementation()
{
	AM_PlayerState* MyPlayerstate = GetPlayerState<AM_PlayerState>();
	if (MyPlayerstate)
	{
		UI_UpdateLobbyInformation();
	}
	else
	{
		GetWorldTimerManager().ClearTimer(UpdatePlayerStateHandle);
		FTimerDelegate UpdatePlayerStateDelegate;
		UpdatePlayerStateDelegate.BindUObject(this, &AMPlayerController::Timer_CheckPlayerState);
		GetWorldTimerManager().SetTimer(UpdatePlayerStateHandle, UpdatePlayerStateDelegate, 0.5, true);
	}
}

void AMPlayerController::JoinATeam_Implementation(int i_TeamIndex)
{
	AMGameMode* MyGameMode = Cast<AMGameMode>(GetWorld()->GetAuthGameMode());
	if (MyGameMode)
	{
		GetPlayerState<AM_PlayerState>()->Server_UpdateTeamIndex(i_TeamIndex);
	}
}

void AMPlayerController::Server_ReadyButtonClick_Implementation()
{
	AM_PlayerState* MyServerPlayerState = GetPlayerState<AM_PlayerState>();

	MyServerPlayerState->Server_UpdatePlayerReadyState();

	AMGameMode* MyGameMode = Cast<AMGameMode>(GetWorld()->GetAuthGameMode());
	if (MyGameMode)
	{
		MyGameMode->CheckGameStart();
	}
}

void AMPlayerController::Server_SetCanMove_Implementation(bool i_CanMove)
{
	CanMove = i_CanMove;
}

void AMPlayerController::UI_InGame_UpdateHealth(float percentage)
{
	if (IsValid(MyInGameHUD))
	{
		MyInGameHUD->InGame_UpdatePlayerHealth(percentage);
	}
}

void AMPlayerController::UI_InGame_OnUseSkill(SkillType UseSkill, float CoolDownTotalTime)
{
	if (IsValid(MyInGameHUD))
	{
		MyInGameHUD->InGame_OnSkillUse(UseSkill, CoolDownTotalTime);
	}
}

void AMPlayerController::UI_InGame_BroadcastInformation_Implementation(int KillerTeamIndex, int DeceasedTeamIndex, const FString& i_KillerName, const FString& i_DeceasedName, UTexture2D* i_WeaponImage)
{
	if (IsValid(MyInGameHUD))
	{
		MyInGameHUD->InGame_BroadcastInformation(KillerTeamIndex, DeceasedTeamIndex, i_KillerName, i_DeceasedName, i_WeaponImage);
	}
}

void AMPlayerController::UI_InGame_BroadcastMiniInformation_Implementation(int KillerTeamIndex, const FString& i_KillerName, const FString& i_MinigameInformation)
{
	if (IsValid(MyInGameHUD))
	{
		MyInGameHUD->InGame_BroadcastMinigameInformation(KillerTeamIndex, i_KillerName, i_MinigameInformation);
	}
}

AMInGameHUD* AMPlayerController::GetInGameHUD()
{
	return IsValid(MyInGameHUD) ? MyInGameHUD : Cast<AMInGameHUD>(GetHUD());
}

void AMPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalPlayerController())
	{
		MyInGameHUD = Cast<AMInGameHUD>(GetHUD());

		if (IsValid(MyInGameHUD))
		{
			// Set input mode
			if (IsLocalPlayerController())
			{
				//FInputModeUIOnly inputMode;
				FInputModeUIOnly inputMode;
				inputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
				this->SetInputMode(inputMode);
				this->SetShowMouseCursor(true);
			}
		}
	}
}

void AMPlayerController::OnNetCleanup(UNetConnection* Connection)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("CHECKEND : OnNetCleanup"));
	UE_LOG(LogTemp, Warning, TEXT("CHECKEND : OnNetCleanup"));
	if(IsLocalPlayerController() && GetNetMode() == NM_Client)
	{
		UEOSGameInstance* GameInstanceRef = Cast<UEOSGameInstance>(GetWorld()->GetGameInstance());
		if(GameInstanceRef)
		{
			GameInstanceRef->DestroySession();
		}
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
	if (IsValid(MyInGameHUD))
	{
		// Hide the lobby menu
		MyInGameHUD->InGame_SetVisibilityLobbyWidget(ESlateVisibility::Hidden);
		
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
		// Update Player Direction Indicator
		AMCharacter* MyCharacter = Cast<AMCharacter>(GetPawn());
		if (MyCharacter)
		{
			MyCharacter->BPF_SetPlayerDirectionIndicatorWidget(MyPlayerState->TeamIndex);
		}
		
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
	AMGameMode* myGameMode = Cast<AMGameMode>(GetWorld()->GetAuthGameMode());

	if (myGameMode)
	{
		myGameMode->Server_RespawnPlayer(this);
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
		AMCharacter* const MyPawn = Cast<AMCharacter>(GetPawn());
		AMGameState* const MyGameState = Cast<AMGameState>(GetWorld()->GetGameState());
		if (MyPawn && !MyPawn->GetIsDead() && MyGameState && CanMove)
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
