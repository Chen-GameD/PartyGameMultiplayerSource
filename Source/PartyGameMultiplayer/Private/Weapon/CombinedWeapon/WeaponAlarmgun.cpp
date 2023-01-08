// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/CombinedWeapon/WeaponAlarmgun.h"
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
AWeaponAlarmgun::AWeaponAlarmgun()
{
	IsCombined = true;
	WeaponType = EnumWeaponType::Bomb;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("/Game/ArtAssets/Models/AlarmGun/AlarmGun.AlarmGun"));
	//Set the Static Mesh and its position/scale if we successfully found a mesh asset to use.
	if (DefaultMesh.Succeeded())
	{
		WeaponMesh->SetStaticMesh(DefaultMesh.Object);
	}

	AttackDetectComponent = WeaponMesh;

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
	Damage = 50.0f;

	// WeaponName
	WeaponName = "Alarmgun";

}


// should only be called on server
void AWeaponAlarmgun::AttackStart()
{
	Super::AttackStart();

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Test shooting a bullet"));
	FVector spawnLocation = GetActorLocation() + (GetActorUpVector() * 0.0f);
	FRotator spawnRotation = GetActorRotation();
	auto spawnRotationEuler = spawnRotation.Euler();
	spawnRotationEuler.X = -spawnRotationEuler.X;
	spawnRotation = spawnRotationEuler.Rotation();

	FActorSpawnParameters spawnParameters;
	// Instigator: The APawn that is responsible for damage done by the spawned Actor. (Can be left as NULL).
	spawnParameters.Instigator = GetInstigator();
	spawnParameters.Owner = this;

	auto spawnedProjectile = GetWorld()->SpawnActor<ABaseProjectile>(spawnLocation, spawnRotation, spawnParameters);

}