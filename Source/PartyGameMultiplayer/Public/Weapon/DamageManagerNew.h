// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseWeapon.h"
#include "Character/MCharacter.h"
#include "DamageManagerDataAsset.h"
#include "DamageManagerNew.generated.h"

UCLASS()
class PARTYGAMEMULTIPLAYER_API ADamageManagerNew : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADamageManagerNew();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	static bool DealDamageAndBuffBetweenActors(ABaseWeapon* AttackingWeapon, class AActor* DamagedActor);
	static bool ApplyBuff(EnumAttackBuff AttackBuff, ABaseWeapon* AttackingWeapon, class AMCharacter* DamagedActor);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	static UDamageManagerDataAsset* DamageManagerDataAsset;
private:
private:	
};
