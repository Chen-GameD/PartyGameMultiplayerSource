// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/BaseProjectile.h"
#include "ProjectileAlarm.generated.h"

/**
 * 
 */
UCLASS()
class PARTYGAMEMULTIPLAYER_API AProjectileAlarm : public ABaseProjectile
{
	GENERATED_BODY()
public:
	AProjectileAlarm();	
	virtual void Tick(float DeltaTime) override;
protected:
	virtual void OnRep_HasExploded() override;

protected:
	float Shake_TimeSinceChangeDirection;
	bool Shake_Clockwise;
};
