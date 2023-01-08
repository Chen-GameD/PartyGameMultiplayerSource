// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseWeapon.h"
#include "Character/MCharacter.h"

class PARTYGAMEMULTIPLAYER_API DamageManager
{
public:
	DamageManager();
	~DamageManager();

	static bool DealDamageAndBuffBetweenActors(ABaseWeapon* AttackingWeapon, class AActor* DamagedActor);
	static bool ApplyBuff(EnumAttackBuff AttackBuff, ABaseWeapon* AttackingWeapon, class AMCharacter* DamagedActor);
protected:
private:

public:
protected:
private:

};
