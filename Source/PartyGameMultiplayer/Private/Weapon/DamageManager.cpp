// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon/DamageManager.h"

#include "Kismet/GameplayStatics.h"

#include "Weapon/BaseWeapon.h"
#include "Weapon/BaseProjectile.h"
#include "Weapon/CombinedWeapon/WeaponBomb.h"
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
		// Special situation: Bomb's fork
		if (AttackingWeapon->WeaponType == EnumWeaponType::Bomb && DeltaTime == -1.0f)
		{
			if (AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map.Contains("Fork"))
				Damage = AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map["Fork"];
		}
		AController* EventInstigator = AttackingWeapon->GetInstigator()->Controller;
		//pCharacter->TakeDamageRe(Damage, WeaponType, EventInstigator, AttackingWeapon);
		UGameplayStatics::ApplyDamage(DamagedActor, Damage, AttackingWeapon->GetInstigator()->Controller, AttackingWeapon, AttackingWeapon->DamageType);
	}
	else if (Cast<AMinigameMainObjective>(DamagedActor))
	{
		float Damage = 0.0f;
		FString ParName = AttackingWeapon->GetWeaponName();
		if (AWeaponDataHelper::DamageManagerDataAsset->MiniGame_Damage_Map.Contains(ParName))
			Damage = AWeaponDataHelper::DamageManagerDataAsset->MiniGame_Damage_Map[ParName];
		if (AttackingWeapon->AttackType == EnumAttackType::Constant)
			Damage *= DeltaTime;
		UGameplayStatics::ApplyDamage(DamagedActor, Damage, AttackingWeapon->GetInstigator()->Controller, AttackingWeapon, AttackingWeapon->MiniGameDamageType);
	}
	else
	{
		return false;
	}
	return true;
}

bool ADamageManager::TryApplyRadialDamage(ABaseWeapon* AttackingWeapon, FVector Epicenter)
{
	if (!AttackingWeapon)
		return false;

	FVector Origin = Epicenter;
	float DamageRadius = 500.0f;
	float BaseDamage = 30.0f;

	TArray<AActor*> IgnoredActors;
	// TODO: Add teammates
	IgnoredActors.Add(AttackingWeapon->GetHoldingPlayer());

	// Apply radial damage and impulse to all actors within the explosion radius
	UGameplayStatics::ApplyRadialDamage(
		AttackingWeapon,   //const UObject* WorldContextObject
		BaseDamage,
		Origin,
		DamageRadius,
		UDamageType::StaticClass(),  //TSubclassOf<UDamageType> DamageTypeClass
		IgnoredActors,				//const TArray<AActor*>& IgnoreActors
		AttackingWeapon,		//AActor* DamageCauser
		AttackingWeapon->GetHoldingPlayer()->GetController(), //AController* InstigatedByController
		true			  // bDoFullDamage
		//ECC_Visibility	  // DamagePreventionChannel
	);
	if(AttackingWeapon)
		DrawDebugSphere(AttackingWeapon->GetWorld(), Origin, DamageRadius, 12, FColor::Red, false, 5.0f);

	return true;
}

bool ADamageManager::ApplyBuff(ABaseWeapon* AttackingWeapon, TSubclassOf<UDamageType> DamageTypeClass, class AMCharacter* DamagedCharacter)
{	
	if (AttackingWeapon->WeaponType == EnumWeaponType::None || !AWeaponDataHelper::DamageManagerDataAsset)
		return false;

	TArray<EnumAttackBuff> AttackBuffs;
	// Fork
	if (AttackingWeapon->WeaponType == EnumWeaponType::Fork)
		AttackBuffs.Add(EnumAttackBuff::Knockback);
	// Bomb's fork
	else if (AttackingWeapon->WeaponType == EnumWeaponType::Bomb && DamageTypeClass == UDamageType::StaticClass())
	{
		AttackBuffs.Add(EnumAttackBuff::Knockback);
	}
	// Blower
	else if (AttackingWeapon->WeaponType == EnumWeaponType::Blower)
		AttackBuffs.Add(EnumAttackBuff::Knockback);
	// Lighter
	else if (AttackingWeapon->WeaponType == EnumWeaponType::Lighter)
		AttackBuffs.Add(EnumAttackBuff::Burning);
	// Flamethrower
	else if (AttackingWeapon->WeaponType == EnumWeaponType::Flamethrower)
	{
		AttackBuffs.Add(EnumAttackBuff::Burning);
		AttackBuffs.Add(EnumAttackBuff::Knockback);
	}
	// Flamefork
	else if (AttackingWeapon->WeaponType == EnumWeaponType::Flamefork)
		AttackBuffs.Add(EnumAttackBuff::Burning);
	// Taser
	else if (AttackingWeapon->WeaponType == EnumWeaponType::Taser)
		AttackBuffs.Add(EnumAttackBuff::Paralysis);

	for (int32 i = 0; i < AttackBuffs.Num(); i++)
	{
		EnumAttackBuff AttackBuff = AttackBuffs[i];
		if (AttackBuff == EnumAttackBuff::Burning)
		{
			float buffPoints = 0.0f;
			FString ParName = "BurningBuffPoints";
			if (AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map.Contains(ParName))
				buffPoints = AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map[ParName];
			DamagedCharacter->AccumulateAttackedBuff(EnumAttackBuff::Burning, buffPoints, FVector3d::Zero(),
				AttackingWeapon->GetInstigator()->Controller, AttackingWeapon);
		}
		else if (AttackBuff == EnumAttackBuff::Paralysis)
		{
			float buffPoints = 1.0f;
			DamagedCharacter->AccumulateAttackedBuff(EnumAttackBuff::Paralysis, buffPoints, FVector3d::Zero(),
				AttackingWeapon->GetInstigator()->Controller, AttackingWeapon);
		}
		else if (AttackBuff == EnumAttackBuff::Knockback)
		{
			float buffPoints = 1.0f;
			check(AttackingWeapon->GetHoldingPlayer());
			FRotator AttackerControlRotation = AttackingWeapon->GetHoldingPlayer()->GetControlRotation();
			FVector3d AttackerControlDir = AttackerControlRotation.RotateVector(FVector3d::ForwardVector);
			DamagedCharacter->AccumulateAttackedBuff(EnumAttackBuff::Knockback, buffPoints, AttackerControlDir,
				AttackingWeapon->GetInstigator()->Controller, AttackingWeapon);
		}
		else
		{
			return false;
		}
	}	
	return true;
}