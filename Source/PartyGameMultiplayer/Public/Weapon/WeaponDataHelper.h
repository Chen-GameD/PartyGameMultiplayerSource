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
	Cannon,
	Shell
};

enum EnumAttackType
{
	OneHit,
	Constant,
	SpawnProjectile
};

// It is better named EnumBuff
enum EnumAttackBuff
{
	Burning,
	Knockback,
	Paralysis,
	Saltcure,
	Shellheal,
};


UCLASS()
class PARTYGAMEMULTIPLAYER_API AWeaponDataHelper : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeaponDataHelper();

public:
	static UDamageManagerDataAsset* DamageManagerDataAsset;
	static float interval_ConstantWeaponApplyKnockback;

	static TMap<EnumWeaponType, FString> WeaponEnumToString_Map;
	static TMap<EnumWeaponType, EnumAttackType> WeaponEnumToAttackTypeEnum_Map;
	static TMap<EnumAttackBuff, FString> AttackBuffEnumToString_Map;
};
