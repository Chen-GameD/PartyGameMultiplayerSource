// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelInteraction/Portal/PortalManager.h"
#include "LevelInteraction/Portal/Portal.h"
#include "Character/MCharacter.h"

//APortalManager::APortalManager()
//{
//	PrimaryActorTick.bCanEverTick = true;
//
//}
//
//void APortalManager::BeginPlay()
//{
//	Super::BeginPlay();
//	
//	for (int i = 0; i < Portals.Num(); i++)
//	{
//		Portals[i]->MyPortalManager = this;
//	}
//}
//
//void APortalManager::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//
//}
//
//void APortalManager::StartToPort(class APortal* pPortal, class AMCharacter* pMCharacter)
//{
//	if (!pPortal || !pMCharacter)
//		return;
//	
//	// Recover
//	float CharacterCurHealth = pMCharacter->GetCurrentHealth();
//	float HealthToRecover = 20.0f;
//	pMCharacter->SetCurrentHealth(CharacterCurHealth + HealthToRecover);
//
//	// Port
//	auto& PortalList = pPortal->PortalExits;
//	int32 Size = PortalList.Num();
//	int32 StartID = 0;
//	int32 EndId = Size - 1;
//	int32 TargetPortalID = FMath::RandRange(StartID, EndId);
//	bool bFindValidPort;
//	for (size_t i = 0; i < Size; i++)
//	{
//		if (TargetPortalID < Size)
//		{
//			auto& Portal = PortalList[TargetPortalID];
//			if (1.0f < GetWorld()->TimeSeconds - Portal->Server_LastPortTime)
//			{
//				bFindValidPort = true;
//				break;
//			}
//			else
//				TargetPortalID = (TargetPortalID + 1) % Size;
//		}
//		else
//			return;
//	}
//
//	if (!bFindValidPort)
//		return;
//
//	auto& Portal = PortalList[TargetPortalID];
//	FVector NewWorldLocation = Portal->LaunchPointMesh->GetComponentLocation();
//	FRotator NewWorldRotation = Portal->LaunchPointMesh->GetComponentRotation();
//	pMCharacter->NetMulticast_SetWorldLocationRotation(NewWorldLocation, pMCharacter->GetActorRotation());
//
//	FVector RotationVector = NewWorldRotation.Vector();
//	RotationVector.Normalize();
//	DrawDebugLine
//	(
//		GetWorld(),
//		NewWorldLocation,
//		NewWorldLocation + RotationVector * 100.0f,
//		FColor::Purple,
//		true,
//		10.0f,
//		1,
//		100.0f
//	);
//
//	FTimerHandle TimerHandle;
//	float LaunchDelay = 0.05f;
//	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [pMCharacter, RotationVector]()
//		{
//			if (pMCharacter)
//				pMCharacter->LaunchCharacter(RotationVector * 1000.0f, true, false);
//
//		}, LaunchDelay, false);
//}

