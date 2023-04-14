// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/CombinedWeapon/WeaponBomb.h"

#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/BoxComponent.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Net/UnrealNetwork.h"

#include "Weapon/DamageManager.h"
#include "Character/MCharacter.h"

#include "Weapon/CombinedWeapon/ProjectileBomb.h"


// Sets default values
AWeaponBomb::AWeaponBomb()
{
	IsCombineWeapon = true;
	WeaponType = EnumWeaponType::Bomb;
	AttackType = EnumAttackType::SpawnProjectile;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh_1(TEXT("/Game/ArtAssets/Models/Bomb/Bomb.Bomb"));
	if (DefaultMesh_1.Succeeded())
	{
		WeaponMesh->SetStaticMesh(DefaultMesh_1.Object);
	}
	WeaponMesh->SetRelativeScale3D(FVector(10.0f, 10.0f, 10.0f));

	WeaponMesh_WithoutBomb = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh_WithoutBomb"));
	WeaponMesh_WithoutBomb->SetupAttachment(DisplayCase);
	WeaponMesh_WithoutBomb->SetCollisionProfileName(TEXT("Trigger"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh_2(TEXT("/Game/ArtAssets/Models/Fork/Fork.Fork"));
	if (DefaultMesh_2.Succeeded())
	{
		WeaponMesh_WithoutBomb->SetStaticMesh(DefaultMesh_2.Object);
	}
	WeaponMesh_WithoutBomb->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	WeaponMesh_WithoutBomb->SetVisibility(false);

	AttackDetectComponent = WeaponMesh_WithoutBomb;  // Bomb is special, Projectile and WeaponMesh_WithoutBomb can both apply damage

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> DefaultAttackOnEffect(TEXT("/Game/ArtAssets/Niagara/NS_FlameForkNew.NS_FlameForkNew"));
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


void AWeaponBomb::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CD_MaxEnergy <= CD_LeftEnergy)
	{
		if(!WeaponMesh->IsVisible())
			WeaponMesh->SetVisibility(true);
	}
}


void AWeaponBomb::AttackStart(float AttackTargetDistance)
{
	if (bAttackOn || !GetOwner())
		return;

	bAttackOn = true;
	// Listen server
	if (GetNetMode() == NM_ListenServer)
	{
		OnRep_bAttackOn();
	}	
	ApplyDamageCounter = 0;

	SetActorEnableCollision(bAttackOn);

	// Whether spawn a projectile
	if (0.0f < CD_MaxEnergy && CD_MinEnergyToAttak <= CD_LeftEnergy)
	{
		CD_LeftEnergy -= CD_MinEnergyToAttak;
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, AttackTargetDistance] {SpawnProjectile(AttackTargetDistance); }, 0.1f, false);
	}
}


//void AWeaponBomb::AttackStop()
//{
//	if (!bAttackOn || !GetOwner())
//		return;
//
//	bAttackOn = false;
//	// Listen server
//	if (GetNetMode() == NM_ListenServer)
//	{
//		OnRep_bAttackOn();
//	}
//	ApplyDamageCounter = 0;
//	AttackObjectMap.Empty();
//
//	SetActorEnableCollision(bAttackOn);
//}


void AWeaponBomb::SpawnProjectile(float AttackTargetDistance)
{
	auto pCharacter = GetOwner();
	if (pCharacter && SpecificProjectileClass)
	{
		FVector spawnLocation = SpawnProjectilePointMesh->GetComponentLocation();
		FRotator spawnRotation = (pCharacter->GetActorRotation().Vector() - 0.0 * pCharacter->GetActorRightVector()).Rotation();

		FActorSpawnParameters spawnParameters;
		spawnParameters.Instigator = GetInstigator();
		spawnParameters.Owner = this;

		ABaseProjectile* pProjectile = GetWorld()->SpawnActor<ABaseProjectile>(SpecificProjectileClass, spawnLocation, spawnRotation, spawnParameters);
		if (pProjectile)
		{
			float SpeedRatio = AttackTargetDistance / 450.0f;
			SpeedRatio = FMath::Clamp(SpeedRatio, 0.3f, 2.5f);
			pProjectile->NetMulticast_ChangeSpeed(SpeedRatio);
		}
	}
}

void AWeaponBomb::OnRep_bAttackOn()
{
	Super::OnRep_bAttackOn();

	if (bAttackOn)
	{
		if(WeaponMesh->IsVisible())
			WeaponMesh->SetVisibility(false);
	}		
}


void AWeaponBomb::OnRep_IsPickedUp()
{
	Super::OnRep_IsPickedUp();

	if (IsPickedUp)
	{
		// Show weapon silouette on teammates' end
		int TeammateCheckResult = ADamageManager::IsTeammate(GetOwner(), GetWorld()->GetFirstPlayerController());
		if (TeammateCheckResult == 1)
		{
			// Exclude self
			if (auto pMCharacter = Cast<AMCharacter>(GetOwner()))
			{
				if (pMCharacter->GetController() != GetWorld()->GetFirstPlayerController())
				{
					WeaponMesh_WithoutBomb->SetRenderCustomDepth(true);
					WeaponMesh_WithoutBomb->SetCustomDepthStencilValue(252);
				}
			}
		}
	}
	else
	{
		WeaponMesh_WithoutBomb->SetRenderCustomDepth(false);
	}
}