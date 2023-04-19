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
	
	if (TargetActor)
	{		
		SpotLightMesh->SetWorldLocation(TargetActor->GetActorLocation());
		FVector Vector_LightSource_to_Target = SourceLocation - SpotLightMesh->GetComponentLocation();
		/*GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Vector_LightSource_to_Target.Size(): %f"), Vector_LightSource_to_Target.Size()));*/
		SpotLightMesh->SetWorldRotation(Vector_LightSource_to_Target.Rotation());
		FVector NewScale = SpotLightMesh->GetComponentScale();
		NewScale.X = 2.0f * Vector_LightSource_to_Target.Size() / 2350.0f;
		SpotLightMesh->SetWorldScale3D(NewScale);
	}
}

