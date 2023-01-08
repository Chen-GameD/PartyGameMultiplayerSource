// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/BaseWeapon.h"
#include "WeaponBlower.generated.h"

UCLASS()
class PARTYGAMEMULTIPLAYER_API AWeaponBlower : public ABaseWeapon
{
	GENERATED_BODY()

// MEMBER METHODS
public:
	AWeaponBlower();

protected:
	/*virtual void GenerateAttackHitEffect() override;
	virtual void GenerateDamage(class AActor* DamagedActor) override;*/

private:

// MEMBER VARIABLES
public:
protected:
	// wind area
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		class UBoxComponent* BoxComponent;
private:
};

