// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/BaseProjectile.h"
#include "Weapon/BaseWeapon.h"
#include "Weapon/DamageManager.h"
#include "Weapon/WeaponDataHelper.h"
#include "LevelInteraction/MinigameMainObjective.h"
#include "Character/MCharacter.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/Character.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"


ABaseProjectile::ABaseProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	StaticMesh->SetupAttachment(RootComponent);
	StaticMesh->SetCollisionProfileName(TEXT("Trigger"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("/Game/StarterContent/Shapes/Shape_Sphere.Shape_Sphere"));
	if (DefaultMesh.Succeeded())
	{
		StaticMesh->SetStaticMesh(DefaultMesh.Object);
		StaticMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
		StaticMesh->SetRelativeScale3D(FVector(0.75f, 0.75f, 0.75f));
	}	

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovementComponent->SetUpdatedComponent(StaticMesh);

	/*ProjectileMovementComponent->InitialSpeed = 1500.0f;
	ProjectileMovementComponent->MaxSpeed = 1500.0f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->ProjectileGravityScale = 0.0f;*/

	//AttackHitEffect_NSComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("AttackHitEffect_NSComponent"));
	//AttackHitEffect_NSComponent->SetupAttachment(StaticMesh);

	LiveTime = 0.0f;
	MaxLiveTime = 10.0f;

	TotalTime_ApplyDamage = 0.0f;
	TotalDamage_ForTotalTime = 0.0f;
	Origin = FVector::ZeroVector;
	DamageRadius = 0.0f;
	bApplyConstantDamage = false;
	BaseDamage = 0.0f;
	HasExploded = false;
	TimePassed_SinceExplosion = 0.0f;
	TimePassed_SinceLastTryApplyRadialDamage = 0.0f;
}

void ABaseProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);	
	
	// Server
	if (GetLocalRole() == ROLE_Authority)
	{
		// In most cases, we expect the projectile to be destoryed way much shorter than MaxLiveTime, 
		// because it will overlap with something soon or get below the KillZ value quickly.
		LiveTime += DeltaTime;
		if (MaxLiveTime < LiveTime)
		{
			Destroy();
			return;
		}

		// Explosion has started && is constant
		if (HasExploded && bApplyConstantDamage)
		{
			// Explosion is finished
			if (TotalTime_ApplyDamage < TimePassed_SinceExplosion)
			{
				Destroy();
				return;
			}

			// Explosion is not finished
			// TryApplyRadialDamage when reaches the timeinterval
			if (ADamageManager::interval_ApplyDamage <= TimePassed_SinceLastTryApplyRadialDamage)
			{
				ADamageManager::TryApplyRadialDamage(this, Controller, Origin, DamageRadius, BaseDamage);
				TimePassed_SinceLastTryApplyRadialDamage = 0.0f;
			}
			TimePassed_SinceExplosion += DeltaTime;
			TimePassed_SinceLastTryApplyRadialDamage += DeltaTime;
		}
	}
}



void ABaseProjectile::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseProjectile, HasExploded);	
}


void ABaseProjectile::BeginPlay()
{
	Super::BeginPlay();

	// Server
	if (GetLocalRole() == ROLE_Authority)
	{
		auto pWeapon = Cast<ABaseWeapon>(GetOwner());
		check(pWeapon && pWeapon->IsPickedUp);
		WeaponType = pWeapon->WeaponType;
		Controller = pWeapon->GetHoldingController();
		check(Controller);
	}	

	/* Assign member variables by map */
	FString ParName = "";
	FString WeaponName = AWeaponDataHelper::WeaponEnumToString_Map[WeaponType];
	// total time	
	ParName = WeaponName + "_TotalTime";
	if (AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map.Contains(ParName))
		TotalTime_ApplyDamage = AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map[ParName];
	// total damage
	ParName = WeaponName + "_TotalDamage";
	if (AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map.Contains(ParName))
		TotalDamage_ForTotalTime = AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map[ParName];
	// damage radius
	ParName = WeaponName + "_DamageRadius";
	if (AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map.Contains(ParName))
		DamageRadius = AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map[ParName];
	// Is constant damage
	if (0.0f < TotalTime_ApplyDamage)
		bApplyConstantDamage = true;
	// BaseDamage
	float numIntervals = TotalTime_ApplyDamage / ADamageManager::interval_ApplyDamage;
	BaseDamage = bApplyConstantDamage ? (TotalDamage_ForTotalTime / numIntervals) : TotalDamage_ForTotalTime;

	// Server duty
	if (GetLocalRole() == ROLE_Authority)
	{
		StaticMesh->OnComponentBeginOverlap.AddDynamic(this, &ABaseProjectile::OnProjectileOverlapBegin);
	}
}


void ABaseProjectile::OnRep_HasExploded()
{
	if (HasExploded)
	{
		StaticMesh->SetVisibility(false);
		ProjectileMovementComponent->StopMovementImmediately();		
		if (AttackHitEffect_NSSystem)
			AttackHitEffect_NSComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), AttackHitEffect_NSSystem, GetActorLocation());	
	}
}


void ABaseProjectile::Destroyed()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("Destroyed")));
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Enter Destroyed")));

	if (AttackHitEffect_NSComponent)
	{
		AttackHitEffect_NSComponent->Deactivate();
		AttackHitEffect_NSComponent = nullptr;
	}
}


void ABaseProjectile::OnProjectileOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
	class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasExploded)
		return;

	if (Cast<ABaseWeapon>(OtherActor))
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
	ADamageManager::TryApplyDamageToAnActor(this, Controller, UDamageType::StaticClass(), OtherActor);

	// Range Damage		
	ADamageManager::TryApplyRadialDamage(this, Controller, Origin, DamageRadius, BaseDamage);
	DrawDebugSphere(GetWorld(), Origin, DamageRadius, 12, FColor::Red, false, 5.0f);
}
