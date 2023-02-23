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
	//virtual void OnRep_bAttackOn() override;
	UFUNCTION()
		virtual void OnRep_ForkWorldTransform();
	virtual void OnAttackOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
		class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:

// MEMBER VARIABLES
public:
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		class UStaticMeshComponent* TaserForkMesh;
	bool bShouldStretchOut;
	UPROPERTY(EditAnywhere, Category = "TaserTmp")
	float originalX;
	UPROPERTY(EditAnywhere, Category = "TaserTmp")
	float maxLen;
	UPROPERTY(EditAnywhere, Category = "TaserTmp")
	float strechOutSpeed;
	UPROPERTY(EditAnywhere, Category = "TaserTmp")
	float strechInSpeed;

	UPROPERTY(ReplicatedUsing = OnRep_ForkWorldTransform)
		FVector ForkWorldLocation;
	UPROPERTY(ReplicatedUsing = OnRep_ForkWorldTransform)
		FRotator ForkWorldRotation;
	FVector ForkWorldLocation_WhenFirstHitTarget;
	FRotator ForkWorldRotation_WhenFirstHitTarget;

	bool bHitTarget;

private:

};
