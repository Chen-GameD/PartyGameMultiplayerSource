// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/BaseWeapon.h"

#include "WeaponCannon.generated.h"


UCLASS()
class PARTYGAMEMULTIPLAYER_API AWeaponCannon : public ABaseWeapon
{
	GENERATED_BODY()

// MEMBER METHODS
public:
	AWeaponCannon();

	// should only be called on server
	virtual void SpawnProjectile(float AttackTargetDistance) override;

protected:
private:

// MEMBER VARIABLES
public:
protected:
private:
};
