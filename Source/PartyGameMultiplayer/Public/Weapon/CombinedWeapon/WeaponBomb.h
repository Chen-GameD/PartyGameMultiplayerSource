// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/BaseWeapon.h"
#include "WeaponBomb.generated.h"

/**
 * 
 */
UCLASS()
class PARTYGAMEMULTIPLAYER_API AWeaponBomb : public ABaseWeapon
{
	GENERATED_BODY()

// MEMBER METHODS
public:
	AWeaponBomb();

	// should only be called on server
	virtual void SpawnProjectile() override;

protected:
	virtual void OnRep_bAttackOn() override;
private:

// MEMBER VARIABLES
public:
	// Static Mesh used to provide a visual representation of the object.
	UPROPERTY(EditAnywhere, Category = "Components")
		class UStaticMeshComponent* WeaponMesh_WithoutBomb;
protected:
private:
	
};
