// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/WeaponDataHelper.h"

#include "Kismet/GameplayStatics.h"


UDamageManagerDataAsset* AWeaponDataHelper::DamageManagerDataAsset = nullptr;

TMap<EnumWeaponType, FString> AWeaponDataHelper::WeaponEnumToString_Map =
{
	{EnumWeaponType::None, "None"},
	{EnumWeaponType::Fork, "Fork"},
	{EnumWeaponType::Blower, "Blower"},
	{EnumWeaponType::Lighter, "Lighter"},
	{EnumWeaponType::Alarm, "Alarm"},
	{EnumWeaponType::Flamethrower, "Flamethrower"},
	{EnumWeaponType::Flamefork, "Flamefork"},
	{EnumWeaponType::Taser, "Taser"},
	{EnumWeaponType::Alarmgun, "Alarmgun"},
	{EnumWeaponType::Bomb, "Bomb"},
	{EnumWeaponType::Cannon, "Cannon"},
};

TMap<EnumWeaponType, EnumAttackType> AWeaponDataHelper::WeaponEnumToAttackTypeEnum_Map =
{
	{EnumWeaponType::Fork, EnumAttackType::OneHit},
	{EnumWeaponType::Blower, EnumAttackType::Constant},
	{EnumWeaponType::Lighter, EnumAttackType::OneHit},
	{EnumWeaponType::Alarm, EnumAttackType::SpawnProjectile},
	{EnumWeaponType::Flamethrower, EnumAttackType::Constant},
	{EnumWeaponType::Flamefork, EnumAttackType::OneHit},
	{EnumWeaponType::Taser, EnumAttackType::Constant},
	{EnumWeaponType::Alarmgun, EnumAttackType::SpawnProjectile},
	{EnumWeaponType::Bomb, EnumAttackType::SpawnProjectile},
	{EnumWeaponType::Cannon, EnumAttackType::SpawnProjectile},
};

TMap<EnumAttackBuff, FString> AWeaponDataHelper::AttackBuffEnumToString_Map =
{
	{EnumAttackBuff::Burning, "Burning"},
	{EnumAttackBuff::Paralysis, "Paralysis"},
	{EnumAttackBuff::Blowing, "Blowing"},
	{EnumAttackBuff::Knockback, "Knockback"},
};



AWeaponDataHelper::AWeaponDataHelper()
{
	if (!DamageManagerDataAsset)
	{
		//static ConstructorHelpers::FObjectFinder<UDamageManagerDataAsset> DefaultDamageManagerDataAsset(*RandomFileName);
		static ConstructorHelpers::FObjectFinder<UDamageManagerDataAsset> DefaultDamageManagerDataAsset(TEXT("/Game/DataFiles/Weapon/DamageManagerDataAsset.DamageManagerDataAsset"));
		if (DefaultDamageManagerDataAsset.Succeeded())
		{
			DamageManagerDataAsset = DefaultDamageManagerDataAsset.Object;
		}
	}
}


