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


bool ADamageManager::TryApplyDamageToAnActor(AActor* DamageCauser, AController* Controller, TSubclassOf<UDamageType> DamageTypeClass, AActor* DamagedActor, float DeltaTime)
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

	if (Cast<AMCharacter>(DamagedActor) || Cast<AMinigameMainObjective>(DamagedActor))
	{		
		float Damage = 0.0f;
		FString WeaponName = AWeaponDataHelper::WeaponEnumToString_Map[WeaponType];
		if (AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map.Contains(WeaponName))
			Damage = AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map[WeaponName];
		if (0 < DeltaTime)
			Damage *= DeltaTime;
		// Special situation: Bomb's fork, Taser's fork
		if( (WeaponType == EnumWeaponType::Bomb && DamageTypeClass == UMeleeDamageType::StaticClass()) || 
			WeaponType == EnumWeaponType::Taser && DamageTypeClass == UMeleeDamageType::StaticClass() )
		{
			if (AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map.Contains("Fork"))
				Damage = AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map["Fork"];
		}
		UGameplayStatics::ApplyDamage(DamagedActor, Damage, Controller, DamageCauser, DamageTypeClass);
	}
	else
	{
		return false;
	}
	return true;
}

//bool ADamageManager::TryApplyRadialDamage(AActor* DamageCauser, AController* Controller, FVector Origin, float DamageRadius, float BaseDamage)
//{
//	return TryApplyRadialDamage(DamageCauser, Controller, Origin, 0.0f, DamageRadius, BaseDamage);
//}

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

bool ADamageManager::ApplyBuff(EnumWeaponType WeaponType, AController* Controller, TSubclassOf<UDamageType> DamageTypeClass, class AMCharacter* DamagedCharacter, float DeltaTime)
{	
	if (WeaponType == EnumWeaponType::None || !Controller || !DamagedCharacter || !AWeaponDataHelper::DamageManagerDataAsset)
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
	{
		AttackBuffs.Add(EnumAttackBuff::Burning);
		AttackBuffs.Add(EnumAttackBuff::Knockback);
	}		
	else if (WeaponType == EnumWeaponType::Taser)
		AttackBuffs.Add(EnumAttackBuff::Paralysis);

	for (int32 i = 0; i < AttackBuffs.Num(); i++)
	{
		EnumAttackBuff AttackBuff = AttackBuffs[i];

		if (!DamagedCharacter->CheckBuffMap(AttackBuff))
			continue;

		float& BuffPoints = DamagedCharacter->BuffMap[AttackBuff][0];
		float& BuffRemainedTime = DamagedCharacter->BuffMap[AttackBuff][1];
		float& BuffAccumulatedTime = DamagedCharacter->BuffMap[AttackBuff][2];

		float buffPointsToAdd = 0.0f;
		float buffTimeToAdd = 0.0f;
		if (AttackBuff == EnumAttackBuff::Knockback && Controller->GetPawn())
		{
			FVector AttackingDirection = Controller->GetPawn()->GetControlRotation().RotateVector(FVector3d::ForwardVector);
			AttackingDirection.Normalize();
			DamagedCharacter->KnockbackDirection_SinceLastApplyBuff = AttackingDirection;
		}	
		if (AttackBuff == EnumAttackBuff::Paralysis && Controller->GetPawn())
		{
			FVector Direction_SelfToAttacker = Controller->GetPawn()->GetActorLocation() - DamagedCharacter->GetActorLocation();
			if (100.0f < Direction_SelfToAttacker.Length())
			{
				Direction_SelfToAttacker.Normalize();
				DamagedCharacter->TaserDragDirection_SinceLastApplyBuff = Direction_SelfToAttacker;
			}			
		}
		
		FString ParName = "";
		// Assign buffPointsToAdd (up to 3 times, the value may be overwrote by the next Search And Assign)
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
			buffPointsToAdd = DeltaTime * AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map[ParName];
		BuffPoints += buffPointsToAdd;
		if (1.0f <= BuffPoints)
		{
			// time to add		
			ParName = AWeaponDataHelper::AttackBuffEnumToString_Map[AttackBuff] + "_TimeToAdd";	
			// When it is burning, we add the designated time and substract 1 from BuffPoints; 	
			// We know only Burning has "Burning_TimeToAdd"
			if (AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map.Contains(ParName)) 
			{
				buffTimeToAdd = AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map[ParName];
				BuffRemainedTime += buffTimeToAdd;
				BuffAccumulatedTime += buffTimeToAdd;
				BuffPoints -= floorf(BuffPoints);
			}		
			// When it is paralysis, we will add DeltaTime(because that's the damage frequency of Taser), 
			// not touch the BuffPoints	
			if (AttackBuff == EnumAttackBuff::Paralysis)
			{
				buffTimeToAdd = DeltaTime;
				BuffRemainedTime += buffTimeToAdd;
				BuffAccumulatedTime += buffTimeToAdd;
			}
			// When it is knockback, we will neither add time nor touch the BuffPoints
		}
	}	
	DamagedCharacter->ActByBuff_PerDamage(DeltaTime);
	return true;
}


