// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelInteraction/MinigameObject/MinigameObj_Statue.h"

#include <string>
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

#include "M_PlayerState.h"
#include "Character/MPlayerController.h"
#include "Components/SphereComponent.h"
#include "Engine/StaticMeshActor.h"
#include "GameBase/MGameMode.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/ElementWeapon/WeaponShell.h"
#include "Components/WidgetComponent.h"
#include "UI/MinigameObjFollowWidget.h"

AMinigameObj_Statue::AMinigameObj_Statue()
{
	RootMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RootMesh"));
	RootMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	RootMesh->SetupAttachment(RootComponent);

	GodRayMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GodRayMesh"));
	GodRayMesh->SetupAttachment(RootMesh);
	GodRayMesh->SetCollisionProfileName(TEXT("Trigger"));
	GodRayMesh->SetRelativeScale3D(FVector(0.01f, 0.01f, 1.0f));

	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(RootMesh);

	CrabCenterMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CrabCenterMesh"));
	CrabCenterMesh->SetupAttachment(SkeletalMesh);
	CrabCenterMesh->SetCollisionProfileName(TEXT("NoCollision"));

	ShellOverlapComponent = CreateDefaultSubobject<USphereComponent>(TEXT("ShellOverlapSphere"));
	ShellOverlapComponent->SetupAttachment(RootMesh);
	ShellOverlapComponent->SetSphereRadius(300, true);

	EffectDarkBubbleOn = CreateDefaultSubobject<UNiagaraComponent>(TEXT("EffectDarkBubbleOn"));
	EffectDarkBubbleOn->SetupAttachment(ShellOverlapComponent);
	EffectDarkBubbleOn->bAutoActivate = true;

	FollowWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("FollowWidget"));
	FollowWidget->SetupAttachment(RootMesh);

	Landing_NC = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Landing_NC"));
	Landing_NC->SetupAttachment(SkeletalMesh);
	Landing_NC->bAutoActivate = false;

	Explode_NC = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ExplodeVfx"));
	Explode_NC->SetupAttachment(SkeletalMesh);
	Explode_NC->bAutoActivate = false;
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> DefaultExplodeNS(TEXT("/Game/ArtAssets/Niagara/NS_CrabExplode.NS_CrabExplode"));
	if (DefaultExplodeNS.Succeeded())
		Explode_NC->SetAsset(DefaultExplodeNS.Object);
	
	MaxHealth = 7;
	CurrentHealth = MaxHealth;
	CurrentSocketIndex = 0;

	ShellOverlapComponent_TargetScale = 1.0f;
	ShellOverlapComponent_MinScale = 0.6f;
	ShellOverlapComponent_MaxScale = 1.0f;

	IsDropping = true;
	DroppingTargetHeight = 100.0f;
	DroppingSpeed = 0;

	ExplodeDelay = 2.0f;
	LittleCrabDelay = 4.25f;
	RespawnDelay = 7.5f;
}


void AMinigameObj_Statue::BeginPlay()
{
	Super::BeginPlay();

	// Server
	if (GetLocalRole() == ROLE_Authority)
	{
		//  Set DisplayCaseCollision to active
		SetActorEnableCollision(true);
		
		if (ShellOverlapComponent)
		{
			ShellOverlapComponent->SetCollisionProfileName(TEXT("Trigger"));
			ShellOverlapComponent->SetGenerateOverlapEvents(true);

			ShellOverlapComponent->OnComponentBeginOverlap.AddDynamic(this, &AMinigameObj_Statue::OnShellOverlapBegin);
			ShellOverlapComponent->OnComponentEndOverlap.AddDynamic(this, &AMinigameObj_Statue::OnShellOverlapEnd);

			GodRayMesh->OnComponentBeginOverlap.AddDynamic(this, &AMinigameObj_Statue::OnGodRayOverlapBegin);
		}
	}
	// Client
	else
	{
		// Cannot disable the collision(to improve performance), 
		// because the server would correct client's position during collsion happening on the server, causing shaking in client's end.
		//SetActorEnableCollision(false);
	}

	ShellOverlapComponent->SetRelativeScale3D(FVector(0.01f, 0.01f, 0.01f));

	GodRayMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	FVector GodRayMeshWorldLocation = GodRayMesh->GetComponentLocation();
	GodRayMeshWorldLocation.Z = DroppingTargetHeight;
	GodRayMesh->SetWorldLocation(GodRayMeshWorldLocation);

	// UI
	if (UMinigameObjFollowWidget* pFollowWidget = Cast<UMinigameObjFollowWidget>(FollowWidget->GetUserWidgetObject()))
		pFollowWidget->SetHealthByPercentage(0);

	// Sfx
	CallStartSfx();
}

void AMinigameObj_Statue::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float GodRayExpandSpeed = 0;
	if (IsDropping && DroppingTargetHeight < GetActorLocation().Z)
	{
		// Statue
		float g = 980.0f;
		DroppingSpeed += g * DeltaTime;
		FVector TargetLocation = GetActorLocation() + FVector::DownVector * DroppingSpeed * DeltaTime;
		// The End
		if (TargetLocation.Z <= DroppingTargetHeight)
		{
			TargetLocation.Z = DroppingTargetHeight;
			//FollowWidget->SetVisibility(true);
			GodRayMesh->SetCollisionProfileName(TEXT("NoCollision"));
			GodRayMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			IsDropping = false;

			// Vfx
			if (Landing_NC && !Landing_NC->IsActive())
				Landing_NC->Activate();
			// Shake localcontrolled character's camera
			if (auto pController = GetWorld()->GetFirstPlayerController())
			{
				//if (auto pMCharacter = Cast<AMCharacter>(pController->GetPawn()))
				//{
				//	pMCharacter->CameraShakingTime = 0.7f;
				//	pMCharacter->CameraShakingIntensity = 10.0f;
				//}
				if (auto pCamMg = pController->PlayerCameraManager)
					pCamMg->StartCameraShake(CameraShakeTriggered);
			}
		}
		SetActorLocation(TargetLocation);

		GodRayExpandSpeed = 2.5f;		
	}
	else
	{
		FVector GodRayMesh_CurRelativeScale = GodRayMesh->GetRelativeScale3D();
		if(GodRayMesh_CurRelativeScale.X < 200.0f)
			GodRayExpandSpeed = 50.0f;
		else
			GodRayExpandSpeed = 75.0f;
	}

	// Dark bubble
	if (!IsDropping)
	{
		FVector ShellOverlapComponent_CurRelativeScale = ShellOverlapComponent->GetRelativeScale3D();
		if (ShellOverlapComponent_CurRelativeScale.X < ShellOverlapComponent_TargetScale)
		{
			// Scale Up
			float ShellOverlapComponent_ExpandSpeed = 0.75f;
			FVector NewRelativeScale = ShellOverlapComponent_CurRelativeScale + FVector::OneVector * DeltaTime * ShellOverlapComponent_ExpandSpeed;
			if (ShellOverlapComponent_TargetScale < NewRelativeScale.X)
				NewRelativeScale = FVector::OneVector * ShellOverlapComponent_TargetScale;
			ShellOverlapComponent->SetRelativeScale3D(NewRelativeScale);
			// Rotate
			float RotateSpeed = 100.0f;
			FRotator NewRotation = ShellOverlapComponent->GetComponentRotation();
			NewRotation.Roll += DeltaTime * RotateSpeed;
			//NewRotation.Pitch += DeltaTime * RotateSpeed;
			NewRotation.Yaw += DeltaTime * RotateSpeed;
			ShellOverlapComponent->SetWorldRotation(NewRotation);
		}
		else if (ShellOverlapComponent_TargetScale < ShellOverlapComponent_CurRelativeScale.X)
		{
			// Scale Down
			float ShellOverlapComponent_ShrinkSpeed = (0 == ShellOverlapComponent_TargetScale) ? 1.0f : 0.25f;
			FVector NewRelativeScale = ShellOverlapComponent_CurRelativeScale - FVector::OneVector * DeltaTime * ShellOverlapComponent_ShrinkSpeed;
			if (NewRelativeScale.X < ShellOverlapComponent_TargetScale)
				NewRelativeScale = FVector::OneVector * ShellOverlapComponent_TargetScale;
			ShellOverlapComponent->SetRelativeScale3D(NewRelativeScale);
			// Rotate
			if (0 == ShellOverlapComponent_TargetScale)
			{
				float RotateSpeed = 300.0f;
				FRotator NewRotation = ShellOverlapComponent->GetComponentRotation();
				NewRotation.Roll -= DeltaTime * RotateSpeed;
				//NewRotation.Pitch -= DeltaTime * RotateSpeed;
				NewRotation.Yaw -= DeltaTime * RotateSpeed;
				ShellOverlapComponent->SetWorldRotation(NewRotation);
			}
			// Transform
			if (0 == ShellOverlapComponent_TargetScale)
			{
				float LerpValue = 1.0f / ShellOverlapComponent_MinScale * FMath::Clamp(ShellOverlapComponent_MinScale - NewRelativeScale.X, 0.0f, ShellOverlapComponent_MinScale);
				FVector NewWorldLocation = FMath::Lerp(ShellOverlapComponent->GetComponentLocation(), CrabCenterMesh->GetComponentLocation(), LerpValue);
				ShellOverlapComponent->SetWorldLocation(NewWorldLocation);
			}
		}
	}
	// GodRay
	FVector GodRayMesh_CurRelativeScale = GodRayMesh->GetRelativeScale3D();
	if (GodRayMesh->IsVisible() && GodRayMesh_CurRelativeScale.X < 500.0f)
	{
		GodRayMesh_CurRelativeScale.X += GodRayExpandSpeed * DeltaTime;
		GodRayMesh_CurRelativeScale.Y += GodRayExpandSpeed * DeltaTime;
		GodRayMesh->SetRelativeScale3D(GodRayMesh_CurRelativeScale);
	}	
	else
	{
		GodRayMesh->SetVisibility(false);
	}
}


USkeletalMeshComponent* AMinigameObj_Statue::GetSkeletalMesh()
{
	return SkeletalMesh;
}

void AMinigameObj_Statue::OnShellOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AWeaponShell* OverlapShell = Cast<AWeaponShell>(OtherActor))
	{
		if (IsCompleteBuild || IsDropping)
			return;

		if (OverlapShell->IsPickedUp && OverlapShell->GetHoldingController())
		{
			if (auto pMCharacter = Cast<AMCharacter>(OverlapShell->GetHoldingController()->GetPawn()))
			{
				pMCharacter->Server_GiveShellToStatue(OverlapShell);
			}
		}

		CurrentHealth--;
		if (GetNetMode() == NM_ListenServer)
			OnRep_CurrentHealth();
		CurrentSocketIndex++;

		// Detect which player drop the shell
		// Add Score
		if (OverlapShell->GetPreHoldingController())
		{
			AMPlayerController* CurrentController = Cast<AMPlayerController>(OverlapShell->GetPreHoldingController());
			if (CurrentController)
			{
				AM_PlayerState* CurrentPS = CurrentController->GetPlayerState<AM_PlayerState>();
				if (CurrentPS)
				{
					CurrentPS->addScore(OverlapShell->GetScoreCanGet());
				}
			}
		}

		// Spawn a shell mesh and attach to the socket
		if (ShellMeshRef)
		{
			AMinigameChild_Statue_Shell* NewShellActor = GetWorld()->SpawnActor<AMinigameChild_Statue_Shell>(ShellMeshRef, OtherActor->GetActorLocation(), OtherActor->GetActorRotation());
			if (NewShellActor)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, TEXT("Shell Spawn Success!"));
				NewShellActor->TartgetStatue = this;
				FString SocketNameString = "Shell" + FString::FromInt(CurrentSocketIndex);
				NewShellActor->TargetSocketName = FName(*SocketNameString);
			}
		}

		// Respawn Sculpture
		AMGameMode* MyGameMode = Cast<AMGameMode>(GetWorld()->GetAuthGameMode());
		if (MyGameMode)
		{
			MyGameMode->Server_RespawnShellObject(OverlapShell->GetConfigIndex());
		}

		if (CurrentHealth <= 0)
		{
			IsCompleteBuild = true;

			// Add Sculpture Complete Score
			if (OverlapShell->GetPreHoldingController())
			{
				AMPlayerController* CurrentController = Cast<AMPlayerController>(OverlapShell->GetPreHoldingController());
				if (CurrentController)
				{
					AM_PlayerState* CurrentPS = CurrentController->GetPlayerState<AM_PlayerState>();
					if (CurrentPS)
					{
						CurrentPS->addScore(ScoreCanGet);

						// Broadcast information to all clients
						for (FConstPlayerControllerIterator iter = GetWorld()->GetPlayerControllerIterator(); iter; ++iter)
						{
							AMPlayerController* currentController = Cast<AMPlayerController>(*iter);
							currentController->UI_InGame_BroadcastMiniInformation(CurrentPS->TeamIndex, CurrentPS->PlayerNameString, MinigameInformation);
						}
					}
				}
			}

			Server_WhenDead();
		}

		// Destroy Shell
		OtherActor->Destroy();
	}
	else if (auto pMCharacter = Cast<AMCharacter>(OtherActor))
	{
		pMCharacter->Server_EnableEffectByCrabBubble(true);
	}
}

void AMinigameObj_Statue::OnShellOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (auto pMCharacter = Cast<AMCharacter>(OtherActor))
	{
		pMCharacter->Server_EnableEffectByCrabBubble(false);
	}
}

void AMinigameObj_Statue::OnGodRayOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (auto pMCharacter = Cast<AMCharacter>(OtherActor))
	{
		FVector MCharacterLocation = pMCharacter->GetActorLocation();
		FVector SourceLocation = FVector(0, -400.0f, 180.0f);
		SourceLocation.Z = MCharacterLocation.Z;
		FVector PushDirection = MCharacterLocation - SourceLocation;
		PushDirection.Normalize();
		pMCharacter->LaunchCharacter(PushDirection * 2000.0f, true, false);
	}
}

void AMinigameObj_Statue::OnRep_CurrentHealth()
{
	if (CurrentHealth <= 0)
	{
		// Exlode Vfx
		FTimerHandle ExplodeTimerHandle;
		GetWorldTimerManager().SetTimer(ExplodeTimerHandle, [this]
			{
				if (Explode_NC)
					Explode_NC->Activate();
			}, ExplodeDelay, false);

		// Hide Statue
		FTimerHandle HideStatueTimerHandle;
		GetWorldTimerManager().SetTimer(HideStatueTimerHandle, [this]
			{
				SetActorEnableCollision(false);
				SetActorLocation(GetActorLocation() + FVector(0, 0, -1000.0f));
				/*if(FollowWidget)
					FollowWidget->SetVisibility(false);*/

				// TODO: Add little crab sequencer

			}, LittleCrabDelay, false);

		// TODO
		//OnStatueFinishedEvent();

		// Sfx
		CallDeathSfx();
	}

	if (UMinigameObjFollowWidget* pFollowWidget = Cast<UMinigameObjFollowWidget>(FollowWidget->GetUserWidgetObject()))
		pFollowWidget->SetHealthByPercentage(CurrentHealth / MaxHealth);
}

void AMinigameObj_Statue::NewShellHasBeenInserted()
{
	ShellOverlapComponent_TargetScale = ShellOverlapComponent_MinScale +
		(ShellOverlapComponent_MaxScale - ShellOverlapComponent_MinScale) * CurrentHealth / MaxHealth;

	if (CurrentHealth <= 0)
	{
		FTimerHandle TimerHandle;
		float TimeFromFinalShellInsertToStartShrinkToZero = 0.8f;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
			{
				ShellOverlapComponent_TargetScale = 0;
			}, TimeFromFinalShellInsertToStartShrinkToZero, false);
	}
}

void AMinigameObj_Statue::Server_WhenDead()
{
	// Respawn(Destroy)
	FTimerHandle RespawnMinigameObjectTimerHandle;
	GetWorldTimerManager().SetTimer(RespawnMinigameObjectTimerHandle, this, &AMinigameObj_Statue::StartToRespawnActor, RespawnDelay, false);
}