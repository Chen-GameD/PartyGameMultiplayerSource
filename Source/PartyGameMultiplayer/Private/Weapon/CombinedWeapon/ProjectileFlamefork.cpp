// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/CombinedWeapon/ProjectileFlamefork.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

#include "Weapon/DamageManager.h"
#include "Weapon/WeaponDataHelper.h"
#include "LevelInteraction/MinigameMainObjective.h"
#include "Character/MCharacter.h"

AProjectileFlamefork::AProjectileFlamefork()
{
	ProjectileMovementComponent->InitialSpeed = 1150.0f;
	ProjectileMovementComponent->MaxSpeed = 1150.0f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->ProjectileGravityScale = 0.25f;

	//static ConstructorHelpers::FObjectFinder<UNiagaraSystem> DefaultAttackHitEffect(TEXT("/Game/ArtAssets/Niagara/NS_Soundwave.NS_Soundwave"));
	//if (DefaultAttackHitEffect.Succeeded())
	//{
	//	UNiagaraSystem* AttackHitEffect_NiagaraSystem = DefaultAttackHitEffect.Object;
	//	AttackHitEffect_NSComponent->SetAsset(AttackHitEffect_NiagaraSystem);
	//}
}


void AProjectileFlamefork::BeginPlay()
{
	Super::BeginPlay();

	MaxLiveTime = 0.3f;
	// Server
	if (GetLocalRole() == ROLE_Authority)
	{
		// We need pawn's transform to spawn the ns on both the clients and server.
		// But only the server has controller(and pawn), so we have to make it in a multi-cast way
		if (Controller && Controller->GetPawn())
			SpawnWaveNS(GetActorLocation(), Controller->GetPawn()->GetActorRotation());
	}
}


void AProjectileFlamefork::Destroyed()
{
	if (Wave_NSComponent)
	{
		Wave_NSComponent->Deactivate();
		Wave_NSComponent->SetVisibility(false);
		Wave_NSComponent = nullptr;
	}
}

void AProjectileFlamefork::SpawnWaveNS_Implementation(FVector SpawnLocation, FRotator SpawnRotation)
{
	FRotator AdjustedSpawnRotation = SpawnRotation;
	AdjustedSpawnRotation.Yaw += 45.0f;  // fix the z-axis bias
	FVector SpawnScale = FVector(0.5, 0.5, 0.1);
	if (Wave_NSSystem)
		Wave_NSComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), Wave_NSSystem, SpawnLocation, AdjustedSpawnRotation, SpawnScale);
}

void AProjectileFlamefork::OnProjectileOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
	class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasExploded)
		return;

	if (Cast<ABaseWeapon>(OtherActor) || Cast<ABaseProjectile>(OtherActor))
		return;
	if (Cast<APawn>(OtherActor) && Controller && OtherActor == Controller->GetPawn())
		return;

	Origin = this->GetActorLocation();
	HasExploded = true;
	if (GetNetMode() == NM_ListenServer)
	{
		OnRep_HasExploded();
	}

	// Direct Hit Damage
	ADamageManager::TryApplyDamageToAnActor(this, Controller, UDamageType::StaticClass(), OtherActor, 0);
	//// Apply knockback buff
	//ADamageManager::ApplyOneTimeBuff(WeaponType, EnumAttackBuff::Knockback, Controller, Cast<AMCharacter>(OtherActor), 0);
	// Add burning buff points
	FString ParName = "Flamefork_Burning_PointsToAdd_PerHit";
	if (AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map.Contains(ParName))
	{
		float buffPointsToAdd = AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map[ParName];
		ADamageManager::AddBuffPoints(WeaponType, EnumAttackBuff::Burning, Controller, Cast<AMCharacter>(OtherActor), buffPointsToAdd);
	}

	// Range Damage		
	if (0 < TotalDamage)
	{
		DrawDebugSphere(GetWorld(), Origin, DamageRadius, 12, FColor::Red, false, 5.0f);
		if (!bApplyConstantDamage)
			ADamageManager::TryApplyRadialDamage(this, Controller, Origin, 0, DamageRadius, TotalDamage);
	}
}