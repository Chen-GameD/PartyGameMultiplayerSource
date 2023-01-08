// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/DamageTypeToCharacter.h"

UDamageTypeToCharacter::UDamageTypeToCharacter()
{
	TestString = FString("UDamageTypeToCharacter's TestString");
}

UDamageTypeToCharacter::UDamageTypeToCharacter(FString i_TestString)
{
	TestString = i_TestString;
}