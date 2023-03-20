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

	BombMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BombMesh"));
	BombMesh->SetupAttachment(StaticMesh);

	BombMesh->SetCollisionProfileName(TEXT("NoCollision"));
	if (DefaultMesh.Succeeded())
	{
		BombMesh->SetStaticMesh(DefaultMesh.Object);
		BombMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
		BombMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	}

	ProjectileMovementComponent->InitialSpeed = 1000.0f;
	ProjectileMovementComponent->MaxSpeed = 1000.0f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->ProjectileGravityScale = 0.3f;

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

	FRotator NewRotation = BombMesh->GetRelativeRotation();
	float DeltaRotation = DeltaTime * 400.0f;
	NewRotation.Yaw += DeltaRotation;
	BombMesh->SetRelativeRotation(NewRotation);

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


void AProjectileBomb::OnRep_HasExploded()
{
	if (HasExploded)
	{
		ProjectileMovementComponent->StopMovementImmediately();
		StaticMesh->SetSimulatePhysics(false);
		StaticMesh->SetEnableGravity(false);
		StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		

		if (AttackHitEffect_NSSystem)
			AttackHitEffect_NSComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), AttackHitEffect_NSSystem, GetActorLocation());
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
	// Apply knockback buff
	ADamageManager::ApplyOneTimeBuff(WeaponType, EnumAttackBuff::Knockback, Controller, Cast<AMCharacter>(OtherActor), 0);

	// Bomb's Range Damage will be delayed
	//ADamageManager::TryApplyRadialDamage(this, Controller, Origin, 0, DamageRadius, TotalDamage);
	DrawDebugSphere(GetWorld(), Origin, DamageRadius, 12, FColor::Red, false, 5.0f);
}
