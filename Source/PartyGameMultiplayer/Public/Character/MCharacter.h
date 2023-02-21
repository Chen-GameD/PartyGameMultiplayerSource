// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <stack>

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "UI/InventoryMenu.h"
#include "Weapon/BaseWeapon.h"
#include "../M_PlayerState.h"
#include "MCharacter.generated.h"

//#define IS_LISTEN_SERVER

UENUM()
enum SkillType { SKILL_DASH };

UCLASS()
class PARTYGAMEMULTIPLAYER_API AMCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
	
	/**	Health Bar UI widget */
	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* PlayerFollowWidget;

	/**	Inventory Menu UI widget Reference */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	UInventoryMenu* InventoryMenuWidget;

// Function
// ==============================================================
public:
	// Sets default values for this character's properties
	AMCharacter();

	virtual void Restart() override;

	/** Property replication */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Returns CameraBoom subobject **/
	class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** Getter for Max Health.*/
	UFUNCTION(BlueprintPure, Category = "Health")
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }

	/** Getter for Current Health.*/
	UFUNCTION(BlueprintPure, Category = "Health")
	FORCEINLINE float GetCurrentHealth() const { return CurrentHealth; }

	/** Setter for Current Health. Clamps the value between 0 and MaxHealth and calls OnHealthUpdate. Should only be called on the server.*/
	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetCurrentHealth(float healthValue);

	/** Event for taking damage. Overridden from APawn.*/
	UFUNCTION(BlueprintCallable, Category = "Health")
	float TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	// customized TakeDamge Function
	float TakeDamageRe(float DamageTaken, EnumWeaponType WeaponType, AController* EventInstigator, ABaseWeapon* DamageCauser);

	/*float AccumulateAttackedBuff(EnumAttackBuff BuffType, float BuffPointsReceived, FVector3d AttackedDir, 
		AController* EventInstigator, ABaseWeapon* DamageCauser);*/

	/**	Update HealthBar UI for character */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetHealthBarUI();

	/**	Add Score/Kill/Death to current character, pass negative value to decrement positive to increment */
	UFUNCTION(BlueprintCallable, Category = "Score")
	void UpdateSKD(int scoreToAdd, int killToAdd, int deathToAdd);

	UFUNCTION(BlueprintCallable)
	bool GetIsDead();

	// Server force respawn after character die
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_ForceRespawn(float Delay);
	
	// Virtual function that use to respawn a character, temp call in Blueprint
	UFUNCTION()
	void StartToRespawn(float Delay);

	UFUNCTION(Client, Reliable)
	void Client_Respawn();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetMesh(USkeletalMesh* i_changeMesh);

	UFUNCTION()
	void SetGameUIVisibility(bool isVisible);

	UFUNCTION()
	void SetLocallyControlledGameUI(bool isVisible);

	UFUNCTION(BlueprintCallable)
	void SetOutlineEffect(bool isVisible);

	UFUNCTION()
	void SetThisCharacterMesh(int TeamIndex);

	UFUNCTION(Server, Reliable)
	void Server_SetCanMove(bool i_CanMove);

	UFUNCTION()
	float GetCurrentEnergyWeaponUIUpdatePercent();
	
protected:

	// Health
	// ==========================
	/** RepNotify for changes made to current health.*/
	UFUNCTION()
	void OnRep_CurrentHealth();

	/** Response to health being updated. Called on the server immediately after modification, and on clients in response to a RepNotify*/
	void OnHealthUpdate();

	UFUNCTION()
	void OnRep_IsOnGround();

	UFUNCTION()
	void OnRep_IsAllowDash();

	// Multicast die result
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_DieResult();

	// Movement
	// ==========================
	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

	// Action
	// ==========================
	/* Called for Attack input */
	UFUNCTION(Server, Reliable)
	void Attack();
	DECLARE_DELEGATE_OneParam(FIsMeleeRelease, bool);
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void StopAttack(bool isMeleeRelease);

	/* Called for Dash input */
	UFUNCTION(Server, Reliable)
	void Dash();
	void RefreshDash();
	void SetDash();
	void TurnOffDashEffect();

	/* Called for Pick Up input */
	DECLARE_DELEGATE_OneParam(FPickUpDelegate, bool);
	UFUNCTION(Server, Reliable)
	void PickUp(bool isLeft);

	UFUNCTION()
	void DropOffWeapon(bool isLeft);

	UFUNCTION()
	void OnCombineWeapon();

	// Test
	// ======================
	// UFUNCTION(Server, Reliable)
	// void Test();

	// UI
	// ======================
	UFUNCTION(Client, Reliable)
	void SetTipUI(bool isShowing);

	// Collision
	UFUNCTION(Category = "Weapon")
	void OnWeaponOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
			class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION(Category = "Weapon")
	void OnWeaponOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

	virtual void ActByBuff(float DeltaTime);

// Members
// ==============================================================

public:	

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Input)
	float TurnRateGamePad;

	// Weapon BP class ref
	// Write it here temporarily, optimize it later
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<TSubclassOf<ABaseWeapon>> weaponArray;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated)
	bool isAttacking = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated)
	bool isHoldingCombineWeapon = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated)
	bool isLeftHeld = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated)
	bool isRightHeld = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated)
	bool isDualShootingWeaponHeld = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated)
	bool isSingleShootingWeaponHeld = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated)
	bool isFlamethrowerHeld = false;

	// Anim state vector format: 
	// [isSingleMelee, isSingleShooting, isDualMelee, isDualShooting, isDualHeavy, isLeftHeld, isRightHeld, isAttack]
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated)
	TArray<bool> AnimState = {false, false, false, false, false, false, false, false};

	// Scores Kill Death Array Format:
	// [Scores, Kill, Death]
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated)
	TArray<int> SKDArray = { 0, 0, 0 };

	UPROPERTY(EditAnywhere, Category = "Effects")
		class UNiagaraComponent* EffectRun;
	UPROPERTY(EditAnywhere, Category = "Effects")
		class UNiagaraComponent* EffectDash;
	UPROPERTY(EditAnywhere, Category = "Effects")
		class UNiagaraComponent* EffectJump;
	UPROPERTY(EditAnywhere, Category = "Effects")
		class UNiagaraComponent* EffectLand;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TArray<USkeletalMesh*> CharacterBPArray;

	// Buff Map
	// BuffName: BuffPoints, BuffRemainedTime
	TMap<EnumAttackBuff, TArray<float>> BuffMap;

protected:

	/** The player's maximum health. This is the highest that their health can be, and the value that their health starts at when spawned.*/
	UPROPERTY(EditDefaultsOnly, Category = "Health")
	float MaxHealth;

	/** The player's current health. When reduced to 0, they are considered dead.*/
	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth)
	float CurrentHealth;

	UPROPERTY(Replicated)
	bool IsDead;

	// Action
	UPROPERTY(ReplicatedUsing = OnRep_IsOnGround)
	bool IsOnGround;
	UPROPERTY(ReplicatedUsing = OnRep_IsAllowDash)
	bool IsAllowDash;
	UPROPERTY(EditAnywhere, Category = "Dash")
	float DashDistance;
	UPROPERTY(EditAnywhere, Category = "Dash")
	float DashTime;
	float OriginalMaxWalkSpeed;
	float DashSpeed;
	FTimerHandle DashingTimer;

	// buff
	bool CanMove; // only used on the Server, only for paralysis rn

	// Weapon
	// to do
	UPROPERTY(EditDefaultsOnly, ReplicatedUsing=SetTextureInUI, Category = "Weapon")
	ABaseWeapon* LeftWeapon;
	
	UPROPERTY(EditDefaultsOnly, ReplicatedUsing=SetTextureInUI, Category = "Weapon")
	ABaseWeapon* RightWeapon;
	
	UPROPERTY(EditDefaultsOnly, ReplicatedUsing=SetTextureInUI, Category = "Weapon")
	ABaseWeapon*  CombineWeapon;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	TArray<ABaseWeapon*>  CurrentTouchedWeapon;

	UFUNCTION()
	void SetTextureInUI();

	//Action locker for picking a weapon
	//When we are picking up a weapon, it may be dropped another one.
	//So when we drop it, it collides with the character.
	//So it will sign to CurrentTouchedWeapon and get messed with other logic.
	//So we add this bool to prevent that happen
	bool IsPickingWeapon = false;
};
