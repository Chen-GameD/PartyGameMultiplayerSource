// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LevelInteraction/Interactable.h"
#include "MinigameMainObjective.generated.h"

/**
 * This Class's only duty: Manage the get, set, sync of health-realated variables
 */
UCLASS()
class PARTYGAMEMULTIPLAYER_API AMinigameMainObjective : public AInteractable
{
	GENERATED_BODY()

public:
	AMinigameMainObjective();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetCurrentHealth() const { return CurrentHealth; }
	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetMaxHealth() const { return MaxHealth; }
	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetRemainHealthPercentage() const { return CurrentHealth/MaxHealth; }

	// Update Score
	UFUNCTION()
	void UpdateScoreCanGet(int n_Score);

protected:
	virtual void BeginPlay() override;
	UFUNCTION()
	virtual void OnRep_CurrentHealth();

	UFUNCTION()
	void StartToRespawnActor();

public:

	// Effects
	// =============================
	UPROPERTY(EditAnywhere)
		TSubclassOf<class UCameraShakeBase> CameraShakeTriggered;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FString MinigameInformation;

protected:
	UPROPERTY(EditAnywhere, Category = "Health")
	float MaxHealth;
	UPROPERTY(EditAnywhere, Category = "Health", ReplicatedUsing = OnRep_CurrentHealth)
	float CurrentHealth;
	UPROPERTY()
	int ScoreCanGet;

}; 
