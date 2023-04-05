// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon/DamageManager.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

#include "Weapon/BaseWeapon.h"
#include "Weapon/BaseProjectile.h"
#include "Weapon/BaseDamageCauser.h"
#include "Weapon/CombinedWeapon/WeaponBomb.h"
//#include "Weapon/JsonFactory.h"
#include "Weapon/WeaponDataHelper.h"
#include "Weapon/DamageType/MeleeDamageType.h"
#include "Character/MCharacter.h"
#include "Character/MPlayerController.h"
#include "LevelInteraction/MinigameObject/MinigameObj_Enemy.h"


bool ADamageManager::TryApplyDamageToAnActor(AActor* DamageCauser, AController* Controller, TSubclassOf<UDamageType> DamageTypeClass, AActor* DamagedActor, float DeltaTime)
{
	if (!DamageCauser || !DamagedActor || !AWeaponDataHelper::DamageManagerDataAsset)
		return false;

	EnumWeaponType WeaponType = EnumWeaponType::None;
	bool IsBigWeapon = false;
	if (auto p = Cast<ABaseWeapon>(DamageCauser))
	{
		WeaponType = p->WeaponType;
		IsBigWeapon = p->IsBigWeapon;
	}
	if (auto p = Cast<ABaseProjectile>(DamageCauser))
	{
		WeaponType = p->WeaponType;
		IsBigWeapon = p->IsBigWeapon;
	}
	if (WeaponType == EnumWeaponType::None)
		return false;

	if (Cast<AMCharacter>(DamagedActor) || Cast<AMinigameObj_Enemy>(DamagedActor))
	{
		float Damage = 0.0f;
		FString WeaponName = AWeaponDataHelper::WeaponEnumToString_Map[WeaponType];
		if (IsBigWeapon)
			WeaponName = "Big" + WeaponName;
		if (AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map.Contains(WeaponName))
			Damage = AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map[WeaponName];
		if (0 < DeltaTime)
			Damage *= DeltaTime;
		// Special situation: Flamefork
		if (WeaponType == EnumWeaponType::Flamefork)
		{
			if (DamageTypeClass == UMeleeDamageType::StaticClass())
			{
				if (AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map.Contains("Flamefork_Melee"))
					Damage = AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map["Flamefork_Melee"];
			}
			else
			{
				if (AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map.Contains("Flamefork_Wave"))
					Damage = AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map["Flamefork_Wave"];
			}			
		}
		// Special situation: Bomb's fork, Taser's fork
		if( (WeaponType == EnumWeaponType::Bomb && DamageTypeClass == UMeleeDamageType::StaticClass()) || 
			WeaponType == EnumWeaponType::Taser && DamageTypeClass == UMeleeDamageType::StaticClass() )
		{
			if (AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map.Contains("Fork"))
				Damage = AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map["Fork"];
		}
		UGameplayStatics::ApplyDamage(DamagedActor, Damage, Controller, DamageCauser, DamageTypeClass);
	}
	else
	{
		return false;
	}
	return true;
}

//bool ADamageManager::TryApplyRadialDamage(AActor* DamageCauser, AController* Controller, FVector Origin, float DamageRadius, float BaseDamage)
//{
//	return TryApplyRadialDamage(DamageCauser, Controller, Origin, 0.0f, DamageRadius, BaseDamage);
//}

bool ADamageManager::TryApplyRadialDamage(AActor* DamageCauser, AController* Controller, FVector Origin, float DamageInnerRadius, float DamageOuterRadius, float BaseDamage)
{
	if (!DamageCauser || !Controller)
		return false;

	EnumWeaponType WeaponType = EnumWeaponType::None;
	if (auto p = Cast<ABaseWeapon>(DamageCauser))
		WeaponType = p->WeaponType;
	if (auto p = Cast<ABaseProjectile>(DamageCauser))
		WeaponType = p->WeaponType;
	if (WeaponType == EnumWeaponType::None)
		return false;

	TArray<AActor*> IgnoredActors;
	auto AttackingPlayer = Controller->GetPawn();
	if (AttackingPlayer)
		IgnoredActors.Add(AttackingPlayer);

	bool bDoFullDamage = true;
	float DamageFalloff = bDoFullDamage ? 0.f : 1.f;

	UGameplayStatics::ApplyRadialDamageWithFalloff(
		Controller,   //const UObject* WorldContextObject
		BaseDamage,
		0.0f,			// float MinDamage
		Origin,
		DamageInnerRadius,			// float DamageInnerRadius
		DamageOuterRadius,
		DamageFalloff,
		UDamageType::StaticClass(),  //TSubclassOf<UDamageType> DamageTypeClass
		IgnoredActors,				//const TArray<AActor*>& IgnoreActors
		DamageCauser,		//AActor* DamageCauser
		Controller,				 //AController* InstigatedByController
		ECollisionChannel::ECC_MAX	  // DamagePreventionChannel
	);

	return true;
}

bool ADamageManager::AddBuffPoints(EnumWeaponType WeaponType, EnumAttackBuff AttackBuff, AController* AttackerController, class AMCharacter* DamagedCharacter, float buffPointsToAdd)
{
	if (!DamagedCharacter || !DamagedCharacter->CheckBuffMap(AttackBuff) || DamagedCharacter->IsInvincible || !AttackerController || !AWeaponDataHelper::DamageManagerDataAsset)
		return false;

	float& BuffPoints = DamagedCharacter->BuffMap[AttackBuff][0];
	float& BuffRemainedTime = DamagedCharacter->BuffMap[AttackBuff][1];
	float& BuffAccumulatedTime = DamagedCharacter->BuffMap[AttackBuff][2];

	float OldBuffPoints = BuffPoints;
	BuffPoints += buffPointsToAdd;
	if (AttackBuff == EnumAttackBuff::Saltcure || AttackBuff == EnumAttackBuff::Shellheal)
		BuffPoints = FMath::Clamp(BuffPoints, 0.0f, 1.0f);

	if (OldBuffPoints < 1.0f && 1.0f <= BuffPoints)
	{
		if (AttackBuff == EnumAttackBuff::Paralysis)
		{
			// Set Character's IsParalyzed to true
			DamagedCharacter->IsParalyzed = true;
			if (DamagedCharacter->GetNetMode() == NM_ListenServer)
				DamagedCharacter->OnRep_IsParalyzed();
		}
		else if (AttackBuff == EnumAttackBuff::Saltcure)
		{			
			// Set Character's IsHealing to true
			DamagedCharacter->IsHealing = true;
			if (DamagedCharacter->GetNetMode() == NM_ListenServer)
				DamagedCharacter->OnRep_IsHealing();
		}
		else if (AttackBuff == EnumAttackBuff::Shellheal)
		{
			// Set Character's IsHealing to true
			DamagedCharacter->IsHealing = true;
			if (DamagedCharacter->GetNetMode() == NM_ListenServer)
				DamagedCharacter->OnRep_IsHealing();
		}
	}

	// When the Burning BuffPoints increases to the next integer(1, 2, 3, 4 ...), with add buff time and etc.
	if (AttackBuff == EnumAttackBuff::Burning)
	{
		if ( !DamagedCharacter->IsHealing && 0.99f <= (FMath::FloorToInt(BuffPoints) - FMath::FloorToInt(OldBuffPoints)))
		{
			// Burning_TimeToAdd
			FString ParName = AWeaponDataHelper::AttackBuffEnumToString_Map[AttackBuff] + "_TimeToAdd";
			if (AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map.Contains(ParName))
			{
				float buffTimeToAdd = AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map[ParName];
				BuffRemainedTime += buffTimeToAdd;
				BuffAccumulatedTime += buffTimeToAdd;
			}
			// Set Character's IsBurned to true
			DamagedCharacter->IsBurned = true;
			if (DamagedCharacter->GetNetMode() == NM_ListenServer)
				DamagedCharacter->OnRep_IsBurned();
			// Set Server_TheControllerApplyingLatestBurningBuff 
			DamagedCharacter->Server_TheControllerApplyingLatestBurningBuff = AttackerController;
		}		
	}
	
	return true;
}

bool ADamageManager::ApplyOneTimeBuff(EnumWeaponType WeaponType, EnumAttackBuff AttackBuff, AController* AttackerController, class AMCharacter* DamagedCharacter, float DeltaTime)
{	
	if (!DamagedCharacter || !DamagedCharacter->CheckBuffMap(AttackBuff) || DamagedCharacter->IsInvincible || !AttackerController || !AWeaponDataHelper::DamageManagerDataAsset)
		return false;

	/* Knockback */
	if (AttackBuff == EnumAttackBuff::Knockback && AttackerController->GetPawn())
	{
		FVector AttackingDirection = AttackerController->GetPawn()->GetControlRotation().RotateVector(FVector3d::ForwardVector);
		AttackingDirection.Normalize();
		DamagedCharacter->KnockbackDirection_SinceLastApplyBuff = AttackingDirection;
		float Knockback_MoveSpeed = 0.0f;
		FString ParName = "";
		ParName = "Knockback_MoveSpeed";
		if (AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map.Contains(ParName))
			Knockback_MoveSpeed = AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map[ParName];
		ParName = AWeaponDataHelper::WeaponEnumToString_Map[WeaponType] + "_Knockback_MoveSpeed";
		if (AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map.Contains(ParName))
			Knockback_MoveSpeed = AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map[ParName];
		AttackingDirection.Z = 0.0f;
		AttackingDirection *= Knockback_MoveSpeed;
		DamagedCharacter->LaunchCharacter(AttackingDirection, true, false);
	}
	/* Paralysis */
	else if (AttackBuff == EnumAttackBuff::Paralysis && AttackerController->GetPawn())
	{
		FVector Direction_TargetToAttacker = AttackerController->GetPawn()->GetActorLocation() - DamagedCharacter->GetActorLocation();
		if (100.0f < Direction_TargetToAttacker.Length())
		{
			Direction_TargetToAttacker.Normalize();
			float DragSpeedRatio = 0.0f;
			FString ParName = "Paralysis_DragSpeedRatio";
			if (AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map.Contains(ParName))
				DragSpeedRatio = AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map[ParName];

			//DamagedCharacter->SetActorLocation(DamagedCharacter->GetActorLocation() + Direction_TargetToAttacker * DragSpeed * DeltaTime);
			DamagedCharacter->Client_MoveCharacter(Direction_TargetToAttacker, DragSpeedRatio);
		}
	}
	return true;
}


