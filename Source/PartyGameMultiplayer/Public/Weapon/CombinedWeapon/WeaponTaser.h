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

	// should only be called on server
	virtual void AttackStart() override;
	// should only be called on server
	virtual void AttackStop() override;

protected:
	virtual void CheckInitilization() override;
	/*virtual void GenerateAttackHitEffect() override;*/
	//virtual void GenerateDamage(class AActor* DamagedActor) override;
	virtual void OnRep_bAttackOn() override;

private:

// MEMBER VARIABLES
public:
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		class UStaticMeshComponent* TaserForkMesh;
	bool bStretching;
	UPROPERTY(EditAnywhere, Category = "TaserTmp")
	float originalX;
	UPROPERTY(EditAnywhere, Category = "TaserTmp")
	float maxLen;
	UPROPERTY(EditAnywhere, Category = "TaserTmp")
	float strechOutSpeed;
	UPROPERTY(EditAnywhere, Category = "TaserTmp")
	float strechInSpeed;
private:

};
