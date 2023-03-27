// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/BaseWeapon.h"
#include "WeaponFlamefork.generated.h"

UCLASS()
class PARTYGAMEMULTIPLAYER_API AWeaponFlamefork : public ABaseWeapon
{
	GENERATED_BODY()

// MEMBER METHODS
public:
	AWeaponFlamefork();

	virtual void AttackStart(float AttackTargetDistance) override;
	// should only be called on server
	virtual void SpawnProjectile(float AttackTargetDistance) override;

protected:

private:

// MEMBER VARIABLES
public:
protected:
private:

};
