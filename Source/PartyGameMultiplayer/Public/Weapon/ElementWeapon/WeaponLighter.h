// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/BaseWeapon.h"
#include "WeaponLighter.generated.h"

UCLASS()
class PARTYGAMEMULTIPLAYER_API AWeaponLighter : public ABaseWeapon
{
	GENERATED_BODY()

// MEMBER METHODS
public:
	AWeaponLighter();

protected:
	virtual void CheckInitilization() override;
	/*virtual void BeginPlay() override;
	virtual void Destroyed() override; */
	/*virtual void GenerateAttackHitEffect() override;
	virtual void GenerateDamage(class AActor* DamagedActor) override;*/

private:

// MEMBER VARIABLES
public:
protected:
private:

};
