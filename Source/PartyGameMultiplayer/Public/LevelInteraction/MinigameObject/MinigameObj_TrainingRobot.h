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
	UFUNCTION(Server, Reliable)
	void Server_WhenDead();
	
	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetCurrentHealth(float healthValue);

	// Buff
	// ===================================================================
	bool CheckBuffMap(EnumAttackBuff AttackBuff);
	void ActByBuff_PerTick(float DeltaTime);

	// Movement
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateRobotMovement(FVector i_MoveDir, bool isShock);

protected:
	virtual void BeginPlay() override;
	virtual void OnRep_CurrentHealth() override;

public:
	// Buff
	// ===================================================================
	TMap<EnumAttackBuff, TArray<float>> BuffMap;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
		class UNiagaraComponent* EffectBurn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* FollowWidget;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UStaticMeshComponent* RootMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class USkeletalMeshComponent* RobotMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UStaticMeshComponent* RobotCenterMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UStaticMeshComponent* CollisionMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RespawnDelay;

	// Animation
	UPROPERTY(BlueprintReadOnly)
	bool IsDead;

	UPROPERTY(BlueprintReadOnly)
	bool IsShock;
};
