// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LevelInteraction/Interactable.h"
#include "MinigameMainObjective.generated.h"

/**
 * 
 */
UCLASS()
class PARTYGAMEMULTIPLAYER_API AMinigameMainObjective : public AInteractable
{
	GENERATED_BODY()

public:
	AMinigameMainObjective();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	// Effects
	// =============================
	UFUNCTION(BlueprintImplementableEvent)
		void CallGetHitSfx();
	UFUNCTION(BlueprintImplementableEvent)
		void CallDeathSfx();

protected:
	virtual void BeginPlay() override;
	UFUNCTION()
	virtual void OnRep_CurrentHealth();

	UFUNCTION()
	void StartToRespawnActor();

public:
	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetCurrentHealth() const { return CurrentHealth; }
	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetMaxHealth() const { return MaxHealth; }
	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetRemainHealthPercentage() const { return CurrentHealth/MaxHealth; }

	// Update Score
	UFUNCTION()
	void UpdateScoreCanGet(int n_Score);

public:
	// Effects
	// =============================
	float CallGetHitSfxVfx_MinInterval;
	float LastTime_CallGetHitSfxVfx;
protected:
	UPROPERTY(EditAnywhere, Category = "Health")
	float MaxHealth;
	UPROPERTY(EditAnywhere, Category = "Health", ReplicatedUsing = OnRep_CurrentHealth)
	float CurrentHealth;
	UPROPERTY()
	int ScoreCanGet;

}; 
