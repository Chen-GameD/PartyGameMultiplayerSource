// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/BaseWeapon.h"
#include "LevelInteraction/ShellSpotLight.h"
#include "WeaponShell.generated.h"

/**
 * 
 */
UCLASS()
class PARTYGAMEMULTIPLAYER_API AWeaponShell : public ABaseWeapon
{
	GENERATED_BODY()

/* MEMBER METHODS */
public:
	AWeaponShell();

	virtual void GetPickedUp(ACharacter* pCharacter) override;
	virtual void GetThrewAway() override;
	virtual void AttackStart(float AttackTargetDistance) override;
	virtual void AttackStop() override;

	UFUNCTION(BlueprintCallable)
	AController* GetPreHoldingController();

	UFUNCTION()
	void UpdateScoreCanGet(int N_Score);
	UFUNCTION()
	int GetScoreCanGet();

	UFUNCTION()
	void UpdateConfigIndex(int N_Index);
	UFUNCTION()
	int GetConfigIndex();

protected:
	virtual void BeginPlay() override;
	virtual void Destroyed() override;
	virtual void OnAttackOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
		class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

public:
	bool Server_bDetectedByStatue;
	
protected:
	AController* PreHoldingController;
	UPROPERTY()
	int ConfigIndex = -1;
	UPROPERTY()
	int ScoreCanGet = 0;

	class AShellSpotLight* pShellSpotLight;
	UPROPERTY(EditAnywhere)
		TSubclassOf<class AShellSpotLight> SpecificShellSpotLightClass;
};
