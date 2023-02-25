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
	virtual void AttackStart() override;
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
		virtual void OnRep_bHitTarget();

	void SetTaserForkAttached(bool bShouldAttachToWeapon);

private:

// MEMBER VARIABLES
public:
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		class UStaticMeshComponent* TaserForkMesh;
	UPROPERTY(EditAnywhere, Category = "Effects")
		class UNiagaraComponent* AttackOnEffect_TaserFork;

	// TaserFork original transform
	FVector TaserFork_OriginalRelativeLocation;
	FRotator TaserFork_OriginalRelativeRotation;
	FVector TaserFork_RelativeLocation_WhenAttackStop;
	FRotator TaserFork_RelativeRotation_WhenAttackStop;
	//FVector TaserFork_OriginalRelativeScale;

	// For stretch behaviors
	UPROPERTY(EditAnywhere, Category = "TaserFork_Stretch")
	float MaxLen;
	UPROPERTY(EditAnywhere, Category = "TaserFork_Stretch")
	float StrechOutSpeed;
	UPROPERTY(EditAnywhere, Category = "TaserFork_Stretch")
	float StrechInSpeed;
	bool IsForkOut;
	float StrechInTime;

	// hit and attached
	UPROPERTY(ReplicatedUsing = OnRep_bHitTarget)
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
