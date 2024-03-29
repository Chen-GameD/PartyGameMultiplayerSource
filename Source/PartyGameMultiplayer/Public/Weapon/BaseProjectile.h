// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Weapon/BaseWeapon.h"

#include "BaseProjectile.generated.h"

UCLASS()
class PARTYGAMEMULTIPLAYER_API ABaseProjectile : public AActor
{
	GENERATED_BODY()

// MEMBER METHODS
public:
	ABaseProjectile();

	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(NetMulticast, Reliable)
		void NetMulticast_ChangeSpeed(float SpeedRatio);

	// Sfx
	// ==================================
	UFUNCTION(BlueprintImplementableEvent)
		void CallExplodeSfx();

protected:
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

	UFUNCTION()
		virtual void OnRep_HasExploded();

	// Server specific
	UFUNCTION(Category = "Projectile")
		virtual void OnProjectileOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
			class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:

// MEMBER VARIABLES
public:
	EnumWeaponType WeaponType;
	bool IsBigWeapon;
	class AController* Controller;
	float CurDeltaTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		class UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(EditAnywhere, Category = "Effects")
	UTexture2D* WeaponImage_Broadcast;
	UPROPERTY(EditAnywhere, Category = "Effects")
	UTexture2D* WeaponImage_Message;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		class UStaticMeshComponent* StaticMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
		class UPrimitiveComponent* AttackDetectComponent;
	UPROPERTY(EditAnywhere, Category = "Effects")
		class UNiagaraComponent* AttackHitEffect_NSComponent;
	UPROPERTY(EditAnywhere, Category = "Effects")
		class UNiagaraSystem* AttackHitEffect_NSSystem;
	UPROPERTY(EditAnywhere, Category = "Effects")
		class UNiagaraComponent* AttackOnEffect_NSComponent;

	float LiveTime;
	float MaxLiveTime;

	float TotalDamageTime;
	float TotalDamage;
	FVector Origin;
	float DamageRadius;
	bool bApplyConstantDamage;
	float BaseDamage;
	UPROPERTY(ReplicatedUsing = OnRep_HasExploded)
		bool HasExploded;
	float TimePassed_SinceLastTryApplyRadialDamage;
	UPROPERTY(Replicated)
		float TimePassed_SinceExplosion;

private:

};
