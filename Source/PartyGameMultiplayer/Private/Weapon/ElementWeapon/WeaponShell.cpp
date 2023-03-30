// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ElementWeapon/WeaponShell.h"

#include "GameFramework/Character.h"

AWeaponShell::AWeaponShell()
{
	IsCombineWeapon = false;
	WeaponType = EnumWeaponType::Shell;

	//static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("/Game/ArtAssets/Models/Fork/Fork.Fork"));
	//if (DefaultMesh.Succeeded())
	//{
	//	WeaponMesh->SetStaticMesh(DefaultMesh.Object);
	//}
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

void AWeaponShell::Destroyed()
{
	if (pShellSpotLight)
		pShellSpotLight->Destroy();
}

void AWeaponShell::GetPickedUp(ACharacter* pCharacter)
{
	Super::GetPickedUp(pCharacter);

	PreHoldingController = pCharacter->GetController();
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
