// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelInteraction/MinigameObject/MinigameObj_Statue.h"

#include <string>

#include "Character/MPlayerController.h"
#include "Components/SphereComponent.h"
#include "Engine/StaticMeshActor.h"
#include "GameBase/MGameMode.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/ElementWeapon/WeaponShell.h"

AMinigameObj_Statue::AMinigameObj_Statue()
{
	RootMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RootMesh"));
	RootMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	RootMesh->SetupAttachment(RootComponent);

	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(RootMesh);

	ShellOverlapComponent = CreateDefaultSubobject<USphereComponent>(TEXT("ShellOverlapSphere"));
	ShellOverlapComponent->SetupAttachment(RootMesh);
	ShellOverlapComponent->SetSphereRadius(300, true);
	
	MaxHealth = 7;
	CurrentHealth = MaxHealth;
	CurrentSocketIndex = 0;
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
		// Disable some high-cost behaviors
		SetActorEnableCollision(false);
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
			this->CurrentHealth--;
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
				if (GetNetMode() == NM_ListenServer)
				{
					OnRep_CurrentHealth();
				}

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

				// Consider respawn the shell
				// Set timer and respawn this actor
				FTimerHandle RespawnMinigameObjectTimerHandle;
				GetWorldTimerManager().SetTimer(RespawnMinigameObjectTimerHandle, this, &AMinigameObj_Statue::StartToRespawnActor, 5, false);
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
	Super::OnRep_CurrentHealth();

	if (CurrentHealth <= 0)
	{
		// if (SkeletalMesh)
		// {
		// 	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Destroying MiniGameObjective's SkeletalMesh on Client"));
		// 	SkeletalMesh->DestroyComponent();
		// }
		// if (BlowUpEffect)
		// {
		// 	BlowUpEffect->Activate();
		// }
		// EnableBlowUpGeometryCacheComponent();

		// Destroy VFX & Effect
		// TODO
		OnStatueFinishedEvent();
	}
}
