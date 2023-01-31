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

	// Set the Static Mesh and its position/scale if we successfully found a mesh asset to use.
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

	DamageType = UDamageType::StaticClass();
	Damage = 25.0f;

	// WeaponName
	WeaponName = "Taser";

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
	//OnRep_bAttackOn();
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


//void AWeaponTaser::GenerateAttackHitEffect()
//{
//	FVector spawnLocation = GetActorLocation();
//	UGameplayStatics::SpawnEmitterAtLocation(this, AttackHitEffect, spawnLocation, FRotator::ZeroRotator, true, EPSCPoolMethod::AutoRelease);
//}
//
//
//void AWeaponTaser::GenerateDamage(class AActor* DamagedActor)
//{
//	// Note: The 3rd parameter is EventInstigator, be careful if the weapon has an instigator or not.
//	// if it doesn't and the 3rd parameter is set to GetInstigator()->Controller, the game would crash when overlap happens
//	// (The projectile in the demo has an instigator, because the instigator parameter is assigned when the the character spawns it in HandleFire function)
//	check(GetInstigator()->Controller);
//	UGameplayStatics::ApplyDamage(DamagedActor, Damage, GetInstigator()->Controller, this, DamageType);
//}

void AWeaponTaser::OnRep_bAttackOn()
{
	Super::OnRep_bAttackOn();
	/*if(bAttackOn)
		TaserForkMesh->SetRelativeLocation(TaserForkMesh->GetRelativeLocation() + FVector3d(-150.0f, 0, 0));
	else
		TaserForkMesh->SetRelativeLocation(TaserForkMesh->GetRelativeLocation() - FVector3d(-150.0f, 0, 0));*/
}