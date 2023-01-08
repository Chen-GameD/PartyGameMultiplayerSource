﻿#include "WeaponConfig.h"

#include "CombinedWeapon/WeaponFlamefork.h"
#include "CombinedWeapon/WeaponFlamethrower.h"
#include "CombinedWeapon/WeaponTaser.h"

FName WeaponConfig::GetWeaponSocketName(FString weaponName)
{
	return SocketName[weaponName];
}

EnumWeaponType WeaponConfig::GetCombineWeaponIndex(FString weaponName_1, FString weaponName_2)
{
	FString CombineString = weaponName_1 + "+" + weaponName_2;
	return CombineWeaponIndex[CombineString];
}

int WeaponConfig::GetOnCombineClassRef(FString weaponName_1, FString weaponName_2)
{
	EnumWeaponType newType = EnumWeaponType::None;
	int CombineClass = -1;

	newType = GetCombineWeaponIndex(weaponName_1, weaponName_2);

	switch (newType)
	{
		case None:
		case Fork:
		case Blower:
		case Lighter:
			break;
		case Flamethrower:
			CombineClass = 0;
			break;
		case Flamefork:
			CombineClass = 1;
			break;
		case Taser:
			CombineClass = 2;
			break;
		default: ;
	}

	return CombineClass;
}

int WeaponConfig::GetAnimStateIndex(EnumWeaponType i_wpType) {
	return WeaponAnimStateMap[i_wpType];
}