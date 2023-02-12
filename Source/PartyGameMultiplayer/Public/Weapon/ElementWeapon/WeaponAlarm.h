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
	virtual void AttackStart() override;
	// should only be called on server
	virtual void AttackStop() override;

protected:
private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class ABaseProjectile> SpecificProjectileClass;

// MEMBER VARIABLES
public:
protected:
private:
	
};
