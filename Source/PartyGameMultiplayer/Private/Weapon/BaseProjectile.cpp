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
		StaticMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	}	

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovementComponent->SetUpdatedComponent(StaticMesh);

	/*ProjectileMovementComponent->InitialSpeed = 1500.0f;
	ProjectileMovementComponent->MaxSpeed = 1500.0f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->ProjectileGravityScale = 0.0f;*/

	AttackOnEffect_NSComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("AttackOnEffect_NSComponent"));
	AttackOnEffect_NSComponent->SetupAttachment(StaticMesh);

	LiveTime = 0.0f;
	MaxLiveTime = 5.0f;

	TotalDamageTime = 0.0f;
	TotalDamage = 0.0f;
	Origin = FVector::ZeroVector;
	DamageRadius = 0.0f;
	bApplyConstantDamage = false;
	HasExploded = false;
	TimePassed_SinceExplosion = 0.0f;
	//TimePassed_SinceLastTryApplyRadialDamage = 0.0f;
}

void ABaseProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);	
	CurDeltaTime = DeltaTime;
	
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

		// Explosion has started
		if (HasExploded)
		{
			//  Is constant damage type
			if (bApplyConstantDamage)
			{
				// Explosion is finished
				if (TotalDamageTime < TimePassed_SinceExplosion)
				{
					Destroy();
					return;
				}

				// Explosion is not finished
				ADamageManager::TryApplyRadialDamage(this, Controller, Origin, 0, DamageRadius, DeltaTime* TotalDamage / TotalDamageTime);
			}	
			TimePassed_SinceExplosion += DeltaTime;
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
		if (!pWeapon || !pWeapon->IsPickedUp)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Unexpected situation in ABaseProjectile::BeginPlay"));
			Destroy();
			return;
		}
		WeaponType = pWeapon->WeaponType;
		Controller = pWeapon->GetHoldingController();
		if (!Controller)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Unexpected situation in ABaseProjectile::BeginPlay"));
			Destroy();
			return;
		}
	}	

	/* Assign member variables by map */
	FString ParName = "";
	FString WeaponName = AWeaponDataHelper::WeaponEnumToString_Map[WeaponType];
	// total time	
	ParName = WeaponName + "_TotalTime";
	if (AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map.Contains(ParName))
		TotalDamageTime = AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map[ParName];
	// total damage
	ParName = WeaponName + "_TotalDamage";
	if (AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map.Contains(ParName))
		TotalDamage = AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map[ParName];
	// damage radius
	ParName = WeaponName + "_DamageRadius";
	if (AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map.Contains(ParName))
		DamageRadius = AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map[ParName];
	// Is constant damage
	if (0.0f < TotalDamageTime)
		bApplyConstantDamage = true;

	// Server
	if (GetLocalRole() == ROLE_Authority)
	{
		StaticMesh->OnComponentBeginOverlap.AddDynamic(this, &ABaseProjectile::OnProjectileOverlapBegin);
	}

	// Client(Listen Server)
	if (GetLocalRole() != ROLE_Authority || GetNetMode() == NM_ListenServer)
	{
		if (AttackOnEffect_NSComponent)
			AttackOnEffect_NSComponent->Activate();
	}
}


void ABaseProjectile::OnRep_HasExploded()
{
	if (HasExploded)
	{
		if (AttackOnEffect_NSComponent)
		{
			AttackOnEffect_NSComponent->Deactivate();  // Not working somehow
			AttackOnEffect_NSComponent->SetVisibility(false);
		}
			

		StaticMesh->SetVisibility(false);
		ProjectileMovementComponent->StopMovementImmediately();		
		if (AttackHitEffect_NSSystem)
			AttackHitEffect_NSComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), AttackHitEffect_NSSystem, GetActorLocation());
	}
}


void ABaseProjectile::Destroyed()
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Enter Destroyed")));

	if (AttackHitEffect_NSComponent)
	{
		AttackHitEffect_NSComponent->Deactivate();
		AttackHitEffect_NSComponent->SetVisibility(false);
		AttackHitEffect_NSComponent = nullptr;
	}
}


void ABaseProjectile::OnProjectileOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
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

	// Range Damage		
	if (0 < TotalDamage)
	{
		DrawDebugSphere(GetWorld(), Origin, DamageRadius, 12, FColor::Red, false, 5.0f);
		if (!bApplyConstantDamage)
			ADamageManager::TryApplyRadialDamage(this, Controller, Origin, 0, DamageRadius, TotalDamage);
	}	
}
