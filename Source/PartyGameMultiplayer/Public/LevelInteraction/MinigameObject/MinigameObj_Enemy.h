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

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	void Server_WhenDead();

	UFUNCTION(NetMulticast, Reliable)
		void NetMulticast_ShowNoDamageHint(AController* pController, FVector HitLocation);
	UFUNCTION(NetMulticast, Reliable)
		void NetMulticast_CallGetCorrectHitSfx();

	UFUNCTION(BlueprintImplementableEvent)
		void CallGetCorrectHitSfx();
	UFUNCTION(BlueprintImplementableEvent)
		void CallGetIncorrectHitSfx();

	UFUNCTION(BlueprintImplementableEvent)
	void BPF_BroadcastCrabAnimation();

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
		class UStaticMeshComponent* CrabCenterMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UStaticMeshComponent* CollisionMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UStaticMeshComponent* BigWeaponMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UStaticMeshComponent* LittleCrabMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<class ABaseWeapon> SpecificWeaponClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<class AActor> SpecificNoDamageHintActorClass;
	// Deprecated
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<class AActor> SpecificLittleCrabClass;

	// For animation
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated)
		bool isAttacked = false;

	float Server_CallGetHitEffects_MinInterval;
	float Server_LastTime_CallGetHitEffects;

	// Vfx
	// =========================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
		class UNiagaraComponent* Explode_NC;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
		class UNiagaraComponent* Rising_NC;
	float GetHitAnimMinLastingTime;

	// UI
	// =============================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class UWidgetComponent* FollowWidget;
	float Local_ShowNoDamageHint_Interval;
	float Local_ShowNoDamageHint_LastTime;

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
