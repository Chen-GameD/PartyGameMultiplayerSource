// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LevelInteraction/MinigameMainObjective.h"
#include "Weapon/WeaponDataHelper.h"
#include "MinigameObj_TrainingRobot.generated.h"

/**
 * 
 */
UCLASS()
class PARTYGAMEMULTIPLAYER_API AMinigameObj_TrainingRobot : public AMinigameMainObjective
{
	GENERATED_BODY()

public:
	AMinigameObj_TrainingRobot();

	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(BlueprintCallable, Category = "Health")
	float TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	
	UFUNCTION(BlueprintCallable, Category = "Health")
		void SetCurrentHealth(float healthValue);

	//virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//UFUNCTION(BlueprintCallable, Category = "Health")
		//void SetHealthBarUI();

	// Buff
	// ===================================================================
	bool CheckBuffMap(EnumAttackBuff AttackBuff);
	void ActByBuff_PerTick(float DeltaTime);

protected:
	//virtual void BeginPlay() override;
	//virtual void OnRep_CurrentHealth() override;

public:
	// Buff
	// ===================================================================
	TMap<EnumAttackBuff, TArray<float>> BuffMap;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
		class UNiagaraComponent* EffectBurn;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	//	class UWidgetComponent* FollowWidget;
	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//	class UStaticMeshComponent* RootMesh;
	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//	class USkeletalMeshComponent* RobotMesh;
	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//	class UStaticMeshComponent* CrabCenterMesh;
	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//	class UStaticMeshComponent* CollisionMesh;
};
