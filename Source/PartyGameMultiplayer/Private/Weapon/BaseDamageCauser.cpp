// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/BaseDamageCauser.h"
#include "Weapon/BaseWeapon.h"

ABaseDamageCauser::ABaseDamageCauser(EnumWeaponType i_WeaponType)
{
	PrimaryActorTick.bCanEverTick = false;

	WeaponType = i_WeaponType;
}
