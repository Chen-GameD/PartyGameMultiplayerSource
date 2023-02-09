// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/DamageManagerNew.h"
#include "Weapon/BaseWeapon.h"
#include "Weapon/JsonFactory.h"
#include "Character/MCharacter.h"
#include "LevelInteraction/MinigameMainObjective.h"
#include "Kismet/GameplayStatics.h"


UDamageManagerDataAsset* ADamageManagerNew::DamageManagerDataAsset = nullptr;

// Sets default values
ADamageManagerNew::ADamageManagerNew()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// pops error from time to time when building, unstable
	if (!DamageManagerDataAsset)
	{
		static ConstructorHelpers::FObjectFinder<UDamageManagerDataAsset> DefaultDamageManagerDataAsset(TEXT("/Game/DataFiles/Weapon/DamageManagerDataAsset.DamageManagerDataAsset"));
		if (DefaultDamageManagerDataAsset.Succeeded())
		{
			DamageManagerDataAsset = DefaultDamageManagerDataAsset.Object;
		}
	}
}

// Called when the game starts or when spawned
void ADamageManagerNew::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADamageManagerNew::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


bool ADamageManagerNew::DealDamageAndBuffBetweenActors(ABaseWeapon* AttackingWeapon, AActor* DamagedActor)
{
	if (!AttackingWeapon || !DamagedActor)
		return false;

	if (auto pCharacter = Cast<AMCharacter>(DamagedActor))
	{
		// check holding player
		auto AttackingWeaponHoldingPlayer = AttackingWeapon->GetHoldingPlayer();
		if (!AttackingWeaponHoldingPlayer)
			return false;
		// check both player controllers
		auto AttackingWeaponHoldingCharacterController = AttackingWeaponHoldingPlayer->GetController();
		auto DamagedCharacterController = pCharacter->GetController();
		if (!AttackingWeaponHoldingCharacterController || !DamagedCharacterController)
			return false;
		// check both player states
		AM_PlayerState* pCharacterPS = AttackingWeaponHoldingCharacterController->GetPlayerState<AM_PlayerState>();
		AM_PlayerState* AttakingWeaponPS = DamagedCharacterController->GetPlayerState<AM_PlayerState>();
		if (!pCharacterPS || !AttakingWeaponPS)
			return false;
		if (pCharacterPS->TeamIndex == AttakingWeaponPS->TeamIndex)
			return false;

		EnumWeaponType WeaponType = AttackingWeapon->WeaponType;
		float Damage = AttackingWeapon->Damage;
		FString ParName = AttackingWeapon->GetWeaponName();
		if (DamageManagerDataAsset->Character_Damage_Map.Contains(ParName))
			Damage = DamageManagerDataAsset->Character_Damage_Map[ParName];

		AController* EventInstigator = AttackingWeapon->GetInstigator()->Controller;

		if (AttackingWeapon->WeaponType == EnumWeaponType::None)
		{
			return false;
		}
		// Fork
		else if (AttackingWeapon->WeaponType == EnumWeaponType::Fork)
		{
			ApplyBuff(EnumAttackBuff::Knockback, AttackingWeapon, pCharacter);
		}
		// Blower
		else if (AttackingWeapon->WeaponType == EnumWeaponType::Blower)
		{
			ApplyBuff(EnumAttackBuff::Knockback, AttackingWeapon, pCharacter);
		}
		// Lighter
		else if (AttackingWeapon->WeaponType == EnumWeaponType::Lighter)
		{
			ApplyBuff(EnumAttackBuff::Burning, AttackingWeapon, pCharacter);
		}
		// Flamethrower
		else if (AttackingWeapon->WeaponType == EnumWeaponType::Flamethrower)
		{
			ApplyBuff(EnumAttackBuff::Burning, AttackingWeapon, pCharacter);
			ApplyBuff(EnumAttackBuff::Knockback, AttackingWeapon, pCharacter);
		}
		// Flamefork
		else if (AttackingWeapon->WeaponType == EnumWeaponType::Flamefork)
		{
			ApplyBuff(EnumAttackBuff::Burning, AttackingWeapon, pCharacter);
		}
		// Taser
		else if (AttackingWeapon->WeaponType == EnumWeaponType::Taser)
		{
			ApplyBuff(EnumAttackBuff::Paralysis, AttackingWeapon, pCharacter);
		}

		// TODO: judge if it is a teammate
		pCharacter->TakeDamageRe(Damage, WeaponType, EventInstigator, AttackingWeapon);

	}
	else if (dynamic_cast<AMinigameMainObjective*>(DamagedActor))
	{
		float Damage = AttackingWeapon->MiniGameDamage;
		FString ParName = AttackingWeapon->GetWeaponName();
		if (DamageManagerDataAsset->MiniGame_Damage_Map.Contains(ParName))
			Damage = DamageManagerDataAsset->MiniGame_Damage_Map[ParName];

		//// Temporary; weird to assign MiniGameAccumulatedTimeToGenerateDamage here
		//if (999.0f < AttackingWeapon->MiniGameAccumulatedTimeToGenerateDamage)
		//	AttackingWeapon->MiniGameAccumulatedTimeToGenerateDamage = AttackingWeapon->AccumulatedTimeToGenerateDamage;

		UGameplayStatics::ApplyDamage(DamagedActor, Damage, AttackingWeapon->GetInstigator()->Controller, AttackingWeapon, UDamageType::StaticClass());
	}
	else
	{
		return false;
	}
	return true;
}

bool ADamageManagerNew::ApplyBuff(EnumAttackBuff AttackBuff, ABaseWeapon* AttackingWeapon, class AMCharacter* DamagedActor)
{
	if (AttackBuff == EnumAttackBuff::Burning)
	{
		float buffPoints = 0.0f;		
		FString ParName = "BurningBuffPoints";
		if (DamageManagerDataAsset->Character_Buff_Map.Contains(ParName))
			buffPoints = DamageManagerDataAsset->Character_Buff_Map[ParName];
		DamagedActor->AccumulateAttackedBuff(EnumAttackBuff::Burning, buffPoints, FVector3d::Zero(),
			AttackingWeapon->GetInstigator()->Controller, AttackingWeapon);
	}
	else if (AttackBuff == EnumAttackBuff::Paralysis)
	{
		float buffPoints = 1.0f;
		DamagedActor->AccumulateAttackedBuff(EnumAttackBuff::Paralysis, buffPoints, FVector3d::Zero(),
			AttackingWeapon->GetInstigator()->Controller, AttackingWeapon);
	}
	else if (AttackBuff == EnumAttackBuff::Knockback)
	{
		float buffPoints = 1.0f;
		check(AttackingWeapon->GetHoldingPlayer());
		FRotator AttackerControlRotation = AttackingWeapon->GetHoldingPlayer()->GetControlRotation();
		FVector3d AttackerControlDir = AttackerControlRotation.RotateVector(FVector3d::ForwardVector);
		DamagedActor->AccumulateAttackedBuff(EnumAttackBuff::Knockback, buffPoints, AttackerControlDir,
			AttackingWeapon->GetInstigator()->Controller, AttackingWeapon);
	}
	else
	{
		return false;
	}
	return true;
}