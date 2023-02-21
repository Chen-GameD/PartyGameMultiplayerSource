// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon/DamageManager.h"

#include "Kismet/GameplayStatics.h"

#include "Weapon/BaseWeapon.h"
#include "Weapon/BaseProjectile.h"
#include "Weapon/CombinedWeapon/WeaponBomb.h"
//#include "Weapon/JsonFactory.h"
#include "Weapon/WeaponDataHelper.h"
#include "Character/MCharacter.h"
#include "LevelInteraction/MinigameMainObjective.h"

FTimerHandle* ADamageManager::TimerHandle_Loop = nullptr;
float ADamageManager::interval_ApplyDamage = 0.1f; // preset value for all constant damage


bool ADamageManager::TryApplyDamageToAnActor(ABaseWeapon* AttackingWeapon, TSubclassOf<UDamageType> DamageTypeClass, AActor* DamagedActor)
{
	if (!AttackingWeapon || !DamagedActor || !AWeaponDataHelper::DamageManagerDataAsset)
		return false;

	if (auto pCharacter = Cast<AMCharacter>(DamagedActor))
	{
		// check holding player
		auto AttackingWeaponHoldingPlayer = AttackingWeapon->GetHoldingPlayer();
		if (!AttackingWeaponHoldingPlayer)
			return false;
		// check both player controllers
		auto AttackingWeaponHoldingCharacterController = AttackingWeaponHoldingPlayer->GetController();
		auto DamagedCharacterController = pCharacter->GetController();
		if (!AttackingWeaponHoldingCharacterController || !DamagedCharacterController)
			return false;
		// check both player states
		AM_PlayerState* pCharacterPS = AttackingWeaponHoldingCharacterController->GetPlayerState<AM_PlayerState>();
		AM_PlayerState* AttakingWeaponPS = DamagedCharacterController->GetPlayerState<AM_PlayerState>();
		if (!pCharacterPS || !AttakingWeaponPS)
			return false;
		if (pCharacterPS->TeamIndex == AttakingWeaponPS->TeamIndex)
			return false;

		EnumWeaponType WeaponType = AttackingWeapon->WeaponType;
		float Damage = 0.0f;
		FString ParName = AttackingWeapon->GetWeaponName();
		if (AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map.Contains(ParName))
			Damage = AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map[ParName];
		if (AttackingWeapon->AttackType == EnumAttackType::Constant)
			Damage *= interval_ApplyDamage;
		// Special situation: Bomb's fork
		if (AttackingWeapon->WeaponType == EnumWeaponType::Bomb && DamageTypeClass == UDamageType::StaticClass())
		{
			if (AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map.Contains("Fork"))
				Damage = AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map["Fork"];
		}
		AController* EventInstigator = AttackingWeapon->GetInstigator()->Controller;
		//pCharacter->TakeDamageRe(Damage, WeaponType, EventInstigator, AttackingWeapon);
		UGameplayStatics::ApplyDamage(DamagedActor, Damage, AttackingWeapon->GetInstigator()->Controller, AttackingWeapon, AttackingWeapon->DamageType);
	}
	else if (Cast<AMinigameMainObjective>(DamagedActor))
	{
		float Damage = 0.0f;
		FString ParName = AttackingWeapon->GetWeaponName();
		if (AWeaponDataHelper::DamageManagerDataAsset->MiniGame_Damage_Map.Contains(ParName))
			Damage = AWeaponDataHelper::DamageManagerDataAsset->MiniGame_Damage_Map[ParName];
		if (AttackingWeapon->AttackType == EnumAttackType::Constant)
			Damage *= interval_ApplyDamage;
		UGameplayStatics::ApplyDamage(DamagedActor, Damage, AttackingWeapon->GetInstigator()->Controller, AttackingWeapon, AttackingWeapon->MiniGameDamageType);
	}
	else
	{
		return false;
	}
	return true;
}


bool ADamageManager::TryApplyRadialDamage(ABaseWeapon* AttackingWeapon, FVector Origin)
{
	if (!AttackingWeapon)
		return false;
	
	/* Get the following from map */
	float TotalTime_ApplyDamage = 0.0f;
	float TotalDamage_ForTotalTime = 0.0f;
	float DamageRadius = 0.0f;
	bool bApplyConstantDamage = false;
	// total time
	FString ParName = "";
	ParName = AttackingWeapon->GetWeaponName() + "_TotalTime";
	if (AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map.Contains(ParName))
		TotalTime_ApplyDamage = AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map[ParName];
	// total damage
	ParName = AttackingWeapon->GetWeaponName() + "_TotalDamage";
	if (AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map.Contains(ParName))
		TotalDamage_ForTotalTime = AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map[ParName];
	// damage radius
	ParName = AttackingWeapon->GetWeaponName() + "_DamageRadius";
	if (AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map.Contains(ParName))
		DamageRadius = AWeaponDataHelper::DamageManagerDataAsset->Character_Damage_Map[ParName];
	if (0.0f < TotalTime_ApplyDamage)
		bApplyConstantDamage = true;
	
	float numIntervals = TotalTime_ApplyDamage / interval_ApplyDamage;
	float BaseDamage = bApplyConstantDamage ? (TotalDamage_ForTotalTime / numIntervals) : TotalDamage_ForTotalTime;
	ADamageManager::ApplyRadialDamageOnce(AttackingWeapon, Origin, DamageRadius, BaseDamage);
	DrawDebugSphere(AttackingWeapon->GetWorld(), Origin, DamageRadius, 12, FColor::Red, false, bApplyConstantDamage ? TotalTime_ApplyDamage : 5.0f);
	if (bApplyConstantDamage)
	{
		TimerHandle_Loop = new FTimerHandle;
		AttackingWeapon->GetWorld()->GetTimerManager().SetTimer(*TimerHandle_Loop, [AttackingWeapon, Origin, DamageRadius, BaseDamage]()
			{
				ADamageManager::ApplyRadialDamageOnce(AttackingWeapon, Origin, DamageRadius, BaseDamage);
			}, interval_ApplyDamage, true);  // the bool paramter determines if the function will be called in loop or not

		FTimerHandle TimerHandle_SelfDestroy;
		AttackingWeapon->GetWorld()->GetTimerManager().SetTimer(TimerHandle_SelfDestroy, [AttackingWeapon]()
			{
				AttackingWeapon->GetWorldTimerManager().ClearTimer(*TimerHandle_Loop);
			}, TotalTime_ApplyDamage + 0.01f, false);
	}
	return true;
}


bool ADamageManager::ApplyBuff(ABaseWeapon* AttackingWeapon, TSubclassOf<UDamageType> DamageTypeClass, class AMCharacter* DamagedCharacter)
{	
	if (!AttackingWeapon || !DamagedCharacter || AttackingWeapon->WeaponType == EnumWeaponType::None || !AWeaponDataHelper::DamageManagerDataAsset)
		return false;

	TArray<EnumAttackBuff> AttackBuffs;
	if (AttackingWeapon->WeaponType == EnumWeaponType::Fork)
		AttackBuffs.Add(EnumAttackBuff::Knockback);
	else if (AttackingWeapon->WeaponType == EnumWeaponType::Bomb && DamageTypeClass == UDamageType::StaticClass())
		AttackBuffs.Add(EnumAttackBuff::Knockback);
	else if (AttackingWeapon->WeaponType == EnumWeaponType::Blower)
		AttackBuffs.Add(EnumAttackBuff::Knockback);
	else if (AttackingWeapon->WeaponType == EnumWeaponType::Lighter)
		AttackBuffs.Add(EnumAttackBuff::Burning);
	else if (AttackingWeapon->WeaponType == EnumWeaponType::Flamethrower)
	{
		AttackBuffs.Add(EnumAttackBuff::Burning);
		AttackBuffs.Add(EnumAttackBuff::Knockback);
	}
	else if (AttackingWeapon->WeaponType == EnumWeaponType::Flamefork)
		AttackBuffs.Add(EnumAttackBuff::Burning);
	else if (AttackingWeapon->WeaponType == EnumWeaponType::Taser)
		AttackBuffs.Add(EnumAttackBuff::Paralysis);

	for (int32 i = 0; i < AttackBuffs.Num(); i++)
	{
		EnumAttackBuff AttackBuff = AttackBuffs[i];

		check(DamagedCharacter->CheckBuffMap(AttackBuff));
		float& BuffPoints = DamagedCharacter->BuffMap[AttackBuff][0];
		float& BuffRemainedTime = DamagedCharacter->BuffMap[AttackBuff][1];
		float& BuffAccumulatedTime = DamagedCharacter->BuffMap[AttackBuff][2];

		float buffPointsToAdd = 0.0f;
		float buffTimeToAdd = 0.0f;
		if (AttackBuff == EnumAttackBuff::Knockback)
		{
			ACharacter* AttakingPlayer = AttackingWeapon->GetHoldingPlayer();
			check(AttakingPlayer);
			FVector3d AttackingDirection = AttakingPlayer->GetControlRotation().RotateVector(FVector3d::ForwardVector);
			DamagedCharacter->KnockbackDirection_DuringLastFrame += AttackingDirection;
		}		
		
		FString ParName = "";
		// points to add
		ParName = ABaseWeapon::AttackBuffEnumToString_Map[AttackBuff] + "_PointsToAdd_PerHit";
		if (AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map.Contains(ParName))
			buffPointsToAdd = AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map[ParName];
		ParName = ABaseWeapon::WeaponEnumToString_Map[AttackingWeapon->WeaponType] + "_" + 
				ABaseWeapon::AttackBuffEnumToString_Map[AttackBuff] + "_PointsToAdd_PerHit";
		if (AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map.Contains(ParName))
			buffPointsToAdd = AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map[ParName];
		ParName = ABaseWeapon::WeaponEnumToString_Map[AttackingWeapon->WeaponType] + "_" +
			ABaseWeapon::AttackBuffEnumToString_Map[AttackBuff] + "_PointsToAdd_PerSec";
		if (AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map.Contains(ParName))
			buffPointsToAdd = interval_ApplyDamage * AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map[ParName];
		BuffPoints += buffPointsToAdd;
		if(AttackBuff == EnumAttackBuff::Burning)
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("BuffPoints: %f"), BuffPoints));
		if (1.0f <= BuffPoints)
		{
			// time to add		
			ParName = ABaseWeapon::AttackBuffEnumToString_Map[AttackBuff] + "_TimeToAdd";	
			// when it is time-based buff(burning), we add the time and substract 1 from BuffPoints; 
			// otherwise(paralysis, knockback), we will neither add the time nor touch the BuffPoints
			if (AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map.Contains(ParName))
			{
				buffTimeToAdd = AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map[ParName];
				BuffRemainedTime += buffTimeToAdd;
				BuffAccumulatedTime += buffTimeToAdd;
				BuffPoints -= floorf(BuffPoints);
			}				
		}
	}	
	return true;
}


bool ADamageManager::ApplyRadialDamageOnce(ABaseWeapon* AttackingWeapon, FVector Origin, float DamageRadius, float BaseDamage)
{
	if (!AttackingWeapon)
		return false;

	TArray<AActor*> IgnoredActors;
	// TODO: Add teammates
	IgnoredActors.Add(AttackingWeapon->GetHoldingPlayer());

	bool bDoFullDamage = true;
	float DamageFalloff = bDoFullDamage ? 0.f : 1.f;
	UGameplayStatics::ApplyRadialDamageWithFalloff(
		AttackingWeapon,   //const UObject* WorldContextObject
		BaseDamage,	
		0.0f,			// float MinDamage
		Origin,
		0.f,			// float DamageInnerRadius
		DamageRadius,
		DamageFalloff,
		UDamageType::StaticClass(),  //TSubclassOf<UDamageType> DamageTypeClass
		IgnoredActors,				//const TArray<AActor*>& IgnoreActors
		AttackingWeapon,		//AActor* DamageCauser
		AttackingWeapon->GetHoldingPlayer()->GetController() //AController* InstigatedByController
		//ECC_Visibility	  // DamagePreventionChannel
	);

	return true;
}
