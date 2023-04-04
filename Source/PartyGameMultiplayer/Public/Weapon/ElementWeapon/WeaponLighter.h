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
	virtual void OnAttackOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
		class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
private:

// MEMBER VARIABLES
public:
protected:
private:

};
