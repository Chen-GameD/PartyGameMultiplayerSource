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
#include "Character/MCharacter.h"


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

	// Currently, they are decided by derived BP
	//MaxLen = 0.0f;
	//StrechOutSpeed = 360.0f;
	//StrechInSpeed = 160.0f;

	IsForkOut = false;

	Server_ActorBeingHit = nullptr;
	bHitTarget = false;
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
			if (!bHitTarget)
			{
				// stretch out to the limit
				//FVector TaserFork_CurRelativeLocation = TaserForkMesh->GetRelativeLocation();
				//if (TaserFork_OriginalRelativeLocation.X - MaxLen <= TaserFork_CurRelativeLocation.X)
				//{
				//	TaserForkMesh->SetRelativeLocation(TaserFork_CurRelativeLocation + DeltaTime * FVector3d(-StrechOutSpeed, 0, 0));
				//}
				FVector TaserFork_CurWorldLocation = TaserForkMesh->GetComponentLocation();
				if (FVector::Distance(TaserFork_CurWorldLocation, TaserFork_WorldLocation_WhenAttackStart) < MaxLen)
				{
					GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("Diffs: %f, %f"), FVector::Distance(TaserFork_CurWorldLocation, TaserFork_WorldLocation_WhenAttackStart), MaxLen));
					TaserForkMesh->SetWorldLocation(TaserFork_CurWorldLocation + DeltaTime * TaserFork_WorldRotation_WhenAttackStart.Vector() * -StrechOutSpeed);
				}
				else
					AttackStop();
			}
			// if hit a target 
			else
			{				
				// change the transform of the TaserFork
				if (Server_ActorBeingHit)
				{
					// location: keep the same offset
					TaserForkMesh->SetWorldLocation(Server_ActorBeingHit->GetActorLocation() + Server_ActorBeingHit_To_TaserFork_WhenHit);
					// Rotation
					FVector Server_ActorBeingHit_To_WeaponMesh_Now = GetActorLocation() - Server_ActorBeingHit->GetActorLocation();
					float Angle = FMath::Acos(FVector::DotProduct(Server_ActorBeingHit_To_WeaponMesh_WhenHit.GetSafeNormal(), Server_ActorBeingHit_To_WeaponMesh_Now.GetSafeNormal())) * (180.0f / PI);
					bool bNowVectorIsOnTheRight = 0 < FVector::DotProduct(FVector::UpVector, FVector::CrossProduct(Server_ActorBeingHit_To_WeaponMesh_WhenHit.GetSafeNormal(), Server_ActorBeingHit_To_WeaponMesh_Now.GetSafeNormal()));
					FRotator CurRotation = TaserForkMesh->GetRelativeRotation();
					CurRotation.Yaw = Server_TaserForkRotationYaw_WhenHit + (bNowVectorIsOnTheRight ? Angle : -Angle);
					TaserForkMesh->SetWorldRotation(CurRotation);
				}
				
				if (AMCharacter* pCharacter = Cast<AMCharacter>(Server_ActorBeingHit))
				{
					// Apply paralysis buff(drag towards the attacker)
					ADamageManager::ApplyOneTimeBuff(WeaponType, EnumAttackBuff::Paralysis, HoldingController, pCharacter, DeltaTime);
					// Stop attack when the character is dead
					if (pCharacter->GetIsDead())
						AttackStop();
				}
				// Stop attack when the MinigameMainObjective is dead
				else if (AMinigameMainObjective* pMinigameMainObjective = Cast<AMinigameMainObjective>(Server_ActorBeingHit))
				{
					if (pMinigameMainObjective->GetCurrentHealth() <= 0)
						AttackStop();
				}
			}
		}
		ServerForkWorldLocation = TaserForkMesh->GetComponentLocation();
		ServerForkWorldRotation = TaserForkMesh->GetComponentRotation();
	}
	
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, (bAttackOn ? TEXT("true") : TEXT("false")));

	// if attack stops, the fork will be back instantly
	if (!bAttackOn)
	{
		if (TaserForkMesh->GetRelativeLocation() != TaserFork_OriginalRelativeLocation ||
			TaserForkMesh->GetRelativeRotation() != TaserFork_OriginalRelativeRotation)
		{
			TaserForkMesh->SetRelativeLocation(TaserFork_OriginalRelativeLocation);
			TaserForkMesh->SetRelativeRotation(TaserFork_OriginalRelativeRotation);
			IsForkOut = false;
		}
		else
		{
			IsForkOut = false;
		}

		if (!IsForkOut)
		{
			if (TaserForkMesh->GetRelativeScale3D() != TaserFork_OriginalRelativeScale)
				TaserForkMesh->SetRelativeScale3D(TaserFork_OriginalRelativeScale);
		}
	}

}

void AWeaponTaser::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeaponTaser, ServerForkWorldLocation);
	DOREPLIFETIME(AWeaponTaser, ServerForkWorldRotation);
	DOREPLIFETIME(AWeaponTaser, bHitTarget);
	DOREPLIFETIME(AWeaponTaser, IsForkOut);
}



void AWeaponTaser::AttackStart()
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
	ApplyDamageCounter = 0;

	SetActorEnableCollision(bAttackOn);

	TaserForkMesh->SetRelativeScale3D(TaserFork_OriginalRelativeScale * Ratio_ScaleUpOnRelativeScale);
	TaserFork_WorldLocation_WhenAttackStart = TaserForkMesh->GetComponentLocation();
	TaserFork_WorldRotation_WhenAttackStart = TaserForkMesh->GetComponentRotation();
	IsForkOut = true;
	SetTaserForkAttached(false);
}

void AWeaponTaser::AttackStop()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Enter Taser AttackStop"));
	bAttackOn = false;
	// Listen server
	if (GetNetMode() == NM_ListenServer)
	{
		OnRep_bAttackOn();
	}
	ApplyDamageCounter = 0;
	for (auto& Elem : AttackObjectMap)
	{
		if (auto pMCharacter = Cast<AMCharacter>(Elem.Key))
		{
			if(pMCharacter == Server_ActorBeingHit && pMCharacter->CheckBuffMap(EnumAttackBuff::Paralysis))
				pMCharacter->BuffMap[EnumAttackBuff::Paralysis][0] -= 1.0f;
		}
	}
	AttackObjectMap.Empty();

	if (AttackType == EnumAttackType::Constant)
		CD_CanRecover = false;
	TimePassed_SinceAttackStop = 0.0f;

	SetActorEnableCollision(bAttackOn);
	
	SetTaserForkAttached(true);
	bHitTarget = false;
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, (bAttackOn ? TEXT("AttackStopEnding: true") : TEXT("AttackStopEnding: false")));
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
	if (bHitTarget)
		return;

	if (IsPickedUp && GetOwner())
	{
		if ((Cast<AMCharacter>(OtherActor) && OtherActor != GetOwner()) ||
			Cast<AMinigameMainObjective>(OtherActor))
		{
			// if it is AMCharacter
			if (auto pCharacterBeingHit = Cast<AMCharacter>(OtherActor))
			{
				// Check if it hits teammates
				auto MyController = HoldingController;
				if (!MyController)
					return;
				AM_PlayerState* MyPS = MyController->GetPlayerState<AM_PlayerState>();
				AM_PlayerState* TheOtherCharacterPS = pCharacterBeingHit->GetPlayerState<AM_PlayerState>();
				if (!MyPS || !TheOtherCharacterPS || MyPS->TeamIndex == TheOtherCharacterPS->TeamIndex)
				{
					AttackStop();
					return;
				}
				// Apply paralysis buff
				if (pCharacterBeingHit->CheckBuffMap(EnumAttackBuff::Paralysis))
					pCharacterBeingHit->BuffMap[EnumAttackBuff::Paralysis][0] += 1.0f;
			}

			bHitTarget = true;
			Server_ActorBeingHit = OtherActor;
			Server_ActorBeingHit_To_TaserFork_WhenHit = TaserForkMesh->GetComponentLocation() - Server_ActorBeingHit->GetActorLocation();
			Server_ActorBeingHit_To_WeaponMesh_WhenHit = GetActorLocation() - Server_ActorBeingHit->GetActorLocation();
			Server_TaserForkRotationYaw_WhenHit = TaserForkMesh->GetRelativeRotation().Yaw;

			//ServerTaserForkWorldLocation_WhenFirstHitTarget = TaserForkMesh->GetComponentLocation();
			//ServerTaserForkWorldRotation_WhenFirstHitTarget = TaserForkMesh->GetComponentRotation();

			if (!AttackObjectMap.Contains(OtherActor))
				AttackObjectMap.Add(OtherActor);
			AttackObjectMap[OtherActor] = 0.0f;
			bAttackOverlap = true;
			// Listen server
			if (GetNetMode() == NM_ListenServer)
			{
				OnRep_bAttackOverlap();
			}

			if (ApplyDamageCounter == 0 && HoldingController)
			{
				ADamageManager::TryApplyDamageToAnActor(this, HoldingController, UMeleeDamageType::StaticClass(), OtherActor, 0);
				ADamageManager::ApplyOneTimeBuff(WeaponType, EnumAttackBuff::Knockback, HoldingController, Cast<AMCharacter>(OtherActor), 0);
				ApplyDamageCounter++;
			}
		}
		else
		{
			// if hit something other than the following(like building, rocks, etc), the attack should stop
			if (!Cast<AMCharacter>(OtherActor) && !Cast<ABaseWeapon>(OtherActor) && !Cast<ABaseProjectile>(OtherActor))
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
	//if (IsPickedUp && GetOwner())
	//{
	//	if ((Cast<ACharacter>(OtherActor) && OtherActor != GetOwner()) ||
	//		Cast<AMinigameMainObjective>(OtherActor))
	//	{
	//		if (AttackObjectMap.Contains(OtherActor))
	//		{
	//			AttackObjectMap.Remove(OtherActor);
	//		}
	//		bAttackOverlap = false;
	//	}
	//}
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
	/*if (IsForkOut)
	{
		if (bForkAttachedToWeapon)
			SetTaserForkAttached(false);
	}
	else
	{
		if (!bForkAttachedToWeapon)
			SetTaserForkAttached(true);
	}*/
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