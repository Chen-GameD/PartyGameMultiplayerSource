// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon/ElementWeapon/WeaponBlower.h"

#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/PrimitiveComponent.h"
#include "Net/UnrealNetwork.h"

#include "Weapon/DamageManager.h"


AWeaponBlower::AWeaponBlower()
{
	IsCombineWeapon = false;
	WeaponType = EnumWeaponType::Blower;
	AttackType = EnumAttackType::Constant;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("/Game/ArtAssets/Models/Blower/Blower.Blower"));
	if (DefaultMesh.Succeeded())
	{
		WeaponMesh->SetStaticMesh(DefaultMesh.Object);
	}

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("WindCollisionArea"));
	BoxComponent->SetRelativeLocation(FVector(-280.0f, 0.0f, 20.5f));
	BoxComponent->SetBoxExtent(FVector3d(225.0f, 75.0f, 75.0f));
	BoxComponent->SetupAttachment(WeaponMesh);

	AttackDetectComponent = BoxComponent;

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> DefaultAttackOnEffect(TEXT("/Game/ArtAssets/Niagara/NS_Wind.NS_Wind"));
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

void AWeaponBlower::Tick(float DeltaTime)
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
					// Cannot used CopiedAttackObjectMap this time because we need to change the value of AttackObjectMap
					FScopeLock Lock(&DataGuard);
					for (auto& Elem : AttackObjectMap)
					{
						// Apply knockback buff at a fixed frequency
						Elem.Value += DeltaTime;
						if (AWeaponDataHelper::interval_ConstantWeaponApplyKnockback <= Elem.Value)
						{
							GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Unexpected situation in blower"));
							ADamageManager::ApplyOneTimeBuff(WeaponType, EnumAttackBuff::Knockback, HoldingController, Cast<AMCharacter>(Elem.Key), DeltaTime);
							Elem.Value -= AWeaponDataHelper::interval_ConstantWeaponApplyKnockback;
						}
					}
				}
			}
		}
	}
}

void AWeaponBlower::AttackStart(float AttackTargetDistance)
{
	if (bAttackOn || !GetOwner())
		return;

	// If the weapon has cd
	if (0 < CD_MaxEnergy)
	{
		if (AttackType == EnumAttackType::Constant)
		{
			if (CD_LeftEnergy <= 0)
				return;
		}
		else
		{
			if (CD_MinEnergyToAttak <= CD_LeftEnergy)
				CD_LeftEnergy -= CD_MinEnergyToAttak;
			else
				return;
		}
	}

	bAttackOn = true;
	// Listen server
	if (GetNetMode() == NM_ListenServer)
		OnRep_bAttackOn();
	ApplyDamageCounter = 0;

	float ApplyDamageAndKnockbackDelay = 0.15f;
	FTimerHandle ApplyDamageAndKnockbackTimerHandle;
	GetWorldTimerManager().SetTimer(ApplyDamageAndKnockbackTimerHandle, [this]
		{
			if (bAttackOn)
				SetActorEnableCollision(true);
		}, ApplyDamageAndKnockbackDelay, false);
}