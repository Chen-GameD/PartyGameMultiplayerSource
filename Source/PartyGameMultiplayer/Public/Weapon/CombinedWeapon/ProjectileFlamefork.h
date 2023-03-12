// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/BaseProjectile.h"
#include "Engine/Engine.h"
#include "ProjectileFlamefork.generated.h"


UCLASS()
class PARTYGAMEMULTIPLAYER_API AProjectileFlamefork : public ABaseProjectile
{
	GENERATED_BODY()
public:
	AProjectileFlamefork();
protected:
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

	UFUNCTION(NetMulticast, Reliable)
	void SpawnWaveNS(FVector SpawnLocation, FRotator SpawnRotation);
	void OnProjectileOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
		class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
private:

public:
protected:
	UPROPERTY(EditAnywhere, Category = "Effects")
		class UNiagaraComponent* Wave_NSComponent;
	UPROPERTY(EditAnywhere, Category = "Effects")
		class UNiagaraSystem* Wave_NSSystem;
private:
	
};
