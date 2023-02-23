// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon/DamageManager.h"

#include "Kismet/GameplayStatics.h"

#include "Weapon/BaseWeapon.h"
#include "Weapon/BaseProjectile.h"
#include "Weapon/BaseDamageCauser.h"
#include "Weapon/CombinedWeapon/WeaponBomb.h"
//#include "Weapon/JsonFactory.h"
#include "Weapon/WeaponDataHelper.h"
#include "Weapon/DamageType/MeleeDamageType.h"
#include "Character/MCharacter.h"
#include "LevelInteraction/MinigameMainObjective.h"

//FTimerHandle* ADamageManager::TimerHandle_Loop = nullptr;
float ADamageManager::interval_ApplyDamage = 0.1f; // preset value for all constant damage


bool ADamageManager::TryApplyDamageToAnActor(AActor* DamageCauser, AController* Controller, TSubclassOf<UDamageType> DamageTypeClass, AActor* DamagedActor)
{
	if (!DamageCauser || !DamagedActor || !AWeaponDataHelper::DamageManagerDataAsset)
		return false;

	EnumWeaponType WeaponType = EnumWeaponType::None;
	if (auto p = Cast<ABaseWeapon>(DamageCauser))
		WeaponType = p->WeaponType;
	if (auto p = Cast<ABaseProjectile>(DamageCauser))
		WeaponType = p->WeaponType;
	if (WeaponType == EnumWeaponType::None)
		return false;

	if (auto pPawn = Controller->GetPawn())
	{		
		float Damage = 0.0f;
		FString ParName = AWeaponDataHelper::WeaponEnumToString_Map[WeaponType];
		if (AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map.Contains(ParName))
			Damage = AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map[ParName];
		if (AWeaponDataHelper::WeaponEnumToAttackTypeEnum_Map[WeaponType] == EnumAttackType::Constant)
			Damage *= interval_ApplyDamage;
		// Special situation: Bomb's fork
		if (WeaponType == EnumWeaponType::Bomb && DamageTypeClass == UMeleeDamageType::StaticClass())
		{
			if (AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map.Contains("Fork"))
				Damage = AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map["Fork"];
		}
		UGameplayStatics::ApplyDamage(DamagedActor, Damage, Controller, DamageCauser, DamageTypeClass);
	}
	else if (Cast<AMinigameMainObjective>(DamagedActor))
	{
		float Damage = 0.0f;
		FString ParName = AWeaponDataHelper::WeaponEnumToString_Map[WeaponType];
		if (AWeaponDataHelper::DamageManagerDataAsset->MiniGame_Damage_Map.Contains(ParName))
			Damage = AWeaponDataHelper::DamageManagerDataAsset->MiniGame_Damage_Map[ParName];
		if (AWeaponDataHelper::WeaponEnumToAttackTypeEnum_Map[WeaponType] == EnumAttackType::Constant)
			Damage *= interval_ApplyDamage;
		UGameplayStatics::ApplyDamage(DamagedActor, Damage, Controller, DamageCauser, DamageTypeClass);
	}
	else
	{
		return false;
	}
	return true;
}


bool ADamageManager::TryApplyRadialDamage(AActor* DamageCauser, AController* Controller, FVector Origin, float DamageRadius, float BaseDamage)
{
	return TryApplyRadialDamage(DamageCauser, Controller, Origin, 0.0f, DamageRadius, BaseDamage);
}

bool ADamageManager::TryApplyRadialDamage(AActor* DamageCauser, AController* Controller, FVector Origin, float DamageInnerRadius, float DamageOuterRadius, float BaseDamage)
{
	if (!DamageCauser || !Controller)
		return false;

	EnumWeaponType WeaponType = EnumWeaponType::None;
	if (auto p = Cast<ABaseWeapon>(DamageCauser))
		WeaponType = p->WeaponType;
	if (auto p = Cast<ABaseProjectile>(DamageCauser))
		WeaponType = p->WeaponType;
	if (WeaponType == EnumWeaponType::None)
		return false;

	TArray<AActor*> IgnoredActors;
	auto AttackingPlayer = Controller->GetPawn();
	if (AttackingPlayer)
		IgnoredActors.Add(AttackingPlayer);

	bool bDoFullDamage = true;
	float DamageFalloff = bDoFullDamage ? 0.f : 1.f;

	UGameplayStatics::ApplyRadialDamageWithFalloff(
		Controller,   //const UObject* WorldContextObject
		BaseDamage,
		0.0f,			// float MinDamage
		Origin,
		DamageInnerRadius,			// float DamageInnerRadius
		DamageOuterRadius,
		DamageFalloff,
		UDamageType::StaticClass(),  //TSubclassOf<UDamageType> DamageTypeClass
		IgnoredActors,				//const TArray<AActor*>& IgnoreActors
		DamageCauser,		//AActor* DamageCauser
		Controller				 //AController* InstigatedByController
		//ECC_Visibility	  // DamagePreventionChannel
	);

	return true;
}

bool ADamageManager::ApplyBuff(AActor* DamageCauser, AController* Controller, TSubclassOf<UDamageType> DamageTypeClass, class AMCharacter* DamagedCharacter)
{	
	if (!DamageCauser || !Controller || !DamagedCharacter || !AWeaponDataHelper::DamageManagerDataAsset)
		return false;

	EnumWeaponType WeaponType = EnumWeaponType::None;
	if (auto p = Cast<ABaseWeapon>(DamageCauser))
		WeaponType = p->WeaponType;
	if (auto p = Cast<ABaseProjectile>(DamageCauser))
		WeaponType = p->WeaponType;
	if (WeaponType == EnumWeaponType::None)
		return false;

	TArray<EnumAttackBuff> AttackBuffs;
	if (WeaponType == EnumWeaponType::Fork)
		AttackBuffs.Add(EnumAttackBuff::Knockback);
	else if (WeaponType == EnumWeaponType::Bomb && DamageTypeClass == UMeleeDamageType::StaticClass())
		AttackBuffs.Add(EnumAttackBuff::Knockback);
	else if (WeaponType == EnumWeaponType::Blower)
		AttackBuffs.Add(EnumAttackBuff::Knockback);
	else if (WeaponType == EnumWeaponType::Lighter)
		AttackBuffs.Add(EnumAttackBuff::Burning);
	else if (WeaponType == EnumWeaponType::Flamethrower)
	{
		AttackBuffs.Add(EnumAttackBuff::Burning);
		AttackBuffs.Add(EnumAttackBuff::Knockback);
	}
	else if (WeaponType == EnumWeaponType::Flamefork)
		AttackBuffs.Add(EnumAttackBuff::Burning);
	else if (WeaponType == EnumWeaponType::Taser)
		AttackBuffs.Add(EnumAttackBuff::Paralysis);

	for (int32 i = 0; i < AttackBuffs.Num(); i++)
	{
		EnumAttackBuff AttackBuff = AttackBuffs[i];

		check(DamagedCharacter->CheckBuffMap(AttackBuff));
		float& BuffPoints = DamagedCharacter->BuffMap[AttackBuff][0];
		float& BuffRemainedTime = DamagedCharacter->BuffMap[AttackBuff][1];
		float& BuffAccumulatedTime = DamagedCharacter->BuffMap[AttackBuff][2];

		float buffPointsToAdd = 0.0f;
		float buffTimeToAdd = 0.0f;
		if (AttackBuff == EnumAttackBuff::Knockback && Controller->GetPawn())
		{
			FVector3d AttackingDirection = Controller->GetPawn()->GetControlRotation().RotateVector(FVector3d::ForwardVector);
			DamagedCharacter->KnockbackDirection_DuringLastFrame += AttackingDirection;
		}		
		
		FString ParName = "";
		// points to add
		ParName = AWeaponDataHelper::AttackBuffEnumToString_Map[AttackBuff] + "_PointsToAdd_PerHit";
		if (AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map.Contains(ParName))
			buffPointsToAdd = AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map[ParName];
		ParName = AWeaponDataHelper::WeaponEnumToString_Map[WeaponType] + "_" + 
				AWeaponDataHelper::AttackBuffEnumToString_Map[AttackBuff] + "_PointsToAdd_PerHit";
		if (AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map.Contains(ParName))
			buffPointsToAdd = AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map[ParName];
		ParName = AWeaponDataHelper::WeaponEnumToString_Map[WeaponType] + "_" +
			AWeaponDataHelper::AttackBuffEnumToString_Map[AttackBuff] + "_PointsToAdd_PerSec";
		if (AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map.Contains(ParName))
			buffPointsToAdd = interval_ApplyDamage * AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map[ParName];
		BuffPoints += buffPointsToAdd;
		if(AttackBuff == EnumAttackBuff::Burning)
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("BuffPoints: %f"), BuffPoints));
		if (1.0f <= BuffPoints)
		{
			// time to add		
			ParName = AWeaponDataHelper::AttackBuffEnumToString_Map[AttackBuff] + "_TimeToAdd";	
			// when it is time-based buff(burning), we add the time and substract 1 from BuffPoints; 
			// otherwise(paralysis, knockback), we will neither add the time nor touch the BuffPoints
			if (AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map.Contains(ParName))
			{
				buffTimeToAdd = AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map[ParName];
				BuffRemainedTime += buffTimeToAdd;
				BuffAccumulatedTime += buffTimeToAdd;
				BuffPoints -= floorf(BuffPoints);
			}				
		}
	}	
	return true;
}


