// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class PARTYGAMEMULTIPLAYER_API TestInfo
{
public:
	TestInfo();
	~TestInfo();
	static size_t CntPlayerCreated;
	static bool SomeoneHasWeapon;
};
