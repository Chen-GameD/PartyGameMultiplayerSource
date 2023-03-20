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

void AWeaponShell::GetPickedUp(ACharacter* pCharacter)
{
	Super::GetPickedUp(pCharacter);

	PreHoldingController = pCharacter->GetController();
}

AController* AWeaponShell::GetPreHoldingController()
{
	return PreHoldingController;
}
