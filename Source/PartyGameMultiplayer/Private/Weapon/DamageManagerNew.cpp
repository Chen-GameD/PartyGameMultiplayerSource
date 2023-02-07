// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/DamageManagerNew.h"
#include "Weapon/BaseWeapon.h"
#include "Weapon/JsonFactory.h"
#include "Character/MCharacter.h"
#include "LevelInteraction/MinigameMainObjective.h"
#include "Kismet/GameplayStatics.h"


UDamageManagerDataAsset* ADamageManagerNew::DamageManagerDataAsset = nullptr;

// Sets default values
ADamageManagerNew::ADamageManagerNew()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	static ConstructorHelpers::FObjectFinder<UDamageManagerDataAsset> DefaultDamageManagerDataAsset(TEXT("/Game/DataFiles/Weapon/DamageManagerDataAsset.DamageManagerDataAsset"));
	if (DefaultDamageManagerDataAsset.Succeeded())
	{
		DamageManagerDataAsset = DefaultDamageManagerDataAsset.Object;
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, TEXT("find bp data asset object!"));
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, FString::Printf(TEXT("%f"), DamageManagerDataAsset->TestDamageNumber));
	}
	else
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, TEXT("cannot find bp data asset object!"));
}

// Called when the game starts or when spawned
void ADamageManagerNew::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADamageManagerNew::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


bool ADamageManagerNew::DealDamageAndBuffBetweenActors(ABaseWeapon* AttackingWeapon, AActor* DamagedActor)
{
	if (!AttackingWeapon || !DamagedActor)
		return false;

	//auto jsonObject = UJsonFactory::GetJsonObject_1();
	//if (!jsonObject)
	//{
	//	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, "Read json failed during DealDamageAndBuffBetweenActors()!");
	//	return false;
	//}	

	if (DamageManagerDataAsset)
	{
		float tmpFloat = DamageManagerDataAsset->TestDamageNumber;
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, FString::Printf(TEXT("tmpFloat is %f"), tmpFloat));
	}

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
		float Damage = AttackingWeapon->Damage;
		FString weaponDamageParName = AttackingWeapon->GetWeaponName() + TEXT("Damage");
		//if(!jsonObject->TryGetNumberField(weaponDamageParName, Damage))
		//	Damage = 0.0f;
		AController* EventInstigator = AttackingWeapon->GetInstigator()->Controller;

		//FString testStr = UJsonFactory::LoadFileToString("DataFiles/test.txt");
		//GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Blue, testStr);	

		/*auto jsonObject = UJsonFactory::ReadJson("DataFiles/test.json");
		if (jsonObject)
		{
			FString targetString = "";
			double targetDouble = 0;
			bool targetBoolen = true;
			jsonObject->TryGetStringField("a1", targetString);
			jsonObject->TryGetNumberField("a2", targetDouble);
			jsonObject->TryGetBoolField("a3", targetBoolen);
			GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Blue, FString::Printf(TEXT("Json string: %s"), *targetString));
			GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Blue, FString::Printf(TEXT("Json float: %lf"), targetDouble));
			GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Blue, FString::Printf(TEXT("Json bool: %s"), targetBoolen ? TEXT("true") : TEXT("false")));
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Blue, FString::Printf(TEXT("Json Failed")));
		}*/

		if (AttackingWeapon->WeaponType == EnumWeaponType::None)
		{
			return false;
		}
		// Fork
		else if (AttackingWeapon->WeaponType == EnumWeaponType::Fork)
		{
			ApplyBuff(EnumAttackBuff::Knockback, AttackingWeapon, pCharacter);
		}
		// Blower
		else if (AttackingWeapon->WeaponType == EnumWeaponType::Blower)
		{
			ApplyBuff(EnumAttackBuff::Knockback, AttackingWeapon, pCharacter);
		}
		// Lighter
		else if (AttackingWeapon->WeaponType == EnumWeaponType::Lighter)
		{
			ApplyBuff(EnumAttackBuff::Burning, AttackingWeapon, pCharacter);
		}
		// Flamethrower
		else if (AttackingWeapon->WeaponType == EnumWeaponType::Flamethrower)
		{
			ApplyBuff(EnumAttackBuff::Burning, AttackingWeapon, pCharacter);
			ApplyBuff(EnumAttackBuff::Knockback, AttackingWeapon, pCharacter);
		}
		// Flamefork
		else if (AttackingWeapon->WeaponType == EnumWeaponType::Flamefork)
		{
			ApplyBuff(EnumAttackBuff::Burning, AttackingWeapon, pCharacter);
		}
		// Taser
		else if (AttackingWeapon->WeaponType == EnumWeaponType::Taser)
		{
			ApplyBuff(EnumAttackBuff::Paralysis, AttackingWeapon, pCharacter);
		}

		// TODO: judge if it is a teammate
		pCharacter->TakeDamageRe(Damage, WeaponType, EventInstigator, AttackingWeapon);

	}
	else if (dynamic_cast<AMinigameMainObjective*>(DamagedActor))
	{
		float Damage = AttackingWeapon->MiniGameDamage;
		FString weaponDamageParName = AttackingWeapon->GetWeaponName() + TEXT("MiniGameDamage");
		//if (!jsonObject->TryGetNumberField(weaponDamageParName, Damage))
		//	Damage = 0.0f;
		//// Temporary; weird to assign MiniGameAccumulatedTimeToGenerateDamage here
		//if (999.0f < AttackingWeapon->MiniGameAccumulatedTimeToGenerateDamage)
		//	AttackingWeapon->MiniGameAccumulatedTimeToGenerateDamage = AttackingWeapon->AccumulatedTimeToGenerateDamage;
		UGameplayStatics::ApplyDamage(DamagedActor, Damage, AttackingWeapon->GetInstigator()->Controller, AttackingWeapon, UDamageType::StaticClass());
	}
	else
	{
		return false;
	}
	return true;
}

bool ADamageManagerNew::ApplyBuff(EnumAttackBuff AttackBuff, ABaseWeapon* AttackingWeapon, class AMCharacter* DamagedActor)
{
	auto jsonObject = UJsonFactory::GetJsonObject_1();
	if (!jsonObject)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, "Read json failed during ApplyBuff()!");
		return false;
	}

	if (AttackBuff == EnumAttackBuff::Burning)
	{
		float buffPoints = 0.0f;
		FString weaponBuffPointsParName = AttackingWeapon->GetWeaponName() + TEXT("BurningBuffPoints");
		if (!jsonObject->TryGetNumberField(weaponBuffPointsParName, buffPoints))
			buffPoints = 0.0f;
		DamagedActor->AccumulateAttackedBuff(EnumAttackBuff::Burning, buffPoints, FVector3d::Zero(),
			AttackingWeapon->GetInstigator()->Controller, AttackingWeapon);
	}
	else if (AttackBuff == EnumAttackBuff::Paralysis)
	{
		DamagedActor->AccumulateAttackedBuff(EnumAttackBuff::Paralysis, 1.0f, FVector3d::Zero(),
			AttackingWeapon->GetInstigator()->Controller, AttackingWeapon);
	}
	else if (AttackBuff == EnumAttackBuff::Knockback)
	{
		check(AttackingWeapon->GetHoldingPlayer());
		FRotator AttackerControlRotation = AttackingWeapon->GetHoldingPlayer()->GetControlRotation();
		FVector3d AttackerControlDir = AttackerControlRotation.RotateVector(FVector3d::ForwardVector);
		DamagedActor->AccumulateAttackedBuff(EnumAttackBuff::Knockback, 1.0f, AttackerControlDir,
			AttackingWeapon->GetInstigator()->Controller, AttackingWeapon);
	}
	else
	{
		return false;
	}
	return true;
}