// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon/DamageManager.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

#include "Weapon/BaseWeapon.h"
#include "Weapon/BaseProjectile.h"
#include "Weapon/BaseDamageCauser.h"
#include "Weapon/ElementWeapon/WeaponFork.h"
#include "Weapon/ElementWeapon/WeaponLighter.h"
#include "Weapon/ElementWeapon/WeaponBlower.h"
#include "Weapon/ElementWeapon/WeaponAlarm.h"
#include "Weapon/CombinedWeapon/WeaponBomb.h"
//#include "Weapon/JsonFactory.h"
#include "Weapon/WeaponDataHelper.h"
#include "Weapon/DamageType/MeleeDamageType.h"
#include "Character/MCharacter.h"
#include "Character/MPlayerController.h"
#include "M_PlayerState.h"
#include "LevelInteraction/MinigameObject/MinigameObj_Enemy.h"


bool ADamageManager::TryApplyDamageToAnActor(AActor* DamageCauser, AController* Controller, TSubclassOf<UDamageType> DamageTypeClass, AActor* DamagedActor, float DeltaTime)
{
	if (!DamageCauser || !DamagedActor || !AWeaponDataHelper::DamageManagerDataAsset)
		return false;

	EnumWeaponType WeaponType = EnumWeaponType::None;
	bool IsBigWeapon = false;
	AController* pController = nullptr;
	if (auto p = Cast<ABaseWeapon>(DamageCauser))
	{
		WeaponType = p->WeaponType;
		IsBigWeapon = p->IsBigWeapon;
		pController = p->GetHoldingController();
	}
	if (auto p = Cast<ABaseProjectile>(DamageCauser))
	{
		WeaponType = p->WeaponType;
		IsBigWeapon = p->IsBigWeapon;
		pController = p->Controller;
	}
	if (WeaponType == EnumWeaponType::None)
		return false;

	if (Cast<AMCharacter>(DamagedActor) || Cast<AMinigameObj_Enemy>(DamagedActor))
	{
		// If need to show "No Damage" when hitting the enemy crab
		if (auto pMinigameEnemy = Cast<AMinigameObj_Enemy>(DamagedActor))
		{
			if (!ADamageManager::CanApplyDamageToEnemyCrab(pMinigameEnemy->SpecificWeaponClass, WeaponType))
			{
				pMinigameEnemy->NetMulticast_ShowNoDamageHint(pController, 0.5f * (pMinigameEnemy->CrabCenterMesh->GetComponentLocation() + DamageCauser->GetActorLocation()));
				return 0.0f;
			}			
		}

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
		if(WeaponType == EnumWeaponType::Bomb && DamageTypeClass == UMeleeDamageType::StaticClass())
		{
			if (AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map.Contains("Bomb_Fork"))
				Damage = AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map["Bomb_Fork"];
		}
		if (WeaponType == EnumWeaponType::Taser && DamageTypeClass == UMeleeDamageType::StaticClass())
		{
			if (AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map.Contains("Taser_Fork"))
				Damage = AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map["Taser_Fork"];
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

	// if it is the teammate
	if (AttackBuff == EnumAttackBuff::Burning || AttackBuff == EnumAttackBuff::Paralysis)
	{
		int TeammateCheckResult = ADamageManager::IsTeammate(AttackerController, DamagedCharacter);
		if (TeammateCheckResult != 0)
			return false;
	}

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

	// if it is the teammate
	int TeammateCheckResult = IsTeammate(AttackerController, DamagedCharacter->GetController());
	if (TeammateCheckResult != 0)
		return false;		

	/* Knockback */
	if (AttackBuff == EnumAttackBuff::Knockback && AttackerController->GetPawn())
	{
		FVector AttackingDirection = AttackerController->GetPawn()->GetControlRotation().RotateVector(FVector3d::ForwardVector);
		AttackingDirection.Z = 0.0f;
		AttackingDirection.Normalize();
		DamagedCharacter->KnockbackDirection_SinceLastApplyBuff = AttackingDirection; // Deprecated
		float Knockback_MoveSpeed = 0.0f;
		FString ParName = "";
		ParName = "Knockback_MoveSpeed";
		if (AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map.Contains(ParName))
			Knockback_MoveSpeed = AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map[ParName];
		ParName = AWeaponDataHelper::WeaponEnumToString_Map[WeaponType] + "_Knockback_MoveSpeed";
		if (AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map.Contains(ParName))
			Knockback_MoveSpeed = AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map[ParName];
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

			DamagedCharacter->Server_Direction_SelfToTaserAttacker = Direction_TargetToAttacker;
			// DamagedCharacter->LaunchCharacter(Direction_TargetToAttacker * DragSpeed * DeltaTime, true, false);
			// DamagedCharacter->SetActorLocation(DamagedCharacter->GetActorLocation() + Direction_TargetToAttacker * 100.0f * DeltaTime);
			// DamagedCharacter->Client_MoveCharacter(Direction_TargetToAttacker, DragSpeedRatio);
		}
		else
		{
			DamagedCharacter->Server_Direction_SelfToTaserAttacker = FVector::Zero();
		}
	}
	return true;
}


bool ADamageManager::CanApplyDamageToEnemyCrab(TSubclassOf<class ABaseWeapon> EnemyCrab_SpecificWeaponClass, EnumWeaponType WeaponType)
{
	bool beingAttackedByValidWeapon = true;
	if (EnemyCrab_SpecificWeaponClass->IsChildOf(AWeaponFork::StaticClass()))
	{
		if (WeaponType != EnumWeaponType::Fork && WeaponType != EnumWeaponType::Taser && WeaponType != EnumWeaponType::Flamefork && WeaponType != EnumWeaponType::Bomb)
			beingAttackedByValidWeapon = false;
	}
	else if (EnemyCrab_SpecificWeaponClass->IsChildOf(AWeaponLighter::StaticClass()))
	{
		if (WeaponType != EnumWeaponType::Lighter && WeaponType != EnumWeaponType::Flamefork && WeaponType != EnumWeaponType::Flamethrower && WeaponType != EnumWeaponType::Cannon)
			beingAttackedByValidWeapon = false;
	}
	else if (EnemyCrab_SpecificWeaponClass->IsChildOf(AWeaponBlower::StaticClass()))
	{
		if (WeaponType != EnumWeaponType::Blower && WeaponType != EnumWeaponType::Taser && WeaponType != EnumWeaponType::Flamethrower && WeaponType != EnumWeaponType::Alarmgun)
			beingAttackedByValidWeapon = false;
	}
	else if (EnemyCrab_SpecificWeaponClass->IsChildOf(AWeaponAlarm::StaticClass()))
	{
		if (WeaponType != EnumWeaponType::Alarm && WeaponType != EnumWeaponType::Bomb && WeaponType != EnumWeaponType::Cannon && WeaponType != EnumWeaponType::Alarmgun)
			beingAttackedByValidWeapon = false;
	}

	return beingAttackedByValidWeapon;
}

int ADamageManager::IsTeammate(AActor* a, AActor* b)
{
	AM_PlayerState* PS1 = nullptr;
	AM_PlayerState* PS2 = nullptr;
	if (auto pCtrl = Cast<AController>(a))
		PS1 = pCtrl->GetPlayerState<AM_PlayerState>();
	else if (auto pPawn = Cast<APawn>(a))
		PS1 = pPawn->GetPlayerState<AM_PlayerState>();
	else
		return -1;

	if (auto pCtrl = Cast<AController>(b))
		PS2 = pCtrl->GetPlayerState<AM_PlayerState>();
	else if (auto pPawn = Cast<APawn>(b))
		PS2 = pPawn->GetPlayerState<AM_PlayerState>();
	else
		return -1;

	if (!PS1 || !PS2)
		return -1;
	if (PS1->TeamIndex == PS2->TeamIndex)
		return 1;
	else
		return 0;
}
