// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/WeaponDataHelper.h"

#include "Kismet/GameplayStatics.h"


FString AWeaponDataHelper::RandomFileName = "";  // useless as of now
UDamageManagerDataAsset* AWeaponDataHelper::DamageManagerDataAsset = nullptr;

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


