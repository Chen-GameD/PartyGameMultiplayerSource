// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon/CombinedWeapon/WeaponTaser.h"

#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/PrimitiveComponent.h"
#include "Net/UnrealNetwork.h"

#include "Weapon/BaseProjectile.h"
#include "Weapon/DamageManager.h"
#include "Weapon/DamageType/MeleeDamageType.h"
#include "LevelInteraction/MinigameMainObjective.h"


AWeaponTaser::AWeaponTaser()
{
	IsCombineWeapon = true;
	WeaponType = EnumWeaponType::Taser;

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

	// Currently, they are decided by derived BP
	//originalX = TaserForkMesh->GetRelativeLocation().X;
	//MaxLen = 0.0f;
	//StrechOutSpeed = 360.0f;
	//StrechInSpeed = 160.0f;

	bHitTarget = false;
	bForkAttachedToWeapon = true;
}


void AWeaponTaser::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Server
	if (GetLocalRole() == ROLE_Authority)
	{
		FVector TaserFork_CurRelativeLocation = TaserForkMesh->GetRelativeLocation();
		// if attack on and not hit a target, the server fork would stretch out and the client fork would copy the location 
		if (bAttackOn && !bHitTarget)
		{
			// stretch out to the limit
			if (TaserFork_OriginalRelativeLocation.X - MaxLen <= TaserFork_CurRelativeLocation.X)
			{
				TaserForkMesh->SetRelativeLocation(TaserFork_CurRelativeLocation + DeltaTime * FVector3d(-StrechOutSpeed, 0, 0));
			}
			else
				AttackStop();
		}
		
		ServerForkWorldLocation = TaserForkMesh->GetComponentLocation();
		ServerForkWorldRotation = TaserForkMesh->GetComponentRotation();
	}
	// if attack stops, both the clients and server should stretch in. Now we don't need to keep them identical.
	if (!bAttackOn)
	{
		FVector TaserFork_CurRelativeLocation = TaserForkMesh->GetRelativeLocation();
		if (TaserFork_CurRelativeLocation.X < TaserFork_OriginalRelativeLocation.X)
		{
			TaserForkMesh->SetRelativeLocation(TaserFork_CurRelativeLocation + DeltaTime * FVector3d(StrechInSpeed, 0, 0));
		}
	}
	// Server
	if (GetLocalRole() == ROLE_Authority)
	{
		ServerForkWorldLocation = TaserForkMesh->GetComponentLocation();
		ServerForkWorldRotation = TaserForkMesh->GetComponentRotation();
	}
		
}

void AWeaponTaser::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeaponTaser, ServerForkWorldLocation);
	DOREPLIFETIME(AWeaponTaser, ServerForkWorldRotation);
	DOREPLIFETIME(AWeaponTaser, bHitTarget);
}



void AWeaponTaser::AttackStart()
{
	Super::AttackStart();
	//bShouldStretchOut = true;
}

void AWeaponTaser::AttackStop()
{
	Super::AttackStop();

	//bShouldStretchOut = false;
	bHitTarget = false;
	OnRep_bHitTarget();
}

void AWeaponTaser::BeginPlay()
{
	Super::BeginPlay();

	TaserFork_OriginalRelativeLocation = TaserForkMesh->GetRelativeLocation();
	TaserFork_OriginalRelativeRotation = TaserForkMesh->GetRelativeRotation();
	TaserFork_OriginalRelativeScale = TaserForkMesh->GetRelativeScale3D();
}

void AWeaponTaser::OnAttackOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
	class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bHitTarget)
		return;

	if (IsPickedUp && GetOwner())
	{
		if ((Cast<ACharacter>(OtherActor) && OtherActor != GetOwner()) ||
			Cast<AMinigameMainObjective>(OtherActor))
		{
			// Check if it hits teammates
			if (auto pCharacterBeingHit = Cast<ACharacter>(OtherActor))
			{
				auto MyController = HoldingController;
				if (!MyController)
					return;
				AM_PlayerState* MyPS = MyController->GetPlayerState<AM_PlayerState>();
				AM_PlayerState* TheOtherCharacterPS = pCharacterBeingHit->GetPlayerState<AM_PlayerState>();

				if (!MyPS || !TheOtherCharacterPS || MyPS->TeamIndex == TheOtherCharacterPS->TeamIndex)
					return;
			}			

			bHitTarget = true;
			OnRep_bHitTarget();

			TaserForkWorldLocation_WhenFirstHitTarget = TaserForkMesh->GetComponentLocation();
			TaserForkWorldRotation_WhenFirstHitTarget = TaserForkMesh->GetComponentRotation();

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
				ADamageManager::TryApplyDamageToAnActor(this, HoldingController, UMeleeDamageType::StaticClass(), OtherActor);
				ApplyDamageCounter++;
			}
		}
		else
		{
			// if hit something other than the following(like building, rocks, etc), the attack should stop
			if (!Cast<ACharacter>(OtherActor) && !Cast<ABaseWeapon>(OtherActor) && !Cast<ABaseProjectile>(OtherActor))
				AttackStop();
		}
	}
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

void AWeaponTaser::OnRep_bHitTarget()
{
	if (bHitTarget)
	{
		if (bForkAttachedToWeapon)
			SetTaserForkAttached(false);
	}
	else
	{
		if (!bForkAttachedToWeapon)
			SetTaserForkAttached(true);
	}
}




void AWeaponTaser::SetTaserForkAttached(bool bShouldAttachToWeapon)
{
	// Attach Back to Weapon
	if(bShouldAttachToWeapon)
	{
		TaserForkMesh->AttachToComponent(WeaponMesh, FAttachmentTransformRules::KeepWorldTransform);
		TaserForkMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
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