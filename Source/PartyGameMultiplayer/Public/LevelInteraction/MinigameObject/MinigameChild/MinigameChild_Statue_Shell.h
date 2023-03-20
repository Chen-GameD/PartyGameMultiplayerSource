// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelInteraction/MinigameMainObjective.h"
#include "MinigameChild_Statue_Shell.generated.h"

UCLASS()
class PARTYGAMEMULTIPLAYER_API AMinigameChild_Statue_Shell : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMinigameChild_Statue_Shell();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(Replicated)
	FName TargetSocketName = "None";

	UPROPERTY(Replicated)
	AMinigameMainObjective* TartgetStatue;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
	UStaticMeshComponent* ShellMeshComponent;

private:
	float TimeElapsed = 0;
	float LerpDuration = 5;

};
