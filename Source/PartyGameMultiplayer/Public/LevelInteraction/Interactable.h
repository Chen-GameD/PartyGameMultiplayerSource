// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.generated.h"

// This class is useless now, the same as AActor

UCLASS()
class PARTYGAMEMULTIPLAYER_API AInteractable : public AActor
{
	GENERATED_BODY()
	
public:	
	AInteractable();
	virtual void Tick(float DeltaTime) override;

	//virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

	/*UFUNCTION()*/
		//virtual void OnRep_Transform();
private:
	

public:
protected:
	//UPROPERTY(ReplicatedUsing = OnRep_Transform)
	//	FVector RootLocation;
	//UPROPERTY(ReplicatedUsing = OnRep_Transform)
	//	FRotator RootRotation;
	//UPROPERTY(ReplicatedUsing = OnRep_Transform)
	//	FVector RootScale;
private:

};
