// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/BaseWeapon.h"
#include "WeaponFork.generated.h"

UCLASS()
class PARTYGAMEMULTIPLAYER_API AWeaponFork : public ABaseWeapon
{
	GENERATED_BODY()

// MEMBER METHODS
public:
	AWeaponFork();

protected:
	virtual void CheckInitilization() override;
	/*virtual void GenerateAttackHitEffect() override;
	virtual void GenerateDamage(class AActor* DamagedActor) override;*/

private:

	// MEMBER VARIABLES
public:
protected:
private:

};
