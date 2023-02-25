// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/CombinedWeapon/ProjectileBomb.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

#include "Weapon/DamageManager.h"


AProjectileBomb::AProjectileBomb()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("/Game/ArtAssets/Models/Bomb/AlarmBomb.AlarmBomb"));
	if (DefaultMesh.Succeeded())
	{
		StaticMesh->SetStaticMesh(DefaultMesh.Object);
		StaticMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
		StaticMesh->SetRelativeScale3D(FVector(0.75f, 0.75f, 0.75f));
	}

	ProjectileMovementComponent->InitialSpeed = 800.0f;
	ProjectileMovementComponent->MaxSpeed = 800.0f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->ProjectileGravityScale = 0.65f;

	//static ConstructorHelpers::FObjectFinder<UNiagaraSystem> DefaultAttackHitEffect(TEXT("/Game/ArtAssets/Niagara/NS_Soundwave.NS_Soundwave"));
	//if (DefaultAttackHitEffect.Succeeded())
	//{
	//	UNiagaraSystem* AttackHitEffect_NiagaraSystem = DefaultAttackHitEffect.Object;
	//	AttackHitEffect_NSComponent->SetAsset(AttackHitEffect_NiagaraSystem);
	//}

	HasAppliedNeedleRainDamage = false;
}


void AProjectileBomb::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Server
	if (GetLocalRole() == ROLE_Authority)
	{
		if (2.0f <= TimePassed_SinceExplosion && !HasAppliedNeedleRainDamage)
		{
			ADamageManager::TryApplyRadialDamage(this, Controller, Origin, 0, DamageRadius, TotalDamage);
			HasAppliedNeedleRainDamage = true;
		}
	}
}



void AProjectileBomb::OnProjectileOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
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
	HasAppliedNeedleRainDamage = false;

	// Direct Hit Damage
	ADamageManager::TryApplyDamageToAnActor(this, Controller, UDamageType::StaticClass(), OtherActor, 0);

	// Bomb's Range Damage is different
	//ADamageManager::TryApplyRadialDamage(this, Controller, Origin, 0, DamageRadius, TotalDamage);
	DrawDebugSphere(GetWorld(), Origin, DamageRadius, 12, FColor::Red, false, 5.0f);
}
