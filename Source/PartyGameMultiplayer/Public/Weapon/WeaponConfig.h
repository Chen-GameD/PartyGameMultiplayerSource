#pragma once
#include <map>
#include <vector>

#include "BaseWeapon.h"
#include "Weapon/WeaponDataHelper.h"

class WeaponConfig
{

// Interfaces
public:

	static WeaponConfig* GetInstance()
	{
		if (Instance == nullptr)
		{
			Instance = new WeaponConfig();
		}

		return Instance;
	}
	
	FName GetWeaponSocketName(FString weaponName);
	EnumWeaponType GetCombineWeaponIndex(FString weaponName_1, FString weaponName_2);
	int GetOnCombineClassRef(FString weaponName_1, FString weaponName_2);
	int GetAnimStateIndex(EnumWeaponType i_wpType);

private:
	WeaponConfig(){}
	~WeaponConfig(){}

// Members
public:

private:
	static WeaponConfig* Instance;
	
	std::map<FString, FName> SocketName{
		{"Blower", "Blower_Socket"},
		{"Fork", "Fork_Socket"},
		{"Lighter", "Lighter_Socket"},
		{"Alarm", "Alarm_Socket"},
		{"Flamefork", "Flamefork_Socket"},
		{"Flamethrower", "Flamethrower_Socket"},
		{"Taser", "Taser_Socket"},
		{"Alarmgun", "Alarmgun_Socket"},
		{"Bomb", "Bomb_Socket"},
		{"Cannon", "Cannon_Socket"},
		{"Shell", "Shell_Socket"}
	};

	std::map<FString, EnumWeaponType> CombineWeaponIndex{
		{"Lighter+Fork", EnumWeaponType::Flamefork},
		{"Blower+Lighter", EnumWeaponType::Flamethrower},
		{"Blower+Fork", EnumWeaponType::Taser},
		{"Fork+Lighter", EnumWeaponType::Flamefork},
		{"Lighter+Blower", EnumWeaponType::Flamethrower},
		{"Fork+Blower", EnumWeaponType::Taser},
		{"Alarm+Blower", EnumWeaponType::Alarmgun},
		{"Blower+Alarm", EnumWeaponType::Alarmgun},
		{"Alarm+Fork", EnumWeaponType::Bomb},
		{"Fork+Alarm", EnumWeaponType::Bomb},
		{"Alarm+Lighter", EnumWeaponType::Cannon},
		{"Lighter+Alarm", EnumWeaponType::Cannon},
	};

	// Weapon Attack Style state Map - Weapon Type: Attack Type State Index in Anim State Vector
	// Anim state vector format: 
	// [isSingleMelee, isSingleShooting, isDualMelee, isDualShooting, isDualHeavy, isLeftHeld, isRightHeld, isAttack]
	std::map<EnumWeaponType, int> WeaponAnimStateMap{
		{EnumWeaponType::Fork, 0},
		{EnumWeaponType::Blower, 1},
		{EnumWeaponType::Lighter, 0},
		{EnumWeaponType::Alarm, 0},
		{EnumWeaponType::Flamethrower, 4},
		{EnumWeaponType::Flamefork, 0},
		{EnumWeaponType::Taser, 1},
		{EnumWeaponType::Alarmgun, 4},
		{EnumWeaponType::Bomb, 0},
		{EnumWeaponType::Cannon, 4}
	};

private:
};

