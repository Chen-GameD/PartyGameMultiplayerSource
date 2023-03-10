// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/MCharacter.h"

//#include "SAdvancedTransformInputBox.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "Weapon/BaseProjectile.h"
#include "Weapon/WeaponConfig.h"
#include "Weapon/JsonFactory.h"
#include "Weapon/DamageManager.h"
#include "Weapon/WeaponDataHelper.h"
#include <Character/animUtils.h>

#include "Character/MPlayerController.h"
#include "Components/WidgetComponent.h"
#include "GameBase/MGameState.h"
#include "UI/MCharacterFollowWidget.h"

// Constructor
// ===================================================
#pragma region Constructor
AMCharacter::AMCharacter()
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
	CameraBoom->TargetArmLength = 800.0f; // The camera follows at this distance behind the character
	CameraBoom->SetRelativeRotation(FRotator(60.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	//Create HealthBar UI Widget
	PlayerFollowWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("FollowWidget"));
	PlayerFollowWidget->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	// Collision Event
	GetCapsuleComponent()->SetGenerateOverlapEvents(true);
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &AMCharacter::OnWeaponOverlapBegin);
	GetCapsuleComponent()->OnComponentEndOverlap.AddDynamic(this, &AMCharacter::OnWeaponOverlapEnd);

	IsDead = false;

	IsAllowDash = true;
	OriginalMaxWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
    DashDistance = 200.0f;
    DashTime = 0.2f;
    DashSpeed = DashDistance / DashTime;

	//MeshRotation = GetMesh()->GetRelativeRotation();

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

	CanMove = true;
	BeingKnockbackBeforeThisTick = false;
}

void AMCharacter::Restart()
{
	Super::Restart();

	if (IsLocallyControlled())
	{
		AMPlayerController* playerController = Cast<AMPlayerController>(Controller);
		playerController->UI_InGame_UpdateHealth(CurrentHealth/MaxHealth);
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
	DOREPLIFETIME(AMCharacter, IsOnGround);
	DOREPLIFETIME(AMCharacter, IsAllowDash);
	
	//DOREPLIFETIME(AMCharacter, MeshRotation);

	//Replicate weapons information
	DOREPLIFETIME(AMCharacter, LeftWeapon);
	DOREPLIFETIME(AMCharacter, RightWeapon);
	DOREPLIFETIME(AMCharacter, CombineWeapon);

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

	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &AMCharacter::Attack);
	PlayerInputComponent->BindAction<FIsMeleeRelease>("Attack", IE_Released, this, &AMCharacter::StopAttack, false);

	PlayerInputComponent->BindAction<FPickUpDelegate>("PickUpLeft", IE_Pressed, this, &AMCharacter::PickUp, true);
	PlayerInputComponent->BindAction<FPickUpDelegate>("PickUpRight", IE_Pressed, this, &AMCharacter::PickUp, false);

	PlayerInputComponent->BindAction("Dash", IE_Pressed, this, &AMCharacter::Dash);

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
			InventoryMenuWidget->SetLeftItemUI(LeftWeapon == nullptr ? nullptr : LeftWeapon->textureUI);
			InventoryMenuWidget->SetRightItemUI(RightWeapon == nullptr ? nullptr : RightWeapon->textureUI);
			InventoryMenuWidget->SetWeaponUI(CombineWeapon == nullptr ? nullptr : CombineWeapon->textureUI);
		}
	}
}

void AMCharacter::Attack_Implementation()
{
	if ((LeftWeapon || RightWeapon || CombineWeapon) && !IsDead)
	{
		// Can't Attack(No WeaponAttackStart, No attack animation) during Paralysis
		if (CheckBuffMap(EnumAttackBuff::Paralysis) && 0.0f < BuffMap[EnumAttackBuff::Paralysis][1])
			return;

		FString AttackMessage = FString::Printf(TEXT("You Start Attack."));
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, AttackMessage);
		
		if (CombineWeapon)
		{
			CombineWeapon->AttackStart();
		}
		else
		{
			if (LeftWeapon)
			{
				LeftWeapon->AttackStart();
			}
			if (RightWeapon)
			{
				RightWeapon->AttackStart();
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

	if (CurrentTouchedWeapon.IsEmpty())
	{
		// Check if should drop weapon
		if (isLeft)
		{
			if (LeftWeapon)
			{
				// Drop off left weapon
				if (CombineWeapon)
				{
					CombineWeapon->Destroy();
					CombineWeapon = nullptr;
				}
				DropOffWeapon(isLeft);
				
			}
		}
		else
		{
			if (RightWeapon)
			{
				// Drop off left weapon
				if (CombineWeapon)
				{
					CombineWeapon->Destroy();
					CombineWeapon = nullptr;
				}
				DropOffWeapon(isLeft);
			}
		}
		return;
	}
	
	if (CurrentTouchedWeapon[0] && !IsDead)
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
				DropOffWeapon(isLeft);
			}

			LeftWeapon = CurrentTouchedWeapon[0];
			//SetTextureInUI(Left, LeftWeapon->textureUI);
			FName SocketName = WeaponConfig::GetInstance()->GetWeaponSocketName(LeftWeapon->GetWeaponName());
			FString temp = SocketName.ToString();
			temp += "_Left";
			SocketName = FName(*temp);
			LeftWeapon->GetPickedUp(this);
			LeftWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, SocketName);
			
			// Set isLeftHeld Anim State
			this->AnimState[5] = true;

			// Check if need combine
			if (RightWeapon)
			{	
				// Set isRightHeld Anim State
				this->AnimState[6] = true;
				OnCombineWeapon();
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
				DropOffWeapon(isLeft);
			}
			
			RightWeapon = CurrentTouchedWeapon[0];
			//SetTextureInUI(Right, RightWeapon->textureUI);
			FName SocketName = WeaponConfig::GetInstance()->GetWeaponSocketName(RightWeapon->GetWeaponName());
			FString temp = SocketName.ToString();
			temp += "_Right";
			SocketName = FName(*temp);
			RightWeapon->GetPickedUp(this);
			RightWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, SocketName);
			
			// Set isRightHeld Anim State
			this->AnimState[6] = true;

			// Check if need combine
			if (LeftWeapon)
			{
				// Set isLeftHeld Anim State
				this->AnimState[5] = true;
				OnCombineWeapon();
			}

			// Update Weapon Type Anim State
			AnimUtils::updateAnimStateWeaponType(AnimState, CombineWeapon, LeftWeapon, RightWeapon);
		}
	}

	IsPickingWeapon = false;

	if (GetNetMode() == NM_ListenServer)
	{
		SetTextureInUI();
	}
}

void AMCharacter::DropOffWeapon(bool isLeft)
{
	if (isLeft && LeftWeapon)
	{
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
}

void AMCharacter::OnCombineWeapon()
{
	if (IsDead)
		return;
	
	if (LeftWeapon && RightWeapon)
	{
		if (CombineWeapon)
		{
			CombineWeapon->Destroy();
			CombineWeapon = nullptr;
			//SetTextureInUI(Main, nullptr);
		}

		TSubclassOf<ABaseWeapon> combineWeaponRef;
		if (WeaponConfig::GetInstance()->GetOnCombineClassRef(LeftWeapon->GetWeaponName(), RightWeapon->GetWeaponName()) >= 0)
		{
			combineWeaponRef = weaponArray[WeaponConfig::GetInstance()->GetOnCombineClassRef(LeftWeapon->GetWeaponName(), RightWeapon->GetWeaponName())];
			isHoldingCombineWeapon = true;
		}
		else
		{
			LeftWeapon->SetActorHiddenInGame(false);
			RightWeapon->SetActorHiddenInGame(false);
			isHoldingCombineWeapon = false;
			return;
		}
		
		FString CombineMessage = FString::Printf(TEXT("Combine weapon reference name is: ")) + combineWeaponRef->GetName();
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, CombineMessage);

		ABaseWeapon* spawnedCombineWeapon = GetWorld()->SpawnActor<ABaseWeapon>(combineWeaponRef, FVector(0,0,0), FRotator::ZeroRotator);
		CombineWeapon = spawnedCombineWeapon;
		isFlamethrowerHeld = (CombineWeapon->WeaponType == EnumWeaponType::Flamethrower || 
							CombineWeapon->WeaponType == EnumWeaponType::Cannon ||
							CombineWeapon->WeaponType == EnumWeaponType::Alarmgun);
		CombineWeapon->GetPickedUp(this);
		spawnedCombineWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, WeaponConfig::GetInstance()->GetWeaponSocketName(spawnedCombineWeapon->GetWeaponName()));

		//Hide left and right weapon
		LeftWeapon->SetActorHiddenInGame(true);
		LeftWeapon->HasBeenCombined = true;
		RightWeapon->SetActorHiddenInGame(true);
		RightWeapon->HasBeenCombined = true;

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

void AMCharacter::Dash_Implementation()
{
	if (IsAllowDash && OriginalMaxWalkSpeed * 0.2f < GetCharacterMovement()->Velocity.Size())
	{
		IsAllowDash = false;

		// Listen Server
		if (GetNetMode() == NM_ListenServer)
		{
			OnRep_IsAllowDash();
		}

		// Dash implement
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Dashing"));
		DashSpeed = DashDistance / DashTime;
		GetCharacterMovement()->MaxWalkSpeed = DashSpeed;
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
	GetWorld()->GetTimerManager().SetTimer(tempHandle, this, &AMCharacter::SetDash, 1.5, false);
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
			playerController->UI_InGame_UpdateHealth(CurrentHealth/MaxHealth);
		}
	}
	
	if (IsDead)
		return;
	
	//Client-specific functionality
	if (IsLocallyControlled())
	{
		//FString healthMessage = FString::Printf(TEXT("You now have %f health remaining."), CurrentHealth);
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);
		
		if (CurrentHealth <= 0)
		{
			// Death
			// to do
			//FString deathMessage = FString::Printf(TEXT("You have been killed."));
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, deathMessage);
			
			// Force respawn
			Server_ForceRespawn(5);
		}
	}

	//Server-specific functionality
	if (GetLocalRole() == ROLE_Authority)
	{
		//FString healthMessage = FString::Printf(TEXT("%s now has %f health remaining."), *GetFName().ToString(), CurrentHealth);
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);

		if (CurrentHealth <= 0)
		{
			if (CombineWeapon)
			{
				CombineWeapon->Destroy();
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
	Multicast_DieResult();

	StartToRespawn(Delay);
	SetTipUI(false);
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
		GetWorldTimerManager().SetTimer(StartToRespawnTimerHandle, this, &AMCharacter::Client_Respawn, Delay, false);
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
void AMCharacter::Multicast_DieResult_Implementation()
{
	this->GetMesh()->SetSimulatePhysics(true);
	this->GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	this->GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	this->GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
}

void AMCharacter::Multicast_SetMesh_Implementation(USkeletalMesh* i_changeMesh)
{
	GetMesh()->SetSkeletalMesh(i_changeMesh);
}

void AMCharacter::SetGameUIVisibility(bool isVisible)
{
	if (isVisible)
	{
		UMCharacterFollowWidget* CharacterFollowWidget = Cast<UMCharacterFollowWidget>(PlayerFollowWidget->GetUserWidgetObject());
		CharacterFollowWidget->HideTip();
		CharacterFollowWidget->SetVisibility(ESlateVisibility::Visible);
		//AM_PlayerState* MyPlayerState = Cast<AM_PlayerState>(GetPlayerState());
		//CharacterFollowWidget->SetPlayerName(MyPlayerState->PlayerNameString);
	}
	else
	{
		UMCharacterFollowWidget* CharacterFollowWidget = Cast<UMCharacterFollowWidget>(PlayerFollowWidget->GetUserWidgetObject());
		CharacterFollowWidget->HideTip();
		CharacterFollowWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

void AMCharacter::SetLocallyControlledGameUI(bool isVisible)
{
	if (isVisible)
	{
		UMCharacterFollowWidget* CharacterFollowWidget = Cast<UMCharacterFollowWidget>(PlayerFollowWidget->GetUserWidgetObject());
		CharacterFollowWidget->HideTip();
		CharacterFollowWidget->SetVisibility(ESlateVisibility::Visible);
		CharacterFollowWidget->SetLocalControlledUI();
	}
	else
	{
		UMCharacterFollowWidget* CharacterFollowWidget = Cast<UMCharacterFollowWidget>(PlayerFollowWidget->GetUserWidgetObject());
		CharacterFollowWidget->HideTip();
		CharacterFollowWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

void AMCharacter::SetOutlineEffect(bool isVisible)
{
	GetMesh()->SetRenderCustomDepth(isVisible);
}

void AMCharacter::SetThisCharacterMesh(int TeamIndex)
{
	if (TeamIndex == 1)
	{
		GetMesh()->SetSkeletalMesh(CharacterBPArray[0]);
	}
	else
	{
		GetMesh()->SetSkeletalMesh(CharacterBPArray[1]);
	}
}

void AMCharacter::Server_SetCanMove_Implementation(bool i_CanMove)
{
	AMPlayerController* playerController = Cast<AMPlayerController>(Controller);
	playerController->SetCanMove(i_CanMove);
}

bool AMCharacter::CheckBuffMap(EnumAttackBuff AttackBuff)
{
	if (!BuffMap.Contains(AttackBuff))
	{
		BuffMap.Add(AttackBuff);
		TArray<float> buffParArr;
		for(int i = 0; i < 3; i++)
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
		else if (RightWeapon)
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

// RepNotify function
void AMCharacter::OnRep_CurrentHealth()
{
	OnHealthUpdate();
}

void AMCharacter::CallJumpLandEffect_Implementation()
{
	if (!EffectJump || !EffectLand)
		return;

	if (IsOnGround)
	{
		EffectLand->Activate();
		EffectJump->Deactivate();
	}
	else
	{
		EffectJump->Activate();
		EffectLand->Deactivate();
	}
}

void AMCharacter::OnRep_IsAllowDash()
{
	// Update client care only, like UI

	// EffectDash
	if (!EffectDash)
		return;
	
	if (!IsAllowDash)
	{
		EffectDash->Activate();
		FTimerHandle tmpHandle;
		GetWorld()->GetTimerManager().SetTimer(tmpHandle, this, &AMCharacter::TurnOffDashEffect, DashTime, false);
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
	if (!IsDead)
	{
		// Check if it is the attack from self or teammates
		auto MyController = GetController();
		if (!MyController)
			return 0.0f;
		AM_PlayerState* AttackerPS = EventInstigator->GetPlayerState<AM_PlayerState>();
		AM_PlayerState* MyPS = MyController->GetPlayerState<AM_PlayerState>();
		if (!AttackerPS || !MyPS)
			return 0.0f;
		if (AttackerPS->TeamIndex == MyPS->TeamIndex)
			return 0.0f;

		float damageApplied = CurrentHealth - DamageTaken;
		SetCurrentHealth(damageApplied);		

		EnumWeaponType WeaponType = EnumWeaponType::None;
		float DeltaTime_DamageCauser = 0;
		if (auto p = Cast<ABaseWeapon>(DamageCauser))
		{
			WeaponType = p->WeaponType;
			DeltaTime_DamageCauser = p->CurDeltaTime;
		}			
		if (auto p = Cast<ABaseProjectile>(DamageCauser))
		{
			WeaponType = p->WeaponType;
			DeltaTime_DamageCauser = p->CurDeltaTime;
		}
		if (WeaponType == EnumWeaponType::None)
			return false;
		ADamageManager::ApplyBuff(WeaponType, EventInstigator, DamageEvent.DamageTypeClass, this, DeltaTime_DamageCauser);

		// If holding a taser, the attack should stop
		if (isHoldingCombineWeapon && CombineWeapon && CombineWeapon->WeaponType == EnumWeaponType::Taser)
			CombineWeapon->AttackStop();

		// Score Kill Death Handling
		if (CurrentHealth <= 0 && HasAuthority()) 
		{
			AttackerPS->addScore(5);
			AttackerPS->addKill(1);
			MyPS->addDeath(1);
		}

		return damageApplied;
	}

	return 0.0f;
}


float AMCharacter::TakeDamageRe(float DamageTaken, EnumWeaponType WeaponType, AController* EventInstigator, ABaseWeapon* DamageCauser)
{
	//if (!IsDead)
	//{
	//	float damageApplied = CurrentHealth - DamageTaken;
	//	SetCurrentHealth(damageApplied);

	//	// Score Kill Death Handling
	//	if (CurrentHealth <= 0 && HasAuthority()) {
	//		if (auto killer = Cast<AMCharacter>(DamageCauser->GetHoldingPlayer())) {
	//			if (AM_PlayerState* killerPS = killer->GetPlayerState<AM_PlayerState>()) {
	//				killerPS->addScore(5);
	//				killerPS->addKill(1);
	//			}
	//			if (AM_PlayerState* myPS = GetPlayerState<AM_PlayerState>()) {
	//				myPS->addDeath(1);
	//			}
	//		}
	//	}

	//	return damageApplied;
	//}

	return 0.0f;
}


void AMCharacter::SetHealthBarUI()
{
	UMCharacterFollowWidget* healthBar = Cast<UMCharacterFollowWidget>(PlayerFollowWidget->GetUserWidgetObject());
	healthBar->SetHealthToProgressBar(CurrentHealth/MaxHealth);
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

#pragma endregion Score

// UI
// ===========================================
#pragma region UI

void AMCharacter::SetTipUI_Implementation(bool isShowing, ABaseWeapon* CurrentTouchWeapon)
{
	if (IsLocallyControlled() || GetNetMode() == NM_ListenServer)
	{
		UMCharacterFollowWidget* CharacterFollowWidget = Cast<UMCharacterFollowWidget>(PlayerFollowWidget->GetUserWidgetObject());
		if (isShowing && CurrentTouchWeapon)
		{
			CharacterFollowWidget->ShowTip();
			// Update Weapon UI
			// Left
			if (RightWeapon)
			{
				if (WeaponConfig::GetInstance()->GetOnCombineClassRef(CurrentTouchWeapon->GetWeaponName(), RightWeapon->GetWeaponName()) >= 0)
				{
					TSubclassOf<ABaseWeapon> combineWeaponRef;
					combineWeaponRef = weaponArray[WeaponConfig::GetInstance()->GetOnCombineClassRef(CurrentTouchWeapon->GetWeaponName(), RightWeapon->GetWeaponName())];
					ABaseWeapon* combineWeapon = Cast<ABaseWeapon>(combineWeaponRef->GetDefaultObject());
					CharacterFollowWidget->SetLeftWeaponTipUI(combineWeapon->textureUI);
				}
				else
				{
					CharacterFollowWidget->SetLeftWeaponTipUI(CurrentTouchWeapon->textureUI);
				}
			}
			else
			{
				CharacterFollowWidget->SetLeftWeaponTipUI(CurrentTouchWeapon->textureUI);
			}

			// Right
			if (LeftWeapon)
			{
				if (WeaponConfig::GetInstance()->GetOnCombineClassRef(LeftWeapon->GetWeaponName(), CurrentTouchWeapon->GetWeaponName()) >= 0)
				{
					TSubclassOf<ABaseWeapon> combineWeaponRef;
					combineWeaponRef = weaponArray[WeaponConfig::GetInstance()->GetOnCombineClassRef(LeftWeapon->GetWeaponName(), CurrentTouchWeapon->GetWeaponName())];
					ABaseWeapon* combineWeapon = Cast<ABaseWeapon>(combineWeaponRef->GetDefaultObject());
					CharacterFollowWidget->SetRightWeaponTipUI(combineWeapon->textureUI);
				}
				else
				{
					CharacterFollowWidget->SetRightWeaponTipUI(CurrentTouchWeapon->textureUI);
				}
			}
			else
			{
				CharacterFollowWidget->SetRightWeaponTipUI(CurrentTouchWeapon->textureUI);
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
#pragma endregion Collision

#pragma region Engine life cycle function
// Called when the game starts or when spawned
void AMCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (GetLocalRole() != ROLE_Authority || GetNetMode() == NM_ListenServer)
	{
		UMCharacterFollowWidget* healthBar = Cast<UMCharacterFollowWidget>(PlayerFollowWidget->GetUserWidgetObject());
		healthBar->HideTip();
		AMGameState* MyGameState = Cast<AMGameState>(GetWorld()->GetGameState());
		if (MyGameState)
		{
			SetGameUIVisibility(MyGameState->IsGameStart);
		}
		AM_PlayerState* MyPlayerState = Cast<AM_PlayerState>(GetPlayerState());
		if (MyPlayerState)
		{
			if (MyPlayerState->TeamIndex == 1)
			{
				GetMesh()->SetSkeletalMesh(CharacterBPArray[0]);
			}
			else
			{
				GetMesh()->SetSkeletalMesh(CharacterBPArray[1]);
			}
		}
	}
}


void AMCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Server
	if (GetLocalRole() == ROLE_Authority)
	{
		ActByBuff_PerTick(DeltaTime);

		bool oldIsOnGround = IsOnGround;
		IsOnGround = GetCharacterMovement()->IsMovingOnGround();
		if (oldIsOnGround != IsOnGround)
		{
			if( !(CheckBuffMap(EnumAttackBuff::Paralysis) && 0 < BuffMap[EnumAttackBuff::Paralysis][1]) && 
				 !(BeingKnockbackBeforeThisTick) )
			CallJumpLandEffect();
		}
		BeingKnockbackBeforeThisTick = false;
	}

	// Client(Listen Server)
	if (GetLocalRole() != ROLE_Authority || GetNetMode() == NM_ListenServer)
	{
		if (CheckBuffMap(EnumAttackBuff::Paralysis) && 0 < BuffMap[EnumAttackBuff::Paralysis][1])
			return;

		// EffectRun
		if (EffectRun->IsActive() == false)
		{
			if (OriginalMaxWalkSpeed * 0.6f <= GetCharacterMovement()->Velocity.Size() && IsAllowDash)
			{
				EffectRun->Activate(); 
			}				
		}
		else
		{
			if (GetCharacterMovement()->Velocity.Size() < OriginalMaxWalkSpeed * 0.6f || !IsAllowDash || !IsOnGround)
			{
				EffectRun->Deactivate();
			}				
		}
	}
}
#pragma endregion Engine life cycle function


void AMCharacter::ActByBuff_PerDamage(float DeltaTime)
{
	// Server
	if (GetLocalRole() == ROLE_Authority)
	{
		if (!AWeaponDataHelper::DamageManagerDataAsset)
			return;

		EnumAttackBuff buffType;
		/* Paralysis */
		buffType = EnumAttackBuff::Paralysis;
		if (CheckBuffMap(buffType))
		{
			float& BuffRemainedTime = BuffMap[buffType][1];
			if (0.0f < BuffRemainedTime)
			{
				float DragSpeed = 0.0f;
				FString ParName = "Paralysis_DragSpeed";
				if (AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map.Contains(ParName))
					DragSpeed = AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map[ParName];
				SetActorLocation(GetActorLocation() + TaserDragDirection_SinceLastApplyBuff * DragSpeed * DeltaTime);
			}
		}
		/* Knockback */
		buffType = EnumAttackBuff::Knockback;
		if (CheckBuffMap(buffType))
		{
			// don't apply knockback if the character is being paralyzed
			if (!(CheckBuffMap(EnumAttackBuff::Paralysis) && 0 < BuffMap[EnumAttackBuff::Paralysis][1]))
			{
				float& BuffPoints = BuffMap[buffType][0];
				if (1.0f <= BuffPoints)
				{
					float Knockback_MoveSpeed = 0.0f;
					FString ParName = "Knockback_MoveSpeed";
					if (AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map.Contains(ParName))
						Knockback_MoveSpeed = AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map[ParName];
					KnockbackDirection_SinceLastApplyBuff.Z = 0.0f;
					KnockbackDirection_SinceLastApplyBuff *= Knockback_MoveSpeed;
					LaunchCharacter(KnockbackDirection_SinceLastApplyBuff, true, false);
					BeingKnockbackBeforeThisTick = true;
					BuffPoints = 0.0f;
				}
			}
		}

		KnockbackDirection_SinceLastApplyBuff = FVector::ZeroVector;
		TaserDragDirection_SinceLastApplyBuff = FVector::ZeroVector;
	}
}

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
			if (0.0f < BuffRemainedTime)
			{
				float BurningBuffDamagePerSecond = 0.0f;
				FString ParName = "BurningDamagePerSecond";
				if (AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map.Contains(ParName))
					BurningBuffDamagePerSecond = AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map[ParName];
				SetCurrentHealth(CurrentHealth - DeltaTime * BurningBuffDamagePerSecond);
				BuffRemainedTime = FMath::Max(BuffRemainedTime - DeltaTime, 0.0f);
			}
		}
		/* Paralysis */
		buffType = EnumAttackBuff::Paralysis;
		if (CheckBuffMap(buffType))
		{
			float& BuffRemainedTime = BuffMap[buffType][1];
			if (0.0f < BuffRemainedTime)
			{
				if (CanMove)
				{
					Server_SetCanMove(false);
					SetElectricShockAnimState(true);
					CanMove = false;
				}
				BuffRemainedTime = FMath::Max(BuffRemainedTime - DeltaTime, 0.0f);
			}
			else
			{
				if (!CanMove)
				{
					Server_SetCanMove(true);
					SetElectricShockAnimState(false);
					CanMove = true;
				}
			}
		}
		/* Knockback */
		buffType = EnumAttackBuff::Knockback;
		if (CheckBuffMap(buffType))
		{
		}
	}		
}