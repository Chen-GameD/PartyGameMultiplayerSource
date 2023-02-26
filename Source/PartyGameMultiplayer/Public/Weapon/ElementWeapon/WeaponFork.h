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
	//virtual void OnRep_bAttackOn() override;
private:

// MEMBER VARIABLES
public:
protected:
	//UPROPERTY(EditAnywhere, Category = "Effects")
	//	class UNiagaraComponent* Wave_NSComponent;
	//UPROPERTY(EditAnywhere, Category = "Effects")
	//	class UNiagaraSystem* Wave_NSSystem;
private:

};
