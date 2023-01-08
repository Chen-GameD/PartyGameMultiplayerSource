// Copyright Epic Games, Inc. All Rights Reserved.

#include "PartyGameMultiplayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h" 
#include "GameFramework/SpringArmComponent.h"
#include "TestInfo.h"
#include "Weapon/BaseWeapon.h"
#include "Net/UnrealNetwork.h"


//////////////////////////////////////////////////////////////////////////
// APartyGameMultiplayerCharacter

APartyGameMultiplayerCharacter::APartyGameMultiplayerCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rate for input
	TurnRateGamepad = 50.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

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
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	bAttackOn = false;
	CurWeapon = nullptr;

	OriginalMaxWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
	DashDistance = 200.0f;
	DashTime = 0.2f;
}

void APartyGameMultiplayerCharacter::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Replicate specific variables
	//DOREPLIFETIME(APartyGameMultiplayerCharacter, CurWeapon);
}

//////////////////////////////////////////////////////////////////////////
// Input

void APartyGameMultiplayerCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	//PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	//PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &APartyGameMultiplayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis("Move Right / Left", this, &APartyGameMultiplayerCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn Right / Left Mouse", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("Turn Right / Left Gamepad", this, &APartyGameMultiplayerCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("Look Up / Down Mouse", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Look Up / Down Gamepad", this, &APartyGameMultiplayerCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &APartyGameMultiplayerCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &APartyGameMultiplayerCharacter::TouchStopped);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &APartyGameMultiplayerCharacter::Attack);
	PlayerInputComponent->BindAction("Dash", IE_Pressed, this, &APartyGameMultiplayerCharacter::Dash);
}

void APartyGameMultiplayerCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void APartyGameMultiplayerCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void APartyGameMultiplayerCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void APartyGameMultiplayerCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void APartyGameMultiplayerCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void APartyGameMultiplayerCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}


void APartyGameMultiplayerCharacter::TouchWeapon(ABaseWeapon* pBaseWeapon)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, TEXT("Enter ANTCharacter::TouchWeapon"));
	if (TestInfo::SomeoneHasWeapon)
		return;
	// [Amos_TODO]
	// Generate a specific weapon in hand (only generate it on server, then let the server replicate it)
	if (GetLocalRole() == ROLE_Authority && !TestInfo::SomeoneHasWeapon)
	{
		TestInfo::SomeoneHasWeapon = true;

		//auto SpawnTransform = GetMesh()->GetSocketTransform("hand_r_WeaponSocket");
		//FVector spawnLocation = SpawnTransform.GetLocation();
		//FRotator spawnRotation = SpawnTransform.GetRotation().Rotator();
		//FActorSpawnParameters spawnParameters;
		//spawnParameters.Instigator = GetInstigator();
		//spawnParameters.Owner = this;

		//auto baseWeapon = GetWorld()->SpawnActor<AWeaponFork>(spawnLocation, spawnRotation, spawnParameters);
		//auto baseWeapon = GetWorld()->SpawnActor<AWeaponBlower>(spawnLocation, spawnRotation, spawnParameters);
		CurWeapon = pBaseWeapon;
		/*CurWeapon->SetActorLocationAndRotation(spawnLocation, spawnRotation);*/

		FAttachmentTransformRules attachmentTransformRules = FAttachmentTransformRules::SnapToTargetNotIncludingScale;
		CurWeapon->AttachToComponent(GetMesh(), attachmentTransformRules, "hand_r_WeaponSocket");
	}
}

void APartyGameMultiplayerCharacter::Attack()
{
	HandleAttack();
}

void APartyGameMultiplayerCharacter::Dash()
{
	HandleDash(true);
	UWorld* World = GetWorld();
	World->GetTimerManager().SetTimer(DashingTimer, this, &APartyGameMultiplayerCharacter::StopDashing, DashTime, false);
}

void APartyGameMultiplayerCharacter::StopDashing()
{
	HandleDash(false);
}

void APartyGameMultiplayerCharacter::HandleAttack_Implementation()
{
	if (CurWeapon)
	{
		if (!bAttackOn)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, TEXT("Player starts the Attack!"));
			CurWeapon->AttackStart();
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, TEXT("Player ends the Attack!"));
			CurWeapon->AttackStop();
		}
		bAttackOn = !bAttackOn;
	}
}

void APartyGameMultiplayerCharacter::HandleDash_Implementation(bool DashOn)
{
	if (DashOn)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Dashing"));
		DashSpeed = DashDistance / DashTime;
		GetCharacterMovement()->MaxWalkSpeed = DashSpeed;
		GetCharacterMovement()->Velocity *= DashSpeed / GetCharacterMovement()->Velocity.Size();
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("StopDashing"));
		GetCharacterMovement()->MaxWalkSpeed = OriginalMaxWalkSpeed;
		FVector Velocity_copy = GetCharacterMovement()->Velocity;
		if (OriginalMaxWalkSpeed < Velocity_copy.Size())
		{
			Velocity_copy.Normalize();
			GetCharacterMovement()->Velocity = Velocity_copy * OriginalMaxWalkSpeed;
		}
	}
}