// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon/CombinedWeapon/WeaponFlamethrower.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/PrimitiveComponent.h"

#include "Weapon/DamageManager.h"

AWeaponFlamethrower::AWeaponFlamethrower()
{
	IsCombineWeapon = true;
	WeaponType = EnumWeaponType::Flamethrower;
	AttackType = EnumAttackType::Constant;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("/Game/ArtAssets/Models/Flamethrower/Flame.Flame"));
	if (DefaultMesh.Succeeded())
	{
		WeaponMesh->SetStaticMesh(DefaultMesh.Object);
	}

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("WindCollisionArea"));
	BoxComponent->SetRelativeLocation(FVector(-280.0f, 0.0f, 20.5f));
	BoxComponent->SetBoxExtent(FVector3d(225.0f, 75.0f, 75.0f));
	BoxComponent->SetupAttachment(WeaponMesh);
	AttackDetectComponent = BoxComponent;

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> DefaultAttackOnEffect(TEXT("/Game/ArtAssets/Niagara/NS_FireFlame.NS_FireFlame"));
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


void AWeaponFlamethrower::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Server
	if (GetLocalRole() == ROLE_Authority)
	{
		if (IsPickedUp)
		{
			if (!HasBeenCombined)
			{
				if (AttackType == EnumAttackType::Constant && bAttackOn && CD_MinEnergyToAttak <= CD_LeftEnergy)
				{
					for (auto& Elem : AttackObjectMap)
					{
						// Apply knockback buff at a fixed frequency
						Elem.Value += DeltaTime;
						if (AWeaponDataHelper::interval_ConstantWeaponApplyKnockback <= Elem.Value)
						{
							ADamageManager::ApplyOneTimeBuff(WeaponType, EnumAttackBuff::Knockback, HoldingController, Cast<AMCharacter>(Elem.Key), DeltaTime);
							Elem.Value -= AWeaponDataHelper::interval_ConstantWeaponApplyKnockback;
						}
						// Add burning buff points
						FString ParName = "Flamethrower_Burning_PointsToAdd_PerSec";
						if (AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map.Contains(ParName))
						{
							float buffPointsToAdd = DeltaTime * AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map[ParName];
							ADamageManager::AddBuffPoints(WeaponType, EnumAttackBuff::Burning, HoldingController, Cast<AMCharacter>(Elem.Key), buffPointsToAdd);
						}
					}
				}
			}
		}
	}
}