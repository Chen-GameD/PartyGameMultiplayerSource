// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon/ElementWeapon/WeaponLighter.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/PrimitiveComponent.h"
#include "Net/UnrealNetwork.h"


AWeaponLighter::AWeaponLighter()
{
	IsCombineWeapon = false;
	WeaponType = EnumWeaponType::Lighter;
	WeaponName = WeaponEnumToString_Map[WeaponType];

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("/Game/ArtAssets/Models/Lighter/Lighter.Lighter"));
	if (DefaultMesh.Succeeded())
	{
		WeaponMesh->SetStaticMesh(DefaultMesh.Object);
	}

	AttackDetectComponent = WeaponMesh;

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> DefaultAttackOnEffect(TEXT("/Game/ArtAssets/Niagara/NS_Fire.NS_Fire"));
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


void AWeaponLighter::CheckInitilization()
{
	Super::CheckInitilization();
	// do something specific to this weapon
	check(AttackHitEffect);
}
