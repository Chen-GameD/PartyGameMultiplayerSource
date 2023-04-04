// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon/ElementWeapon/WeaponLighter.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/PrimitiveComponent.h"
#include "Net/UnrealNetwork.h"

#include "Weapon/DamageManager.h"
#include "Weapon/DamageType/MeleeDamageType.h"
#include "LevelInteraction/MinigameObject/MinigameObj_Enemy.h"

AWeaponLighter::AWeaponLighter()
{
	IsCombineWeapon = false;
	WeaponType = EnumWeaponType::Lighter;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("/Game/ArtAssets/Models/Lighter/Lighter.Lighter"));
	if (DefaultMesh.Succeeded())
	{
		WeaponMesh->SetStaticMesh(DefaultMesh.Object);
	}

	AttackDetectComponent = WeaponMesh;

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> DefaultAttackOnEffect(TEXT("/Game/ArtAssets/Niagara/NS_Fire.NS_Fire"));
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


void AWeaponLighter::OnAttackOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
	class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IsPickedUp && HoldingController && GetOwner())
	{
		if ((Cast<ACharacter>(OtherActor) && OtherActor != GetOwner()) ||
			Cast<AMinigameObj_Enemy>(OtherActor))
		{
			if (!AttackObjectMap.Contains(OtherActor))
				AttackObjectMap.Add(OtherActor);
			AttackObjectMap[OtherActor] = 0.0f;
			bAttackOverlap = true;
			// Listen server
			if (GetNetMode() == NM_ListenServer)
			{
				OnRep_bAttackOverlap();
			}

			if ((!IsBigWeapon && ApplyDamageCounter == 0) || IsBigWeapon)
			{
				ADamageManager::TryApplyDamageToAnActor(this, HoldingController, UMeleeDamageType::StaticClass(), OtherActor, 0);
				ADamageManager::ApplyOneTimeBuff(WeaponType, EnumAttackBuff::Knockback, HoldingController, Cast<AMCharacter>(OtherActor), 0);
				// Add burning buff points
				FString ParName = "Lighter_Burning_PointsToAdd_PerHit";
				if (AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map.Contains(ParName))
				{
					float buffPointsToAdd = AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map[ParName];
					ADamageManager::AddBuffPoints(WeaponType, EnumAttackBuff::Burning, HoldingController, Cast<AMCharacter>(OtherActor), buffPointsToAdd);
					GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("buffPointsToAdd: %f"), buffPointsToAdd));
				}
				ApplyDamageCounter++;
			}
		}
	}
}