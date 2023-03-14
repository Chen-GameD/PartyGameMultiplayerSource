// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelInteraction/MinigameObject/MinigameObj_Statue.h"

#include "Weapon/ElementWeapon/WeaponShell.h"

AMinigameObj_Statue::AMinigameObj_Statue()
{
	RootMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RootMesh"));
	RootMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	RootMesh->SetupAttachment(RootComponent);
	MaxHealth = 7;
	CurrentHealth = MaxHealth;
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

void AMinigameObj_Statue::OnShellOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		AWeaponShell* OverlapShell = Cast<AWeaponShell>(OtherActor);
		if (OverlapShell)
		{
			this->CurrentHealth--;
			// Detect which player drop the shell
			// TODO...
			
			// Consider respawn the shell
			// TODO...
			
			// Destroy Shell
			OtherActor->Destroy();
		}
	}
}

void AMinigameObj_Statue::OnShellOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	
}
