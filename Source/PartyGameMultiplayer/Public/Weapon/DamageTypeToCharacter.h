// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DamageType.h"
#include "DamageTypeToCharacter.generated.h"

/**
 * 
 */
UCLASS()
class PARTYGAMEMULTIPLAYER_API UDamageTypeToCharacter : public UDamageType
{
	GENERATED_BODY()

public:
	UDamageTypeToCharacter();
	UDamageTypeToCharacter(FString i_TestString);
protected:
private:

public:
	FString TestString;
protected:
private:
};
