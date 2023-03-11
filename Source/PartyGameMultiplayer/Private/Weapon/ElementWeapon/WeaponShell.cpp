// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ElementWeapon/WeaponShell.h"

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
