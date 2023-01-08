// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelInteraction/Interactable.h"
#include "Net/UnrealNetwork.h"


AInteractable::AInteractable()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
}


void AInteractable::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetLocalRole() == ROLE_Authority)
	{
		RootLocation = GetActorLocation();
		RootRotation = GetActorRotation();
		RootScale = GetActorScale3D();
	}
}


void AInteractable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Replicate specific variables
	DOREPLIFETIME(AInteractable, RootLocation);
	DOREPLIFETIME(AInteractable, RootRotation);
	DOREPLIFETIME(AInteractable, RootScale);
}


// Called when the game starts or when spawned
void AInteractable::BeginPlay()
{
	Super::BeginPlay();	
}


void AInteractable::OnRep_Transform()
{
	//SetWorldLocation(RootLocation);
	//SetWorldRotation(RootRotation);
	SetActorLocationAndRotation(RootLocation, RootRotation);
	SetActorScale3D(RootScale);
}


