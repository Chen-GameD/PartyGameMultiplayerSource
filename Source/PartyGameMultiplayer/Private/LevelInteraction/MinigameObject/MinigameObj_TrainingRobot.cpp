// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelInteraction/MinigameObject/MinigameObj_TrainingRobot.h"
#include "Weapon/WeaponDataHelper.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/WidgetComponent.h"
#include "UI/MinigameObjFollowWidget.h"

AMinigameObj_TrainingRobot::AMinigameObj_TrainingRobot()
{
	RootMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RootMesh"));	
	RootMesh->SetupAttachment(RootComponent);
	RootMesh->SetCollisionProfileName(TEXT("NoCollision"));
	
	RobotMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RobotMesh"));
	RobotMesh->SetupAttachment(RootMesh);
	RobotMesh->SetCollisionProfileName(TEXT("Custom"));
	
	RobotCenterMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RobotCenterMesh"));
	RobotCenterMesh->SetupAttachment(RobotMesh);
	RobotCenterMesh->SetCollisionProfileName(TEXT("NoCollision"));
	
	CollisionMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CollisionMesh"));
	CollisionMesh->SetupAttachment(RobotMesh);
	CollisionMesh->SetCollisionProfileName(TEXT("Custom"));
	
	FollowWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("FollowWidget"));
	FollowWidget->SetupAttachment(RootMesh);
	
	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;
	
	RespawnDelay = 5.0f;
	
	EffectBurn = CreateDefaultSubobject<UNiagaraComponent>(TEXT("EffectBurn"));
	EffectBurn->SetupAttachment(RootComponent);
	EffectBurn->bAutoActivate = false;
	
	IsDead = false;
}

void AMinigameObj_TrainingRobot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Server
	if (GetLocalRole() == ROLE_Authority)
	{
		// Deal with buff
		ActByBuff_PerTick(DeltaTime);
	}
}

void AMinigameObj_TrainingRobot::ActByBuff_PerTick(float DeltaTime)
{
	// Server
	if (GetLocalRole() == ROLE_Authority)
	{
		if (!AWeaponDataHelper::DamageManagerDataAsset)
			return;

		EnumAttackBuff buffType;
		/*  Burning */
		buffType = EnumAttackBuff::Burning;
		if (CheckBuffMap(buffType))
		{
			float& BuffRemainedTime = BuffMap[buffType][1];
			// Is Burning
			if (0.0f < BuffRemainedTime && 0 < CurrentHealth)
			{
				float BurningBuffDamagePerSecond = 0.0f;
				FString ParName = "BurningDamagePerSecond";
				if (AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map.Contains(ParName))
					BurningBuffDamagePerSecond = AWeaponDataHelper::DamageManagerDataAsset->Character_Buff_Map[ParName];

				float TargetCurrentHealth = CurrentHealth - DeltaTime * BurningBuffDamagePerSecond;
				SetCurrentHealth(TargetCurrentHealth);

				BuffRemainedTime = FMath::Max(BuffRemainedTime - DeltaTime, 0.0f);

				if(!EffectBurn->IsActive())
					EffectBurn->Activate();
			}
			// Is Not Burning
			else
			{
				if (EffectBurn->IsActive())
					EffectBurn->Deactivate();
			}
		}
		/* Paralysis */
		buffType = EnumAttackBuff::Paralysis;
		if (CheckBuffMap(buffType))
		{
			float& BuffPoints = BuffMap[buffType][0];
			//  Is Paralyzed
			if (1.0f <= BuffPoints && 0 < CurrentHealth)
			{
				
			}
			// Is Not Paralyzed
			else
			{
			}
		}
	}
}

void AMinigameObj_TrainingRobot::BeginPlay()
{
	Super::BeginPlay();

	if (UMinigameObjFollowWidget* pFollowWidget = Cast<UMinigameObjFollowWidget>(FollowWidget->GetUserWidgetObject()))
	{
		pFollowWidget->SetHealthByPercentage(1);
		FollowWidget->SetVisibility(true);
	}	
}

void AMinigameObj_TrainingRobot::OnRep_CurrentHealth()
{
	Super::OnRep_CurrentHealth();

	if (CurrentHealth <= 0)
	{
		// Respawn robot
		// TODO
		IsDead = true;
	}

	// Set UI: Health Bar
	if(UMinigameObjFollowWidget* pFollowWidget = Cast<UMinigameObjFollowWidget>(FollowWidget->GetUserWidgetObject()))
		pFollowWidget->SetHealthByPercentage(CurrentHealth / MaxHealth);
}


void AMinigameObj_TrainingRobot::SetCurrentHealth(float healthValue)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		CurrentHealth = FMath::Clamp(healthValue, 0.f, MaxHealth);
		OnRep_CurrentHealth();
	}
}


float AMinigameObj_TrainingRobot::TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (DamageTaken <= 0 || CurrentHealth <= 0 || !EventInstigator || !DamageCauser)
		return 0.0f;

	CurrentHealth = FMath::Max(CurrentHealth - DamageTaken, 0);
	if (GetNetMode() == NM_ListenServer)
		OnRep_CurrentHealth();

	if (CurrentHealth <= 0)
	{
		// Respawn
		Server_WhenDead();
	}
	return 0.0f;
}

void AMinigameObj_TrainingRobot::Server_WhenDead()
{
	// Respawn(Destroy)
	FTimerHandle RespawnMinigameObjectTimerHandle;
	GetWorldTimerManager().SetTimer(RespawnMinigameObjectTimerHandle, this, &AMinigameObj_TrainingRobot::StartToRespawnActor, RespawnDelay, false);
}


bool AMinigameObj_TrainingRobot::CheckBuffMap(EnumAttackBuff AttackBuff)
{
	if (!BuffMap.Contains(AttackBuff))
	{
		BuffMap.Add(AttackBuff);
		TArray<float> buffParArr;
		for (int i = 0; i < 3; i++)
			buffParArr.Add(0.0f);
		BuffMap[AttackBuff] = buffParArr;
	}
	return(BuffMap[AttackBuff].Num() == 3);
}
