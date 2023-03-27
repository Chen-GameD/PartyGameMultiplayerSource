// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ElementWeapon/WeaponAlarm.h"
#include "Weapon/ElementWeapon/ProjectileAlarm.h"
#include "Character/MCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/PrimitiveComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AWeaponAlarm::AWeaponAlarm()
{
	IsCombineWeapon = false;
	WeaponType = EnumWeaponType::Alarm;
	AttackType = EnumAttackType::SpawnProjectile;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("/Game/ArtAssets/Models/Alarm/Alarm01.Alarm01"));
	if (DefaultMesh.Succeeded())
	{
		WeaponMesh->SetStaticMesh(DefaultMesh.Object);
	}

	//AttackDetectComponent = WeaponMesh;  // No AttackDetectComponent is needed for Alarm

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

	IsHidden = false;
}

void AWeaponAlarm::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CD_MaxEnergy <= CD_LeftEnergy && IsHidden)
	{
		if (!HasBeenCombined)
		{
			SetActorHiddenInGame(false);
			IsHidden = false;
		}			
	}
}



void AWeaponAlarm::OnRep_bAttackOn()
{
	Super::OnRep_bAttackOn();

	if (bAttackOn)
	{
		SetActorHiddenInGame(true);
		IsHidden = true;
	}		
}


void AWeaponAlarm::SpawnProjectile(float AttackTargetDistance)
{
	auto pCharacter = GetOwner();
	if (pCharacter && SpecificProjectileClass)
	{
		FVector spawnLocation = SpawnProjectilePointMesh->GetComponentLocation();
		FRotator spawnRotation = (pCharacter->GetActorRotation().Vector() + 0.3 * pCharacter->GetActorUpVector()).Rotation();  // character up

		FActorSpawnParameters spawnParameters;
		spawnParameters.Instigator = GetInstigator();
		spawnParameters.Owner = this;

		auto pProjectile = GetWorld()->SpawnActor<ABaseProjectile>(SpecificProjectileClass, spawnLocation, spawnRotation, spawnParameters);
		if (pProjectile)
		{
			float SpeedRatio = AttackTargetDistance / 400.0f;
			SpeedRatio = FMath::Clamp(SpeedRatio, 0.3f, 2.2f);
			pProjectile->NetMulticast_ChangeSpeed(SpeedRatio);
		}			
	}
}
