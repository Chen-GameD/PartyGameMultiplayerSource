// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ElementWeapon/ProjectileAlarm.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AProjectileAlarm::AProjectileAlarm()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("/Game/ArtAssets/Models/Alarm/Alarm01.Alarm01"));
	if (DefaultMesh.Succeeded())
	{
		StaticMesh->SetStaticMesh(DefaultMesh.Object);
		StaticMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
		StaticMesh->SetRelativeScale3D(FVector(0.75f, 0.75f, 0.75f));
	}

	ProjectileMovementComponent->InitialSpeed = 750.0f;
	ProjectileMovementComponent->MaxSpeed = 750.0f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->ProjectileGravityScale = 1.0f;

	//static ConstructorHelpers::FObjectFinder<UNiagaraSystem> DefaultAttackHitEffect(TEXT("/Game/ArtAssets/Niagara/NS_Soundwave.NS_Soundwave"));
	//if (DefaultAttackHitEffect.Succeeded())
	//{
	//	UNiagaraSystem* AttackHitEffect_NiagaraSystem = DefaultAttackHitEffect.Object;
	//	AttackHitEffect_NSComponent->SetAsset(AttackHitEffect_NiagaraSystem);
	//}

	Shake_TimeSinceChangeDirection = 0;
	Shake_Clockwise = true;
}

void AProjectileAlarm::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasExploded)
	{
		FRotator NewWorldRotation = StaticMesh->GetRelativeRotation();
		NewWorldRotation.Yaw += DeltaTime * (Shake_Clockwise ? 500.0f : -500.0f);
		FVector VectorOfRotation = NewWorldRotation.Vector();
		VectorOfRotation.X += DeltaTime * (Shake_Clockwise ? 13.0f : -13.0f);
		StaticMesh->SetRelativeRotation(VectorOfRotation.Rotation());
		Shake_TimeSinceChangeDirection += DeltaTime;
		if (0.05f < Shake_TimeSinceChangeDirection)
		{
			Shake_TimeSinceChangeDirection = 0;
			Shake_Clockwise = !Shake_Clockwise;
		}
		FVector NewLocation = StaticMesh->GetComponentLocation();
		NewLocation.Z -= DeltaTime * 25.0f;
		StaticMesh->SetWorldLocation(NewLocation);
	}		
}

void AProjectileAlarm::OnRep_HasExploded()
{
	if (HasExploded)
	{
		ProjectileMovementComponent->StopMovementImmediately();
		ProjectileMovementComponent->SetUpdatedComponent(nullptr);
		StaticMesh->SetSimulatePhysics(false);
		StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		if (AttackHitEffect_NSSystem)
			AttackHitEffect_NSComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), AttackHitEffect_NSSystem, GetActorLocation());
	}
}