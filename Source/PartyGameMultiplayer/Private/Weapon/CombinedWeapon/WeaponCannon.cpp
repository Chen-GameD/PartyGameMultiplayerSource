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


void AWeaponCannon::SpawnProjectile()
{
	auto pCharacter = GetOwner();
	if (pCharacter && SpecificProjectileClass)
	{
		FVector spawnLocation = SpawnProjectilePointMesh->GetComponentLocation();
		FRotator spawnRotation = (pCharacter->GetActorRotation().Vector() + 
						FMath::RandRange(0.6f, 0.9f) * pCharacter->GetActorUpVector() + 
						FMath::RandRange(-0.2f, 0.2f) * pCharacter->GetActorRightVector()).Rotation();  // character up with random bias

		FActorSpawnParameters spawnParameters;
		spawnParameters.Instigator = GetInstigator();
		spawnParameters.Owner = this;

		//ABaseProjectile* spawnedProjectile = NewObject<ABaseProjectile>(this, SpecificProjectileClass);
		ABaseProjectile* spawnedProjectile = GetWorld()->SpawnActor<ABaseProjectile>(SpecificProjectileClass, spawnLocation, spawnRotation, spawnParameters);
	}
}