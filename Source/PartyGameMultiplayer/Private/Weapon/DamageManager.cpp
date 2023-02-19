// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon/DamageManager.h"

#include "Kismet/GameplayStatics.h"

#include "Weapon/BaseWeapon.h"
//#include "Weapon/JsonFactory.h"
#include "Weapon/WeaponDataHelper.h"
#include "Character/MCharacter.h"
#include "LevelInteraction/MinigameMainObjective.h"


bool ADamageManager::DealDamageAndBuffBetweenActors(ABaseWeapon* AttackingWeapon, AActor* DamagedActor, float DeltaTime)
{
	if (!AttackingWeapon || !DamagedActor || !AWeaponDataHelper::DamageManagerDataAsset)
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
		float Damage = 0.0f;
		FString ParName = AttackingWeapon->GetWeaponName();
		if (AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map.Contains(ParName))
			Damage = AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map[ParName];
		if (AttackingWeapon->AttackType == EnumAttackType::Constant)
			Damage *= DeltaTime;
		AController* EventInstigator = AttackingWeapon->GetInstigator()->Controller;
		pCharacter->TakeDamageRe(Damage, WeaponType, EventInstigator, AttackingWeapon);

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
	}
	else if (dynamic_cast<AMinigameMainObjective*>(DamagedActor))
	{
		float Damage = 0.0f;
		FString ParName = AttackingWeapon->GetWeaponName();
		if (AWeaponDataHelper::DamageManagerDataAsset->MiniGame_Damage_Map.Contains(ParName))
			Damage = AWeaponDataHelper::DamageManagerDataAsset->MiniGame_Damage_Map[ParName];
		if (AttackingWeapon->AttackType == EnumAttackType::Constant)
			Damage *= DeltaTime;
		UGameplayStatics::ApplyDamage(DamagedActor, Damage, AttackingWeapon->GetInstigator()->Controller, AttackingWeapon, UDamageType::StaticClass());
	}
	else
	{
		return false;
	}
	return true;
}

bool ADamageManager::ApplyBuff(EnumAttackBuff AttackBuff, ABaseWeapon* AttackingWeapon, class AMCharacter* DamagedActor)
{
	if (!AWeaponDataHelper::DamageManagerDataAsset)
		return false;

	if (AttackBuff == EnumAttackBuff::Burning)
	{
		float buffPoints = 0.0f;
		FString ParName = "BurningBuffPoints";
		if (AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map.Contains(ParName))
			buffPoints = AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map[ParName];
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