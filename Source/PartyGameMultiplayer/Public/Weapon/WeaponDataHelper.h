// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "DamageManagerDataAsset.h"

#include "WeaponDataHelper.generated.h"

UCLASS()
class PARTYGAMEMULTIPLAYER_API AWeaponDataHelper : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeaponDataHelper();

public:
	static FString RandomFileName;  // useless as of now
	static UDamageManagerDataAsset* DamageManagerDataAsset;

};
