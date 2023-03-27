// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CursorHitPlane.generated.h"

UCLASS()
class PARTYGAMEMULTIPLAYER_API ACursorHitPlane : public AActor
{
	GENERATED_BODY()
	
public:	
	ACursorHitPlane();
	virtual void Tick(float DeltaTime) override;
	
protected:
	virtual void BeginPlay() override;
		

public:
	class AMCharacter* pMCharacter;
protected:	
	UPROPERTY(EditAnywhere)
		class UStaticMeshComponent* CollisionPlane;
};
