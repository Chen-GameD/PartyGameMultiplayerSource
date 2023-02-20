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

	// The damaged actors are determined when entering this function( however, they can be teammates )
	static bool DealDamageAndBuffBetweenActors(ABaseWeapon* AttackingWeapon, class AActor* DamagedActor, float DeltaTime=0.0f);
	// The damaged actors are not determined when entering this function( has to be cacluated by UGameplayStatics::ApplyRadialDamage() )
	static bool ApplyRadialDamageOnce(ABaseWeapon* AttackingWeapon, FVector Origin, float DamageRadius, float BaseDamage);
	static bool TryApplyRadialDamage(ABaseWeapon* AttackingWeapon, FVector Epicenter);
	static bool ApplyBuff(ABaseWeapon* AttackingWeapon, TSubclassOf<UDamageType> DamageTypeClass, class AMCharacter* DamagedActor);

public:
	static FTimerHandle* TimerHandle_Loop;
private:
private:
};
