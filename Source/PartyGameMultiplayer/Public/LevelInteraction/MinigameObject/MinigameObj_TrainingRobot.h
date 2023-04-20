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
	// Sets default values for this character's properties
	AMinigameObj_TrainingRobot(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Property replication */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	/** Event for taking damage. Overridden from APawn.*/
	UFUNCTION(BlueprintCallable, Category = "Health")
	float TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	
	/**	Update HealthBar_Enemy UI for character */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetHealthBarUI();

	void Server_WhenDead();

protected:
	
	virtual void OnRep_CurrentHealth() override;
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* FollowWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UStaticMeshComponent* RootMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class USkeletalMeshComponent* RobotMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UStaticMeshComponent* CrabCenterMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UStaticMeshComponent* CollisionMesh;
};
