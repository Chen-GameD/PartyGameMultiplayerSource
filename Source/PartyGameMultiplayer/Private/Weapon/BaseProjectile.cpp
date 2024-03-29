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
#include "WaterBodyActor.h"
#include "Net/UnrealNetwork.h"


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

	AttackDetectComponent = StaticMesh;

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovementComponent->SetUpdatedComponent(StaticMesh);

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
		//if (GetActorLocation().Z < 0)
		//{
		//	Destroy();
		//	return;
		//}

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


void ABaseProjectile::NetMulticast_ChangeSpeed_Implementation(float SpeedRatio)
{
	ProjectileMovementComponent->MaxSpeed *= SpeedRatio;
	ProjectileMovementComponent->Velocity *= SpeedRatio;
}


void ABaseProjectile::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseProjectile, HasExploded);
	DOREPLIFETIME(ABaseProjectile, TimePassed_SinceExplosion);	
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
		IsBigWeapon = pWeapon->IsBigWeapon;
		Controller = pWeapon->GetHoldingController();
		if (!Controller)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Unexpected situation in ABaseProjectile::BeginPlay"));
			Destroy();
			return;
		}
		AttackDetectComponent->OnComponentBeginOverlap.AddDynamic(this, &ABaseProjectile::OnProjectileOverlapBegin);
	}	
	// Client
	else
	{
		SetActorEnableCollision(false);
	}

	/* Assign member variables by map */
	FString ParName = "";
	FString WeaponName = AWeaponDataHelper::WeaponEnumToString_Map[WeaponType];
	if (IsBigWeapon)
		WeaponName = "Big" + WeaponName;
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

	// Client(Listen Server)
	if (GetLocalRole() != ROLE_Authority || GetNetMode() == NM_ListenServer)
	{
		if (AttackOnEffect_NSComponent)
			AttackOnEffect_NSComponent->Activate();
	}

	//// Show projectile silouette on teammates' end
	//int TeammateCheckResult = ADamageManager::IsTeammate(GetInstigator(), GetWorld()->GetFirstPlayerController());
	//if (TeammateCheckResult == 1)
	//{
	//	// Exclude self
	//	if (auto pMCharacter = Cast<AMCharacter>(GetInstigator()))
	//	{
	//		if (pMCharacter->GetController() != GetWorld()->GetFirstPlayerController())
	//		{
	//			StaticMesh->SetRenderCustomDepth(true);
	//			StaticMesh->SetCustomDepthStencilValue(252);
	//		}
	//	}
	//}
}


void ABaseProjectile::OnRep_HasExploded()
{
	if (HasExploded)
	{
		if (AttackOnEffect_NSComponent)
		{
			AttackOnEffect_NSComponent->Deactivate();
			AttackOnEffect_NSComponent->SetVisibility(false);
		}			

		StaticMesh->SetVisibility(false);
		ProjectileMovementComponent->StopMovementImmediately();		
		if (AttackHitEffect_NSSystem)
			AttackHitEffect_NSComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), AttackHitEffect_NSSystem, GetActorLocation());
	
		CallExplodeSfx();
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
		OnRep_HasExploded();
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, OtherActor->GetName());

	// Direct Hit Damage
	ADamageManager::TryApplyDamageToAnActor(this, Controller, UDamageType::StaticClass(), OtherActor, 0);
	// Apply knockback buff
	ADamageManager::ApplyOneTimeBuff(WeaponType, EnumAttackBuff::Knockback, Controller, OtherActor, 0);

	// Range Damage		
	if (0 < TotalDamage)
	{
		DrawDebugSphere(GetWorld(), Origin, DamageRadius, 12, FColor::Red, false, 5.0f);
		if (!bApplyConstantDamage)
			ADamageManager::TryApplyRadialDamage(this, Controller, Origin, 0, DamageRadius, TotalDamage);
	}	
}
