// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShellSpotLight.generated.h"

UCLASS()
class PARTYGAMEMULTIPLAYER_API AShellSpotLight : public AActor
{
	GENERATED_BODY()
	
public:	
	AShellSpotLight();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;	

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UStaticMeshComponent* SpotLightMesh;

	FVector SourceLocation;
	AActor* TargetActor;
};
