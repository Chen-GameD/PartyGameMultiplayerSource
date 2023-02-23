// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Weapon/BaseWeapon.h"

#include "BaseDamageCauser.generated.h"

UCLASS()
class PARTYGAMEMULTIPLAYER_API ABaseDamageCauser : public AActor
{
	GENERATED_BODY()
	
public:	
	ABaseDamageCauser() {};
	ABaseDamageCauser(EnumWeaponType i_WeaponType);

public:
	EnumWeaponType WeaponType;
};
