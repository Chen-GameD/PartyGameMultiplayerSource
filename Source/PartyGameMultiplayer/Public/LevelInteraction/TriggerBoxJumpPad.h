// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "TriggerBoxJumpPad.generated.h"

/**
 * 
 */
UCLASS()
class PARTYGAMEMULTIPLAYER_API ATriggerBoxJumpPad : public ATriggerBox
{
	GENERATED_BODY()
public:
	ATriggerBoxJumpPad();

	UFUNCTION()
		void OnJumpPadOverlapBegin(class AActor* OverlappedActor, class AActor* OtherActor);
	UFUNCTION()
		void OnJumpPadOverlapEnd(class AActor* OverlappedActor, class AActor* OtherActor);
protected:
	virtual void BeginPlay() override;
private:



public:
	UPROPERTY(EditAnywhere, Category = "JumpPad")
	FVector LaunchVelocity;
	UPROPERTY(EditAnywhere, Category = "JumpPad")
	bool bXYOverride;
	UPROPERTY(EditAnywhere, Category = "JumpPad")
	bool bZOverride;
protected:
private:
	
};
