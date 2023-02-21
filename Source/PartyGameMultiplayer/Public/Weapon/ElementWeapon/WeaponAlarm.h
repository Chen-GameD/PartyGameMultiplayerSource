// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/BaseWeapon.h"
#include "WeaponAlarm.generated.h"

/**
 * 
 */
UCLASS()
class PARTYGAMEMULTIPLAYER_API AWeaponAlarm : public ABaseWeapon
{
	GENERATED_BODY()

// MEMBER METHODS
public:
	AWeaponAlarm();

	// should only be called on server
	virtual void SpawnProjectile() override;

protected:
	virtual void OnRep_bAttackOn() override;
private:

// MEMBER VARIABLES
public:
protected:
private:
};
