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
	/*
		bCanEverTick is set to true in BaseWeapon Class.
		You can uncomment the following line and turn this off to improve performance if you don't need it.
	*/
	//PrimaryActorTick.bCanEverTick = false;

	IsCombined = false;
	WeaponType = EnumWeaponType::Fork;
	WeaponName = WeaponEnumToString_Map[WeaponType];

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("/Game/ArtAssets/Models/Fork/Fork.Fork"));
	//Set the Static Mesh and its position/scale if we successfully found a mesh asset to use.
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

	DamageType = UDamageType::StaticClass();

}



void AWeaponFork::CheckInitilization()
{
	Super::CheckInitilization();
	// do something specific to this weapon
	check(AttackHitEffect);
}



//void AWeaponFork::GenerateAttackHitEffect()
//{
//	FVector spawnLocation = GetActorLocation();
//	UGameplayStatics::SpawnEmitterAtLocation(this, AttackHitEffect, spawnLocation, FRotator::ZeroRotator, true, EPSCPoolMethod::AutoRelease);
//}
//
//
//void AWeaponFork::GenerateDamage(class AActor* DamagedActor)
//{
//	// Note: The 3rd parameter is EventInstigator, be careful if the weapon has an instigator or not.
//	// if it doesn't and the 3rd parameter is set to GetInstigator()->Controller, the game would crash when overlap happens
//	// (The projectile in the demo has an instigator, because the instigator parameter is assigned when the the character spawns it in HandleFire function)
//	check(GetInstigator()->Controller);
//	UGameplayStatics::ApplyDamage(DamagedActor, Damage, GetInstigator()->Controller, this, DamageType);
//}

