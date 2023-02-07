// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DamageManagerDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class PARTYGAMEMULTIPLAYER_API UDamageManagerDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UDamageManagerDataAsset()
	{
		TestDamageNumber = 0.0f;
	}

	UPROPERTY(EditAnywhere, Category = "Damage Manager")
	float TestDamageNumber;
};
