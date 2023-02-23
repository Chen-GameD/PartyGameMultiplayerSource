// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/CombinedWeapon/ProjectileCannon.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AProjectileCannon::AProjectileCannon()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("/Game/ArtAssets/Models/Cannon/CannonBomb.CannonBomb"));
	if (DefaultMesh.Succeeded())
	{
		StaticMesh->SetStaticMesh(DefaultMesh.Object);
		StaticMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
		StaticMesh->SetRelativeScale3D(FVector(8.0f, 8.0f, 8.0f));
	}
	StaticMesh->SetVisibility(false);

	CannonMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CannonMesh"));
	CannonMesh->SetupAttachment(StaticMesh);

	CannonMesh->SetCollisionProfileName(TEXT("NoCollision"));
	if (DefaultMesh.Succeeded())
	{
		CannonMesh->SetStaticMesh(DefaultMesh.Object);
		CannonMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
		CannonMesh->SetRelativeScale3D(FVector(8.0f, 8.0f, 8.0f));
	}

	ProjectileMovementComponent->InitialSpeed = 1000.0f;
	ProjectileMovementComponent->MaxSpeed = 1000.0f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->ProjectileGravityScale = 1.0f;

}


void AProjectileCannon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FRotator NewRotation = CannonMesh->GetRelativeRotation();
	float DeltaRotation = DeltaTime * 300.0f;
	NewRotation.Yaw += DeltaRotation;
	CannonMesh->SetRelativeRotation(NewRotation);
}


void AProjectileCannon::BeginPlay()
{
	Super::BeginPlay();

	CannonMesh->SetRelativeRotation(FVector(FMath::RandRange(0.f, 3.14f), FMath::RandRange(0.f, 3.14f), FMath::RandRange(0.f, 3.14f)).Rotation());
}

void AProjectileCannon::OnRep_HasExploded()
{
	if (HasExploded)
	{
		StaticMesh->SetVisibility(false);
		CannonMesh->SetVisibility(false);
		ProjectileMovementComponent->StopMovementImmediately();
		if (AttackHitEffect_NSSystem)
			AttackHitEffect_NSComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), AttackHitEffect_NSSystem, GetActorLocation());
	}
}
