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
	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	void Server_WhenDead();
protected:
	virtual void BeginPlay() override;
	virtual void OnRep_CurrentHealth() override;
private:
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UStaticMeshComponent* RootMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class USkeletalMeshComponent* CrabMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UStaticMeshComponent* CollisionMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UStaticMeshComponent* BigWeaponMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<class ABaseWeapon> SpecificWeaponClass;


	// Vfx
	// =========================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
		class UNiagaraComponent* Explode_NC;
	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
		class UNiagaraSystem* Explode_NS;*/

	// UI
	// =============================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class UWidgetComponent* FollowWidget;

	// Spawn related
	// ===========================
	FVector Server_SpawnBigWeaponLocation;
	FRotator Server_SpawnBigWeaponRotation;
	bool IsRisingFromSand;
	float RisingTargetHeight;
	float RisingSpeed;

	// Death related
	// ===========================
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float DropWeaponDelay;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float RespawnDelay;

protected:
private:
	
};
