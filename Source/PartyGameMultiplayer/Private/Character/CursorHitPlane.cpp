// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/CursorHitPlane.h"
#include "Character/MCharacter.h"


ACursorHitPlane::ACursorHitPlane()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionPlane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CollisionPlane"));
	CollisionPlane->SetupAttachment(RootComponent);
}


void ACursorHitPlane::BeginPlay()
{
	Super::BeginPlay();
}


void ACursorHitPlane::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (pMCharacter)
	{
		CollisionPlane->SetWorldLocation(pMCharacter->GetActorLocation());
		CollisionPlane->SetWorldRotation(pMCharacter->GetActorRotation());
	}
}

