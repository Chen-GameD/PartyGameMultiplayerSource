// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon/CombinedWeapon/WeaponTaser.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/PrimitiveComponent.h"
#include "Net/UnrealNetwork.h"


AWeaponTaser::AWeaponTaser()
{
	IsCombined = true;
	WeaponType = EnumWeaponType::Taser;
	WeaponName = WeaponEnumToString_Map[WeaponType];

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("/Game/ArtAssets/Models/Taser/Taser_Body.Taser_Body"));
	if (DefaultMesh.Succeeded())
	{
		WeaponMesh->SetStaticMesh(DefaultMesh.Object);
	}

	// create a secondary weapon mesh which is specific for Taser
	TaserForkMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TaserForkMesh"));
	TaserForkMesh->SetCollisionProfileName(TEXT("Trigger"));
	/*TaserForkMesh->SetCollisionProfileName(TEXT("Custom"));
	TaserForkMesh->SetSimulatePhysics(true);
	TaserForkMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	TaserForkMesh->SetCollisionResponseToAllChannels(ECR_Overlap);*/
	TaserForkMesh->SetupAttachment(WeaponMesh);
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
int AWeaponTaser::AttackStart()
{
	int result = Super::AttackStart();
	if (result == 0)
	{
		bStretching = true;
		//OnRep_bAttackOn();
		return 0;
	}	
	return -1;
}

// should only be called on server
void AWeaponTaser::AttackStop()
{
	Super::AttackStop();
	bStretching = false;
	//OnRep_bAttackOn();
}


void AWeaponTaser::CheckInitilization()
{
	Super::CheckInitilization();
	// do something specific to this weapon
	check(AttackHitEffect);
}
