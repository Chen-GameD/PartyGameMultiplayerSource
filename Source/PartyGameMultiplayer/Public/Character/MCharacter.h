// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <stack>

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "UI/InventoryMenu.h"
#include "Weapon/BaseWeapon.h"
#include "Weapon/WeaponDataHelper.h"
#include "../Matchmaking/EOSGameInstance.h"
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

	/**	Opponent Marker UI widget */
	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
		class UWidgetComponent* OpponentMarkerWidget;

	/**	Inventory Menu UI widget Reference */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	UInventoryMenu* InventoryMenuWidget;

// Function
// ==============================================================
public:
	// Sets default values for this character's properties
	AMCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void Restart() override;

	virtual void OnRep_PlayerState() override;

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

	/*float AccumulateAttackedBuff(EnumAttackBuff BuffType, float BuffPointsReceived, FVector AttackedDir, 
		AController* EventInstigator, ABaseWeapon* DamageCauser);*/

	/**	Update HealthBar_Enemy UI for character */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetHealthBarUI();

	/**	Add Score/Kill/Death to current character, pass negative value to decrement positive to increment */
	UFUNCTION(BlueprintCallable, Category = "Score")
	void UpdateSKD(int scoreToAdd, int killToAdd, int deathToAdd);

	UFUNCTION(BlueprintCallable)
	bool GetIsDead();

	UFUNCTION(BlueprintCallable)
	void SetIsDead(bool n_IsDead);

	// Server force respawn after character die
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_ForceRespawn(float Delay);
	
	// Virtual function that use to respawn a character, temp call in Blueprint
	UFUNCTION()
	void StartToRespawn(float Delay);

	UFUNCTION(Client, Reliable)
	void Client_Respawn();
	
	UFUNCTION()
		void SetFollowWidgetVisibility(bool IsVisible);
	UFUNCTION()
		void SetFollowWidgetHealthBarIsEnemy(bool IsEnemy);
	UFUNCTION()
		void SetFollowWidgetHealthBarByTeamID(int TeamID);

	UFUNCTION()
	void SetPlayerNameUIInformation();

	UFUNCTION()
	void SetPlayerSkin();

	UFUNCTION(BlueprintImplementableEvent)
	void BPF_SetPlayerSkin();

	UFUNCTION()
	void InitFollowWidget();

	UFUNCTION(BlueprintImplementableEvent)
	void BPF_SetPlayerDirectionIndicatorWidget(int TeamIndex);

	UFUNCTION(BlueprintCallable)
	void SetOutlineEffect(bool isVisible);

	// UFUNCTION()
	// void SetThisCharacterMesh(int TeamIndex);

	UFUNCTION(Server, Reliable)
	void Server_SetCanMove(bool i_CanMove);

	// Check if the buffmap is valid with the input buff or if it can be valid with it after the operation
	bool CheckBuffMap(EnumAttackBuff AttackBuff);

	UFUNCTION()
	float GetCurrentEnergyWeaponUIUpdatePercent();

	UFUNCTION(BlueprintCallable)
	void SetElectricShockAnimState(bool i_state);

	void ActByBuff_PerTick(float DeltaTime);   // This DeltaTime will be from self

	//virtual void FollowWidget_PerTick(float DeltaTime); // This DeltaTime will be from self

	// Multicast die result
	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_DieResult();

	// Multicast respawn result
	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_RespawnResult();

	UFUNCTION(NetMulticast, Reliable)
		void NetMulticast_SetWorldLocationRotation(FVector NewWorldLocation, FRotator NewWorldRotation);

	UFUNCTION(BlueprintImplementableEvent)
	void BPF_DeathCameraAnimation(bool isBroadcast);

	UFUNCTION()
	void ResetCharacterStatus();

	UFUNCTION()
		void OnRep_IsHealing();
	UFUNCTION()
		void OnRep_IsBurned();
	UFUNCTION()
		void OnRep_IsParalyzed();
	UFUNCTION()
		void OnRep_IsInvincible();
	UFUNCTION(Client, Reliable)
		void Client_MoveCharacter(FVector MoveDirection, float SpeedRatio);

	// Effects
	// =============================
	UFUNCTION(NetMulticast, Reliable)
		void NetMulticast_CallGetHitVfx();
	UFUNCTION(NetMulticast, Reliable)
		void NetMulticast_CallGetHitSfx();
	UFUNCTION(BlueprintImplementableEvent)
		void CallGetHitSfx();
	UFUNCTION(BlueprintImplementableEvent)
		void CallDashSfx();
	UFUNCTION(BlueprintImplementableEvent)
		void CallDeathSfx();
	UFUNCTION(BlueprintImplementableEvent)
		void CallBurningBuffStartSfx();
	UFUNCTION(BlueprintImplementableEvent)
		void CallBurningBuffStopSfx();
	UFUNCTION(BlueprintImplementableEvent)
		void CallParalysisBuffStartSfx();
	UFUNCTION(BlueprintImplementableEvent)
		void CallParalysisBuffStopSfx();

	void Server_GiveShellToStatue(class AWeaponShell* pShell);
	void Server_EnableEffectByCrabBubble(bool bEnable);

protected:

	// Health
	// ==========================
	/** RepNotify for changes made to current health.*/
	UFUNCTION()
	void OnRep_CurrentHealth();

	/** Response to health being updated. Called on the server immediately after modification, and on clients in response to a RepNotify*/
	void OnHealthUpdate();

	UFUNCTION()
		void OnRep_IsAllowDash();

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
	void Client_Attack();
	UFUNCTION(Server, Reliable)
	void Attack(float AttackTargetDistance=0.0f);
	DECLARE_DELEGATE_OneParam(FIsMeleeRelease, bool);
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void StopAttack(bool isMeleeRelease);

	DECLARE_DELEGATE_OneParam(FIsZooming, bool);
	void Zoom(bool bZooming);

	/* Called for Server_Dash input */
	UFUNCTION()
	void Dash();
	UFUNCTION(Server, Reliable)
	void Server_Dash();
	void RefreshDash();
	void SetDash();
	void TurnOffDashEffect();

	float Server_GetMaxWalkSpeedRatioByWeapons();

	void Server_SetMaxWalkSpeed();
	UFUNCTION(NetMulticast, Reliable)
		void NetMulticast_AdjustMaxWalkSpeed(float MaxWalkSpeedRatioByWeapon);
	UFUNCTION(NetMulticast, Reliable)
		void NetMulticast_SetHealingBubbleStatus(bool i_bBubbleOn, bool i_bDoubleSize);

	/* Called for Pick Up input */
	DECLARE_DELEGATE_OneParam(FPickUpDelegate, bool);
	UFUNCTION(Server, Reliable)
	void PickUp(bool isLeft);

	UFUNCTION()
	void DropOffWeapon(bool isLeft, bool bDropToReplace = false);

	UFUNCTION()
	void OnCombineWeapon(bool bJustPickedLeft);

	// Test
	// ======================
	// UFUNCTION(Server, Reliable)
	// void Test();

	// UI
	// ======================
	UFUNCTION(Client, Reliable)
	void SetTipUI(bool isShowing, ABaseWeapon* CurrentTouchWeapon = nullptr);

	// Collision
	UFUNCTION(Category = "Weapon")
		void OnWeaponOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
			class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION(Category = "Weapon")
		void OnWeaponOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	UFUNCTION(Category = "Weapon")
		void OnHealingBubbleColliderOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
			class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION(Category = "Weapon")
		void OnHealingBubbleColliderOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

	// Keep Check PlayerFollowWidget is null or not every 0.5 sec;
	// When it not null anymore, start to Init all the pawn related information
	UFUNCTION()
	void CheckPlayerFollowWidgetTick();

	void Server_CheckBubble();
	void EnablebHealingBubble(bool bEnable);

	// Broadcast function
	UFUNCTION()
	void BroadcastToAllController(AActor* AttackActor, bool IsFireBuff);

	void PreventRefreshingCombineWeaponCD_ByDropPick(ABaseWeapon* pCombineWeapon);

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
	// [isSingleMelee, isSingleShooting, isDualMelee, isDualShooting, isDualHeavy, isLeftHeld, isRightHeld, isAttack, isShock]
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated)
	TArray<bool> AnimState = {false, false, false, false, false, false, false, false, false};

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated)
	int DeadAnimIndex = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int DeadAnimNum;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TArray<FName> CharacterMatParamNameArray;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		UMaterialParameterCollection* characaterMaterialParameterCollection;

	// Scores Kill Death Array Format:
	// [Scores, Kill, Death]
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated)
	TArray<int> SKDArray = { 0, 0, 0 };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
		class USphereComponent* HealingBubbleCollider;
	UPROPERTY(EditAnywhere, Category = "Effects")
		class UNiagaraComponent* EffectHealingBubbleOn;
	bool bHealingBubbleOn;
	bool bDoubleHealingBubbleSize;
	//UPROPERTY(Replicated)
		bool bHealingBubbleTouchingStatue;

	UPROPERTY(EditAnywhere, Category = "Effects")
		class UNiagaraComponent* EffectBubbleStart;
	UPROPERTY(EditAnywhere, Category = "Effects")
		class UNiagaraComponent* EffectBubbleOn;
	UPROPERTY(EditAnywhere, Category = "Effects")
		class UNiagaraComponent* EffectBubbleEnd;
	UPROPERTY(EditAnywhere, Category = "Effects")
		class UNiagaraComponent* EffectHeal;
	UPROPERTY(EditAnywhere, Category = "Effects")
		class UNiagaraComponent* EffectGetHit;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
		class UNiagaraComponent* EffectBurn;

	UPROPERTY(EditAnywhere, Category = "Effects")
		class UNiagaraComponent* EffectRun;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
		class UNiagaraComponent* EffectDash;
	UPROPERTY(EditAnywhere, Category = "Effects")
		class UNiagaraComponent* EffectJump;
	UPROPERTY(EditAnywhere, Category = "Effects")
		class UNiagaraComponent* EffectLand;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TArray<USkeletalMesh*> CharacterBPArray;

	float Server_MaxWalkSpeed;
	float Client_LowSpeedWalkAccumulateTime;

	bool bZooming;
	float CurFov;
	float MinFov;
	float MaxFov;	
	float Local_CurCameraArmLength;
	float Local_MinCameraArmLength;
	float Local_MaxCameraArmLength;
	FVector FollowCameraRelativeRotationVector;
	FVector FollowCameraOriginalRelativeLocation;
	float CameraShakingTime;
	float CameraShakingIntensity;
	float TimePassedSinceShaking;

	// Buff Map
	// BuffName: BuffPoints, BuffRemainedTime, BuffAccumulatedTime
	// When the BuffPoints >= 1, the buff will be activated
	TMap<EnumAttackBuff, TArray<float>> BuffMap;
	class AController* Server_TheControllerApplyingLatestBurningBuff;
	FVector KnockbackDirection_SinceLastApplyBuff;
	FVector TaserDragDirection_SinceLastApplyBuff;
	bool BeingKnockbackBeforeThisTick;

	UPROPERTY(ReplicatedUsing = OnRep_IsHealing)
		bool IsHealing;	
	UPROPERTY(ReplicatedUsing = OnRep_IsBurned)
		bool IsBurned;
	UPROPERTY(ReplicatedUsing = OnRep_IsParalyzed)
		bool IsParalyzed;
	FVector Server_Direction_SelfToTaserAttacker;
	UPROPERTY(ReplicatedUsing = OnRep_IsInvincible)
		bool IsInvincible;
	UPROPERTY(Replicated)
		bool IsAffectedByCrabBubble;
	UPROPERTY(Replicated)
		bool IsSaltCure;
	float InvincibleTimer;
	UPROPERTY(EditAnywhere)
		float InvincibleMaxTime;

	TMap<class AController*, float> AttackedRecord;
	
	// Effects
	// =============================
	float Server_CallGetHitSfxVfx_MinInterval;
	float Server_LastTime_CallGetHitSfxVfx;

	// UPROPERTY(EditAnywhere, Category="PlayerFollowWidget Tick Timer")
	// float PlayerFollowWidget_ShowTime = 5.0;
	// UPROPERTY()
	// bool PlayerFollowWidget_NeedDisappear = false;
	// UPROPERTY()
	// float PlayerFollowWidget_RenderOpacity = 1;

	// Fire Image
	UPROPERTY(EditDefaultsOnly, Category = "FireBuff")
	UTexture2D* FireImage;

	// Teammates and Opponents in the same world
	TArray<AMCharacter*> Teammates;
	TArray<AMCharacter*> Opponents;

protected:

	/** The player's maximum health. This is the highest that their health can be, and the value that their health starts at when spawned.*/
	UPROPERTY(EditDefaultsOnly, Category = "Health")
	float MaxHealth;

	/** The player's current health. When reduced to 0, they are considered dead.*/
	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth)
	float CurrentHealth;

	UPROPERTY(Replicated)
	bool IsDead;
	int Server_DeadTimes;

	// Action
	bool IsOnGround;
	float OriginalMaxWalkSpeed;
	float Client_MaxHeightDuringLastTimeOffGround;
	UPROPERTY(ReplicatedUsing = OnRep_IsAllowDash)
	bool IsAllowDash;
	UPROPERTY(EditAnywhere, Category = "Dash")
	float DashDistance;
	UPROPERTY(EditAnywhere, Category = "Dash")
	float DashTime;
	UPROPERTY(EditAnywhere, Category = "Dash")
	float DashCoolDown;
	FTimerHandle DashingTimer;

	// buff
	bool Server_CanMove; // only used on the Server, only for paralysis rn

	// Weapon
	UPROPERTY(EditDefaultsOnly, ReplicatedUsing=SetTextureInUI, Category = "Weapon")
	ABaseWeapon* LeftWeapon;
	
	UPROPERTY(EditDefaultsOnly, ReplicatedUsing=SetTextureInUI, Category = "Weapon")
	ABaseWeapon* RightWeapon;
	
	UPROPERTY(EditDefaultsOnly, ReplicatedUsing=SetTextureInUI, Category = "Weapon")
	ABaseWeapon*  CombineWeapon;

	EnumWeaponType Server_LastDitchCombineWeaponType;
	float Server_LastDitchCombineWeaponTime;
	float Server_LastDitchCombineWeapon_CD_LeftEnergy;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	TArray<ABaseWeapon*>  CurrentTouchedWeapon;

	UPROPERTY(EditAnywhere)
		TSubclassOf<class ACursorHitPlane> CursorHitPlaneSubClass;

	UFUNCTION()
	void SetTextureInUI();

	//Action locker for picking a weapon
	//When we are picking up a weapon, it may be dropped another one.
	//So when we drop it, it collides with the character.
	//So it will sign to CurrentTouchedWeapon and get messed with other logic.
	//So we add this bool to prevent that happen
	bool IsPickingWeapon = false;

	FTimerHandle InitPlayerInformationTimer;

	mutable FCriticalSection DataGuard;
	bool IsAlreadySetPlayerSkin = false;
};
