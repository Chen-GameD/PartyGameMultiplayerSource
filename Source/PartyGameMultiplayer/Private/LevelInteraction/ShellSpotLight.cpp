// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelInteraction/ShellSpotLight.h"

AShellSpotLight::AShellSpotLight()
{
	PrimaryActorTick.bCanEverTick = true;

	SpotLightMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpotLightMesh"));
	SpotLightMesh->SetupAttachment(RootComponent);
	SpotLightMesh->SetCollisionProfileName(TEXT("NoCollision"));

	TargetActor = nullptr;
}

void AShellSpotLight::BeginPlay()
{
	Super::BeginPlay();
	
}

void AShellSpotLight::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (TargetActor && SpotLightMesh)
	{
		FVector Vector_LightSource_to_Target = TargetActor->GetActorLocation() - SpotLightMesh->GetComponentLocation();
		SpotLightMesh->SetWorldRotation(Vector_LightSource_to_Target.Rotation());
	}
}

