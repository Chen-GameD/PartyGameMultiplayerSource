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

	/* 
		why not pass weapon pointer but AActor* DamageCauser(can be either weapon* or projectile)
		because weapon* may be destroyed or transfered to another player when projectile is apply damage
	*/
	
	// The damaged actors are determined when entering this function( however, they can be teammates )
	static bool TryApplyDamageToAnActor(AActor* DamageCauser, AController* Controller, TSubclassOf<UDamageType> DamageTypeClass, class AActor* DamagedActor, float DeltaTime);

	// The damaged actors are not determined when entering this function( has to be cacluated by UGameplayStatics::ApplyRadialDamage() )
	//static bool TryApplyRadialDamage(AActor* DamageCauser, AController* Controller, FVector Origin, float DamageRadius, float BaseDamage);
	static bool TryApplyRadialDamage(AActor* DamageCauser, AController* Controller, FVector Origin, float DamageInnerRadius, float DamageOuterRadius, float BaseDamage);
	
	// Burning, Saltcure
	static bool AddBuffPoints(EnumWeaponType WeaponType, EnumAttackBuff AttackBuff, AController* Controller, class AMCharacter* DamagedCharacter, float buffPointsToAdd);
	// Knockback, Paralysis
	static bool ApplyOneTimeBuff(EnumWeaponType WeaponType, EnumAttackBuff AttackBuff, AController* Controller, class AMCharacter* DamagedCharacter, float DeltaTime);

};
