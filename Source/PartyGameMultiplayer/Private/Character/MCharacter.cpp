// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/MCharacter.h"

//#include "SAdvancedTransformInputBox.h"
#include <filesystem>

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Weapon/BaseProjectile.h"
#include "Weapon/WeaponConfig.h"
#include "Weapon/DamageManager.h"
#include "Weapon/WeaponDataHelper.h"
#include "Weapon/ElementWeapon/WeaponShell.h"
#include "WaterBodyActor.h"
#include "LevelInteraction/SaltcureArea.h"
#include "LevelInteraction/MinigameObject/MinigameObj_Statue.h"
#include "Character/animUtils.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "M_PlayerState.h"

#include "EngineUtils.h"
#include "Character/MPlayerController.h"
#include "Character/CursorHitPlane.h"
#include "Components/WidgetComponent.h"
#include "GameBase/MGameState.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "UI/MCharacterFollowWidget.h"
#include "UI/PlayerUI/OpponentMarkerWidget.h"

// Constructor
// ===================================================
#pragma region Constructor
AMCharacter::AMCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Initialize the player's Health
	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rate for input
	TurnRateGamePad = 50.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 1500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true); // Don't want arm to rotate when character does
	CameraBoom->TargetArmLength = 850.0f; // The camera follows at this distance behind the character
	Local_MinCameraArmLength = Local_CurCameraArmLength = CameraBoom->TargetArmLength;
	Local_MaxCameraArmLength = 1100.0f;
	CameraBoom->SetRelativeRotation(FRotator(60.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
	CurFov = MinFov = 90.0f;
	MaxFov = 108.0f;
	FollowCameraRelativeRotationVector = FVector(0, 0, 0);
	FollowCameraOriginalRelativeLocation = FollowCamera->GetRelativeLocation();
	CameraShakingTime = 0;
	CameraShakingIntensity = 0;
	TimePassedSinceShaking = 0;

	//Create HealthBar_Enemy UI Widget
	PlayerFollowWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("FollowWidget"));
	PlayerFollowWidget->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	OpponentMarkerWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OpponentMarkerWidget"));
	OpponentMarkerWidget->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	// Collision Event
	GetCapsuleComponent()->SetGenerateOverlapEvents(true);
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &AMCharacter::OnWeaponOverlapBegin);
	GetCapsuleComponent()->OnComponentEndOverlap.AddDynamic(this, &AMCharacter::OnWeaponOverlapEnd);

	IsDead = false;
	Server_DeadTimes = 0;

	IsAllowDash = true;
	DashDistance = 300.0f;
	DashTime = 0.2f;
	DashCoolDown = 5.0f;

	//MeshRotation = GetMesh()->GetRelativeRotation();

	Client_MaxHeightDuringLastTimeOffGround = TNumericLimits<float>::Min();
	Client_LowSpeedWalkAccumulateTime = 0;

	HealingBubbleCollider = CreateDefaultSubobject<USphereComponent>(TEXT("HealingBubbleCollider"));
	HealingBubbleCollider->SetupAttachment(RootComponent);

	EffectHealingBubbleOn = CreateDefaultSubobject<UNiagaraComponent>(TEXT("EffectHealingBubbleOn"));
	EffectHealingBubbleOn->SetupAttachment(HealingBubbleCollider);
	EffectHealingBubbleOn->bAutoActivate = false;

	bHealingBubbleOn = false;
	bDoubleHealingBubbleSize = false;
	bHealingBubbleTouchingStatue = false;
	
	EffectBubbleStart = CreateDefaultSubobject<UNiagaraComponent>(TEXT("EffectBubbleStart"));
	EffectBubbleStart->SetupAttachment(RootComponent);
	EffectBubbleStart->bAutoActivate = false;

	EffectBubbleOn = CreateDefaultSubobject<UNiagaraComponent>(TEXT("EffectBubbleOn"));
	EffectBubbleOn->SetupAttachment(RootComponent);
	EffectBubbleOn->bAutoActivate = false;

	EffectBubbleEnd = CreateDefaultSubobject<UNiagaraComponent>(TEXT("EffectBubbleEnd"));
	EffectBubbleEnd->SetupAttachment(RootComponent);
	EffectBubbleEnd->bAutoActivate = false;

	IsHealing = false;
	IsSaltCure = false;
	EffectHeal = CreateDefaultSubobject<UNiagaraComponent>(TEXT("EffectHeal"));
	EffectHeal->SetupAttachment(RootComponent);
	EffectHeal->bAutoActivate = false;

	EffectGetHit = CreateDefaultSubobject<UNiagaraComponent>(TEXT("EffectGetHit"));
	EffectGetHit->SetupAttachment(RootComponent);
	EffectGetHit->bAutoActivate = false;

	IsBurned = false;
	EffectBurn = CreateDefaultSubobject<UNiagaraComponent>(TEXT("EffectBurn"));
	EffectBurn->SetupAttachment(RootComponent);
	EffectBurn->bAutoActivate = false;

	EffectRun = CreateDefaultSubobject<UNiagaraComponent>(TEXT("EffectRun"));
	EffectRun->SetupAttachment(RootComponent);
	EffectRun->bAutoActivate = false;

	EffectDash = CreateDefaultSubobject<UNiagaraComponent>(TEXT("EffectDash"));
	EffectDash->SetupAttachment(RootComponent);
	EffectDash->bAutoActivate = false;

	EffectJump = CreateDefaultSubobject<UNiagaraComponent>(TEXT("EffectJump"));
	EffectJump->SetupAttachment(RootComponent);
	EffectJump->bAutoActivate = false;

	EffectLand = CreateDefaultSubobject<UNiagaraComponent>(TEXT("EffectLand"));
	EffectLand->SetupAttachment(RootComponent);
	EffectLand->bAutoActivate = false;

	Server_TheControllerApplyingLatestBurningBuff = nullptr;

	Server_CanMove = true;

	Server_CallGetHitSfxVfx_MinInterval = 0.5f;
	Server_LastTime_CallGetHitSfxVfx = -1.0f;

	IsInvincible = false;
	InvincibleTimer = 0;
	InvincibleMaxTime = 15.0f;

	// Anti-Cheat lol: prevent use double Q/E to refresh the Combine Weapon CD
	Server_LastDitchCombineWeaponType = EnumWeaponType::None;
	Server_LastDitchCombineWeaponTime = -1.0f;
	Server_LastDitchCombineWeapon_CD_LeftEnergy = 0.0f;
}

void AMCharacter::Restart()
{
	Super::Restart();

	if (IsLocallyControlled())
	{
		if(AMPlayerController* playerController = Cast<AMPlayerController>(Controller))
			playerController->UI_InGame_UpdateHealth(CurrentHealth / MaxHealth);
	}
}

void AMCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	GetWorldTimerManager().SetTimer(InitPlayerInformationTimer, this, &AMCharacter::CheckPlayerFollowWidgetTick, 0.5, true);
}

void AMCharacter::CheckPlayerFollowWidgetTick()
{
	if (PlayerFollowWidget->GetUserWidgetObject())
	{
		SetPlayerNameUIInformation();
		SetPlayerSkin();
		InitFollowWidget();
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("ListenServer pawn's PlayerFollowWdiget is ready!"));
		GetWorldTimerManager().ClearTimer(InitPlayerInformationTimer);

		for (TActorIterator<AMPlayerController> ControllerItr(GetWorld()); ControllerItr; ++ControllerItr)
		{
			AMPlayerController* MyController = Cast<AMPlayerController>(*ControllerItr);
			if (MyController && MyController->IsLocalPlayerController())
			{
				MyController->Client_SyncLobbyInformation_Implementation();
			}
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("ListenServer pawn's PlayerFollowWdiget is not ready!"));
	}
}

#pragma endregion Constructor

// Replicated Properties
// =======================================================
#pragma region Replicated Properties
void AMCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Replicate current health
	DOREPLIFETIME(AMCharacter, CurrentHealth);
	DOREPLIFETIME(AMCharacter, IsDead);

	//Replicate Action
	DOREPLIFETIME(AMCharacter, IsAllowDash);

	//DOREPLIFETIME(AMCharacter, MeshRotation);

	//Replicate weapons information
	DOREPLIFETIME(AMCharacter, LeftWeapon);
	DOREPLIFETIME(AMCharacter, RightWeapon);
	DOREPLIFETIME(AMCharacter, CombineWeapon);

	//Replicate vfx status
	DOREPLIFETIME(AMCharacter, IsHealing);
	DOREPLIFETIME(AMCharacter, IsBurned);
	DOREPLIFETIME(AMCharacter, IsParalyzed);
	DOREPLIFETIME(AMCharacter, IsInvincible);
	//DOREPLIFETIME(AMCharacter, bHealingBubbleTouchingStatue);

	//Replicate animation var
	DOREPLIFETIME(AMCharacter, isAttacking);
	DOREPLIFETIME(AMCharacter, isHoldingCombineWeapon);
	DOREPLIFETIME(AMCharacter, isLeftHeld);
	DOREPLIFETIME(AMCharacter, isRightHeld);
	DOREPLIFETIME(AMCharacter, isDualShootingWeaponHeld);
	DOREPLIFETIME(AMCharacter, isSingleShootingWeaponHeld);
	DOREPLIFETIME(AMCharacter, isFlamethrowerHeld);

	// Replicate AnimState
	DOREPLIFETIME(AMCharacter, AnimState);
	// Replicate SKD Array
	DOREPLIFETIME(AMCharacter, SKDArray);
	// Replicate death animation index
	DOREPLIFETIME(AMCharacter, DeadAnimIndex);
}
#pragma endregion Replicated Properties

// Input
// ====================================================
#pragma region Input
void AMCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	//PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	//PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &AMCharacter::Client_Attack);
	PlayerInputComponent->BindAction<FIsMeleeRelease>("Attack", IE_Released, this, &AMCharacter::StopAttack, false);

	PlayerInputComponent->BindAction<FPickUpDelegate>("PickUpLeft", IE_Pressed, this, &AMCharacter::PickUp, true);
	PlayerInputComponent->BindAction<FPickUpDelegate>("PickUpRight", IE_Pressed, this, &AMCharacter::PickUp, false);

	PlayerInputComponent->BindAction("Dash", IE_Pressed, this, &AMCharacter::Dash);

	PlayerInputComponent->BindAction<FIsZooming>("Zoom", IE_Pressed, this, &AMCharacter::Zoom, true);
	PlayerInputComponent->BindAction<FIsZooming>("Zoom", IE_Released, this, &AMCharacter::Zoom, false);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AMCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AMCharacter::TouchStopped);

	// Test
	// ==============================
	//PlayerInputComponent->BindAction("TestKey", IE_Released, this, &AMCharacter::Test);
}

void AMCharacter::SetTextureInUI()
{
	if (IsLocallyControlled() || GetNetMode() == NM_ListenServer)
	{
		if (InventoryMenuWidget)
		{
			InventoryMenuWidget->SetLeftItemUI(LeftWeapon == nullptr ? nullptr : LeftWeapon->HoldingTextureUI_Q);
			InventoryMenuWidget->SetRightItemUI(RightWeapon == nullptr ? nullptr : RightWeapon->HoldingTextureUI_E);
			InventoryMenuWidget->SetWeaponUI(CombineWeapon == nullptr ? nullptr : CombineWeapon->HoldingTextureUI_Q);
		}
	}
}

void AMCharacter::Client_Attack()
{
	float TargetDistance = 0.0f;
	if (auto pPlayerController = Cast<APlayerController>(GetController()))
	{
		FHitResult Hit;
		bool successHit = pPlayerController->GetHitResultUnderCursor(ECC_GameTraceChannel1, false, Hit);
		if (successHit)
		{
			auto pMCharacter = Cast<AMCharacter>(GetInstigator());
			if (pMCharacter)
			{
				TargetDistance = FVector::Distance(Hit.Location, GetActorLocation());
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("distance: %f"), TargetDistance));
			}
		}
	}

	Attack(TargetDistance);
}

void AMCharacter::Attack_Implementation(float AttackTargetDistance)
{
	if ((LeftWeapon || RightWeapon || CombineWeapon) && !IsDead)
	{
		// Can't Attack during Paralysis
		if (CheckBuffMap(EnumAttackBuff::Paralysis) && 1.0f <= BuffMap[EnumAttackBuff::Paralysis][0])
			return;
		// Can't Attack when holding only shells
		bool HasWeaponOtherThanShells = false;
		if ((LeftWeapon && LeftWeapon->WeaponType != EnumWeaponType::Shell) ||
			(RightWeapon && RightWeapon->WeaponType != EnumWeaponType::Shell) ||
			CombineWeapon)
			HasWeaponOtherThanShells = true;
		if (!HasWeaponOtherThanShells)
			return;

		FString AttackMessage = FString::Printf(TEXT("You Start Attack."));
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, AttackMessage);

		if (CombineWeapon)
		{
			CombineWeapon->AttackStart(AttackTargetDistance);
		}
		else
		{
			if (LeftWeapon)
			{
				LeftWeapon->AttackStart(AttackTargetDistance);
			}
			if (RightWeapon)
			{
				RightWeapon->AttackStart(AttackTargetDistance);
			}
		}

		// Set isAttack to True
		this->AnimState[7] = true;
	}
}

void AMCharacter::StopAttack_Implementation(bool isMeleeRelease)
{
	if (this->AnimState[0] || this->AnimState[2])
	{
		if (isMeleeRelease)
		{
			// stop attack
			if ((LeftWeapon || RightWeapon || CombineWeapon) && !IsDead)
			{
				FString AttackMessage = FString::Printf(TEXT("You Stop Attack."));
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, AttackMessage);

				if (CombineWeapon)
				{
					CombineWeapon->AttackStop();
				}
				else
				{
					if (LeftWeapon)
					{
						LeftWeapon->AttackStop();
					}
					if (RightWeapon)
					{
						RightWeapon->AttackStop();
					}
				}

				// Set isAttack to False
				this->AnimState[7] = false;
			}
		}
	}
	else
	{
		if ((LeftWeapon || RightWeapon || CombineWeapon) && !IsDead)
		{
			FString AttackMessage = FString::Printf(TEXT("You Stop Attack."));
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, AttackMessage);

			if (CombineWeapon)
			{
				CombineWeapon->AttackStop();
			}
			else
			{
				if (LeftWeapon)
				{
					LeftWeapon->AttackStop();
				}
				if (RightWeapon)
				{
					RightWeapon->AttackStop();
				}
			}

			// Set isAttack to False
			this->AnimState[7] = false;
		}
	}
}

float AMCharacter::Server_GetMaxWalkSpeedRatioByWeapons()
{
	FString ParName = "";
	int CntShellHeld = 0;
	bool HoldingBigWeapon = false;
	if (CombineWeapon)
		ParName = CombineWeapon->GetWeaponName();
	else
	{
		if (LeftWeapon)
		{
			ParName = LeftWeapon->GetWeaponName();
			if (LeftWeapon->WeaponType == EnumWeaponType::Shell)
				CntShellHeld++;
			if (LeftWeapon->IsBigWeapon)
				HoldingBigWeapon = true;
		}
		if (RightWeapon)
		{
			ParName = RightWeapon->GetWeaponName();
			if (RightWeapon->WeaponType == EnumWeaponType::Shell)
				CntShellHeld++;
			if (RightWeapon->IsBigWeapon)
				HoldingBigWeapon = true;
		}
	}
	if(HoldingBigWeapon)
		ParName = "BigWeapon";
	else if (1 == CntShellHeld)
		ParName = "OneShell";
	else if (2 == CntShellHeld)
		ParName = "TwoShells";
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, ParName);
	float MaxWalkSpeedRatio = 1.0f;
	if (AWeaponDataHelper::DamageManagerDataAsset->Character_MaxWalkSpeed_Map.Contains(ParName))
		MaxWalkSpeedRatio = AWeaponDataHelper::DamageManagerDataAsset->Character_MaxWalkSpeed_Map[ParName];
	return MaxWalkSpeedRatio;
}


void AMCharacter::NetMulticast_AdjustMaxWalkSpeed_Implementation(float MaxWalkSpeedRatio)
{	
	Server_MaxWalkSpeed = OriginalMaxWalkSpeed * MaxWalkSpeedRatio;
	GetCharacterMovement()->MaxWalkSpeed = Server_MaxWalkSpeed;
}

void AMCharacter::Server_SetMaxWalkSpeed()
{
	float MaxWalkSpeedRatio = 1.0f;
	float MaxWalkSpeedRatioByWeapon = 1.0f;
	float MaxWalkSpeedRatioBySaltCure = 1.0f;
	float MaxWalkSpeedRatioByCrabBubble = 1.0f;
	// Weapon Effect
	MaxWalkSpeedRatioByWeapon = Server_GetMaxWalkSpeedRatioByWeapons();
	MaxWalkSpeedRatio = MaxWalkSpeedRatioByWeapon;
	// Sea Effect
	if (IsSaltCure)
	{
		FString ParName = "Saltcure";
		if (AWeaponDataHelper::DamageManagerDataAsset->Character_MaxWalkSpeed_Map.Contains(ParName))
			MaxWalkSpeedRatioBySaltCure = AWeaponDataHelper::DamageManagerDataAsset->Character_MaxWalkSpeed_Map[ParName];
		MaxWalkSpeedRatio = FMath::Min(MaxWalkSpeedRatio, MaxWalkSpeedRatioBySaltCure);
	}
	// Crab Bubble Effect
	if (IsAffectedByCrabBubble)
	{
		FString ParName = "CrabBubble";
		if (AWeaponDataHelper::DamageManagerDataAsset->Character_MaxWalkSpeed_Map.Contains(ParName))
			MaxWalkSpeedRatioByCrabBubble = AWeaponDataHelper::DamageManagerDataAsset->Character_MaxWalkSpeed_Map[ParName];
		MaxWalkSpeedRatio = FMath::Min(MaxWalkSpeedRatio, MaxWalkSpeedRatioByCrabBubble);
	}
	
	NetMulticast_AdjustMaxWalkSpeed(MaxWalkSpeedRatio);
}

void AMCharacter::NetMulticast_SetHealingBubbleStatus_Implementation(bool i_bBubbleOn, bool i_bDoubleSize)
{
	if (bHealingBubbleOn != i_bBubbleOn)
	{
		bHealingBubbleOn = i_bBubbleOn;
		EnablebHealingBubble(i_bBubbleOn);
	}
	if (bDoubleHealingBubbleSize != i_bDoubleSize)
	{
		bDoubleHealingBubbleSize = i_bDoubleSize;
	}	
}

void AMCharacter::EnablebHealingBubble(bool bEnable)
{
	if (bEnable)
	{
		//// Vfx
		//if (EffectHealingBubbleStart)
		//{
		//	EffectHealingBubbleStart->Activate();
		//}

		if(!EffectHealingBubbleOn->IsActive())
			EffectHealingBubbleOn->Activate();
		EffectHealingBubbleOn->SetVisibility(true);
		HealingBubbleCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	else
	{
		//EffectHealingBubbleOn->Deactivate();

		EffectHealingBubbleOn->SetVisibility(false);
		HealingBubbleCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AMCharacter::PickUp_Implementation(bool isLeft)
{
	// ==========================================================================================================================
	// RPC function when client do weapon PickUp, only call on server.
	// server will receive this function call
	// in this function, we need to deal with the following:
	// 1. Detect which weapon has been pickup. Before attaching the weapon to the character, we need to
	//    detect whether it should combine or not. If the weapon needs to combine, call the combine function and save
	//    weapon information in the right place. Generate the weapon, destroy or hide two base weapons.
	// 2. attach the weapon to the character. Because the weapon is replicated, the transform
	//    information will sync to every client;
	// 3. The weapon has functions like self-rotate and needs to close when picked up.
	//	  The weapon should have a bool that is replicated and also should have a notify function.
	//    Change that bool. It will automatically call the notify function at every client. (This function is dealing
	//    with when the weapon pickup or drop off, it will open or close some function like self-rotate and physics information)
	// 4. Now we are sure that the weapon was picked up by a player, we need to notify every client to change that
	//	  player's animation. The animation variable should be replicated, and changing its value will replicate
	//    to every client.
	// ===========================================================================================================================
	// TO DO

	if (IsParalyzed)
		return;

	// Drop left/right weapon
	if (CurrentTouchedWeapon.IsEmpty())
	{
		// Check if should drop weapon
		if (isLeft)
		{
			if (LeftWeapon)
			{
				// Drop off combine weapon
				if (CombineWeapon)
				{
					PreventRefreshingCombineWeaponCD_ByDropPick(CombineWeapon);
					CombineWeapon->SafeDestroyWhenGetThrew();
					CombineWeapon = nullptr;
				}
				DropOffWeapon(isLeft);
			}
		}
		else
		{
			if (RightWeapon)
			{
				RightWeapon->NetMulticast_CallThrewAwaySfx();
				// Drop off combine weapon
				if (CombineWeapon)
				{
					PreventRefreshingCombineWeaponCD_ByDropPick(CombineWeapon);
					CombineWeapon->SafeDestroyWhenGetThrew();
					CombineWeapon = nullptr;
				}
				DropOffWeapon(isLeft);
			}
		}
	}
	// Pickup left/right weapon
	else if (!IsDead)
	{
		IsPickingWeapon = true;
		if (isLeft)
		{
			// PickUp Left
			// to do
			FString InputMessage = FString::Printf(TEXT("You Pick Up Left."));
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, InputMessage);

			if (LeftWeapon)
			{
				// Drop off left weapon
				DropOffWeapon(isLeft, true);
			}

			if(!CurrentTouchedWeapon.IsEmpty() && CurrentTouchedWeapon[0])
				LeftWeapon = CurrentTouchedWeapon[0];
			//SetTextureInUI(Left, LeftWeapon->HoldingTextureUI);
			FName SocketName = WeaponConfig::GetInstance()->GetWeaponSocketName(LeftWeapon->GetWeaponName());
			FString temp = SocketName.ToString();
			if (LeftWeapon->IsBigWeapon)
				temp = "Big" + temp;
			temp += "_Left";
			SocketName = FName(*temp);
			LeftWeapon->GetPickedUp(this);
			LeftWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, SocketName);

			if (CurrentTouchedWeapon.Contains(LeftWeapon))
				CurrentTouchedWeapon.Remove(LeftWeapon);

			// Set isLeftHeld Anim State
			this->AnimState[5] = true;

			// Pick up left big weapon and drop right weapon and/or combine weapon
			if (LeftWeapon->IsBigWeapon)
			{
				DropOffWeapon(false, true);
				// Drop off combine weapon
				if (CombineWeapon)
				{
					PreventRefreshingCombineWeaponCD_ByDropPick(CombineWeapon);
					CombineWeapon->SafeDestroyWhenGetThrew();
					CombineWeapon = nullptr;
				}
				LeftWeapon->NetMulticast_CallPickedUpSfx();
			}
			// Pick up left regular weapon
			else
			{
				// Check if need combine
				if (RightWeapon)
				{
					if (RightWeapon->IsBigWeapon)
					{
						DropOffWeapon(false, true);
					}
					else
					{
						// Set isRightHeld Anim State
						this->AnimState[6] = true;
						OnCombineWeapon(isLeft);
					}
				}
				// no combining, pick up this weapon alone
				else
				{
					LeftWeapon->NetMulticast_CallPickedUpSfx();
				}
			}

			// Update Weapon Type Anim State
			AnimUtils::updateAnimStateWeaponType(AnimState, CombineWeapon, LeftWeapon, RightWeapon);
		}
		else
		{
			// PickUp Right
			// to do
			FString InputMessage = FString::Printf(TEXT("You Pick Up Right."));
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, InputMessage);

			if (RightWeapon)
			{
				// Drop off left weapon
				DropOffWeapon(isLeft, true);
			}

			if (!CurrentTouchedWeapon.IsEmpty() && CurrentTouchedWeapon[0])
				RightWeapon = CurrentTouchedWeapon[0];
			//SetTextureInUI(Right, RightWeapon->HoldingTextureUI);
			FName SocketName = WeaponConfig::GetInstance()->GetWeaponSocketName(RightWeapon->GetWeaponName());
			FString temp = SocketName.ToString();
			temp += "_Right";
			if (RightWeapon->IsBigWeapon)
				temp = "Big" + temp;
			SocketName = FName(*temp);
			RightWeapon->GetPickedUp(this);
			RightWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, SocketName);

			if (CurrentTouchedWeapon.Contains(RightWeapon))
				CurrentTouchedWeapon.Remove(RightWeapon);

			// Set isRightHeld Anim State
			this->AnimState[6] = true;

			// Pick up right big weapon and drop left weapon and/or combine weapon
			if (RightWeapon->IsBigWeapon)
			{
				DropOffWeapon(true, true);
				// Drop off combine weapon
				if (CombineWeapon)
				{
					PreventRefreshingCombineWeaponCD_ByDropPick(CombineWeapon);
					CombineWeapon->SafeDestroyWhenGetThrew();
					CombineWeapon = nullptr;
				}
				RightWeapon->NetMulticast_CallPickedUpSfx();
			}
			// Pick up right regular weapon
			else
			{
				// Check if need combine
				if (LeftWeapon)
				{
					if (LeftWeapon->IsBigWeapon)
					{
						DropOffWeapon(true, true);
					}
					else
					{
						// Set isLeftHeld Anim State
						this->AnimState[5] = true;
						OnCombineWeapon(isLeft);
					}
				}
				// no combining, pick up this weapon alone
				else
				{
					RightWeapon->NetMulticast_CallPickedUpSfx();
				}
			}

			// Update Weapon Type Anim State
			AnimUtils::updateAnimStateWeaponType(AnimState, CombineWeapon, LeftWeapon, RightWeapon);
		}
	}


	IsPickingWeapon = false;

	// Invincible Status
	if (LeftWeapon || RightWeapon || CombineWeapon)
	{
		if (IsInvincible)
		{
			IsInvincible = false;
			if (GetNetMode() == NM_ListenServer)
				OnRep_IsInvincible();
			InvincibleTimer = 0;
		}
	}

	// UI
	// ======================================================================
	// Set Inventory UI
	if (GetNetMode() == NM_ListenServer)
		SetTextureInUI();
	// Set Tip Weapon UI
	if (CurrentTouchedWeapon.Num() > 0)
	{
		if (!IsDead)
			SetTipUI(true, CurrentTouchedWeapon[0]);
		else
			SetTipUI(false);
	}

	// Adjust the walking speed according to the holding weapon
	Server_SetMaxWalkSpeed();

	// Check Shellheal buff if holding any shells
	Server_CheckBubble();
}

void AMCharacter::DropOffWeapon(bool isLeft, bool bDropToReplace)
{
	if (isLeft && LeftWeapon)
	{
		// Sfx
		if (!bDropToReplace)
			LeftWeapon->NetMulticast_CallThrewAwaySfx();

		LeftWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		// Call detach function in ABaseWeapon
		// TO DO
		LeftWeapon->SetActorHiddenInGame(false);
		LeftWeapon->GetThrewAway();

		// Set the weapon pointer to nullptr
		LeftWeapon = nullptr;

		// Set isAttack to False
		this->AnimState[7] = false;
		// Set isLeftHeld Anim State
		this->AnimState[5] = false;
		// Need to update weapon type for anim

		isFlamethrowerHeld = false;

		if (RightWeapon)
		{
			RightWeapon->SetActorHiddenInGame(false);
			this->AnimState[6] = true;
		}

		AnimUtils::updateAnimStateWeaponType(AnimState, CombineWeapon, LeftWeapon, RightWeapon);

		if (RightWeapon)
		{
			RightWeapon->HasBeenCombined = false;
		}
	}
	else if (!isLeft && RightWeapon)
	{
		// Sfx
		if (!bDropToReplace)
			RightWeapon->NetMulticast_CallThrewAwaySfx();

		RightWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		// Call detach function in ABaseWeapon
		// TO DO
		RightWeapon->SetActorHiddenInGame(false);
		RightWeapon->GetThrewAway();

		// Set the weapon pointer to nullptr
		RightWeapon = nullptr;

		// Set isAttack to False
		this->AnimState[7] = false;
		// Set isRightHeld Anim State
		this->AnimState[6] = false;

		isFlamethrowerHeld = false;

		if (LeftWeapon)
		{
			LeftWeapon->SetActorHiddenInGame(false);
			this->AnimState[5] = true;
		}

		// Need to update weapon type for anim
		AnimUtils::updateAnimStateWeaponType(AnimState, CombineWeapon, LeftWeapon, RightWeapon);

		if (LeftWeapon)
		{
			LeftWeapon->HasBeenCombined = false;
		}
	}

	// UI
	// ======================================================================
	// Set Inventory UI
	if (GetNetMode() == NM_ListenServer)
		SetTextureInUI();

	// Adjust the walking speed according to the holding weapon
	Server_SetMaxWalkSpeed();

	// Check Shellheal buff if holding any shells
	Server_CheckBubble();
}

void AMCharacter::OnCombineWeapon(bool bJustPickedLeft)
{
	if (IsDead)
		return;

	if (LeftWeapon && RightWeapon)
	{
		if (CombineWeapon)
		{
			PreventRefreshingCombineWeaponCD_ByDropPick(CombineWeapon);
			CombineWeapon->SafeDestroyWhenGetThrew();
			CombineWeapon = nullptr;
			//SetTextureInUI(Main, nullptr);
		}

		TSubclassOf<ABaseWeapon> combineWeaponRef;
		// can combine
		if (WeaponConfig::GetInstance()->GetOnCombineClassRef(LeftWeapon->GetWeaponName(), RightWeapon->GetWeaponName()) >= 0)
		{
			combineWeaponRef = weaponArray[WeaponConfig::GetInstance()->GetOnCombineClassRef(LeftWeapon->GetWeaponName(), RightWeapon->GetWeaponName())];
			isHoldingCombineWeapon = true;
		}
		// cannot combine
		else
		{
			// 2 identical weapons or at least one of them is a shell
			LeftWeapon->SetActorHiddenInGame(false);
			RightWeapon->SetActorHiddenInGame(false);
			isHoldingCombineWeapon = false;
			// Sfx
			if (bJustPickedLeft)
				LeftWeapon->NetMulticast_CallPickedUpSfx();
			else
				RightWeapon->NetMulticast_CallPickedUpSfx();
			return;
		}

		FString CombineMessage = FString::Printf(TEXT("Combine weapon reference name is: ")) + combineWeaponRef->GetName();
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, CombineMessage);

		ABaseWeapon* spawnedCombineWeapon = GetWorld()->SpawnActor<ABaseWeapon>(combineWeaponRef, FVector(0, 0, 0), FRotator::ZeroRotator);
		CombineWeapon = spawnedCombineWeapon;
		isFlamethrowerHeld = (CombineWeapon->WeaponType == EnumWeaponType::Flamethrower ||
			CombineWeapon->WeaponType == EnumWeaponType::Cannon ||
			CombineWeapon->WeaponType == EnumWeaponType::Alarmgun);
		CombineWeapon->GetPickedUp(this);
		CombineWeapon->NetMulticast_CallPickedUpSfx();
		spawnedCombineWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, WeaponConfig::GetInstance()->GetWeaponSocketName(spawnedCombineWeapon->GetWeaponName()));
		if (CombineWeapon && CombineWeapon->WeaponType == Server_LastDitchCombineWeaponType && GetWorld()->TimeSeconds - Server_LastDitchCombineWeaponTime < 3.0f)
			CombineWeapon->CD_LeftEnergy = Server_LastDitchCombineWeapon_CD_LeftEnergy;

		//Hide left and right weapon
		LeftWeapon->SetActorHiddenInGame(true);
		LeftWeapon->HasBeenCombined = true;
		LeftWeapon->AttackStop();
		RightWeapon->SetActorHiddenInGame(true);
		RightWeapon->HasBeenCombined = true;
		RightWeapon->AttackStop();

		// If combine successfully, always set isLeftHeld Anim State to false, 
		// this will be reset to true when picking up new item in PickUp_Implementation
		this->AnimState[5] = false;
	}
}

void AMCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	if (IsDead)
		return;

	Jump();
}

void AMCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	if (IsDead)
		return;

	StopJumping();
}

void AMCharacter::Zoom(bool i_bZooming)
{
	bZooming = i_bZooming;
}

void AMCharacter::Dash()
{	
	AMPlayerController* MyPlayerController = Cast<AMPlayerController>(Controller);
	if (MyPlayerController && IsAllowDash && OriginalMaxWalkSpeed * 0.2f < GetCharacterMovement()->Velocity.Size())
	{
		// Update UI
		MyPlayerController->UI_InGame_OnUseSkill(SkillType::SKILL_DASH, DashCoolDown);
		// Call Server Dash
		Server_Dash();
	}
}

void AMCharacter::Server_Dash_Implementation()
{
	//if (IsAllowDash && OriginalMaxWalkSpeed * 0.2f < GetCharacterMovement()->Velocity.Size())
	if (true)
	{
		IsAllowDash = false;
		if (GetNetMode() == NM_ListenServer)
			OnRep_IsAllowDash();

		// Dash implement
		float DashSpeed = DashDistance / DashTime;
		DashSpeed *= GetCharacterMovement()->MaxWalkSpeed / OriginalMaxWalkSpeed;
		NetMulticast_AdjustMaxWalkSpeed(DashSpeed / OriginalMaxWalkSpeed);
		GetCharacterMovement()->Velocity *= DashSpeed / GetCharacterMovement()->Velocity.Size();

		GetWorld()->GetTimerManager().SetTimer(DashingTimer, this, &AMCharacter::RefreshDash, DashTime, false);
	}
}

void AMCharacter::RefreshDash()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("StopDashing"));
	GetCharacterMovement()->MaxWalkSpeed = OriginalMaxWalkSpeed;
	FVector Velocity_copy = GetCharacterMovement()->Velocity;
	if (OriginalMaxWalkSpeed < Velocity_copy.Size())
	{
		Velocity_copy.Normalize();
		GetCharacterMovement()->Velocity = Velocity_copy * OriginalMaxWalkSpeed;
	}

	FTimerHandle tempHandle;
	GetWorld()->GetTimerManager().SetTimer(tempHandle, this, &AMCharacter::SetDash, DashCoolDown, false);

	Server_SetMaxWalkSpeed();
}

void AMCharacter::SetDash()
{
	IsAllowDash = true;
}

void AMCharacter::TurnOffDashEffect()
{
	EffectDash->Deactivate();
}

void AMCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * TurnRateGamePad * GetWorld()->GetDeltaSeconds());

}

void AMCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * TurnRateGamePad * GetWorld()->GetDeltaSeconds());
}

void AMCharacter::MoveForward(float Value)
{
	if (IsDead)
		return;

	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = FollowCamera->GetRelativeRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
		//GetMesh()->SetRelativeRotation(MeshRotation);
	}
}

void AMCharacter::MoveRight(float Value)
{
	if (IsDead)
		return;

	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = FollowCamera->GetRelativeRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
		//GetMesh()->SetRelativeRotation(MeshRotation);
	}
}

#pragma endregion Input

// Test
// ===================================
// void AMCharacter::Test_Implementation()
// {
// 	TSubclassOf<class UDamageType> DamageType = UDamageType::StaticClass();
// 	FHitResult Hit;
// 	UGameplayStatics::ApplyPointDamage(this, 10, FVector(0, 0, 0), Hit, GetInstigator()->Controller, this, DamageType);
// }

// Health
// ============================================
#pragma region Health
void AMCharacter::OnHealthUpdate()
{
	if (GetLocalRole() != ROLE_Authority || GetNetMode() == NM_ListenServer)
	{
		if (!IsLocallyControlled())
		{
			SetHealthBarUI();
		}
		else
		{
			AMPlayerController* playerController = Cast<AMPlayerController>(Controller);
			playerController->UI_InGame_UpdateHealth(CurrentHealth / MaxHealth);
			SetHealthBarUI();
		}
	}

	if (IsDead)
		return;

	if (CurrentHealth <= 0)
		CallDeathSfx();

	//Client-specific functionality
	if (IsLocallyControlled())
	{
		//FString healthMessage = FString::Printf(TEXT("You now have %f health remaining."), CurrentHealth);
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);

		if (CurrentHealth <= 0)
		{
			// Death
			// to do
			// Force respawn
			Server_ForceRespawn(5);
		}
	}

	//Server-specific functionality
	if (GetLocalRole() == ROLE_Authority)
	{
		if (CurrentHealth <= 0)
		{
			if (CombineWeapon)
			{
				PreventRefreshingCombineWeaponCD_ByDropPick(CombineWeapon);
				CombineWeapon->SafeDestroyWhenGetThrew();
				CombineWeapon = nullptr;
				//SetTextureInUI(Main, nullptr);
			}

			if (LeftWeapon)
			{
				DropOffWeapon(true);
			}

			if (RightWeapon)
			{
				DropOffWeapon(false);
			}

			CurrentTouchedWeapon.Empty();
		}
	}

	//Functions that occur on all machines. 
	/*
		Any special functionality that should occur as a result of damage or death should be placed here.
	*/
}

// Server force respawn character
void AMCharacter::Server_ForceRespawn_Implementation(float Delay)
{
	IsDead = true;
	DeadAnimIndex = FMath::RandRange(0, DeadAnimNum - 1);
	NetMulticast_DieResult();
	StartToRespawn(Delay);
}

// Start to respawn
void AMCharacter::StartToRespawn(float Delay)
{
	if (Delay <= 0)
	{
		Client_Respawn();
	}
	else
	{
		// GetWorldTimerManager().SetTimer() InRate cannot <= 0;
		FTimerHandle StartToRespawnTimerHandle;
		AMGameState* MyGameState = Cast<AMGameState>(GetWorld()->GetGameState());
		if (MyGameState->IsGameStart && MyGameState->GameTime >= 4)
		{
			GetWorldTimerManager().SetTimer(StartToRespawnTimerHandle, this, &AMCharacter::Client_Respawn, Delay, false);
		}
	}
}

void AMCharacter::Client_Respawn_Implementation()
{
	AMPlayerController* playerController = Cast<AMPlayerController>(Controller);
	if (playerController)
	{
		playerController->Server_RequestRespawn();
	}
}

// Multicast die result
void AMCharacter::NetMulticast_DieResult_Implementation()
{
	this->GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	SetFollowWidgetVisibility(false);
	SetTipUI(false);

	if (IsLocallyControlled())
	{
		// Broadcast Camera animation
		BPF_DeathCameraAnimation(true);
	}
}

// Multicast respawn result, reset all kinds of variables
void AMCharacter::NetMulticast_RespawnResult_Implementation()
{
	this->GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
	AMGameState* MyGameState = Cast<AMGameState>(GetWorld()->GetGameState());
	if (MyGameState && MyGameState->IsGameStart)
	{
		SetFollowWidgetVisibility(true);
		UMCharacterFollowWidget* CharacterFollowWidget = Cast<UMCharacterFollowWidget>(PlayerFollowWidget->GetUserWidgetObject());
		CharacterFollowWidget->HideTip();
		CharacterFollowWidget->SetVisibility(ESlateVisibility::Visible);
		//AM_PlayerState* MyPlayerState = Cast<AM_PlayerState>(GetPlayerState());
		//CharacterFollowWidget->SetPlayerName(MyPlayerState->PlayerNameString);
		GetCharacterMovement()->MaxWalkSpeed = OriginalMaxWalkSpeed;
		BuffMap.Empty();
		AttackedRecord.Empty();
		InvincibleTimer = 0;
	}

	if (IsLocallyControlled())
	{
		// Reset Camera
		BPF_DeathCameraAnimation(false);
	}
}



void AMCharacter::NetMulticast_SetWorldLocationRotation_Implementation(FVector NewWorldLocation, FRotator NewWorldRotation)
{
	SetActorLocation(NewWorldLocation);
	SetActorRotation(NewWorldRotation);

	// Disable Dash Vfx during instant move
	if (EffectDash && EffectDash->IsVisible())
		EffectDash->SetVisibility(false);
}


void AMCharacter::SetFollowWidgetVisibility(bool IsVisible)
{
	UMCharacterFollowWidget* CharacterFollowWidget = Cast<UMCharacterFollowWidget>(PlayerFollowWidget->GetUserWidgetObject());
	if (CharacterFollowWidget)
	{
		if (IsVisible)
		{
			CharacterFollowWidget->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			CharacterFollowWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void AMCharacter::SetFollowWidgetHealthBarIsEnemy(bool IsEnemy)
{
	UMCharacterFollowWidget* CharacterFollowWidget = Cast<UMCharacterFollowWidget>(PlayerFollowWidget->GetUserWidgetObject());
	if (CharacterFollowWidget && !IsLocallyControlled())
	{
		CharacterFollowWidget->SetIsEnemyHealthBar(IsEnemy);
	}
}

void AMCharacter::SetFollowWidgetHealthBarByTeamID(int TeamID)
{
	UMCharacterFollowWidget* CharacterFollowWidget = Cast<UMCharacterFollowWidget>(PlayerFollowWidget->GetUserWidgetObject());
	if (CharacterFollowWidget)
	{
		CharacterFollowWidget->ShowHealthBarByTeamID(TeamID);
	}
}

void AMCharacter::SetPlayerNameUIInformation()
{
	if (GetNetMode() == NM_ListenServer)
	{
		FString Message = FString::Printf(TEXT("Set Player Name UI Information: ListenServer"));
		GEngine->AddOnScreenDebugMessage(-1, 30.f, FColor::Red, Message);
	}
	else if (GetNetMode() == NM_Client)
	{
		FString Message = FString::Printf(TEXT("Set Player Name UI Information: Client"));
		GEngine->AddOnScreenDebugMessage(-1, 30.f, FColor::Red, Message);
	}

	// If this pawn is local controlled by a controller, then it needs update HUD
	AMPlayerController* MyPlayerController = Cast<AMPlayerController>(GetController());
	AM_PlayerState* MyPlayerState = Cast<AM_PlayerState>(GetPlayerState());
	if (MyPlayerController && IsLocallyControlled() && MyPlayerState)
	{
		MyPlayerController->GetInGameHUD()->InGame_UpdatePlayerNameUI(MyPlayerState->PlayerNameString);
	}

	// Update FollowWidget player name UI
	UMCharacterFollowWidget* MyFollowWidget = Cast<UMCharacterFollowWidget>(PlayerFollowWidget->GetUserWidgetObject());
	if (MyPlayerState && MyFollowWidget)
	{
		MyFollowWidget->SetPlayerName(MyPlayerState->PlayerNameString);
	}
}

void AMCharacter::SetPlayerSkin()
{
	// TODO
	AM_PlayerState* MyPlayerState = Cast<AM_PlayerState>(GetPlayerState());
	if (MyPlayerState && MyPlayerState->characterIndex != -1 && !IsAlreadySetPlayerSkin)
	{
		/*UKismetMaterialLibrary::SetVectorParameterValue(GetWorld(), characaterMaterialParameterCollection, 
			c, MyPlayerState->colorPicked);*/

		FScopeLock Lock(&DataGuard);
		USkeletalMeshComponent* MyMesh = GetMesh();
		int CharacterIndex = MyPlayerState->characterIndex;
		FLinearColor ColorPicked = MyPlayerState->colorPicked;
		MyMesh->SetSkeletalMesh(CharacterBPArray[CharacterIndex]);

		// Kiko
		if (MyPlayerState->characterIndex == 0) {
			UMaterialInstanceDynamic* kiko_body_customize = MyMesh->CreateDynamicMaterialInstance(0, MyMesh->GetMaterial(0));
			UMaterialInstanceDynamic* kiko_hair = MyMesh->CreateDynamicMaterialInstance(1, MyMesh->GetMaterial(1));
			UMaterialInstanceDynamic* kiko_outfit = MyMesh->CreateDynamicMaterialInstance(2, MyMesh->GetMaterial(2));

			kiko_body_customize->SetVectorParameterValue("color", ColorPicked);
			kiko_hair->SetVectorParameterValue("color", ColorPicked);
			kiko_outfit->SetVectorParameterValue("color", ColorPicked);

			MyMesh->SetMaterial(0, kiko_body_customize);
			MyMesh->SetMaterial(1, kiko_hair);
			MyMesh->SetMaterial(2, kiko_outfit);
		}
		else {
			auto test = MyMesh->CreateDynamicMaterialInstance(0, MyMesh->GetMaterial(0));
			test->SetVectorParameterValue("color", ColorPicked);
			MyMesh->SetMaterial(0, test);
		}

		//BPF_SetPlayerSkin();

		IsAlreadySetPlayerSkin = true;
	}
}

void AMCharacter::InitFollowWidget()
{
	UMCharacterFollowWidget* CharacterFollowWidget = Cast<UMCharacterFollowWidget>(PlayerFollowWidget->GetUserWidgetObject());
	if (CharacterFollowWidget)
	{
		CharacterFollowWidget->InitIsLocalControlledCharacterWidget(IsLocallyControlled());
		CharacterFollowWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

// void AMCharacter::SetLocallyControlledGameUI(bool isVisible)
// {
// 	if (isVisible)
// 	{
// 		UMCharacterFollowWidget* CharacterFollowWidget = Cast<UMCharacterFollowWidget>(PlayerFollowWidget->GetUserWidgetObject());
// 		CharacterFollowWidget->HideTip();
// 		CharacterFollowWidget->SetVisibility(ESlateVisibility::Visible);
// 		CharacterFollowWidget->InitIsLocalControlledCharacterWidget(IsLocallyControlled(), false);
// 	}
// 	else
// 	{
// 		UMCharacterFollowWidget* CharacterFollowWidget = Cast<UMCharacterFollowWidget>(PlayerFollowWidget->GetUserWidgetObject());
// 		CharacterFollowWidget->HideTip();
// 		CharacterFollowWidget->SetVisibility(ESlateVisibility::Hidden);
// 	}
// }

void AMCharacter::SetOutlineEffect(bool isVisible)
{
	GetMesh()->SetRenderCustomDepth(isVisible);
}

// void AMCharacter::SetThisCharacterMesh(int TeamIndex)
// {
// 	if (TeamIndex == 1)
// 	{
// 		GetMesh()->SetSkeletalMesh(CharacterBPArray[0]);
// 	}
// 	else
// 	{
// 		GetMesh()->SetSkeletalMesh(CharacterBPArray[1]);
// 	}
// }

void AMCharacter::Server_SetCanMove_Implementation(bool i_CanMove)
{
	AMPlayerController* playerController = Cast<AMPlayerController>(Controller);
	playerController->Server_SetCanMove(i_CanMove);
	Server_CanMove = i_CanMove;
}

bool AMCharacter::CheckBuffMap(EnumAttackBuff AttackBuff)
{
	if (!BuffMap.Contains(AttackBuff))
	{
		BuffMap.Add(AttackBuff);
		TArray<float> buffParArr;
		for (int i = 0; i < 3; i++)
			buffParArr.Add(0.0f);
		BuffMap[AttackBuff] = buffParArr;
	}
	return(BuffMap[AttackBuff].Num() == 3);
}


float AMCharacter::GetCurrentEnergyWeaponUIUpdatePercent()
{
	float retValue = -1;

	if (CombineWeapon)
	{
		if (CombineWeapon->CD_MaxEnergy > 0)
		{
			// Need to show cd UI
			retValue = CombineWeapon->CD_LeftEnergy / CombineWeapon->CD_MaxEnergy;
		}
	}
	else
	{
		if (LeftWeapon)
		{
			if (LeftWeapon->CD_MaxEnergy > 0)
			{
				retValue = LeftWeapon->CD_LeftEnergy / LeftWeapon->CD_MaxEnergy;
			}
		}
		if (RightWeapon)
		{
			if (RightWeapon->CD_MaxEnergy > 0)
			{
				retValue = RightWeapon->CD_LeftEnergy / RightWeapon->CD_MaxEnergy;
			}
		}
	}

	return retValue;
}

void AMCharacter::SetElectricShockAnimState(bool i_state) {
	this->AnimState[8] = i_state;
}

void AMCharacter::ResetCharacterStatus()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		CurrentHealth = MaxHealth;
		OnHealthUpdate();
		IsDead = false;
		if (0 < Server_DeadTimes)
		{
			IsInvincible = true;
			if (GetNetMode() == NM_ListenServer)
				OnRep_IsInvincible();
		}
		NetMulticast_RespawnResult();
	}
}

// RepNotify function
void AMCharacter::OnRep_CurrentHealth()
{
	OnHealthUpdate();
}

void AMCharacter::OnRep_IsAllowDash()
{
	if (!IsAllowDash)
	{
		// Vfx
		if (EffectDash)
		{
			EffectDash->SetVisibility(true);
			EffectDash->Activate();
			FTimerHandle tmpHandle;
			GetWorld()->GetTimerManager().SetTimer(tmpHandle, this, &AMCharacter::TurnOffDashEffect, DashTime, false);
		}
		// Sfx
		CallDashSfx();
	}
}

void AMCharacter::OnRep_IsHealing()
{
	if (IsHealing)
	{
		// Activate VFX
		if (EffectHeal && !EffectHeal->IsActive())
			EffectHeal->Activate();
	}
	else
	{
		// Deactivate VFX
		if (EffectHeal && EffectHeal->IsActive())
			EffectHeal->Deactivate();
	}
}

void AMCharacter::OnRep_IsBurned()
{
	if (IsBurned)
	{
		// Vfx
		if (EffectBurn && !EffectBurn->IsActive())
			EffectBurn->Activate();

		// Sfx
		CallBurningBuffStartSfx();

		// Toggle on Character's Burning buff UI
		auto pMPlayerController = Cast<AMPlayerController>(GetController());
		if (pMPlayerController)
		{
			auto pMInGameHUD = pMPlayerController->GetInGameHUD();
			if (pMInGameHUD)
			{
				pMInGameHUD->InGame_ToggleFireBuffWidget(true);
			}
		}
	}
	else
	{
		// Vfx
		if (EffectBurn && EffectBurn->IsActive())
			EffectBurn->Deactivate();

		// Sfx
		CallBurningBuffStopSfx();

		// Toggle off Character's Burning buff UI
		auto pMPlayerController = Cast<AMPlayerController>(GetController());
		if (pMPlayerController)
		{
			auto pMInGameHUD = pMPlayerController->GetInGameHUD();
			if (pMInGameHUD)
				pMInGameHUD->InGame_ToggleFireBuffWidget(false);
		}
	}
}

void AMCharacter::OnRep_IsParalyzed()
{
	if (IsParalyzed)
	{
		// Vfx
		//if (EffectBurn && !EffectBurn->IsActive())
		//	EffectBurn->Activate();

		// Sfx
		CallParalysisBuffStartSfx();

		// Toggle on Character's Paralysis buff UI
		auto pMPlayerController = Cast<AMPlayerController>(GetController());
		if (pMPlayerController)
		{
			auto pMInGameHUD = pMPlayerController->GetInGameHUD();
			if (pMInGameHUD)
			{
				pMInGameHUD->InGame_ToggleShockBuffWidget(true);
			}
		}
	}
	else
	{
		// Vfx
		//if (EffectBurn && EffectBurn->IsActive())
		//	EffectBurn->Deactivate();

		// Sfx
		CallParalysisBuffStopSfx();

		// Toggle off Character's Paralysis buff UI
		auto pMPlayerController = Cast<AMPlayerController>(GetController());
		if (pMPlayerController)
		{
			auto pMInGameHUD = pMPlayerController->GetInGameHUD();
			if (pMInGameHUD)
				pMInGameHUD->InGame_ToggleShockBuffWidget(false);
		}
	}
}

void AMCharacter::OnRep_IsInvincible()
{
	if (IsInvincible)
	{
		// Vfx
		if (EffectBubbleStart)
		{
			//EffectBubbleStart->ResetSystem();
			EffectBubbleStart->Activate();
			//EffectBubbleStart->SetVisibility(true);
		}
		FTimerHandle ShowBubbleOnVfxTimerHandle;
		GetWorldTimerManager().SetTimer(ShowBubbleOnVfxTimerHandle, [this]
			{
				if (EffectBubbleOn && IsInvincible && this)
				{
					if (!EffectBubbleOn->IsActive())
						EffectBubbleOn->Activate();
					EffectBubbleOn->SetVisibility(true);
				}
			}, 1.0, false);


		// Toggle On Character's Invincible buff UI
		auto pMPlayerController = Cast<AMPlayerController>(GetController());
		if (pMPlayerController)
		{
			auto pMInGameHUD = pMPlayerController->GetInGameHUD();
			if (pMInGameHUD)
				pMInGameHUD->InGame_ToggleInvincibleUI(true);
		}
	}
	else
	{
		// Vfx
		if (EffectBubbleEnd)
			EffectBubbleEnd->Activate();
		if (EffectBubbleOn)
			EffectBubbleOn->SetVisibility(false);
		/*if (EffectBubbleStart && EffectBubbleStart->IsActive())
		{
			EffectBubbleStart->Deactivate();
			EffectBubbleStart->SetVisibility(false);
		}*/

		// Toggle Off Character's Invincible buff UI
		auto pMPlayerController = Cast<AMPlayerController>(GetController());
		if (pMPlayerController)
		{
			auto pMInGameHUD = pMPlayerController->GetInGameHUD();
			if (pMInGameHUD)
				pMInGameHUD->InGame_ToggleInvincibleUI(false);
		}
		InvincibleTimer = 0;
	}
}


void AMCharacter::SetCurrentHealth(float healthValue)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		CurrentHealth = FMath::Clamp(healthValue, 0.f, MaxHealth);
		OnHealthUpdate();
	}
}

// DamageCauser can be either weapon or projectile
float AMCharacter::TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (DamageTaken <= 0 || CurrentHealth <= 0 || IsInvincible || !EventInstigator || !DamageCauser)
		return 0.0f;

	if (!IsDead)
	{
		// Check if it is the attack from self or teammates
		int TeammateCheckResult = ADamageManager::IsTeammate(this, EventInstigator);
		if (TeammateCheckResult != 0)
			return 0.0f;

		// Call GetHit vfx & sfx (cannot call in OnHealthUpdate since health can be increased or decreased)
		if (Server_LastTime_CallGetHitSfxVfx < 0 || Server_CallGetHitSfxVfx_MinInterval <= GetWorld()->TimeSeconds - Server_LastTime_CallGetHitSfxVfx)
		{
			NetMulticast_CallGetHitSfx();
			NetMulticast_CallGetHitVfx();
			Server_LastTime_CallGetHitSfxVfx = GetWorld()->TimeSeconds;
		}

		if (!AttackedRecord.Contains(EventInstigator))
			AttackedRecord.Add(EventInstigator);
		AttackedRecord[EventInstigator] = GetWorld()->TimeSeconds;

		SetCurrentHealth(CurrentHealth - DamageTaken);

		// If holding a taser, the attack should stop
		if (isHoldingCombineWeapon && CombineWeapon && CombineWeapon->WeaponType == EnumWeaponType::Taser)
			CombineWeapon->AttackStop();

		// Score Kill Death Handling
		if (CurrentHealth <= 0 && HasAuthority())
		{
			AM_PlayerState* AttackerPS = EventInstigator->GetPlayerState<AM_PlayerState>();
			AM_PlayerState* MyPS = GetPlayerState<AM_PlayerState>();
			if (!AttackerPS || !MyPS)
				return 0.0f;

			int KillerGetScore = 0;
			AMGameState* MyGameState = Cast<AMGameState>(GetWorld()->GetGameState());
			if (MyGameState)
			{
				KillerGetScore = MyGameState->KillScore;
			}
			AttackerPS->addScore(KillerGetScore);
			AttackerPS->addKill(1);
			// Record Kill Assists
			float TimeNow = GetWorld()->TimeSeconds;
			float MaxTimeCountingAsKA = 3.0f;
			auto AttackedRecordCopy = AttackedRecord;
			for (auto& Elem : AttackedRecordCopy)
			{
				if (!Elem.Key)
					continue;
				if (TimeNow - Elem.Value <= MaxTimeCountingAsKA)
				{
					if (AM_PlayerState* AssistAttackerPS = Elem.Key->GetPlayerState<AM_PlayerState>())
					{
						if(AssistAttackerPS != AttackerPS)
							AssistAttackerPS->addKillAssist(1);
					}
				}
			}
			MyPS->addDeath(1);

			Server_DeadTimes++;

			// Broadcast function
			BroadcastToAllController(DamageCauser, false);
		}

		return DamageTaken;
	}

	return 0.0f;
}


void AMCharacter::SetHealthBarUI()
{
	UMCharacterFollowWidget* healthBar = Cast<UMCharacterFollowWidget>(PlayerFollowWidget->GetUserWidgetObject());
	healthBar->SetHealthToProgressBar(CurrentHealth / MaxHealth);
}
#pragma endregion Health

// Score
// ============================================
#pragma region Score

void AMCharacter::UpdateSKD(int scoreToAdd, int killToAdd, int deathToAdd) {
	SKDArray[0] += scoreToAdd;
	SKDArray[1] += killToAdd;
	SKDArray[2] += deathToAdd;
}

bool AMCharacter::GetIsDead()
{
	return IsDead;
}

void AMCharacter::SetIsDead(bool n_IsDead)
{
	IsDead = n_IsDead;
}

#pragma endregion Score

// UI
// ===========================================
#pragma region UI

void AMCharacter::SetTipUI_Implementation(bool isShowing, ABaseWeapon* CurrentTouchWeapon)
{
	if (IsParalyzed)
		return;

	if (IsLocallyControlled() || GetNetMode() == NM_ListenServer)
	{
		UMCharacterFollowWidget* CharacterFollowWidget = Cast<UMCharacterFollowWidget>(PlayerFollowWidget->GetUserWidgetObject());
		if (isShowing && CurrentTouchWeapon)
		{
			CharacterFollowWidget->ShowTip();
			// Update Weapon UI
			if (CurrentTouchWeapon->IsBigWeapon)
			{
				CharacterFollowWidget->SetLeftWeaponTipUI(CurrentTouchWeapon->PickUpTextureUI_Q);
				CharacterFollowWidget->SetRightWeaponTipUI(CurrentTouchWeapon->PickUpTextureUI_E);
			}
			else
			{
				// Left
				if (RightWeapon && !(RightWeapon->IsBigWeapon))
				{
					if (WeaponConfig::GetInstance()->GetOnCombineClassRef(CurrentTouchWeapon->GetWeaponName(), RightWeapon->GetWeaponName()) >= 0)
					{
						TSubclassOf<ABaseWeapon> combineWeaponRef;
						combineWeaponRef = weaponArray[WeaponConfig::GetInstance()->GetOnCombineClassRef(CurrentTouchWeapon->GetWeaponName(), RightWeapon->GetWeaponName())];
						ABaseWeapon* combineWeapon = Cast<ABaseWeapon>(combineWeaponRef->GetDefaultObject());
						CharacterFollowWidget->SetLeftWeaponTipUI(combineWeapon->PickUpTextureUI_Q);
					}
					else
					{
						CharacterFollowWidget->SetLeftWeaponTipUI(CurrentTouchWeapon->PickUpTextureUI_Q);
					}
				}
				else
				{
					CharacterFollowWidget->SetLeftWeaponTipUI(CurrentTouchWeapon->PickUpTextureUI_Q);
				}

				// Right
				if (LeftWeapon && !(LeftWeapon->IsBigWeapon))
				{
					if (WeaponConfig::GetInstance()->GetOnCombineClassRef(LeftWeapon->GetWeaponName(), CurrentTouchWeapon->GetWeaponName()) >= 0)
					{
						TSubclassOf<ABaseWeapon> combineWeaponRef;
						combineWeaponRef = weaponArray[WeaponConfig::GetInstance()->GetOnCombineClassRef(LeftWeapon->GetWeaponName(), CurrentTouchWeapon->GetWeaponName())];
						ABaseWeapon* combineWeapon = Cast<ABaseWeapon>(combineWeaponRef->GetDefaultObject());
						CharacterFollowWidget->SetRightWeaponTipUI(combineWeapon->PickUpTextureUI_E);
					}
					else
					{
						CharacterFollowWidget->SetRightWeaponTipUI(CurrentTouchWeapon->PickUpTextureUI_E);
					}
				}
				else
				{
					CharacterFollowWidget->SetRightWeaponTipUI(CurrentTouchWeapon->PickUpTextureUI_E);
				}
			}
		}
		else
		{
			CharacterFollowWidget->HideTip();
		}
	}
}

#pragma endregion UI

// Collision
// ===========================================
#pragma region Collision
void AMCharacter::OnWeaponOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
	class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		if (Cast<ASaltcureArea>(OtherActor))
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Actor enters the SaltcureArea"));
			// apply saltcure buff
			if (CheckBuffMap(EnumAttackBuff::Saltcure))
			{
				ADamageManager::AddBuffPoints(EnumWeaponType::None, EnumAttackBuff::Saltcure, GetController(), this, 1.0f);
				IsSaltCure = true;
			}
			Server_SetMaxWalkSpeed();
		}
	}

	if (!(OtherComp->GetName() == "Box_DisplayCase"))
		return;

	ABaseWeapon* tempWeaponTouched = Cast<ABaseWeapon>(OtherActor);
	if (tempWeaponTouched && !IsDead)
	{
		if (!CurrentTouchedWeapon.Contains(tempWeaponTouched))
		{
			CurrentTouchedWeapon.Add(tempWeaponTouched);
			// Output: WeaponOverlap
			FString NetName = "None";
			if (GetNetMode() == NM_ListenServer)
			{
				NetName = "ListenServer";
			}
			else if (GetNetMode() == NM_Client)
			{
				NetName = "Client";
			}
			else if (GetNetMode() == NM_DedicatedServer)
			{
				NetName = "DedicatedServer";
			}
			FString CollisionMessage = FString::Printf(TEXT("Append Weapon to array(%s) : %s"), ToCStr(NetName), *tempWeaponTouched->GetWeaponName());
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, CollisionMessage);
		}

		// Set Tip Weapon Image
		if (CurrentTouchedWeapon.Num() > 0)
		{
			SetTipUI(true, CurrentTouchedWeapon[0]);
		}
	}
}

void AMCharacter::OnWeaponOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		if (Cast<ASaltcureArea>(OtherActor))
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Actor stops overlapping with the waterbody"));
			// cancel saltcure buff
			if (CheckBuffMap(EnumAttackBuff::Saltcure))
			{
				ADamageManager::AddBuffPoints(EnumWeaponType::None, EnumAttackBuff::Saltcure, GetController(), this, -1.0f);
				IsSaltCure = false;
			}
			Server_SetMaxWalkSpeed();
		}
	}

	if (!(OtherComp->GetName() == "Box_DisplayCase"))
		return;

	ABaseWeapon* tempWeaponTouched = Cast<ABaseWeapon>(OtherActor);
	if (tempWeaponTouched && !IsDead)
	{
		if (CurrentTouchedWeapon.Contains(tempWeaponTouched))
		{
			CurrentTouchedWeapon.Remove(tempWeaponTouched);

			// Output: WeaponEngOverlap
			FString CollisionMessage = FString::Printf(TEXT("Remove Weapon in array : %s"), *tempWeaponTouched->GetWeaponName());
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, CollisionMessage);
		}
		SetTipUI(false);
	}
}

void AMCharacter::OnHealingBubbleColliderOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
	class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Server
	if (GetLocalRole() == ROLE_Authority)
	{
		if (auto pMCharacter = Cast<AMCharacter>(OtherActor))
		{
			int TeammateCheckResult = ADamageManager::IsTeammate(this, pMCharacter);
			if (TeammateCheckResult == -1)
				return;
			else if (TeammateCheckResult == 1)
				ADamageManager::AddBuffPoints(EnumWeaponType::Shell, EnumAttackBuff::Shellheal, GetController(), pMCharacter, 1.0f);
		}
	}	
	
	if (auto pMinigameObj_Statue = Cast<AMinigameObj_Statue>(OtherActor))
	{
		bHealingBubbleTouchingStatue = true;
	}
}

void AMCharacter::OnHealingBubbleColliderOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// Server
	if (GetLocalRole() == ROLE_Authority)
	{
		if (auto pMCharacter = Cast<AMCharacter>(OtherActor))
		{
			int TeammateCheckResult = ADamageManager::IsTeammate(this, pMCharacter);
			if (TeammateCheckResult == -1)
				return;
			else if (TeammateCheckResult == 1)
				ADamageManager::AddBuffPoints(EnumWeaponType::Shell, EnumAttackBuff::Shellheal, GetController(), pMCharacter, -1.0f);
		}
	}

	if (auto pMinigameObj_Statue = Cast<AMinigameObj_Statue>(OtherActor))
	{
		bHealingBubbleTouchingStatue = false;
	}
}
#pragma endregion Collision

#pragma region Engine life cycle function
// Called when the game starts or when spawned
void AMCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Assign Variables
	OriginalMaxWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;

	if (HealingBubbleCollider)
	{
		HealingBubbleCollider->OnComponentBeginOverlap.AddDynamic(this, &AMCharacter::OnHealingBubbleColliderOverlapBegin);
		HealingBubbleCollider->OnComponentEndOverlap.AddDynamic(this, &AMCharacter::OnHealingBubbleColliderOverlapEnd);
	}

	if (GetLocalRole() != ROLE_Authority || GetNetMode() == NM_ListenServer)
	{
		UMCharacterFollowWidget* healthBar = Cast<UMCharacterFollowWidget>(PlayerFollowWidget->GetUserWidgetObject());
		healthBar->HideTip();
		AMGameState* MyGameState = Cast<AMGameState>(GetWorld()->GetGameState());
		if (MyGameState)
		{
			SetFollowWidgetVisibility(MyGameState->IsGameStart);
		}
		// AM_PlayerState* MyPlayerState = Cast<AM_PlayerState>(GetPlayerState());
		// if (MyPlayerState)
		// {
		// 	if (MyPlayerState->TeamIndex == 1)
		// 	{
		// 		GetMesh()->SetSkeletalMesh(CharacterBPArray[0]);
		// 	}
		// 	else
		// 	{
		// 		GetMesh()->SetSkeletalMesh(CharacterBPArray[1]);
		// 	}
		// }
	}

	if (IsLocallyControlled())
	{
		// Cursor Plane
		ACursorHitPlane* pCursorHitPlane = GetWorld()->SpawnActor<ACursorHitPlane>(CursorHitPlaneSubClass);
		if (pCursorHitPlane)
			pCursorHitPlane->pMCharacter = this;

		// Toggle off some of the Character's UI
		auto pMPlayerController = Cast<AMPlayerController>(GetController());
		if (pMPlayerController)
		{
			auto pMInGameHUD = pMPlayerController->GetInGameHUD();
			if (pMInGameHUD)
			{
				pMInGameHUD->InGame_ToggleFireBuffWidget(false);
				pMInGameHUD->InGame_ToggleShockBuffWidget(false);
				pMInGameHUD->InGame_ToggleInvincibleUI(false);
			}
		}

		// Show Weapon ShowUp Vfx(Confetti)
		TArray<FTransform> Arr_SpawnTransform;
		TArray<EnumWeaponType> Arr_WeaponTypeCanBeSpawned;
		TMap<EnumWeaponType, unsigned int> Map_WeaponTypeCnt;

		TSubclassOf<ABaseWeapon> ActorClass = ABaseWeapon::StaticClass();
		TArray<AActor*> OutActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ActorClass, OutActors);
		for (size_t i = 0; i < OutActors.Num(); i++)
		{
			if (ABaseWeapon* pWeapon = Cast<ABaseWeapon>(OutActors[i]))
				pWeapon->CallShowUpVfx();
		}
	}
	
	// Opponent Marker
	if (UOpponentMarkerWidget* pOpponentMarkerWidget = Cast<UOpponentMarkerWidget>(OpponentMarkerWidget->GetUserWidgetObject()))
		pOpponentMarkerWidget->SetAllMarkerVisibility(false);
	// Silouette
	GetMesh()->SetRenderCustomDepth(false);
}


void AMCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Adjust Bubble Size if needed
	if (HealingBubbleCollider)
	{
		// Preset parameters
		float TargetScale = 0;
		float SizeChangeSpeed = 0;
		FVector NewRelativeScale = HealingBubbleCollider->GetRelativeScale3D();
		if (bHealingBubbleOn)
		{
			if (!bHealingBubbleTouchingStatue)
			{
				float size1 = 6.5f;
				float size2 = 10.0f;
				TargetScale = bDoubleHealingBubbleSize ? size2 : size1;
				if (NewRelativeScale.X < size1)
					SizeChangeSpeed = 5.0f;
				else if (NewRelativeScale.X <= size2) // must have =
					SizeChangeSpeed = 2.5f;
			}
			else
			{
				TargetScale = 0;
				SizeChangeSpeed = 5.0f;
			}			
		}
		else
		{
			TargetScale = 0;
			SizeChangeSpeed = 1000.0f;
		}
		// Change
		if (NewRelativeScale.X < TargetScale)
		{
			NewRelativeScale += FVector::OneVector * DeltaTime * SizeChangeSpeed;
			if (TargetScale < NewRelativeScale.X)
				NewRelativeScale = FVector(TargetScale, TargetScale, TargetScale);
			HealingBubbleCollider->SetRelativeScale3D(NewRelativeScale);
		}
		else if (TargetScale < NewRelativeScale.X)
		{
			NewRelativeScale -= FVector::OneVector * DeltaTime * SizeChangeSpeed;
			if (NewRelativeScale.X < TargetScale)
				NewRelativeScale = FVector(TargetScale, TargetScale, TargetScale);
			HealingBubbleCollider->SetRelativeScale3D(NewRelativeScale);
		}
		HealingBubbleCollider->SetWorldRotation(FRotator::ZeroRotator);
	}	

	// Server
	if (GetLocalRole() == ROLE_Authority)
	{
		// Deal with buff
		ActByBuff_PerTick(DeltaTime);

		// Deal with respawn invincible buff
		if (IsInvincible)
		{
			InvincibleTimer += DeltaTime;
			if (InvincibleMaxTime < InvincibleTimer)
			{
				InvincibleTimer = 0;
				IsInvincible = false;
				if (GetNetMode() == NM_ListenServer)
					OnRep_IsInvincible();
			}
			if (EffectBubbleOn && EffectBubbleOn->IsActive())
				EffectBubbleOn->SetWorldRotation(FRotator::ZeroRotator);
		}
	}

	// Client(Listen Server)
	if (GetLocalRole() != ROLE_Authority || GetNetMode() == NM_ListenServer)
	{
		// Vfx Land/Jump
		bool oldIsOnGround = IsOnGround;
		IsOnGround = GetCharacterMovement()->IsMovingOnGround();
		if (oldIsOnGround != IsOnGround)
		{
			// call land effect when landing hard
			if (IsOnGround && 30.0f < Client_MaxHeightDuringLastTimeOffGround - GetActorLocation().Z)
			{
				if (EffectLand && EffectLand->GetAsset())
					UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), EffectLand->GetAsset(), EffectLand->GetComponentLocation());
			}
			// call jump effect when jumping hard
			if (!IsOnGround && 500.0f < GetCharacterMovement()->Velocity.Z)
			{
				if (EffectJump && EffectJump->GetAsset())
					UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), EffectJump->GetAsset(), EffectJump->GetComponentLocation());
			}
		}
		if (!IsOnGround)
			Client_MaxHeightDuringLastTimeOffGround = FMath::Max(Client_MaxHeightDuringLastTimeOffGround, GetActorLocation().Z);
		else
			Client_MaxHeightDuringLastTimeOffGround = TNumericLimits<float>::Min();

		// Vfx Run
		if (OriginalMaxWalkSpeed * 0.15f < GetCharacterMovement()->Velocity.Size())
		{
			if (IsOnGround)
			{
				if (EffectRun && !(EffectRun->IsActive()))
					EffectRun->Activate();
			}
			else
			{
				if (EffectRun && EffectRun->IsActive())
					EffectRun->Deactivate();
			}
			Client_LowSpeedWalkAccumulateTime = 0;
		}
		else
		{
			Client_LowSpeedWalkAccumulateTime += DeltaTime;
			if (0.1f < Client_LowSpeedWalkAccumulateTime)
			{
				if (EffectRun && EffectRun->IsActive())
					EffectRun->Deactivate();
			}
		}

		// Vfx Heal (Though we have OnRep_IsHealing() to control the vfx, it is not working as expected every time)
		if (IsHealing)
		{
			// Activate VFX
			if (EffectHeal && !EffectHeal->IsActive())
				EffectHeal->Activate();
		}
		else
		{
			// Deactivate VFX
			if (EffectHeal && EffectHeal->IsActive())
				EffectHeal->Deactivate();
		}
	}

	if (IsLocallyControlled())
	{
		// Opponent Marker on the bottom
		// ========================================================================================================================
		UOpponentMarkerWidget* pOpponentMarkerWidget = Cast<UOpponentMarkerWidget>(OpponentMarkerWidget->GetUserWidgetObject());
		APlayerController* pPlayerController = Cast<APlayerController>(GetController());
		FVector2D ScreenPosition_Self;
		bool Success_GetSelfScreenPos = UGameplayStatics::ProjectWorldToScreen
		(
			Cast<APlayerController>(GetController()),
			GetActorLocation(),
			ScreenPosition_Self,
			false
		);
		int32 viewport_x, viewport_y;
		if(pPlayerController)
			pPlayerController->GetViewportSize(viewport_x, viewport_y);
		if (Success_GetSelfScreenPos && pOpponentMarkerWidget && pPlayerController)
		{
			// Let Widget know my TeamId			
			if (AM_PlayerState* MyPS = GetPlayerState<AM_PlayerState>())
			{
				if (pOpponentMarkerWidget->TeamID != MyPS->TeamIndex)
				{
					pOpponentMarkerWidget->TeamID = MyPS->TeamIndex;
					pOpponentMarkerWidget->ResetMarkers();
				}
			}							
			int MaxPeopleInTeam = 3;
			for (size_t i = 0; i < MaxPeopleInTeam; i++)
			{
				pOpponentMarkerWidget->SetMarkerVisibility(i, false);
				if (i < Opponents.Num() && Opponents[i] && !Opponents[i]->IsDead)
				{
					FVector WorldPos = Opponents[i]->GetActorLocation();
					FVector2D ScreenPosition_Opponent;
					// ProjectWorldToScreen may fail if opponent is too far from self
					bool Success_GetOpScreenPos = UGameplayStatics::ProjectWorldToScreen
					(
						pPlayerController,
						WorldPos,
						ScreenPosition_Opponent,
						false
					);
					if (Success_GetOpScreenPos)
					{
						if (ScreenPosition_Self.Y < ScreenPosition_Opponent.Y && viewport_y * 1.0f < ScreenPosition_Opponent.Y)
						{			
							pOpponentMarkerWidget->SetMarkerVisibility(i, true);
							// line1: y = ax+b;  line2: y = viewport_y;
							float a = (ScreenPosition_Opponent.Y - ScreenPosition_Self.Y) / (ScreenPosition_Opponent.X - ScreenPosition_Self.X);
							float b = ScreenPosition_Opponent.Y - (a * ScreenPosition_Opponent.X);
							float targetX = (viewport_y - b) / a;
							targetX -= (viewport_x * 0.5f);
							targetX = FMath::Clamp(targetX, -viewport_x * 0.5f, viewport_x * 0.5f);
							pOpponentMarkerWidget->SetMarkerTranslation(i, targetX , 0);

							FVector2D Self_To_Opponent = ScreenPosition_Opponent - ScreenPosition_Self;
							Self_To_Opponent.Normalize();
							FVector2D Self_To_BottomCenter = FVector2D(viewport_x * 0.5f, viewport_y) - ScreenPosition_Self;
							Self_To_BottomCenter.Normalize();							
							float OffsetAngle = FMath::RadiansToDegrees(FMath::Acos(FVector2D::DotProduct(Self_To_Opponent, Self_To_BottomCenter)));
							FVector3d CrossProductResult = FVector3d::CrossProduct(FVector3d(Self_To_Opponent, 0.0f), FVector3d(Self_To_BottomCenter, 0.0f));
							if (0 < CrossProductResult.Z)
								OffsetAngle = -OffsetAngle;
							OffsetAngle = FMath::Clamp(OffsetAngle, -65.0f, 65.0f);
							pOpponentMarkerWidget->SetMarkerOffsetAngle(i, OffsetAngle);
						}						
					}					
				}
			}
		}
		
			

		//if (0 < CameraShakingTime)
		//{
		//	if (100.0f == CameraShakingIntensity)
		//	{
		//		FVector NewRelativeLocation = FollowCamera->GetRelativeLocation();
		//		float Y_Speed = 20.0f;
		//		float Z_Speed = 10.0f;
		//		float Y_Interval = 0.05f;
		//		float Z_Interval = 0.3f;
		//		bool Y_odd = ((int)(CameraShakingTime / Y_Interval) & 1) == 1;
		//		bool Z_odd = ((int)(CameraShakingTime / Z_Interval) & 1) == 1;

		//		NewRelativeLocation.Y += Y_Speed * (Y_odd ? 1 : -1);
		//		//NewRelativeLocation.Z += Z_Speed * (Z_odd ? 1 : -1);
		//		FollowCamera->SetRelativeLocation(NewRelativeLocation);
		//		TimePassedSinceShaking += DeltaTime;
		//		CameraShakingTime -= DeltaTime;
		//		if (CameraShakingTime <= 0)
		//		{
		//			TimePassedSinceShaking = 0;
		//			CameraShakingTime = 0;
		//			FollowCamera->SetRelativeLocation(FollowCameraOriginalRelativeLocation);
		//		}
		//	}			
		//}			

		//// Zoom
		//if (auto pPlayerController = Cast<APlayerController>(GetController()))
		//{
		//	float mouse_x, mouse_y;
		//	bool success = pPlayerController->GetMousePosition(mouse_x, mouse_y);
		//	int32 viewport_x, viewport_y;
		//	pPlayerController->GetViewportSize(viewport_x, viewport_y);
		//	if (success && 0 < viewport_x && 0 < viewport_y)
		//	{
		//		float lowerYRatio = FMath::Max((mouse_y - (0.5f * viewport_y)) / (0.5f * viewport_y), 0); // [0, 1]

		//		float FinalX = -500.0f * 1.0f;
		//		float FinalZ = 0.0f * 1.0f;
		//		float FinalRoll = -15.0f * 1.0f;

		//		float TargetX = 0;
		//		float TargetZ = 0;
		//		float TargetRoll = 0;
		//		int Gear = 0;
		//		float TimeFromMinToMax = 0;
		//		
		//		//// Discrete
		//		//int GearCnt = 3;
		//		//float LevelRange = 1.0f / GearCnt;				
		//		//Gear = FMath::Min(1 + (lowerYRatio / LevelRange), GearCnt);
		//		//if (lowerYRatio <= 0)
		//		//	Gear = 0;					
		//		//TargetX = FinalX * Gear * LevelRange;
		//		//TargetZ = FinalZ * Gear * LevelRange;
		//		//TargetRoll = FinalRoll * Gear * LevelRange;
		//		
		//		// Continuous
		//		//TargetX = FinalX * lowerYRatio;
		//		//TargetZ = FinalZ * lowerYRatio;
		//		//TargetRoll = FinalRoll * lowerYRatio;

		//		// Only one gear in one area
		//		
		//		if (0.7f <= lowerYRatio && bZooming)
		//		{
		//			Gear = 1;
		//			TimeFromMinToMax = 0.000004f;
		//		}					
		//		else
		//		{
		//			Gear = 0;
		//			TimeFromMinToMax = 0.0000065f;
		//		}					
		//		TargetX = FinalX * Gear;
		//		TargetZ = FinalZ * Gear;
		//		TargetRoll = FinalRoll * Gear;
		//		
		//		
		//		// Location
		//		FVector CurRelativeLocation = FollowCamera->GetRelativeLocation();
		//		if (CurRelativeLocation.X != TargetX || CurRelativeLocation.Z != TargetZ)
		//		{
		//			// Location X
		//			if (CurRelativeLocation.X < TargetX)
		//			{
		//				CurRelativeLocation.X += DeltaTime * FMath::Abs(FinalX / TimeFromMinToMax);
		//				CurRelativeLocation.X = FMath::Min(CurRelativeLocation.X, TargetX);
		//			}
		//			else if (TargetX < CurRelativeLocation.X)
		//			{
		//				CurRelativeLocation.X -= DeltaTime * FMath::Abs(FinalX / TimeFromMinToMax);
		//				CurRelativeLocation.X = FMath::Max(CurRelativeLocation.X, TargetX);
		//			}
		//			// Location Z
		//			if (CurRelativeLocation.Z < TargetZ)
		//			{
		//				CurRelativeLocation.Z += DeltaTime * FMath::Abs(FinalZ / TimeFromMinToMax);
		//				CurRelativeLocation.Z = FMath::Min(CurRelativeLocation.Z, TargetZ);
		//			}
		//			else if (TargetZ < CurRelativeLocation.Z)
		//			{
		//				CurRelativeLocation.Z -= DeltaTime * FMath::Abs(FinalZ / TimeFromMinToMax);
		//				CurRelativeLocation.Z = FMath::Max(CurRelativeLocation.Z, TargetZ);
		//			}
		//			FollowCamera->SetRelativeLocation(CurRelativeLocation);
		//		}
		//		// Rotation
		//		if (FollowCameraRelativeRotationVector.X != TargetRoll)
		//		{
		//			if (FollowCameraRelativeRotationVector.X < TargetRoll)
		//			{
		//				FollowCameraRelativeRotationVector.X += DeltaTime * FMath::Abs(FinalRoll / TimeFromMinToMax);
		//				FollowCameraRelativeRotationVector.X = FMath::Min(FollowCameraRelativeRotationVector.X, TargetRoll);
		//			}
		//			else if (TargetRoll < FollowCameraRelativeRotationVector.X)
		//			{
		//				FollowCameraRelativeRotationVector.X -= DeltaTime * FMath::Abs(FinalRoll / TimeFromMinToMax);
		//				FollowCameraRelativeRotationVector.X = FMath::Max(FollowCameraRelativeRotationVector.X, TargetRoll);
		//			}
		//			FollowCamera->SetRelativeRotation(FRotator(FollowCameraRelativeRotationVector.X, 0, 0));
		//		}						
		//	}
		//}


		////float ZoomOutSpeed = 100.0f;
		////float ZoomInSpeed = ZoomOutSpeed * 0.5f;
		//float ZoomOutSpeed = 1400.0f;
		//float ZoomInSpeed = ZoomOutSpeed * 0.75f;
		//if (bShouldZoomOut)
		//{
		//	//if (CurFov < MaxFov)
		//	//{
		//	//	CurFov = FMath::Min(MaxFov, CurFov + DeltaTime * ZoomOutSpeed);
		//	//	FollowCamera->SetFieldOfView(CurFov);
		//	//}
		//	if (Local_CurCameraArmLength < Local_MaxCameraArmLength)
		//	{
		//		Local_CurCameraArmLength = FMath::Min(Local_MaxCameraArmLength, Local_CurCameraArmLength + DeltaTime * ZoomOutSpeed);
		//		CameraBoom->TargetArmLength = Local_CurCameraArmLength;
		//	}
		//}
		//else
		//{
		//	//if (MinFov < CurFov)
		//	//{
		//	//	CurFov = FMath::Max(MinFov, CurFov - DeltaTime * ZoomInSpeed);
		//	//	FollowCamera->SetFieldOfView(CurFov);
		//	//}
		//	if (Local_MinCameraArmLength < Local_CurCameraArmLength)
		//	{
		//		Local_CurCameraArmLength = FMath::Max(Local_MinCameraArmLength, Local_CurCameraArmLength - DeltaTime * ZoomInSpeed);
		//		CameraBoom->TargetArmLength = Local_CurCameraArmLength;
		//	}
		//}
	}
}
#pragma endregion Engine life cycle function

void AMCharacter::ActByBuff_PerTick(float DeltaTime)
{
	// Server
	if (GetLocalRole() == ROLE_Authority)
	{
		if (!AWeaponDataHelper::DamageManagerDataAsset)
			return;

		EnumAttackBuff buffType;
		/*  Burning */
		buffType = EnumAttackBuff::Burning;
		if (CheckBuffMap(buffType))
		{
			float& BuffRemainedTime = BuffMap[buffType][1];
			if (0.0f < BuffRemainedTime && 0 < CurrentHealth)
			{
				float BurningBuffDamagePerSecond = 0.0f;
				FString ParName = "BurningDamagePerSecond";
				if (AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map.Contains(ParName))
					BurningBuffDamagePerSecond = AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map[ParName];
				float TargetCurrentHealth = CurrentHealth - DeltaTime * BurningBuffDamagePerSecond;
				SetCurrentHealth(TargetCurrentHealth);
				// Update AttackedRecord
				if (Server_TheControllerApplyingLatestBurningBuff)
				{
					if (!AttackedRecord.Contains(Server_TheControllerApplyingLatestBurningBuff))
						AttackedRecord.Add(Server_TheControllerApplyingLatestBurningBuff);
					AttackedRecord[Server_TheControllerApplyingLatestBurningBuff] = GetWorld()->TimeSeconds;
				}				
				if (TargetCurrentHealth <= 0.0f)
				{
					auto MyController = GetController();
					if (Server_TheControllerApplyingLatestBurningBuff && MyController)
					{
						AM_PlayerState* AttackerPS = Server_TheControllerApplyingLatestBurningBuff->GetPlayerState<AM_PlayerState>();
						AM_PlayerState* MyPS = MyController->GetPlayerState<AM_PlayerState>();
						// Add Scores
						int KillerGetScore = 0;
						AMGameState* MyGameState = Cast<AMGameState>(GetWorld()->GetGameState());
						if (MyGameState)
						{
							KillerGetScore = MyGameState->KillScore;
						}
						AttackerPS->addScore(KillerGetScore);
						AttackerPS->addKill(1);
						// Record Kill Assists
						float TimeNow = GetWorld()->TimeSeconds;
						float MaxTimeCountingAsKA = 3.0f;
						auto AttackedRecordCopy = AttackedRecord;
						for (auto& Elem : AttackedRecordCopy)
						{
							if (!Elem.Key)
								continue;
							if (TimeNow - Elem.Value <= MaxTimeCountingAsKA)
							{
								if (AM_PlayerState* AssistAttackerPS = Elem.Key->GetPlayerState<AM_PlayerState>())
								{
									if (AssistAttackerPS != AttackerPS)
										AssistAttackerPS->addKillAssist(1);
								}
							}
						}
						MyPS->addDeath(1);
						// Broadcast burning kill
						BroadcastToAllController(Server_TheControllerApplyingLatestBurningBuff, true);
					}
				}
				BuffRemainedTime = FMath::Max(BuffRemainedTime - DeltaTime, 0.0f);
			}
			else
			{
				if (IsBurned)
				{
					// Set IsBurned to false
					IsBurned = false;
					if (GetNetMode() == NM_ListenServer)
						OnRep_IsBurned();
				}
			}
		}
		/* Paralysis */
		buffType = EnumAttackBuff::Paralysis;
		if (CheckBuffMap(buffType))
		{
			float& BuffPoints = BuffMap[buffType][0];
			if (1.0f <= BuffPoints && 0 < CurrentHealth)
			{
				if (Server_CanMove)
				{
					Server_SetCanMove(false);
					SetElectricShockAnimState(true);
				}
				/*float DragSpeedRatio = 0.15f;
				FString ParName = "Paralysis_DragSpeedRatio";
				if (AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map.Contains(ParName))
					DragSpeedRatio = AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map[ParName];
				Client_MoveCharacter(Server_Direction_SelfToTaserAttacker, DragSpeedRatio);
				Server_Direction_SelfToTaserAttacker = FVector::Zero();*/
			}
			else
			{
				if (!Server_CanMove)
				{
					Server_SetCanMove(true);
					SetElectricShockAnimState(false);
				}
				if (IsParalyzed)
				{
					// Set IsParalyzed to false
					IsParalyzed = false;
					if (GetNetMode() == NM_ListenServer)
						OnRep_IsParalyzed();
				}
			}
		}
		/* Knockback */
		buffType = EnumAttackBuff::Knockback;
		if (CheckBuffMap(buffType))
		{
		}
		/* Saltcure */
		buffType = EnumAttackBuff::Saltcure;
		if (CheckBuffMap(buffType))
		{
			float& BuffPoints = BuffMap[buffType][0];
			if (1.0f <= BuffPoints && 0 < CurrentHealth)
			{
				float Saltcure_RecoverSpeed = 0;
				FString ParName = "Saltcure_RecoverSpeed";
				if (AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map.Contains(ParName))
					Saltcure_RecoverSpeed = AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map[ParName];
				SetCurrentHealth(CurrentHealth + DeltaTime * Saltcure_RecoverSpeed);
				// clear burning buff
				if (CheckBuffMap(EnumAttackBuff::Burning))
				{
					BuffMap[EnumAttackBuff::Burning][0] = 0;  // reset BuffPoints
					BuffMap[EnumAttackBuff::Burning][1] = 0;  // reset BuffRemainedTime
				}
			}
			else
			{
				if (IsHealing)
				{
					if (!(CheckBuffMap(EnumAttackBuff::Shellheal) && 1.0f <= BuffMap[EnumAttackBuff::Shellheal][0]))
					{
						IsHealing = false;
						if (GetNetMode() == NM_ListenServer)
							OnRep_IsHealing();
					}
				}
			}
		}
		/* Shellheal */
		buffType = EnumAttackBuff::Shellheal;
		if (CheckBuffMap(buffType))
		{
			float& BuffPoints = BuffMap[buffType][0];
			if (1.0f <= BuffPoints && 0 < CurrentHealth)
			{
				float Shellheal_RecoverSpeed = 0;
				FString ParName = "";
				ParName = "Shellheal_RecoverSpeed";
				if (AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map.Contains(ParName))
					Shellheal_RecoverSpeed = AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map[ParName];
				ParName = "Shellheal_RecoverSpeed_OnDoubleShellHolder";
				if(LeftWeapon && LeftWeapon->WeaponType == EnumWeaponType::Shell && RightWeapon && RightWeapon->WeaponType == EnumWeaponType::Shell)
					Shellheal_RecoverSpeed = AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map[ParName];
				SetCurrentHealth(CurrentHealth + DeltaTime * Shellheal_RecoverSpeed);
				// clear burning buff
				if (CheckBuffMap(EnumAttackBuff::Burning))
				{
					BuffMap[EnumAttackBuff::Burning][0] = 0;  // reset BuffPoints
					BuffMap[EnumAttackBuff::Burning][1] = 0;  // reset BuffRemainedTime
				}
			}
			else
			{
				if (IsHealing)
				{
					if (!(CheckBuffMap(EnumAttackBuff::Saltcure) && 1.0f <= BuffMap[EnumAttackBuff::Saltcure][0]))
					{
						IsHealing = false;
						if (GetNetMode() == NM_ListenServer)
							OnRep_IsHealing();
					}
				}
			}
		}
		// Crab Bubble Damage
		if (IsAffectedByCrabBubble)
		{
			float CrabBubbleDamage = 0;
			FString ParName = "CrabBubbleDamage";
			if (AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map.Contains(ParName))
				CrabBubbleDamage = AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map[ParName];
			SetCurrentHealth(CurrentHealth - DeltaTime * CrabBubbleDamage);
		}
	}
}

void AMCharacter::BroadcastToAllController(AActor* AttackActor, bool IsFireBuff)
{
	AM_PlayerState* AttackerPS = nullptr;
	AM_PlayerState* MyPS = nullptr;
	UTexture2D* WeaponImage = nullptr;
	
	if (IsFireBuff)
	{
		AMPlayerController* AttackCauser = Cast<AMPlayerController>(AttackActor);
		AttackerPS = AttackCauser->GetPlayerState<AM_PlayerState>();
		MyPS = GetController()->GetPlayerState<AM_PlayerState>();
		// Broadcast burning kill
		WeaponImage = FireImage;
	}
	else if (Cast<ABaseWeapon>(AttackActor))
	{
		ABaseWeapon* AttackCauser = Cast<ABaseWeapon>(AttackActor);
		AttackerPS = AttackCauser->PreHoldingController->GetPlayerState<AM_PlayerState>();
		MyPS = GetController()->GetPlayerState<AM_PlayerState>();
		// Broadcast burning kill
		WeaponImage = AttackCauser->WeaponImage_Broadcast;
	}
	else if (Cast<ABaseProjectile>(AttackActor))
	{
		ABaseProjectile* AttackCauser = Cast<ABaseProjectile>(AttackActor);
		AttackerPS = AttackCauser->GetInstigator()->GetPlayerState<AM_PlayerState>();
		MyPS = GetController()->GetPlayerState<AM_PlayerState>();
		// Broadcast burning kill
		WeaponImage = AttackCauser->WeaponImage_Broadcast;
	}

	if (AttackerPS && MyPS && WeaponImage)
	{
		for (FConstPlayerControllerIterator iter = GetWorld()->GetPlayerControllerIterator(); iter; ++iter)
		{
			AMPlayerController* currentController = Cast<AMPlayerController>(*iter);
			currentController->UI_InGame_BroadcastInformation(AttackerPS->TeamIndex, MyPS->TeamIndex, AttackerPS->PlayerNameString, MyPS->PlayerNameString, WeaponImage);
		}
	}
}

// void AMCharacter::FollowWidget_PerTick(float DeltaTime)
// {
// 	if (PlayerFollowWidget_NeedDisappear)
// 	{
// 		PlayerFollowWidget_RenderOpacity -= DeltaTime;
// 		PlayerFollowWidget_RenderOpacity = PlayerFollowWidget_RenderOpacity < 0 ? 0 : PlayerFollowWidget_RenderOpacity;
// 		PlayerFollowWidget_NeedDisappear = false;
// 		UMCharacterFollowWidget* MyPlayerFollowWidget = Cast<UMCharacterFollowWidget>(PlayerFollowWidget->GetUserWidgetObject());
// 		if (MyPlayerFollowWidget)
// 		{
// 			MyPlayerFollowWidget->SetHealthBarRenderOpacity(PlayerFollowWidget_RenderOpacity);
// 			MyPlayerFollowWidget->SetPlayerNameRenderOpacity(PlayerFollowWidget_RenderOpacity);
// 		}
// 	}
// }


void AMCharacter::Client_MoveCharacter_Implementation(FVector MoveDirection, float SpeedRatio)
{
	AddMovementInput(MoveDirection, SpeedRatio);
}

#pragma region Effects
void AMCharacter::NetMulticast_CallGetHitSfx_Implementation()
{
	CallGetHitSfx();
}

void AMCharacter::NetMulticast_CallGetHitVfx_Implementation()
{
	EffectGetHit->Activate();
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("EffectGetHit is called"));
}
#pragma endregion Effects


void AMCharacter::Server_GiveShellToStatue(AWeaponShell* pShell)
{
	if (LeftWeapon == pShell)
	{
		pShell->Server_bDetectedByStatue = true;
		DropOffWeapon(true);
	}
	else if (RightWeapon == pShell)
	{
		pShell->Server_bDetectedByStatue = true;
		DropOffWeapon(false);
	}		
}

void AMCharacter::Server_EnableEffectByCrabBubble(bool bEnable)
{
	if (IsAffectedByCrabBubble != bEnable)
	{
		IsAffectedByCrabBubble = bEnable;
		Server_SetMaxWalkSpeed();
	}
}

void AMCharacter::Server_CheckBubble()
{
	int CntShellHeld = 0;
	if (!CombineWeapon)
	{
		if (LeftWeapon && LeftWeapon->WeaponType == EnumWeaponType::Shell)
			CntShellHeld++;
		if (RightWeapon && RightWeapon->WeaponType == EnumWeaponType::Shell)
			CntShellHeld++;
		if (0 < CntShellHeld)
		{
			NetMulticast_SetHealingBubbleStatus(true, CntShellHeld == 2);
		}
		else
		{
			NetMulticast_SetHealingBubbleStatus(false, CntShellHeld == 2);
		}
	}
}

void AMCharacter::PreventRefreshingCombineWeaponCD_ByDropPick(ABaseWeapon* pCombineWeapon)
{
	if (pCombineWeapon)
	{
		Server_LastDitchCombineWeaponType = pCombineWeapon->WeaponType;
		Server_LastDitchCombineWeaponTime = GetWorld()->TimeSeconds;
		Server_LastDitchCombineWeapon_CD_LeftEnergy = pCombineWeapon->CD_LeftEnergy;
	}
}

FString AMCharacter::Server_GetHoldingWeaponType(int WeaponFlag)
{
	FString ret = "";
	if (WeaponFlag == 0)
	{
		if (LeftWeapon)
			ret = LeftWeapon->GetWeaponName();
	}
	else if(WeaponFlag == 1)
	{
		if (RightWeapon)
			ret = RightWeapon->GetWeaponName();
	}
	else if (WeaponFlag == 2)
	{
		if (CombineWeapon)
			ret = CombineWeapon->GetWeaponName();
	}
	return ret;
}