// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ElementWeapon/WeaponShell.h"

#include "GameFramework/Character.h"

#include "Character/MCharacter.h"
#include "LevelInteraction/MinigameObject/MinigameObj_Statue.h"

AWeaponShell::AWeaponShell()
{
	IsCombineWeapon = false;
	WeaponType = EnumWeaponType::Shell;

	//static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("/Game/ArtAssets/Models/Fork/Fork.Fork"));
	//if (DefaultMesh.Succeeded())
	//{
	//	WeaponMesh->SetStaticMesh(DefaultMesh.Object);
	//}

	AttackDetectComponent = WeaponMesh;

	Server_bDetectedByStatue = false;
}

void AWeaponShell::BeginPlay()
{
	Super::BeginPlay();

	FVector spawnLocation = GetActorLocation();
	FRotator spawnRotation = FRotator();
	FActorSpawnParameters spawnParameters;
	spawnParameters.Instigator = GetInstigator();
	spawnParameters.Owner = this;
	pShellSpotLight = GetWorld()->SpawnActor<AShellSpotLight>(SpecificShellSpotLightClass, spawnLocation, spawnRotation, spawnParameters);
	if (pShellSpotLight)
	{
		//pShellSpotLight->SourceLocation = FVector(20.0f, -390.0f, 1000.0f);
		pShellSpotLight->SourceLocation = FVector(20.0f, -390.0f, 380.0f);
		pShellSpotLight->TargetActor = this;		
	}
}

void AWeaponShell::AttackStart(float AttackTargetDistance)
{
	// Override to disable the SetActorEnableCollision in parent class function
}


void AWeaponShell::AttackStop()
{
	// Override to disable the SetActorEnableCollision in parent class function
}

void AWeaponShell::OnAttackOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
	class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
}

void AWeaponShell::Destroyed()
{
	if (pShellSpotLight)
		pShellSpotLight->Destroy();
}

void AWeaponShell::GetPickedUp(ACharacter* pCharacter)
{
	Super::GetPickedUp(pCharacter);

	SetActorEnableCollision(true);
	PreHoldingController = pCharacter->GetController();
}

void AWeaponShell::GetThrewAway()
{
	IsPickedUp = false;
	if (GetNetMode() == NM_ListenServer)
		OnRep_IsPickedUp();
	HasBeenCombined = false;
	HoldingController = nullptr;
	SetInstigator(nullptr);
	SetOwner(nullptr);

	if(!Server_bDetectedByStatue)
		DisplayCaseCollisionSetActive(true);
}

AController* AWeaponShell::GetPreHoldingController()
{
	return PreHoldingController;
}

void AWeaponShell::UpdateScoreCanGet(int N_Score)
{
	ScoreCanGet = N_Score;
}

int AWeaponShell::GetScoreCanGet()
{
	return ScoreCanGet;
}

void AWeaponShell::UpdateConfigIndex(int N_Index)
{
	ConfigIndex = N_Index;
}

int AWeaponShell::GetConfigIndex()
{
	return ConfigIndex;
}
