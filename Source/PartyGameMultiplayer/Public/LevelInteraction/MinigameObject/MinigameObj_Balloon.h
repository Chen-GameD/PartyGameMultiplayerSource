// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LevelInteraction/MinigameMainObjective.h"
#include "MinigameObj_Balloon.generated.h"

/**
 * 
 */
UCLASS()
class PARTYGAMEMULTIPLAYER_API AMinigameObj_Balloon : public AMinigameMainObjective
{
	GENERATED_BODY()

public:
	AMinigameObj_Balloon();
	
	UFUNCTION(BlueprintImplementableEvent)
	void EnableBlowUpGeometryCacheComponent();

	virtual float TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

protected:
	virtual void BeginPlay() override;
	virtual void OnRep_CurrentHealth() override;

	
protected:
	UPROPERTY(EditAnywhere, Category = "Components")
	class UStaticMeshComponent* RootMesh;
	UPROPERTY(EditAnywhere, Category = "Components")
	class USkeletalMeshComponent* SkeletalMesh;
	UPROPERTY(EditAnywhere, Category = "Effects")
	class UNiagaraComponent* BlowUpEffect;
};
