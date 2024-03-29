// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon/CombinedWeapon/WeaponTaser.h"

#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/PrimitiveComponent.h"
#include "Net/UnrealNetwork.h"

#include "Weapon/BaseProjectile.h"
#include "Weapon/DamageManager.h"
#include "Weapon/DamageType/MeleeDamageType.h"
#include "LevelInteraction/MinigameMainObjective.h"
#include "LevelInteraction/MinigameObject/MinigameObj_Enemy.h"
#include "LevelInteraction/MinigameObject/MinigameObj_Statue.h"
#include "LevelInteraction/MinigameObject/MinigameObj_TrainingRobot.h"
#include "Character/MCharacter.h"
#include "M_PlayerState.h"


AWeaponTaser::AWeaponTaser()
{
	IsCombineWeapon = true;
	WeaponType = EnumWeaponType::Taser;
	AttackType = EnumAttackType::Constant;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("/Game/ArtAssets/Models/Taser/Taser_Body.Taser_Body"));
	if (DefaultMesh.Succeeded())
	{
		WeaponMesh->SetStaticMesh(DefaultMesh.Object);
	}

	// Create a fork mesh specific to Taser
	TaserForkMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TaserForkMesh"));
	TaserForkMesh->SetupAttachment(WeaponMesh);
	TaserForkMesh->SetCollisionProfileName(TEXT("Trigger"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultTaserForkMesh(TEXT("/Game/ArtAssets/Models/Taser/Taser_Fork.Taser_Fork"));
	if (DefaultTaserForkMesh.Succeeded())
	{
		TaserForkMesh->SetStaticMesh(DefaultTaserForkMesh.Object);
	}

	// Currently, they are decided by derived BP
	//TaserForkMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	//TaserForkMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	//TaserForkMesh->SetRelativeScale3D(FVector(2.0f, 2.0f, 2.0f));

	AttackDetectComponent = TaserForkMesh;

	static ConstructorHelpers::FObjectFinder<UParticleSystem> DefaultAttackHitEffect(TEXT("/Game/StarterContent/Particles/P_Explosion.P_Explosion"));
	if (DefaultAttackHitEffect.Succeeded())
	{
		AttackHitEffect = DefaultAttackHitEffect.Object;
	}

	AttackOnEffect_TaserFork = CreateDefaultSubobject<UNiagaraComponent>(TEXT("AttackOnNiagaraEffect_TaserFork"));
	AttackOnEffect_TaserFork->SetupAttachment(TaserForkMesh);

	ElecWire_NSComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ElecWire_NSComponent"));
	ElecWire_NSComponent->SetupAttachment(WeaponMesh);

	// Currently, they are decided by derived BP
	//MaxLen = 0.0f;
	//StrechOutSpeed = 360.0f;
	//StrechInSpeed = 160.0f;

	IsForkOut = false;

	Server_ActorBeingHit = nullptr;
	//bHitTarget = false;
	bForkAttachedToWeapon = true;

	Ratio_ScaleUpOnRelativeScale = 3.0f;
}


void AWeaponTaser::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Server
	if (GetLocalRole() == ROLE_Authority)
	{
		// if attack on
		if (bAttackOn)
		{
			// if not hit a target, the server fork would stretch out and the client fork would copy the location 
			if (!Server_ActorBeingHit)
			{
				// stretch out to the limitd
				FVector TaserFork_CurWorldLocation = TaserForkMesh->GetComponentLocation();
				if (FVector::Distance(TaserFork_CurWorldLocation, TaserFork_WorldLocation_WhenAttackStart) < MaxLen)
				{
					TaserForkMesh->SetWorldLocation(TaserFork_CurWorldLocation + DeltaTime * TaserFork_WorldRotation_WhenAttackStart.Vector() * -StrechOutSpeed);
				}
				else
					AttackStop();
			}
			// if hit a target 
			else
			{					
				if (Cast<AMCharacter>(Server_ActorBeingHit) || Cast<AMinigameObj_TrainingRobot>(Server_ActorBeingHit))
				{
					// Location: keep the same offset
					//TaserForkMesh->SetWorldLocation(Server_ActorBeingHit->GetActorLocation() + Server_ActorBeingHit_To_TaserFork_WhenHit);
					if (AMCharacter* pMCharacter = Cast<AMCharacter>(Server_ActorBeingHit))
						TaserForkMesh->SetWorldLocation(pMCharacter->GetActorLocation() + FVector::Zero());
					else if(AMinigameObj_TrainingRobot* pRobot = Cast<AMinigameObj_TrainingRobot>(Server_ActorBeingHit))
						TaserForkMesh->SetWorldLocation(pRobot->RobotCenterMesh->GetComponentLocation() + FVector::Zero());
					// Rotation
					if (Server_ActorBeingHit)
					{
						FVector Server_ActorBeingHit_To_WeaponMesh_Now = GetActorLocation() - Server_ActorBeingHit->GetActorLocation();
						float Angle = FMath::Acos(FVector::DotProduct(Server_ActorBeingHit_To_WeaponMesh_WhenHit.GetSafeNormal(), Server_ActorBeingHit_To_WeaponMesh_Now.GetSafeNormal())) * (180.0f / PI);
						bool bNowVectorIsOnTheRight = 0 < FVector::DotProduct(FVector::UpVector, FVector::CrossProduct(Server_ActorBeingHit_To_WeaponMesh_WhenHit.GetSafeNormal(), Server_ActorBeingHit_To_WeaponMesh_Now.GetSafeNormal()));
						FRotator CurRotation = TaserForkMesh->GetRelativeRotation();
						CurRotation.Yaw = Server_TaserForkRotationYaw_WhenHit + (bNowVectorIsOnTheRight ? Angle : -Angle);
						TaserForkMesh->SetWorldRotation(CurRotation);
					}
					// Server_ActorBeingHit can be set to null in overlapEnd after SetWorldLocation, which is an unintended situation, but must be dealt with
					else
					{
						AttackStop();
					}
					// Apply paralysis buff(drag towards the attacker)
					ADamageManager::ApplyOneTimeBuff(WeaponType, EnumAttackBuff::Paralysis, HoldingController, Server_ActorBeingHit, DeltaTime);
					// Stop attack when the character is dead
					if (AMCharacter* pCharacter = Cast<AMCharacter>(Server_ActorBeingHit))
					{
						if (pCharacter->GetIsDead())
							AttackStop();
					}
					// Stop attack when the robot is dead
					if (AMinigameObj_TrainingRobot* pRobot = Cast<AMinigameObj_TrainingRobot>(Server_ActorBeingHit))
					{
						if (pRobot->GetCurrentHealth() <= 0)
							AttackStop();
					}
				}
				// Stop attack when the MinigameMainObjective is dead
				else if (AMinigameObj_Enemy* pMinigameObj_Enemy = Cast<AMinigameObj_Enemy>(Server_ActorBeingHit))
				{
					if (pMinigameObj_Enemy->GetCurrentHealth() <= 0)
						AttackStop();
				}
			}
		}
		ServerForkWorldLocation = TaserForkMesh->GetComponentLocation();
		ServerForkWorldRotation = TaserForkMesh->GetComponentRotation();
	}

	// if attack stops, the fork will be back instantly
	if (!bAttackOn)
	{
		if (TaserForkMesh->GetRelativeLocation() != TaserFork_OriginalRelativeLocation ||
			TaserForkMesh->GetRelativeRotation() != TaserFork_OriginalRelativeRotation)
		{
			TaserForkMesh->SetRelativeLocation(TaserFork_OriginalRelativeLocation);
			TaserForkMesh->SetRelativeRotation(TaserFork_OriginalRelativeRotation);
		}
		if (TaserForkMesh->GetRelativeScale3D() != TaserFork_OriginalRelativeScale)
			TaserForkMesh->SetRelativeScale3D(TaserFork_OriginalRelativeScale);
		IsForkOut = false;
		if (GetNetMode() == NM_ListenServer)
			OnRep_IsForkOut();
	}

}

void AWeaponTaser::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeaponTaser, ServerForkWorldLocation);
	DOREPLIFETIME(AWeaponTaser, ServerForkWorldRotation);
	//DOREPLIFETIME(AWeaponTaser, bHitTarget);
	DOREPLIFETIME(AWeaponTaser, IsForkOut);
}



void AWeaponTaser::AttackStart(float AttackTargetDistance)
{
	if (bAttackOn || !GetOwner() || IsForkOut)
		return;
	// If the weapon has cd
	if (0 < CD_MaxEnergy)
	{
		if (AttackType == EnumAttackType::Constant)
		{
			if (CD_LeftEnergy <= 0)
				return;
		}
		else
		{
			if (CD_MinEnergyToAttak <= CD_LeftEnergy)
				CD_LeftEnergy -= CD_MinEnergyToAttak;
			else
				return;
		}
	}
	bAttackOn = true;
	// Listen server
	if (GetNetMode() == NM_ListenServer)
	{
		OnRep_bAttackOn();
	}
	for (auto& Elem : ApplyDamageCounter)
		Elem.Value = 0;

	SetActorEnableCollision(bAttackOn);

	TaserForkMesh->SetRelativeScale3D(TaserFork_OriginalRelativeScale * Ratio_ScaleUpOnRelativeScale);
	TaserFork_WorldLocation_WhenAttackStart = TaserForkMesh->GetComponentLocation();
	TaserFork_WorldRotation_WhenAttackStart = TaserForkMesh->GetComponentRotation();
	IsForkOut = true;
	if (GetNetMode() == NM_ListenServer)
		OnRep_IsForkOut();
	SetTaserForkAttached(false);
}

void AWeaponTaser::AttackStop()
{
	if (!bAttackOn || !GetOwner())
		return;
	bAttackOn = false;
	// Listen server
	if (GetNetMode() == NM_ListenServer)
	{
		OnRep_bAttackOn();
	}
	for (auto& Elem : ApplyDamageCounter)
		Elem.Value = 0;
	AttackObjectMap.Empty();

	if (AttackType == EnumAttackType::Constant)
		CD_CanRecover = false;
	TimePassed_SinceAttackStop = 0.0f;

	SetActorEnableCollision(bAttackOn);
	
	SetTaserForkAttached(true);
}

void AWeaponTaser::BeginPlay()
{
	Super::BeginPlay();

	TaserFork_OriginalRelativeLocation = TaserForkMesh->GetRelativeLocation();
	TaserFork_OriginalRelativeRotation = TaserForkMesh->GetRelativeRotation();
	TaserFork_OriginalRelativeScale = TaserForkMesh->GetRelativeScale3D();
}

void AWeaponTaser::OnRep_bAttackOn()
{
	Super::OnRep_bAttackOn();

	if (bAttackOn)
	{
		AttackOnEffect_TaserFork->Activate();
		TaserForkMesh->SetRelativeScale3D(TaserFork_OriginalRelativeScale * Ratio_ScaleUpOnRelativeScale);
		SetTaserForkAttached(false);
	}
	else
	{
		TimePassed_SinceAttackStop = 0.0f;
		AttackOnEffect_TaserFork->Deactivate();
		SetTaserForkAttached(true);

		TaserFork_WorldLocation_WhenAttackStop = TaserForkMesh->GetRelativeLocation();
		TaserFork_WorldRotation_WhenAttackStop = TaserForkMesh->GetRelativeRotation();
	}
}

void AWeaponTaser::OnAttackOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
	class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Server_ActorBeingHit || OtherActor == GetOwner())
		return;

	// if held by a character without any problem
	if (IsPickedUp && GetOwner())
	{
		// if hit characters / minigame objects
		if (Cast<AMCharacter>(OtherActor) || Cast<AMinigameMainObjective>(OtherActor))
		{
			bool bTargetCanBeAttacked = true;
			// Check if this character can be attacked
			if (auto pCharacterBeingHit = Cast<AMCharacter>(OtherActor))
			{
				// if it hits the teammates
				if(pCharacterBeingHit != GetOwner())
				{
					int TeammateCheckResult = ADamageManager::IsTeammate(GetOwner(), pCharacterBeingHit);
					if (TeammateCheckResult == -1)
						return;
					else if (TeammateCheckResult == 1)
						bTargetCanBeAttacked = false;	
				
				}
				// if it hits an invincible character
				if (pCharacterBeingHit->IsInvincible)
					bTargetCanBeAttacked = false;
			}
			// Check if this minigame can be attacked
			else if(auto pMinigameEnemyBeingHit = Cast<AMinigameObj_Enemy>(OtherActor))
			{
				if (!ADamageManager::CanApplyDamageToEnemyCrab(pMinigameEnemyBeingHit->SpecificWeaponClass, WeaponType))
				{
					bTargetCanBeAttacked = false;
					pMinigameEnemyBeingHit->NetMulticast_ShowNoDamageHint(HoldingController, 0.5f * (pMinigameEnemyBeingHit->CrabCenterMesh->GetComponentLocation() + AttackDetectComponent->GetComponentLocation()));
				}					
			}
			else if (auto pMinigameStatueBeingHit = Cast<AMinigameObj_Statue>(OtherActor))
			{
				bTargetCanBeAttacked = false;
			}

			if (!bTargetCanBeAttacked)
			{
				AttackStop();
				return;
			}
			else
			{
				Server_ActorBeingHit = OtherActor;
				if (Cast<AMCharacter>(OtherActor) || Cast<AMinigameObj_TrainingRobot>(OtherActor))
				{
					// Apply paralysis buff
					ADamageManager::AddBuffPoints(WeaponType, EnumAttackBuff::Paralysis, HoldingController, OtherActor, 1.0f);
					Server_ActorBeingHit_To_TaserFork_WhenHit = FVector::Zero();
				}
				else if (auto pMiniGameObjectBeingHit = Cast<AMinigameObj_Enemy>(OtherActor))
				{
					FVector CrabCenterToTaserFork = TaserForkMesh->GetComponentLocation() - pMiniGameObjectBeingHit->CrabCenterMesh->GetComponentLocation();
					// Adjust location to get it closer to the crab
					TaserForkMesh->SetWorldLocation(pMiniGameObjectBeingHit->CrabCenterMesh->GetComponentLocation() + CrabCenterToTaserFork * 0.5f);
					// Server_ActorBeingHit can be set to null in overlapEnd after SetWorldLocation, which is an unintended situation, but must be dealt with
					if (!Server_ActorBeingHit)
					{
						AttackStop();
						return;
					}
				}
				Server_ActorBeingHit_To_WeaponMesh_WhenHit = GetActorLocation() - Server_ActorBeingHit->GetActorLocation();
				Server_TaserForkRotationYaw_WhenHit = TaserForkMesh->GetRelativeRotation().Yaw;

				if (!AttackObjectMap.Contains(OtherActor))
					AttackObjectMap.Add(OtherActor);
				AttackObjectMap[OtherActor] = 0.0f;
				bAttackOverlap = true;
				// Listen server
				if (GetNetMode() == NM_ListenServer)
					OnRep_bAttackOverlap();

				bool HasAppliedDamageToSth = false;
				for (auto& Elem : ApplyDamageCounter)
				{
					if (Elem.Value != 0)
						HasAppliedDamageToSth = true;
				}					
				if (!HasAppliedDamageToSth && HoldingController)
				{
					ADamageManager::TryApplyDamageToAnActor(this, HoldingController, UMeleeDamageType::StaticClass(), OtherActor, 0);
					ADamageManager::ApplyOneTimeBuff(WeaponType, EnumAttackBuff::Knockback, HoldingController, OtherActor, 0);
					if (!ApplyDamageCounter.Contains(OtherActor))
						ApplyDamageCounter.Add(OtherActor);
					ApplyDamageCounter[OtherActor]++;
				}
			}
		}
		// if hits something other than characters and minigame objects
		else
		{			
			// if hits weapon/projectiles, penetrate, otherwise(building, rocks, etc) goes back
			if (!Cast<ABaseWeapon>(OtherActor) && !Cast<ABaseProjectile>(OtherActor))
				AttackStop();
		}
	}
}

/*
For taser, we deal it specially.Even if Overlap Ends(Fork may get off the damaged actor),
we still keep the damaged actor in the AttackObjectMap in order to keep applying damage
*/
void AWeaponTaser::OnAttackOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
	class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (Server_ActorBeingHit == OtherActor)
	{
		Server_ActorBeingHit = nullptr;
		if (Cast<AMCharacter>(OtherActor) || Cast<AMinigameObj_TrainingRobot>(OtherActor))
			ADamageManager::AddBuffPoints(WeaponType, EnumAttackBuff::Paralysis, HoldingController, OtherActor, -1.0f);
	}

	if (AttackObjectMap.Contains(OtherActor))
	{
		AttackObjectMap.Remove(OtherActor);
	}
	bAttackOverlap = false;

}


void AWeaponTaser::OnRep_ServerForkWorldTransform()
{
	// We only want to make the transform consistent when the attack is on
	if (bAttackOn)
	{
		TaserForkMesh->SetWorldLocation(ServerForkWorldLocation);
		TaserForkMesh->SetWorldRotation(ServerForkWorldRotation);
	}
}


void AWeaponTaser::OnRep_IsForkOut()
{
	if (IsForkOut)
	{
		ElecWire_NSComponent->Activate();
		ElecWire_NSComponent->SetVisibility(true);
	}
	else
	{
		ElecWire_NSComponent->Deactivate();
		ElecWire_NSComponent->SetVisibility(false);
	}
}


void AWeaponTaser::SetTaserForkAttached(bool bShouldAttachToWeapon)
{
	if (bShouldAttachToWeapon == bForkAttachedToWeapon)
		return;

	// Attach Back to Weapon
	if (bShouldAttachToWeapon)
	{	
		TaserForkMesh->AttachToComponent(WeaponMesh, FAttachmentTransformRules::KeepWorldTransform);
		bForkAttachedToWeapon = true;
	}
	// Detach from Weapon
	else
	{
		// Detach the component from its parent
		TaserForkMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		// Attach the component to the world with absolute position, rotation, and scale
		TaserForkMesh->AttachToComponent(nullptr, FAttachmentTransformRules::KeepWorldTransform);
		bForkAttachedToWeapon = false;
	}
}

void AWeaponTaser::OnRep_IsPickedUp()
{
	Super::OnRep_IsPickedUp();

	if (IsPickedUp)
	{
		//// Show weapon silouette on teammates' end
		//int TeammateCheckResult = ADamageManager::IsTeammate(GetOwner(), GetWorld()->GetFirstPlayerController());
		//if (TeammateCheckResult == 1)
		//{
		//	// Exclude self
		//	if (auto pMCharacter = Cast<AMCharacter>(GetOwner()))
		//	{
		//		if (pMCharacter->GetController() != GetWorld()->GetFirstPlayerController())
		//		{
		//			TaserForkMesh->SetRenderCustomDepth(true);
		//			TaserForkMesh->SetCustomDepthStencilValue(252);
		//		}
		//	}
		//}
	}
	else
	{
		//TaserForkMesh->SetRenderCustomDepth(false);
	}
}