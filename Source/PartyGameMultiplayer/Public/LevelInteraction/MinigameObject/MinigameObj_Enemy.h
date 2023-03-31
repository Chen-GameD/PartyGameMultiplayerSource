// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LevelInteraction/MinigameMainObjective.h"
#include "MinigameObj_Enemy.generated.h"


UCLASS()
class PARTYGAMEMULTIPLAYER_API AMinigameObj_Enemy : public AMinigameMainObjective
{
	GENERATED_BODY()

public:
	AMinigameObj_Enemy();
	virtual float TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
protected:
	virtual void BeginPlay() override;
	virtual void OnRep_CurrentHealth() override;
private:
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UStaticMeshComponent* CrabMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UStaticMeshComponent* BigWeaponMesh;
protected:
private:
	
};
