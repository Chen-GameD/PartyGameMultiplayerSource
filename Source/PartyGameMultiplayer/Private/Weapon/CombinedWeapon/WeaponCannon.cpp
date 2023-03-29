// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/CombinedWeapon/WeaponCannon.h"
#include "Weapon/BaseProjectile.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AWeaponCannon::AWeaponCannon()
{
	IsCombineWeapon = true;
	WeaponType = EnumWeaponType::Cannon;
	AttackType = EnumAttackType::SpawnProjectile;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("/Game/ArtAssets/Models/Cannon/Cannon.Cannon"));
	if (DefaultMesh.Succeeded())
	{
		WeaponMesh->SetStaticMesh(DefaultMesh.Object);
	}

	//AttackDetectComponent = WeaponMesh;  // No AttackDetectComponent is needed for SpawnProjectile type weapon

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> DefaultAttackOnEffect(TEXT("/Game/ArtAssets/Niagara/NS_FlameForkNew.NS_FlameForkNew"));
	if (DefaultAttackOnEffect.Succeeded())
	{
		UNiagaraSystem* AttackOnEffect_NiagaraSystem = DefaultAttackOnEffect.Object;
		AttackOnEffect->SetAsset(AttackOnEffect_NiagaraSystem);
	}

	static ConstructorHelpers::FObjectFinder<UParticleSystem> DefaultAttackHitEffect(TEXT("/Game/StarterContent/Particles/P_Explosion.P_Explosion"));
	if (DefaultAttackHitEffect.Succeeded())
	{
		AttackHitEffect = DefaultAttackHitEffect.Object;
	}
}


void AWeaponCannon::SpawnProjectile(float AttackTargetDistance)
{
	auto pCharacter = GetOwner();
	if (pCharacter && SpecificProjectileClass)
	{
		float verticalRatio = 1.275f;

		FVector spawnLocation = SpawnProjectilePointMesh->GetComponentLocation();
		FRotator spawnRotation = (pCharacter->GetActorRotation().Vector() +
			FMath::RandRange(1.25f, 1.3f) * pCharacter->GetActorUpVector() +
			FMath::RandRange(-0.1f, 0.1f) * pCharacter->GetActorRightVector()).Rotation();  // character up with random bias

		FActorSpawnParameters spawnParameters;
		spawnParameters.Instigator = GetInstigator();
		spawnParameters.Owner = this;

		auto pProjectile = GetWorld()->SpawnActor<ABaseProjectile>(SpecificProjectileClass, spawnLocation, spawnRotation, spawnParameters);
		if (pProjectile)
		{
			float g = 980.0f;
			if (pProjectile->ProjectileMovementComponent)
				g *= pProjectile->ProjectileMovementComponent->ProjectileGravityScale;
			float h = 90.0f;
			/*
				V_x, V_y, V are all scalar instead of vector in the following equations.
				V_x * t = D
				V_y = r * V_x (r is verticalRatio)
				V_y * t - 0.5 * g * t^2 = -h
				Finally, V = V_x * (V/V_x) = sqrt(0.5 * g * D^2 / r * D + h) * sqrt(V_x^2 + V_y^2);
			*/
			float NewSpeed = FMath::Sqrt(0.5f * g * FMath::Square(AttackTargetDistance) / (verticalRatio * AttackTargetDistance + h)) *
				FMath::Sqrt(FMath::Square(1.0f) + FMath::Square(verticalRatio));
			float SpeedRatio = 1.0f;
			if (pProjectile->ProjectileMovementComponent)
				SpeedRatio = NewSpeed / pProjectile->ProjectileMovementComponent->MaxSpeed;
			pProjectile->NetMulticast_ChangeSpeed(SpeedRatio);
		}
	}
}