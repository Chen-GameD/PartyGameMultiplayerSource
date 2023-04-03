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

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	USkeletalMeshComponent* GetSkeletalMesh();

	UFUNCTION(BlueprintImplementableEvent)
	void OnStatueFinishedEvent();

protected:
	// only is called on server
	UFUNCTION(Category = "Weapon")
	virtual void OnShellOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION(Category = "Weapon")
	virtual void OnShellOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	virtual void OnRep_CurrentHealth() override;

public:
	// UI
	// =============================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class UWidgetComponent* FollowWidget;

protected:
	UPROPERTY(EditAnywhere, Category = "Components")
	UStaticMeshComponent* RootMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USphereComponent* ShellOverlapComponent;
	UPROPERTY(EditAnywhere, Category = "Components")
	USkeletalMeshComponent* SkeletalMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShellMeshRef")
	TSubclassOf<AMinigameChild_Statue_Shell> ShellMeshRef;
	UPROPERTY()
	int CurrentSocketIndex;
	UPROPERTY()
	bool IsCompleteBuild = false;
};
