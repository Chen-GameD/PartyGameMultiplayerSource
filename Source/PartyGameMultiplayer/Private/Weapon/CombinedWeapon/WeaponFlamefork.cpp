// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon/CombinedWeapon/WeaponFlamefork.h"

#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/PrimitiveComponent.h"
#include "Net/UnrealNetwork.h"

#include "Weapon/BaseProjectile.h"

AWeaponFlamefork::AWeaponFlamefork()
{
	IsCombineWeapon = true;
	WeaponType = EnumWeaponType::Flamefork;
	AttackType = EnumAttackType::SpawnProjectile;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("/Game/ArtAssets/Models/Flamefork/FlameFork.FlameFork"));
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
}


void AWeaponFlamefork::AttackStart(float AttackTargetDistance)
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

	SetActorEnableCollision(bAttackOn);
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, AttackTargetDistance] {SpawnProjectile(AttackTargetDistance);}, 0.2f, false);
}


void AWeaponFlamefork::SpawnProjectile(float AttackTargetDistance)
{
	auto pCharacter = GetOwner();
	if (pCharacter && SpecificProjectileClass)
	{
		FVector spawnLocation = SpawnProjectilePointMesh->GetComponentLocation();
		FRotator spawnRotation = pCharacter->GetActorRotation();

		FActorSpawnParameters spawnParameters;
		spawnParameters.Instigator = GetInstigator();
		spawnParameters.Owner = this;

		ABaseProjectile* spawnedProjectile = GetWorld()->SpawnActor<ABaseProjectile>(SpecificProjectileClass, spawnLocation, spawnRotation, spawnParameters);
	}
}

//void AWeaponFlamefork::OnRep_bAttackOn()
//{
//	Super::OnRep_bAttackOn();
//
//	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), AttackHitEffect_NSSystem, GetActorLocation());
//}