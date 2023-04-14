// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/CombinedWeapon/ProjectileCannon.h"
#include "Weapon/DamageManager.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AProjectileCannon::AProjectileCannon()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("/Game/ArtAssets/Models/Cannon/CannonBomb.CannonBomb"));
	if (DefaultMesh.Succeeded())
	{
		StaticMesh->SetStaticMesh(DefaultMesh.Object);
		StaticMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
		StaticMesh->SetRelativeScale3D(FVector(8.0f, 8.0f, 8.0f));
	}
	StaticMesh->SetVisibility(false);

	CannonMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CannonMesh"));
	CannonMesh->SetupAttachment(StaticMesh);

	CannonMesh->SetCollisionProfileName(TEXT("NoCollision"));
	if (DefaultMesh.Succeeded())
	{
		CannonMesh->SetStaticMesh(DefaultMesh.Object);
		CannonMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
		CannonMesh->SetRelativeScale3D(FVector(8.0f, 8.0f, 8.0f));
	}

	ProjectileMovementComponent->InitialSpeed = 900.0f;
	ProjectileMovementComponent->MaxSpeed = 900.0f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->ProjectileGravityScale = 1.5f;

	HasAppliedRadialDamage = false;
}


void AProjectileCannon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FRotator NewRotation = CannonMesh->GetRelativeRotation();
	float DeltaRotation = DeltaTime * 300.0f;
	NewRotation.Yaw += DeltaRotation;
	CannonMesh->SetRelativeRotation(NewRotation);

	// Server
	if (GetLocalRole() == ROLE_Authority)
	{
		if (0.2f <= TimePassed_SinceExplosion && !HasAppliedRadialDamage)
		{
			ADamageManager::TryApplyRadialDamage(this, Controller, Origin, 0, DamageRadius, TotalDamage);
			HasAppliedRadialDamage = true;
		}
	}
}

void AProjectileCannon::BeginPlay()
{
	Super::BeginPlay();

	CannonMesh->SetRelativeRotation(FVector(FMath::RandRange(0.f, 3.14f), FMath::RandRange(0.f, 3.14f), FMath::RandRange(0.f, 3.14f)).Rotation());
	
	// Show projectile silouette on teammates' end
	int TeammateCheckResult = ADamageManager::IsTeammate(GetInstigator(), GetWorld()->GetFirstPlayerController());
	if (TeammateCheckResult == 1)
	{
		// Exclude self
		if (auto pMCharacter = Cast<AMCharacter>(GetInstigator()))
		{
			if (pMCharacter->GetController() != GetWorld()->GetFirstPlayerController())
			{
				CannonMesh->SetRenderCustomDepth(true);
				CannonMesh->SetCustomDepthStencilValue(252);
			}
		}
	}
}

void AProjectileCannon::OnRep_HasExploded()
{
	if (HasExploded)
	{
		if (AttackOnEffect_NSComponent)
		{
			AttackOnEffect_NSComponent->Deactivate();
			AttackOnEffect_NSComponent->SetVisibility(false);
		}

		StaticMesh->SetVisibility(false);
		CannonMesh->SetVisibility(false);
		ProjectileMovementComponent->StopMovementImmediately();
		if (AttackHitEffect_NSSystem)
			AttackHitEffect_NSComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), AttackHitEffect_NSSystem, GetActorLocation());
	
		CallExplodeSfx();
	}
}


void AProjectileCannon::OnProjectileOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
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
	HasAppliedRadialDamage = false;

	// Direct Hit Damage
	ADamageManager::TryApplyDamageToAnActor(this, Controller, UDamageType::StaticClass(), OtherActor, 0);
	// Apply knockback buff
	ADamageManager::ApplyOneTimeBuff(WeaponType, EnumAttackBuff::Knockback, Controller, Cast<AMCharacter>(OtherActor), 0);

	// Cannon's Range Damage will be delayed
	//ADamageManager::TryApplyRadialDamage(this, Controller, Origin, 0, DamageRadius, TotalDamage);
	DrawDebugSphere(GetWorld(), Origin, DamageRadius, 12, FColor::Red, false, 5.0f);
}