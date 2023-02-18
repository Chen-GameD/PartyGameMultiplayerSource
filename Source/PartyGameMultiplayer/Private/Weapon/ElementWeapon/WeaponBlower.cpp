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


AWeaponBlower::AWeaponBlower()
{
	/*
		bCanEverTick is set to true in BaseWeapon Class.
		You can uncomment the following line and turn this off to improve performance if you don't need it.
	*/
	//PrimaryActorTick.bCanEverTick = false;

	IsCombined = false;
	WeaponType = EnumWeaponType::Blower;
	WeaponName = WeaponEnumToString_Map[WeaponType];
	AttackType = EnumAttackType::Constant;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("/Game/ArtAssets/Models/Blower/Blower.Blower"));
	//Set the Static Mesh and its position/scale if we successfully found a mesh asset to use.
	if (DefaultMesh.Succeeded())
	{
		WeaponMesh->SetStaticMesh(DefaultMesh.Object);
	}

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("WindCollisionArea"));
	BoxComponent->SetRelativeLocation(FVector(-280.0f, 0.0f, 20.5f));
	//BoxComponent->SetBoxExtent(FVector3d(80.0f, 20.0f, 20.0f));
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

	DamageType = UDamageType::StaticClass();
}