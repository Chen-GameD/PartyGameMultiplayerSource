#pragma once

#include <vector>

#include "CoreMinimal.h"
#include "Weapon/BaseWeapon.h"
#include "Weapon/CombinedWeapon/WeaponFlamethrower.h"
#include "Weapon/CombinedWeapon/WeaponTaser.h"
#include "Weapon/ElementWeapon/WeaponBlower.h"
#include "Weapon/WeaponConfig.h"

namespace AnimUtils {
	void updateShootingWeaponHeldState(bool& o_isDualShootingWeaponHeld, bool& o_isSingleShootingWeaponHeld, bool& o_isFlamethrowerHeld, 
		ABaseWeapon* i_CombineWeapon, ABaseWeapon* i_LeftWeapon, ABaseWeapon* i_RightWeapon) {

		o_isDualShootingWeaponHeld = false;
		o_isSingleShootingWeaponHeld = false;
		o_isFlamethrowerHeld = false;

		if (i_CombineWeapon) {
			// Check Taser
			auto Taser = Cast<AWeaponTaser>(i_CombineWeapon);
			if (Taser) {
				o_isSingleShootingWeaponHeld = true;
			}
			// Check Flamethrower
			else {
				auto FlameThrower = Cast<AWeaponFlamethrower>(i_CombineWeapon);
				if (FlameThrower) {
					o_isFlamethrowerHeld = true;
				}
			}
		}
		else {
			// Dual Hairdryer
			if (i_LeftWeapon && i_RightWeapon) {
				auto leftCast = Cast<AWeaponBlower>(i_LeftWeapon);
				auto rightCast = Cast<AWeaponBlower>(i_RightWeapon);
				if (leftCast && rightCast) {
					o_isDualShootingWeaponHeld = true;
				}
			}
			else if (i_LeftWeapon) {
				auto leftCast = Cast<AWeaponBlower>(i_LeftWeapon);
				if (leftCast) {
					o_isSingleShootingWeaponHeld = true;
				}
			}
			else if (i_RightWeapon) {
				auto rightCast = Cast<AWeaponBlower>(i_RightWeapon);
				if (rightCast) {
					o_isSingleShootingWeaponHeld = true;
				}
			}
		}

	}

	void clearAnimStateWeaponType(TArray<bool>& i_AnimState) {
		// Set First 5 state (weapon type states) to false
		for (int i = 0; i < 5; i++) {
			i_AnimState[i] = false;
		}
	}

	void updateAnimStateWeaponType(TArray<bool>& i_AnimState,
		ABaseWeapon* i_CombineWeapon, ABaseWeapon* i_LeftWeapon, ABaseWeapon* i_RightWeapon) {

		// Reset State
		clearAnimStateWeaponType(i_AnimState);

		if (i_CombineWeapon) 
		{
			auto combineWeaponType = i_CombineWeapon->WeaponType;
			int indexToSet = WeaponConfig::GetInstance()->GetAnimStateIndex(combineWeaponType);
			i_AnimState[indexToSet] = true;
		}
		else 
		{
			bool LeftShellInTwoWeapons = false;
			bool RightShellInTwoWeapons = false;
			// Holding 2 weapons
			if (i_LeftWeapon && i_RightWeapon)
			{
				// No shells(2 same weapons)
				if (i_LeftWeapon->WeaponType != EnumWeaponType::Shell && i_RightWeapon->WeaponType != EnumWeaponType::Shell)
				{
					int doubleShootIndex = 3;
					int doubleMeleeIndex = 2;

					auto leftType = i_LeftWeapon->WeaponType;
					auto rightType = i_RightWeapon->WeaponType;

					int leftIndexToSet = WeaponConfig::GetInstance()->GetAnimStateIndex(leftType);
					int rightIndexToSet = WeaponConfig::GetInstance()->GetAnimStateIndex(rightType);

					// Note: There will only be two cases since otherwise it will combine:
					//	1. Double Shooting
					//	2. Double Melee
					// If both single shooting which means Double Shooting
					if (leftIndexToSet && rightIndexToSet)
					{
						i_AnimState[doubleShootIndex] = true;
					}
					// Double Melee
					else
					{
						i_AnimState[doubleMeleeIndex] = true;
					}
				}
				// Both are shells
				else if (i_LeftWeapon->WeaponType == EnumWeaponType::Shell && i_RightWeapon->WeaponType == EnumWeaponType::Shell)
				{
					//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("anim 4"));
					//int doubleShootIndex = 3;
					//int doubleMeleeIndex = 2;

					//i_AnimState[doubleShootIndex] = true;
					////i_AnimState[doubleMeleeIndex] = true;

					i_AnimState[0] = true;
					i_AnimState[5] = false;
				}
				// Only left is shell
				else if (i_LeftWeapon->WeaponType == EnumWeaponType::Shell)
					LeftShellInTwoWeapons = true;
				// Only right is shell
				else if (i_RightWeapon->WeaponType == EnumWeaponType::Shell)
					RightShellInTwoWeapons = true;
			}

			// holding only left weapon / holding left weapon and right shell
			if ((i_LeftWeapon && i_LeftWeapon->WeaponType != EnumWeaponType::Shell && !i_RightWeapon) || RightShellInTwoWeapons)
			{
				auto leftType = i_LeftWeapon->WeaponType;
				int leftIndexToSet = WeaponConfig::GetInstance()->GetAnimStateIndex(leftType);
				i_AnimState[leftIndexToSet] = true;
				i_AnimState[6] = false;
			}
			// holding only right weapon / holding right weapon and left shell
			if ((i_RightWeapon && i_RightWeapon->WeaponType != EnumWeaponType::Shell && !i_LeftWeapon) || LeftShellInTwoWeapons)
			{
				auto rightType = i_RightWeapon->WeaponType;
				int rightIndexToSet = WeaponConfig::GetInstance()->GetAnimStateIndex(rightType);
				i_AnimState[rightIndexToSet] = true;
				i_AnimState[5] = false;
			}

			// holding only left shell
			if (i_LeftWeapon && i_LeftWeapon->WeaponType == EnumWeaponType::Shell && !i_RightWeapon)
			{
				i_AnimState[1] = true;
				i_AnimState[5] = true;
			}
			// holding only right shell
			if (i_RightWeapon && i_RightWeapon->WeaponType == EnumWeaponType::Shell && !i_LeftWeapon)
			{
				i_AnimState[1] = true;
				i_AnimState[6] = true;
			}
		}
	}
}