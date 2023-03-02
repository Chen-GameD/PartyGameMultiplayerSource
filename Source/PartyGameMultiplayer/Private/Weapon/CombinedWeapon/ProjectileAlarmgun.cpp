// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/CombinedWeapon/ProjectileAlarmgun.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AProjectileAlarmgun::AProjectileAlarmgun()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("/Game/ArtAssets/Models/AlarmGun/Alarmpin.Alarmpin"));
	if (DefaultMesh.Succeeded())
	{
		StaticMesh->SetStaticMesh(DefaultMesh.Object);
		StaticMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
		StaticMesh->SetRelativeScale3D(FVector(0.75f, 0.75f, 0.75f));
	}

	ProjectileMovementComponent->InitialSpeed = 1800.0f;
	ProjectileMovementComponent->MaxSpeed = 1800.0f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->ProjectileGravityScale = 0.0f;

}
