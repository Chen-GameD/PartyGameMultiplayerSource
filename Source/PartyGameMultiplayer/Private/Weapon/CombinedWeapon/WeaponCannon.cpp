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
	IsCombined = true;
	WeaponType = EnumWeaponType::Cannon;
	WeaponName = WeaponEnumToString_Map[WeaponType];
	AttackType = EnumAttackType::SpawnProjectile;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("/Game/ArtAssets/Models/Cannon/Cannon.Cannon"));
	//Set the Static Mesh and its position/scale if we successfully found a mesh asset to use.
	if (DefaultMesh.Succeeded())
	{
		WeaponMesh->SetStaticMesh(DefaultMesh.Object);
	}

	//AttackDetectComponent = WeaponMesh;

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

	DamageType = UDamageType::StaticClass();
}


void AWeaponCannon::AttackStart()
{
	if (bAttackOn || !GetOwner())
		return;

	bAttackOn = true;
	// Listen server
	if (GetNetMode() == NM_ListenServer)
	{
		OnRep_bAttackOn();
	}
	ApplyDamageCounter = 0;

	SpawnProjectile();
}


void AWeaponCannon::AttackStop()
{
	if (!bAttackOn || !GetOwner())
		return;

	check(GetOwner() != nullptr);

	bAttackOn = false;
	// Listen server
	if (GetNetMode() == NM_ListenServer)
	{
		OnRep_bAttackOn();
	}
	ApplyDamageCounter = 0;
}


void AWeaponCannon::SpawnProjectile()
{
	auto pCharacter = GetOwner();
	if (pCharacter && SpecificProjectileClass)
	{
		FVector spawnLocation = SpawnProjectilePointMesh->GetComponentLocation();
		FRotator spawnRotation = (pCharacter->GetActorRotation().Vector() * 1.73f + pCharacter->GetActorUpVector()).Rotation();  // character up 30 degree

		FActorSpawnParameters spawnParameters;
		spawnParameters.Instigator = GetInstigator();
		spawnParameters.Owner = this;

		//ABaseProjectile* spawnedProjectile = NewObject<ABaseProjectile>(this, SpecificProjectileClass);
		ABaseProjectile* spawnedProjectile = GetWorld()->SpawnActor<ABaseProjectile>(SpecificProjectileClass, spawnLocation, spawnRotation, spawnParameters);
	}
}