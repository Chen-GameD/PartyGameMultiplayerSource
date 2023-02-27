// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "DamageManagerDataAsset.h"

#include "WeaponDataHelper.generated.h"

enum EnumWeaponType
{
	None,
	Fork,
	Blower,
	Lighter,
	Alarm,
	Flamethrower,
	Flamefork,
	Taser,
	Alarmgun,
	Bomb,
	Cannon
};

enum EnumAttackType
{
	OneHit,
	Constant,
	SpawnProjectile
};

enum EnumAttackBuff
{
	Burning,
	Knockback,
	//Blowing,
	Paralysis
};


UCLASS()
class PARTYGAMEMULTIPLAYER_API AWeaponDataHelper : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeaponDataHelper();

public:
	//static float interval_ApplyDamage; // preset value for all constant damage

	static UDamageManagerDataAsset* DamageManagerDataAsset;

	static TMap<EnumWeaponType, FString> WeaponEnumToString_Map;
	static TMap<EnumWeaponType, EnumAttackType> WeaponEnumToAttackTypeEnum_Map;
	static TMap<EnumAttackBuff, FString> AttackBuffEnumToString_Map;

};
