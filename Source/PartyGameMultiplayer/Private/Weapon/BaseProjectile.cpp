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

	bAttackHit = false;
	
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

	TotalTime_ApplyDamage = 0.0f;
}


void ABaseProjectile::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseProjectile, bAttackHit);
}


void ABaseProjectile::BeginPlay()
{
	Super::BeginPlay();

	// Assign TotalTime_ApplyDamage by map
	auto pWeapon = Cast<ABaseWeapon>(GetOwner());
	check(pWeapon);
	FString ParName = pWeapon->GetWeaponName() + "_TotalTime";
	if (AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map.Contains(ParName))
		TotalTime_ApplyDamage = AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map[ParName];

	// Server duty
	if (GetLocalRole() == ROLE_Authority)
	{
		StaticMesh->OnComponentBeginOverlap.AddDynamic(this, &ABaseProjectile::OnProjectileOverlapBegin);
	}
	FTimerHandle TimerHandle;
	// In most cases, we expect the projectile to be destoryed way much shorter than MaxLiveTime, 
	// because it will overlap with something soon or get below the KillZ value quickly.
	float MaxLiveTime = 10.0f;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]() 
		{ 
			Destroy(); 
		}, MaxLiveTime, false);
}


void ABaseProjectile::OnRep_bAttackHit()
{
	//if (bAttackHit)
	//{
	//	AttackHitEffect_NSComponent->Activate();
	//}
	//else
	//{
	//	AttackHitEffect_NSComponent->Deactivate();
	//}
}


void ABaseProjectile::Destroyed()
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Enter Destroyed")));

	if (!AttackHitEffect_NSSystem)
		return;
	AttackHitEffect_NSComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), AttackHitEffect_NSSystem, GetActorLocation());
	if (0.0f < TotalTime_ApplyDamage)
	{
		if (AttackHitEffect_NSComponent)
		{
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
				{
					AttackHitEffect_NSComponent->Deactivate();
					AttackHitEffect_NSComponent = nullptr;
				}, TotalTime_ApplyDamage, false);
		}
	}	
}


void ABaseProjectile::OnProjectileOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
	class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bAttackHit)
		return;
	auto pWeapon = Cast<ABaseWeapon>(GetOwner());
	if (!pWeapon)
		return;
	auto pHoldingPlayer = pWeapon->GetInstigator();
	if (!pHoldingPlayer)
		return;

	if (GetOwner() && pWeapon->IsPickedUp)
	{
		if (Cast<ABaseWeapon>(OtherActor))
			return;
		if (Cast<ACharacter>(OtherActor) && OtherActor == pHoldingPlayer)
			return;

		bAttackHit = true;
		pWeapon->GenerateDamageLike(OtherActor);
		ADamageManager::TryApplyRadialDamage(pWeapon, this->GetActorLocation());
	
		// Apply damage multiple times
		//GetWorld()->GetTimerManager().SetTimer(TimerHandle_Loop, [pWeapon, this->GetActorLocation()]()
		//	{
		//		ADamageManager::TryApplyRadialDamage(pWeapon, Epicenter);
		//	}, 1.0f, true);  // the bool paramter determines if the function will be called in loop or not
		//FTimerHandle TimerHandle_SelfDestroy;
		//GetWorld()->GetTimerManager().SetTimer(TimerHandle_SelfDestroy, [this]()
		//	{
		//		GetWorldTimerManager().ClearTimer(TimerHandle_Loop);
		//	}, 3.01f, false);

		Destroy();		
	}
}