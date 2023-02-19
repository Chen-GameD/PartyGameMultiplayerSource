// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "BaseWeapon.h"
#include "Character/MCharacter.h"
#include "DamageManagerDataAsset.h"

#include "DamageManager.generated.h"

UCLASS()
class PARTYGAMEMULTIPLAYER_API ADamageManager : public AActor
{
	GENERATED_BODY()

public:
	ADamageManager() {};

	static bool DealDamageAndBuffBetweenActors(ABaseWeapon* AttackingWeapon, class AActor* DamagedActor, float DeltaTime=0.0f);
	static bool ApplyBuff(EnumAttackBuff AttackBuff, ABaseWeapon* AttackingWeapon, class AMCharacter* DamagedActor);

public:
private:
private:
};
