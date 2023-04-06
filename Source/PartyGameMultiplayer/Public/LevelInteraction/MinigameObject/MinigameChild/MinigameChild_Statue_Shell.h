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
	AMinigameChild_Statue_Shell();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintImplementableEvent)
		void CallShellInsertSfx();

protected:
	virtual void BeginPlay() override;



public:
	UPROPERTY(Replicated)
	FName TargetSocketName = "None";

	UPROPERTY(Replicated)
	AMinigameMainObjective* TartgetStatue;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
		UStaticMeshComponent* ShellMeshComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
		class UNiagaraComponent* ShellFly_NC;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
		class UNiagaraComponent* ShellInsert_NC;

private:
	float TimeElapsed = 0;
	float LerpDuration = 4;
	bool bFinishInsert;
};
