// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/BaseWeapon.h"
#include "WeaponFlamethrower.generated.h"

UCLASS()
class PARTYGAMEMULTIPLAYER_API AWeaponFlamethrower : public ABaseWeapon
{
	GENERATED_BODY()

// MEMBER METHODS
public:
	AWeaponFlamethrower();

protected:
	virtual void CheckInitilization() override;
	/*virtual void GenerateAttackHitEffect() override;
	virtual void GenerateDamage(class AActor* DamagedActor) override;*/

private:

// MEMBER VARIABLES
public:
protected:
	// flame area
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		class UBoxComponent* BoxComponent;
private:

};
