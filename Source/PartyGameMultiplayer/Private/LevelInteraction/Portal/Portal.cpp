// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelInteraction/Portal/Portal.h"
//#include "LevelInteraction/Portal/PortalManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Character/MCharacter.h"

APortal::APortal()
{
	PrimaryActorTick.bCanEverTick = true;

	RootMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RootMesh"));
	RootMesh->SetupAttachment(RootComponent);
	RootMesh->SetCollisionProfileName(TEXT("NoCollision"));

	PortalTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("PortalTrigger"));
	PortalTrigger->SetupAttachment(RootMesh);
	PortalTrigger->SetCollisionProfileName(TEXT("Trigger"));

	LaunchPointMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LaunchPointMesh"));
	LaunchPointMesh->SetupAttachment(RootMesh);
	LaunchPointMesh->SetCollisionProfileName(TEXT("NoCollision"));

	MyPortalManager = nullptr;
	Server_LastExitTime = -1.0f;
}

void APortal::BeginPlay()
{
	Super::BeginPlay();

	// Server
	if (GetLocalRole() == ROLE_Authority)
	{
		DrawDebugBox(GetWorld(), PortalTrigger->GetComponentLocation(), PortalTrigger->GetScaledBoxExtent(), FColor::Purple, true, -1, 0, 5);

		PortalTrigger->OnComponentBeginOverlap.AddDynamic(this, &APortal::OnPortalTriggerOverlapBegin);
	}
	// Non-Server
	if (GetLocalRole() != ROLE_Authority)
		SetActorEnableCollision(false);

}

void APortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APortal::OnPortalTriggerOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
	class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AMCharacter* pMCharacter = Cast<AMCharacter>(OtherActor))
	{
		//MyPortalManager->StartToPort(this, pMCharacter);

		// Recover
		float CharacterCurHealth = pMCharacter->GetCurrentHealth();
		float HealthToRecover = 25.0f;
		FString ParName = "PortalHealth";
		if (AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map.Contains(ParName))
			HealthToRecover = AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map[ParName];
		pMCharacter->SetCurrentHealth(CharacterCurHealth + HealthToRecover);

		// Port
		int32 Size = PortalExits.Num();
		int32 StartID = 0;
		int32 EndId = Size - 1;
		int32 TargetPortalID = FMath::RandRange(StartID, EndId);
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("TargetPortalID: %i"), TargetPortalID));
		bool bFindValidPort;
		for (size_t i = 0; i < Size; i++)
		{
			if (TargetPortalID < Size)
			{
				auto& PortalExit = PortalExits[TargetPortalID];
				if (PortalExit->Server_LastExitTime < 0 || 1.0f < GetWorld()->TimeSeconds - PortalExit->Server_LastExitTime)
				{
					bFindValidPort = true;
					break;
				}
				else
					TargetPortalID = (TargetPortalID + 1) % Size;
			}
			else
				return;
		}

		if (!bFindValidPort)
			return;

		auto& PortalExit = PortalExits[TargetPortalID];
		PortalExit->Server_LastExitTime = GetWorld()->TimeSeconds;
		FVector NewWorldLocation = PortalExit->LaunchPointMesh->GetComponentLocation();
		FRotator NewWorldRotation = PortalExit->LaunchPointMesh->GetComponentRotation();
		pMCharacter->NetMulticast_SetWorldLocationRotation(NewWorldLocation, pMCharacter->GetActorRotation());

		FVector RotationVector = NewWorldRotation.Vector();
		RotationVector.Normalize();
		FTimerHandle TimerHandle;
		float LaunchDelay = 0.05f;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [pMCharacter, RotationVector]()
			{
				if (pMCharacter)
					pMCharacter->LaunchCharacter(RotationVector * 500.0f, true, false);

			}, LaunchDelay, false);
	}
}