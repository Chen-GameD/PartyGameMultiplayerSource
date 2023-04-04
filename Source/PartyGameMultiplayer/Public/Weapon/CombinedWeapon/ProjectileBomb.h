// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/BaseProjectile.h"
#include "ProjectileBomb.generated.h"

/**
 * 
 */
UCLASS()
class PARTYGAMEMULTIPLAYER_API AProjectileBomb : public ABaseProjectile
{
	GENERATED_BODY()
public:
	AProjectileBomb();
	virtual void Tick(float DeltaTime) override;
protected:
	virtual void OnProjectileOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
		class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	virtual void OnRep_HasExploded() override;

protected:
	UPROPERTY(EditAnywhere, Category = "Components")
		class UStaticMeshComponent* BombMesh;

	bool HasAppliedNeedleRainDamage;
};
