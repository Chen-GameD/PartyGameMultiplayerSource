// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon/CombinedWeapon/WeaponFlamethrower.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/PrimitiveComponent.h"

AWeaponFlamethrower::AWeaponFlamethrower()
{
	/*
		bCanEverTick is set to true in BaseWeapon Class.
		You can uncomment the following line and turn this off to improve performance if you don't need it.
	*/
	//PrimaryActorTick.bCanEverTick = false;

	IsCombined = true;
	WeaponType = EnumWeaponType::Flamethrower;
	AttackType = EnumAttackType::Constant;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("/Game/ArtAssets/Models/Flamethrower/Flame.Flame"));
	//Set the Static Mesh and its position/scale if we successfully found a mesh asset to use.
	if (DefaultMesh.Succeeded())
	{
		WeaponMesh->SetStaticMesh(DefaultMesh.Object);
	}

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("WindCollisionArea"));
	BoxComponent->SetRelativeLocation(FVector(-280.0f, 0.0f, 20.5f));
	//BoxComponent->SetBoxExtent(FVector3d(80.0f, 20.0f, 20.0f));
	BoxComponent->SetBoxExtent(FVector3d(225.0f, 75.0f, 75.0f));
	BoxComponent->SetupAttachment(WeaponMesh);
	AttackDetectComponent = BoxComponent;

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> DefaultAttackOnEffect(TEXT("/Game/ArtAssets/Niagara/NS_FireFlame.NS_FireFlame"));
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
	Damage = 20.0f;
	AccumulatedTimeToGenerateDamage = 0.5f;

	// WeaponName
	WeaponName = "Flamethrower";
}

void AWeaponFlamethrower::CheckInitilization()
{
	Super::CheckInitilization();
	// do something specific to this weapon
	check(AttackHitEffect);
}


//void AWeaponFlamethrower::GenerateAttackHitEffect()
//{
//	FVector spawnLocation = GetActorLocation();
//	UGameplayStatics::SpawnEmitterAtLocation(this, AttackHitEffect, spawnLocation, FRotator::ZeroRotator, true, EPSCPoolMethod::AutoRelease);
//}
//
//
//void AWeaponFlamethrower::GenerateDamage(class AActor* DamagedActor)
//{
//	// Note: The 3rd parameter is EventInstigator, be careful if the weapon has an instigator or not.
//	// if it doesn't and the 3rd parameter is set to GetInstigator()->Controller, the game would crash when overlap happens
//	// (The projectile in the demo has an instigator, because the instigator parameter is assigned when the the character spawns it in HandleFire function)
//	check(GetInstigator()->Controller);
//	UGameplayStatics::ApplyDamage(DamagedActor, Damage, GetInstigator()->Controller, this, DamageType);
//}
