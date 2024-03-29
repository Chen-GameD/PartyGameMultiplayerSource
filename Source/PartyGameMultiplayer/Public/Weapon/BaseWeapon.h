// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Weapon/WeaponDataHelper.h"

#include "BaseWeapon.generated.h"


UCLASS(Abstract)
class PARTYGAMEMULTIPLAYER_API ABaseWeapon : public AActor
{
	GENERATED_BODY()

/* MEMBER METHODS */
public:
	ABaseWeapon();

	virtual void Tick(float DeltaTime) override;

	/** Property replication */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// should only be called on server
	UFUNCTION(BlueprintCallable)
		virtual void GetPickedUp(ACharacter* pCharacter);
	// should only be called on server
	UFUNCTION(BlueprintCallable)
		virtual void GetThrewAway();
	virtual void SafeDestroyWhenGetThrew();
	// should only be called on server
	virtual void AttackStart(float AttackTargetDistance);
	// should only be called on server
	virtual void AttackStop();
	//Get weapon name
	virtual FString GetWeaponName();
	// Get Weapon Holder
	UFUNCTION(BlueprintCallable)
	AController* GetHoldingController() const;

	// Effects
	// ====================
	UFUNCTION(BlueprintImplementableEvent)
		void CallAttackStartSfx();
	UFUNCTION(BlueprintImplementableEvent)
		void CallAttackStopSfx();
	UFUNCTION(NetMulticast, Reliable)
		void NetMulticast_CallPickedUpSfx();
	UFUNCTION(BlueprintImplementableEvent)
		void CallPickedUpSfx();
	UFUNCTION(NetMulticast, Reliable)
		void NetMulticast_CallThrewAwaySfx();
	UFUNCTION(BlueprintImplementableEvent)
		void CallThrewAwaySfx();
	UFUNCTION(NetMulticast, Reliable)
		void NetMulticast_CallShowUpVfx();
	void CallShowUpVfx();

protected:
	virtual void BeginPlay() override;
	virtual void Destroyed() override; 
	virtual void PlayAnimationWhenNotBeingPickedUp(float DeltaTime);
	// should be only on client
	virtual void DisplayCaseCollisionSetActive(bool IsActive);
	// should be only on client
	virtual void GenerateAttackHitEffect();
	// should only be called on server
	virtual void SpawnProjectile(float AttackTargetDistance);

	/* RepNotify Functions */
	UFUNCTION()
		virtual void OnRep_DisplayCaseTransform();
	UFUNCTION()
		virtual void OnRep_bAttackOn();
	UFUNCTION()
		virtual void OnRep_bAttackOverlap();
	UFUNCTION()
		virtual void OnRep_IsPickedUp();	
	//UFUNCTION()
	//	virtual void OnRep_DamageGenerationCounter();

	// only is called on server, deal with damage applied by the AttackDetectComponent
	UFUNCTION(Category = "Weapon")
		virtual void OnAttackOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
			class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	// only is called on server
	UFUNCTION(Category = "Weapon")
		virtual void OnAttackOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
			class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	// only is called on server(For test, will not be used in the actual play)
	UFUNCTION(Category = "Weapon")
		virtual void OnDisplayCaseOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
			class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:


/* MEMBER VARIABLES */
public:
	EnumWeaponType WeaponType;
	EnumAttackType AttackType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsBigWeapon;
	bool IsCombineWeapon;  // if it is a combine type weapon or not
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "HoldingStatus")
	bool HasBeenCombined; // if the weapon has been combined (to a combine type weapon)
	UPROPERTY(EditAnywhere, Category = "Effects")
	UTexture2D* HoldingTextureUI_E;
	UPROPERTY(EditAnywhere, Category = "Effects")
	UTexture2D* HoldingTextureUI_Q;
	UPROPERTY(EditAnywhere, Category = "Effects")
	UTexture2D* PickUpTextureUI_E;
	UPROPERTY(EditAnywhere, Category = "Effects")
	UTexture2D* PickUpTextureUI_Q;
	UPROPERTY(EditAnywhere, Category = "Effects")
	UTexture2D* WeaponImage_Broadcast;
	UPROPERTY(EditAnywhere, Category = "Effects")
	UTexture2D* WeaponImage_Message;
	// Ele: short for Element
	ABaseWeapon* EleWeapon_1;
	ABaseWeapon* EleWeapon_2;

	// Damage related
	//float Damage;
	//float MiniGameDamage;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Damage")
		TSubclassOf<class UDamageType> DamageType;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Damage")
		TSubclassOf<class UDamageType> MiniGameDamageType;
	// CoolDown related(CD_LeftEnergy needs to replicate so the client can show the correct cd UI)
	float CD_MaxEnergy;
	float CD_MinEnergyToAttak;
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "CoolDown")
	float CD_LeftEnergy;	
	float CD_DropSpeed;
	float CD_RecoverSpeed;
	float CD_RecoverDelay;
	bool CD_CanRecover;
	float TimePassed_SinceAttackStop;
	float TimePassed_SinceGetThrewAway;
	float LastTime_DisplayCaseTransformBeenReplicated;

	UPROPERTY(ReplicatedUsing = OnRep_IsPickedUp)
		bool IsPickedUp;
	bool Server_BigWeaponShouldSink;

	float CurDeltaTime;

	UPROPERTY()
	AController* PreHoldingController;

protected:
	// Might be necessary if there are multiple weapons of the same type
	size_t ID;

	AController* HoldingController;

	UPROPERTY(ReplicatedUsing = OnRep_DisplayCaseTransform)
		FVector DisplayCaseLocation;
	UPROPERTY(ReplicatedUsing = OnRep_DisplayCaseTransform)
		FRotator DisplayCaseRotation;
	UPROPERTY(ReplicatedUsing = OnRep_DisplayCaseTransform)
		FVector DisplayCaseScale;

	FVector WeaponMeshDefaultRelativeLocation;
	FRotator WeaponMeshDefaultRelativeRotation;
	FVector WeaponMeshDefaultRelativeScale;

	// check if ApplyDamage has happend during one AttackOn round, if happened, OneHit type weapon won't apply damage again.
	TMap<AActor*, int> ApplyDamageCounter;

	UPROPERTY(ReplicatedUsing = OnRep_bAttackOn)
		bool bAttackOn;

	UPROPERTY(ReplicatedUsing = OnRep_bAttackOverlap)
		bool bAttackOverlap;

	//UPROPERTY(ReplicatedUsing = OnRep_DamageGenerationCounter)
	//	unsigned int DamageGenerationCounter;

	// Which actor is being attacked - how long they have been attacked
	TMap<AActor*, float> AttackObjectMap;

	/**
		We precede each of the types in these declarations with the class keyword.
		This makes each of them a forward declaration of their own classes
		in addition to being variable declarations, (chronological order: declare -> use -> define)
		which ensures that their classes will be recognized within the header file.
	*/

	// Sphere component used to make sure the weapon could be on the ground after threw away.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		class UBoxComponent* DisplayCase;

	// Used to seperate weapons
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		class UStaticMeshComponent* InnerCase;

	// Static Mesh used to provide a visual representation of the object.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		class UStaticMeshComponent* WeaponMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		class UStaticMeshComponent* SpawnProjectilePointMesh;

	/*	PrimitiveComponent(has OnWeaponOverlapBegin&OnWeaponOverlapEnd) used to test collision
			UPrimitiveComponent->UMeshComponent->UStaticMeshComponent
			UPrimitiveComponent->UShapeComponent->USphereComponent
		This could be the same as StaticMesh and also could be something different that is defined in an child class
		(for instance, the collision of wind/fire released by the weapon) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
		class UPrimitiveComponent* AttackDetectComponent;

	// Movement component that may be necessary
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", Replicated)
		class UMovementComponent* MovementComponent;

	UPROPERTY(EditAnywhere, Category = "Effects")
		class UNiagaraComponent* HaloEffect_NSComponent;
	UPROPERTY(EditAnywhere, Category = "Effects")
		class UNiagaraComponent* ShowUpEffect_NC;

	// Particle System that may be necessary(for instance, wind/fire released by the weapon)
	UPROPERTY(EditAnywhere, Category = "Effects")
		class UNiagaraComponent* AttackOnEffect;

	// Particle System that may be necessary(for instance, exposion)
	UPROPERTY(EditAnywhere, Category = "Effects")
		class UParticleSystem* AttackHitEffect;

	UPROPERTY(EditAnywhere)
		TSubclassOf<class ABaseProjectile> SpecificProjectileClass;

	//WeaponName
	FString WeaponName;

	mutable FCriticalSection DataGuard;

private:

};
