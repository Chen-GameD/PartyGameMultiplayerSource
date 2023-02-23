// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon/CombinedWeapon/WeaponTaser.h"

#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/PrimitiveComponent.h"
#include "Net/UnrealNetwork.h"

#include "Weapon/BaseProjectile.h"
#include "Weapon/DamageManager.h"
#include "Weapon/DamageType/MeleeDamageType.h"
#include "LevelInteraction/MinigameMainObjective.h"


AWeaponTaser::AWeaponTaser()
{
	IsCombineWeapon = true;
	WeaponType = EnumWeaponType::Taser;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("/Game/ArtAssets/Models/Taser/Taser_Body.Taser_Body"));
	if (DefaultMesh.Succeeded())
	{
		WeaponMesh->SetStaticMesh(DefaultMesh.Object);
	}

	// Create a fork mesh specific to Taser
	TaserForkMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TaserForkMesh"));
	TaserForkMesh->SetupAttachment(WeaponMesh);
	TaserForkMesh->SetCollisionProfileName(TEXT("Trigger"));	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultTaserForkMesh(TEXT("/Game/ArtAssets/Models/Taser/Taser_Fork.Taser_Fork"));
	if (DefaultTaserForkMesh.Succeeded())
	{
		TaserForkMesh->SetStaticMesh(DefaultTaserForkMesh.Object);
		TaserForkMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
		TaserForkMesh->SetRelativeScale3D(FVector(2.0f, 2.0f, 2.0f));
	}

	AttackDetectComponent = TaserForkMesh;

	static ConstructorHelpers::FObjectFinder<UParticleSystem> DefaultAttackHitEffect(TEXT("/Game/StarterContent/Particles/P_Explosion.P_Explosion"));
	if (DefaultAttackHitEffect.Succeeded())
	{
		AttackHitEffect = DefaultAttackHitEffect.Object;
	}

	bStretching = true;
	originalX = TaserForkMesh->GetRelativeLocation().X;
	maxLen = 160.0f;
	strechOutSpeed = 360.0f;
	strechInSpeed = 160.0f;
}


void AWeaponTaser::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// currently, client and server shared the same behaviour
	if (bAttackOn)
	{
		if (bStretching)
		{
			if (originalX - maxLen < TaserForkMesh->GetRelativeLocation().X)
				TaserForkMesh->SetRelativeLocation(TaserForkMesh->GetRelativeLocation() + DeltaTime * FVector3d(-strechOutSpeed, 0, 0));
			else
				bStretching = false;
		}
		else
		{
			if (TaserForkMesh->GetRelativeLocation().X < originalX)
				TaserForkMesh->SetRelativeLocation(TaserForkMesh->GetRelativeLocation() + DeltaTime * FVector3d(strechInSpeed, 0, 0));
			else
			{
				if(GetLocalRole() == ROLE_Authority)
					bAttackOn = false;
				bStretching = true;
			}	
		}
	}
	else
	{
		if (TaserForkMesh->GetRelativeLocation().X < originalX)
			TaserForkMesh->SetRelativeLocation(TaserForkMesh->GetRelativeLocation() + DeltaTime * FVector3d(strechInSpeed, 0, 0));
	}
		
}


// should only be called on server
void AWeaponTaser::AttackStart()
{
	Super::AttackStart();
	bStretching = true;
}

// should only be called on server
void AWeaponTaser::AttackStop()
{
	Super::AttackStop();
	bStretching = false;
}


void AWeaponTaser::OnAttackOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
	class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IsPickedUp && GetOwner())
	{
		if ((Cast<ACharacter>(OtherActor) && OtherActor != GetOwner()) ||
			Cast<AMinigameMainObjective>(OtherActor))
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

			if (ApplyDamageCounter == 0 && HoldingController)
			{
				ADamageManager::TryApplyDamageToAnActor(this, HoldingController, UMeleeDamageType::StaticClass(), OtherActor);
				ApplyDamageCounter++;
			}
		}
		else
		{
			// if hit something other than the following(like building, rocks, etc), the attack should stop
			if(!Cast<ACharacter>(OtherActor) && !Cast<ABaseWeapon>(OtherActor) && !Cast<ABaseProjectile>(OtherActor))
				AttackStop();
		}
	}
}