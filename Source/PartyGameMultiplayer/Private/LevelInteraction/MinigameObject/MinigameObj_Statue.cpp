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
	GodRayMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	GodRayMesh->SetRelativeScale3D(FVector(0.01f, 0.01f, 1.0f));

	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(RootMesh);

	ShellOverlapComponent = CreateDefaultSubobject<USphereComponent>(TEXT("ShellOverlapSphere"));
	ShellOverlapComponent->SetupAttachment(RootMesh);
	ShellOverlapComponent->SetSphereRadius(300, true);

	FollowWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("FollowWidget"));
	FollowWidget->SetupAttachment(RootMesh);

	Explode_NC = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ExplodeVfx"));
	Explode_NC->SetupAttachment(SkeletalMesh);
	Explode_NC->bAutoActivate = false;
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> DefaultExplodeNS(TEXT("/Game/ArtAssets/Niagara/NS_CrabExplode.NS_CrabExplode"));
	if (DefaultExplodeNS.Succeeded())
		Explode_NC->SetAsset(DefaultExplodeNS.Object);
	
	MaxHealth = 7;
	CurrentHealth = MaxHealth;
	CurrentSocketIndex = 0;

	IsDropping = true;
	DroppingTargetHeight = 100.0f;
	DroppingSpeed = 0;

	ExplodeDelay = 2.0f;
	LittleCrabDelay = 4.25f;
	RespawnDelay = 10.0f;
}

void AMinigameObj_Statue::BeginPlay()
{
	Super::BeginPlay();

	// Server: Register collision callback functions
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
		}
	}
	// Client
	else
	{
		// Cannot disable the collision(to improve performance), 
		// because the server would correct client's position during collsion happening on the server, causing shaking in client's end.
		//SetActorEnableCollision(false);
	}

	GodRayMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	FVector GodRayMeshWorldLocation = GodRayMesh->GetComponentLocation();
	GodRayMeshWorldLocation.Z = DroppingTargetHeight;
	GodRayMesh->SetWorldLocation(GodRayMeshWorldLocation);

	// UI
	if (UMinigameObjFollowWidget* pFollowWidget = Cast<UMinigameObjFollowWidget>(FollowWidget->GetUserWidgetObject()))
		pFollowWidget->SetHealthByPercentage(0);
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
		}
		SetActorLocation(TargetLocation);

		GodRayExpandSpeed = 2.5f;		
	}
	else
	{
		GodRayExpandSpeed = 30.0f;
	}

	// GodRay
	FVector CurRelativeScale = GodRayMesh->GetRelativeScale3D();
	if (CurRelativeScale.X < 250.0f)
	{
		CurRelativeScale.X += GodRayExpandSpeed * DeltaTime;
		CurRelativeScale.Y += GodRayExpandSpeed * DeltaTime;
		GodRayMesh->SetRelativeScale3D(CurRelativeScale);
	}	
}


USkeletalMeshComponent* AMinigameObj_Statue::GetSkeletalMesh()
{
	return SkeletalMesh;
}

void AMinigameObj_Statue::OnShellOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && !IsCompleteBuild)
	{
		AWeaponShell* OverlapShell = Cast<AWeaponShell>(OtherActor);		
		if (OverlapShell)
		{
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
						}
					}
				}

				Server_WhenDead();
			}

			// Destroy Shell
			OtherActor->Destroy();
		}
	}
}

void AMinigameObj_Statue::OnShellOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	
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
		OnStatueFinishedEvent();

		// Sfx
		CallDeathSfx();
	}

	if (UMinigameObjFollowWidget* pFollowWidget = Cast<UMinigameObjFollowWidget>(FollowWidget->GetUserWidgetObject()))
		pFollowWidget->SetHealthByPercentage(CurrentHealth / MaxHealth);
}

void AMinigameObj_Statue::Server_WhenDead()
{
	// Respawn(Destroy)
	FTimerHandle RespawnMinigameObjectTimerHandle;
	GetWorldTimerManager().SetTimer(RespawnMinigameObjectTimerHandle, this, &AMinigameObj_Statue::StartToRespawnActor, RespawnDelay, false);
}