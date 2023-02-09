// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseProjectile.generated.h"

UCLASS()
class PARTYGAMEMULTIPLAYER_API ABaseProjectile : public AActor
{
	GENERATED_BODY()

// MEMBER METHODS
public:
	ABaseProjectile();

protected:
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

	// Server specific
	UFUNCTION(Category = "Projectile")
		virtual void OnProjectileOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
			class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
private:

// MEMBER VARIABLES
public:
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		class UStaticMeshComponent* StaticMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		class UProjectileMovementComponent* ProjectileMovementComponent;
	UPROPERTY(EditAnywhere, Category = "Effects")
		class UParticleSystem* AttackHitEffect;
private:

};
