// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon/ElementWeapon/WeaponFork.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/PrimitiveComponent.h"
#include "Net/UnrealNetwork.h"


AWeaponFork::AWeaponFork()
{
	IsCombined = false;
	WeaponType = EnumWeaponType::Fork;
	WeaponName = WeaponEnumToString_Map[WeaponType];

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("/Game/ArtAssets/Models/Fork/Fork.Fork"));
	if (DefaultMesh.Succeeded())
	{
		WeaponMesh->SetStaticMesh(DefaultMesh.Object);
	}

	AttackDetectComponent = WeaponMesh;

	static ConstructorHelpers::FObjectFinder<UParticleSystem> DefaultAttackHitEffect(TEXT("/Game/StarterContent/Particles/P_Explosion.P_Explosion"));
	if (DefaultAttackHitEffect.Succeeded())
	{
		AttackHitEffect = DefaultAttackHitEffect.Object;
	}
}



void AWeaponFork::CheckInitilization()
{
	Super::CheckInitilization();
	// do something specific to this weapon
	check(AttackHitEffect);
}

