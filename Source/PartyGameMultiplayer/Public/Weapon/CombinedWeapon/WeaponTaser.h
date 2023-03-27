// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/BaseWeapon.h"
#include "WeaponTaser.generated.h"

UCLASS()
class PARTYGAMEMULTIPLAYER_API AWeaponTaser : public ABaseWeapon
{
	GENERATED_BODY()

// MEMBER METHODS
public:
	AWeaponTaser();

	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void AttackStart(float AttackTargetDistance) override;
	virtual void AttackStop() override;

protected:
	virtual void BeginPlay() override;

	virtual void OnRep_bAttackOn() override;
	virtual void OnAttackOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
		class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;
	virtual void OnAttackOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
		class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

	UFUNCTION()
		virtual void OnRep_ServerForkWorldTransform();
	UFUNCTION()
		virtual void OnRep_IsForkOut();

	void SetTaserForkAttached(bool bShouldAttachToWeapon);

private:

// MEMBER VARIABLES
public:
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		class UStaticMeshComponent* TaserForkMesh;
	UPROPERTY(EditAnywhere, Category = "Effects")
		class UNiagaraComponent* AttackOnEffect_TaserFork;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
		class UNiagaraComponent* ElecWire_NSComponent;

	// TaserFork original transform
	FVector TaserFork_OriginalRelativeLocation;
	FRotator TaserFork_OriginalRelativeRotation;
	FVector TaserFork_OriginalRelativeScale;
	float Ratio_ScaleUpOnRelativeScale;
	FVector TaserFork_WorldLocation_WhenAttackStop;
	FRotator TaserFork_WorldRotation_WhenAttackStop;
	FVector TaserFork_WorldLocation_WhenAttackStart;
	FRotator TaserFork_WorldRotation_WhenAttackStart;
	//FVector TaserFork_OriginalRelativeScale;

	// For stretch behaviors
	UPROPERTY(EditAnywhere, Category = "TaserFork_Stretch")
	float MaxLen;
	UPROPERTY(EditAnywhere, Category = "TaserFork_Stretch")
	float StrechOutSpeed;
	UPROPERTY(ReplicatedUsing = OnRep_IsForkOut)
		bool IsForkOut;

	// hit and attached
	UPROPERTY(Replicated)
		bool bHitTarget;
	bool bForkAttachedToWeapon; // used by both clients and server
	AActor* Server_ActorBeingHit;
	FVector Server_ActorBeingHit_To_TaserFork_WhenHit;
	FVector Server_ActorBeingHit_To_WeaponMesh_WhenHit;
	float Server_TaserForkRotationYaw_WhenHit;

	// transfrom related to hit target
	UPROPERTY(ReplicatedUsing = OnRep_ServerForkWorldTransform)
		FVector ServerForkWorldLocation;
	UPROPERTY(ReplicatedUsing = OnRep_ServerForkWorldTransform)
		FRotator ServerForkWorldRotation;

private:

};
