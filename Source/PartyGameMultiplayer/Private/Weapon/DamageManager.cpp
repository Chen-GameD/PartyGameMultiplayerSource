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
#include "LevelInteraction/MinigameMainObjective.h"
#include "LevelInteraction/MinigameObject/MinigameObj_Enemy.h"
#include "LevelInteraction/MinigameObject/MinigameObj_Statue.h"
#include "LevelInteraction/MinigameObject/MinigameObj_TrainingRobot.h"

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

	if (Cast<AMCharacter>(DamagedActor) || Cast<AMinigameMainObjective>(DamagedActor))
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
		// No damage to AMinigameObj_Statue
		if (auto pMinigameStatue = Cast<AMinigameObj_Statue>(DamagedActor))
		{
			return 0.0f;
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

bool ADamageManager::AddBuffPoints(EnumWeaponType WeaponType, EnumAttackBuff AttackBuff, AController* AttackerController, class AActor* BuffReceiver, float buffPointsToAdd)
{
	if (!BuffReceiver || !AttackerController || !AWeaponDataHelper::DamageManagerDataAsset)
		return false;
	if (!Cast<AMCharacter>(BuffReceiver) && !Cast<AMinigameObj_TrainingRobot>(BuffReceiver))
		return false;

	if (AMCharacter* pMCharacter = Cast<AMCharacter>(BuffReceiver))
	{
		if (!pMCharacter->CheckBuffMap(AttackBuff) || pMCharacter->IsInvincible)
			return false;
		// if it is the teammate
		if (AttackBuff == EnumAttackBuff::Burning || AttackBuff == EnumAttackBuff::Paralysis)
		{
			int TeammateCheckResult = ADamageManager::IsTeammate(AttackerController, pMCharacter);
			if (TeammateCheckResult != 0)
				return false;
		}
	}
	else if(AMinigameObj_TrainingRobot* pRobot = Cast<AMinigameObj_TrainingRobot>(BuffReceiver))
	{
		if (!pRobot->CheckBuffMap(AttackBuff))
			return false;
	}	


	if (AMCharacter* pMCharacter = Cast<AMCharacter>(BuffReceiver))
	{
		float& BuffPoints = pMCharacter->BuffMap[AttackBuff][0];
		float& BuffRemainedTime = pMCharacter->BuffMap[AttackBuff][1];
		float& BuffAccumulatedTime = pMCharacter->BuffMap[AttackBuff][2];

		float OldBuffPoints = BuffPoints;
		BuffPoints += buffPointsToAdd;
		if (AttackBuff == EnumAttackBuff::Saltcure || AttackBuff == EnumAttackBuff::Shellheal)
			BuffPoints = FMath::Clamp(BuffPoints, 0.0f, 1.0f);

		if (OldBuffPoints < 1.0f && 1.0f <= BuffPoints)
		{
			if (AttackBuff == EnumAttackBuff::Paralysis)
			{
				// Set Character's IsParalyzed to true
				pMCharacter->IsParalyzed = true;
				if (pMCharacter->GetNetMode() == NM_ListenServer)
					pMCharacter->OnRep_IsParalyzed();
			}
			else if (AttackBuff == EnumAttackBuff::Saltcure)
			{
				// Set Character's IsHealing to true
				pMCharacter->IsHealing = true;
				if (pMCharacter->GetNetMode() == NM_ListenServer)
					pMCharacter->OnRep_IsHealing();
			}
			else if (AttackBuff == EnumAttackBuff::Shellheal)
			{
				// Set Character's IsHealing to true
				pMCharacter->IsHealing = true;
				if (pMCharacter->GetNetMode() == NM_ListenServer)
					pMCharacter->OnRep_IsHealing();
			}
		}

		// When the Burning BuffPoints increases to the next integer(1, 2, 3, 4 ...), with add buff time and etc.
		if (AttackBuff == EnumAttackBuff::Burning)
		{
			if (!pMCharacter->IsHealing && 0.99f <= (FMath::FloorToInt(BuffPoints) - FMath::FloorToInt(OldBuffPoints)))
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
				pMCharacter->IsBurned = true;
				if (pMCharacter->GetNetMode() == NM_ListenServer)
					pMCharacter->OnRep_IsBurned();
				// Set Server_TheControllerApplyingLatestBurningBuff 
				pMCharacter->Server_TheControllerApplyingLatestBurningBuff = AttackerController;
			}
		}
	}
	else if (AMinigameObj_TrainingRobot* pRobot = Cast<AMinigameObj_TrainingRobot>(BuffReceiver))
	{
		float& BuffPoints = pRobot->BuffMap[AttackBuff][0];
		float& BuffRemainedTime = pRobot->BuffMap[AttackBuff][1];
		float& BuffAccumulatedTime = pRobot->BuffMap[AttackBuff][2];

		float OldBuffPoints = BuffPoints;
		BuffPoints += buffPointsToAdd;

		// When the Burning BuffPoints increases to the next integer(1, 2, 3, 4 ...), with add buff time and etc.
		if (AttackBuff == EnumAttackBuff::Burning)
		{
			if (0.99f <= (FMath::FloorToInt(BuffPoints) - FMath::FloorToInt(OldBuffPoints)))
			{
				// Burning_TimeToAdd
				FString ParName = AWeaponDataHelper::AttackBuffEnumToString_Map[AttackBuff] + "_TimeToAdd";
				if (AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map.Contains(ParName))
				{
					float buffTimeToAdd = AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map[ParName];
					BuffRemainedTime += buffTimeToAdd;
					BuffAccumulatedTime += buffTimeToAdd;
				}
			}
		}
	}
	else
	{
		return false;
	}	
	
	return true;
}

bool ADamageManager::ApplyOneTimeBuff(EnumWeaponType WeaponType, EnumAttackBuff AttackBuff, AController* AttackerController, class AActor* BuffReceiver, float DeltaTime)
{	
	if (!BuffReceiver || !AttackerController || !AWeaponDataHelper::DamageManagerDataAsset)
		return false;
	if (!Cast<AMCharacter>(BuffReceiver) && !Cast<AMinigameObj_TrainingRobot>(BuffReceiver))
		return false;

	if (AMCharacter* pMCharacter = Cast<AMCharacter>(BuffReceiver))
	{
		if (!pMCharacter->CheckBuffMap(AttackBuff) || pMCharacter->IsInvincible)
			return false;
		// if it is the teammate
		int TeammateCheckResult = IsTeammate(AttackerController, pMCharacter->GetController());
		if (TeammateCheckResult != 0)
			return false;
	}
	else if (AMinigameObj_TrainingRobot* pRobot = Cast<AMinigameObj_TrainingRobot>(BuffReceiver))
	{
		if (!pRobot->CheckBuffMap(AttackBuff))
			return false;
	}

	if (AMCharacter* pMCharacter = Cast<AMCharacter>(BuffReceiver))
	{
		/* Knockback */
		if (AttackBuff == EnumAttackBuff::Knockback && AttackerController->GetPawn())
		{
			FVector AttackingDirection = AttackerController->GetPawn()->GetControlRotation().RotateVector(FVector3d::ForwardVector);
			AttackingDirection.Z = 0.0f;
			AttackingDirection.Normalize();
			pMCharacter->KnockbackDirection_SinceLastApplyBuff = AttackingDirection; // Deprecated
			float Knockback_MoveSpeed = 0.0f;
			FString ParName = "";
			ParName = "Knockback_MoveSpeed";
			if (AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map.Contains(ParName))
				Knockback_MoveSpeed = AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map[ParName];
			ParName = AWeaponDataHelper::WeaponEnumToString_Map[WeaponType] + "_Knockback_MoveSpeed";
			if (AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map.Contains(ParName))
				Knockback_MoveSpeed = AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map[ParName];
			AttackingDirection *= Knockback_MoveSpeed;
			pMCharacter->LaunchCharacter(AttackingDirection, true, false);
		}
		/* Paralysis */
		else if (AttackBuff == EnumAttackBuff::Paralysis && AttackerController->GetPawn())
		{
			FVector Direction_TargetToAttacker = AttackerController->GetPawn()->GetActorLocation() - pMCharacter->GetActorLocation();
			if (100.0f < Direction_TargetToAttacker.Length())
			{
				Direction_TargetToAttacker.Normalize();

				pMCharacter->Server_Direction_SelfToTaserAttacker += Direction_TargetToAttacker;
				// pMCharacter->LaunchCharacter(Direction_TargetToAttacker * DragSpeed * DeltaTime, true, false);
				// pMCharacter->SetActorLocation(pMCharacter->GetActorLocation() + Direction_TargetToAttacker * 100.0f * DeltaTime);
				// pMCharacter->Client_MoveCharacter(Direction_TargetToAttacker, DragSpeedRatio);
			}
			else
			{
				pMCharacter->Server_Direction_SelfToTaserAttacker += FVector::Zero();
			}
		}
	}
	else if (AMinigameObj_TrainingRobot* pRobot = Cast<AMinigameObj_TrainingRobot>(BuffReceiver))
	{
		/* Knockback */
		if (AttackBuff == EnumAttackBuff::Knockback && AttackerController->GetPawn())
		{
			//FVector AttackingDirection = AttackerController->GetPawn()->GetControlRotation().RotateVector(FVector3d::ForwardVector);
			//AttackingDirection.Z = 0.0f;
			//AttackingDirection.Normalize();
			//pMCharacter->KnockbackDirection_SinceLastApplyBuff = AttackingDirection; // Deprecated
			//float Knockback_MoveSpeed = 0.0f;
			//FString ParName = "";
			//ParName = "Knockback_MoveSpeed";
			//if (AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map.Contains(ParName))
			//	Knockback_MoveSpeed = AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map[ParName];
			//ParName = AWeaponDataHelper::WeaponEnumToString_Map[WeaponType] + "_Knockback_MoveSpeed";
			//if (AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map.Contains(ParName))
			//	Knockback_MoveSpeed = AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map[ParName];
			//AttackingDirection *= Knockback_MoveSpeed;
			//pMCharacter->LaunchCharacter(AttackingDirection, true, false);
		}
		/* Paralysis */
		else if (AttackBuff == EnumAttackBuff::Paralysis && AttackerController->GetPawn())
		{
			FVector Direction_TargetToAttacker = AttackerController->GetPawn()->GetActorLocation() - pRobot->GetActorLocation();
			if (100.0f < Direction_TargetToAttacker.Length())
			{
				Direction_TargetToAttacker.Normalize();
				//pRobot->LaunchCharacter(Direction_TargetToAttacker * 200.0f * DeltaTime, true, false);
			}
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
