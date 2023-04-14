// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "LevelInteraction/MinigameMainObjective.h"
#include "MinigameChild/MinigameChild_Statue_Shell.h"
#include "MinigameObj_Statue.generated.h"

/**
 * 
 */
UCLASS()
class PARTYGAMEMULTIPLAYER_API AMinigameObj_Statue : public AMinigameMainObjective
{
	GENERATED_BODY()

public:
	AMinigameObj_Statue();

	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	USkeletalMeshComponent* GetSkeletalMesh();

	UFUNCTION(BlueprintImplementableEvent)
	void OnStatueFinishedEvent();

	void NewShellHasBeenInserted();

	void Server_WhenDead();

protected:
	// only is called on server
	UFUNCTION(Category = "Weapon")
		virtual void OnShellOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION(Category = "Weapon")
		virtual void OnShellOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	UFUNCTION()
		virtual void OnGodRayOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	virtual void OnRep_CurrentHealth() override;

public:
	// UI
	// =============================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class UWidgetComponent* FollowWidget;

	// Vfx
	// =========================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
		class UNiagaraComponent* Explode_NC;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
		class UNiagaraComponent* Landing_NC;

	bool IsDropping;
	float DroppingTargetHeight;
	float DroppingSpeed;

	float ShellOverlapComponent_TargetScale;
	float ShellOverlapComponent_MinScale;
	float ShellOverlapComponent_MaxScale;

	// Death related
	// ===========================
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float ExplodeDelay;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float LittleCrabDelay;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float RespawnDelay;

protected:
	UPROPERTY(EditAnywhere, Category = "Components")
		class UStaticMeshComponent* RootMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		class USphereComponent* ShellOverlapComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
		class UNiagaraComponent* EffectDarkBubbleOn;
	UPROPERTY(EditAnywhere, Category = "Components")
		class USkeletalMeshComponent* SkeletalMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
		class UStaticMeshComponent* GodRayMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
		class UStaticMeshComponent* CrabCenterMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShellMeshRef")
		class TSubclassOf<AMinigameChild_Statue_Shell> ShellMeshRef;
	
	UPROPERTY()
	int CurrentSocketIndex;
	UPROPERTY()
	bool IsCompleteBuild = false;
};
