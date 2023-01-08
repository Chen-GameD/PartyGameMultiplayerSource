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

	UFUNCTION(BlueprintCallable, Category = "Health")
		float TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintImplementableEvent)
		void EnableBlowUpGeometryCacheComponent();

protected:
	virtual void BeginPlay() override;
	UFUNCTION()
		void OnRep_CurrentHealth();

	UFUNCTION()
	void StartToRespawnActor();

	
private:

public:
	UFUNCTION(BlueprintCallable, Category = "Health")
		float GetCurrentHealth() const { return CurrentHealth; }
	UFUNCTION(BlueprintCallable, Category = "Health")
		float GetMaxHealth() const { return MaxHealth; }
	UFUNCTION(BlueprintCallable, Category = "Health")
		float GetRemainHealthPercentage() const { return CurrentHealth/MaxHealth; }

protected:
	UPROPERTY(EditAnywhere, Category = "Health")
		float MaxHealth;
	UPROPERTY(EditAnywhere, Category = "Health", ReplicatedUsing = OnRep_CurrentHealth)
		float CurrentHealth;

	/**	Health Bar UI widget */
	UPROPERTY(VisibleAnywhere)
		class UWidgetComponent* HealthWidget;

	UPROPERTY(EditAnywhere, Category = "Components")
		class UStaticMeshComponent* RootMesh;
	UPROPERTY(EditAnywhere, Category = "Components")
		class USkeletalMeshComponent* SkeletalMesh;
	//UPROPERTY(EditAnywhere, Category = "Components")
	//	class UGeometryCacheComponent* BlowUpGeometryCacheComponent;
	UPROPERTY(EditAnywhere, Category = "Effects")
		class UNiagaraComponent* BlowUpEffect;

	//UPROPERTY(EditAnywhere, Category = "Components")
	//TArray<class TriggerBoxDamageTaker*> ArrayUStaticMeshComponent;
private:

}; 
