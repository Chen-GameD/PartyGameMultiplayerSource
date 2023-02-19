// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseWeapon.generated.h"


enum EnumWeaponType
{
	None,
	Fork,
	Blower,
	Lighter,
	Alarm,
	Flamethrower,
	Flamefork,
	Taser,
	Alarmgun,
	Bomb,
	Cannon
};

enum EnumAttackType
{
	OneHit,
	Constant,
	SpawnProjectile
};

enum EnumAttackBuff
{
	Burning,
	Paralysis,
	Blowing,
	Knockback
};


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
	virtual void GetPickedUp(ACharacter* pCharacter);
	// should only be called on server
	virtual void GetThrewAway();
	// should only be called on server
	virtual void AttackStart();
	// should only be called on server
	virtual void AttackStop();
	//Get weapon name
	virtual FString GetWeaponName() const;
	// Get Weapon Holder
	UFUNCTION(BlueprintCallable)
	ACharacter* GetHoldingPlayer() const;

	// only on server, generate stuff like damage, buff and so on
	virtual void GenerateDamageLike(class AActor* DamagedActor);

protected:
	virtual void CheckInitilization();
	virtual void BeginPlay() override;
	virtual void Destroyed() override; 
	virtual void PlayAnimationWhenNotBeingPickedUp(float DeltaTime);
	// should be only on client
	virtual void DisplayCaseCollisionSetActive(bool IsActive);
	// should be only on client
	virtual void GenerateAttackHitEffect();
	// should only be called on server
	virtual void SpawnProjectile();

	/* RepNotify Functions */
	UFUNCTION()
		virtual void OnRep_DisplayCaseTransform();
	UFUNCTION()
		virtual void OnRep_bAttackOn();
	UFUNCTION()
		virtual void OnRep_bAttackOverlap();
	UFUNCTION()
		virtual void OnRep_IsPickedUp();	
	UFUNCTION()
		virtual void OnRep_DamageGenerationCounter();

	// only is called on server
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
	static TMap<EnumWeaponType, FString> WeaponEnumToString_Map;

	EnumWeaponType WeaponType;
	EnumAttackType AttackType;
	bool IsCombined;
	UPROPERTY(EditAnywhere, Category = "Effects")
	UTexture2D* textureUI;
	// Ele: short for Element
	ABaseWeapon* EleWeapon_1;
	ABaseWeapon* EleWeapon_2;

	// Damage related
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
		float Damage;
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
		float MiniGameDamage;
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
		float AccumulatedTimeToGenerateDamage;
				//UPROPERTY(BlueprintReadOnly, Category = "Damage")
				//	float MiniGameAccumulatedTimeToGenerateDamage;
	// CoolDown related(CD_LeftEnergy needs to replicate so the client can show the correct cd UI)
	UPROPERTY(BlueprintReadOnly, Category = "CoolDown")
		float CD_MaxEnergy;
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "CoolDown")
		float CD_MinEnergyToAttak;
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "CoolDown")
		float CD_LeftEnergy;	
	UPROPERTY(BlueprintReadOnly, Category = "CoolDown")
		float CD_DropSpeed;
	UPROPERTY(BlueprintReadOnly, Category = "CoolDown")
		float CD_RecoverSpeed;
	
	UPROPERTY(ReplicatedUsing = OnRep_IsPickedUp)
		bool IsPickedUp;

protected:
	// Might be necessary if there are multiple weapons of the same type
	size_t ID;

	// don't replicate pointers
	ACharacter* HoldingPlayer;

	UPROPERTY(ReplicatedUsing = OnRep_DisplayCaseTransform)
		FVector DisplayCaseLocation;
	UPROPERTY(ReplicatedUsing = OnRep_DisplayCaseTransform)
		FRotator DisplayCaseRotation;
	UPROPERTY(ReplicatedUsing = OnRep_DisplayCaseTransform)
		FVector DisplayCaseScale;

	// check if ApplyDamage has happend during one AttackOn round, if happened, OneHit type weapon won't apply damage again.
	int ApplyDamageCounter;

	UPROPERTY(ReplicatedUsing = OnRep_bAttackOn)
		bool bAttackOn;

	UPROPERTY(ReplicatedUsing = OnRep_bAttackOverlap)
		bool bAttackOverlap;

	UPROPERTY(ReplicatedUsing = OnRep_DamageGenerationCounter)
		unsigned int DamageGenerationCounter;

	// Which actor is being attacked - how long they have been attacked
	TMap<AActor*, float> AttackObjectMap;

	/**
		Note: the following UComonent memeber variables should be checked in CheckInitilization() before using.

		We precede each of the types in these declarations with the class keyword.
		This makes each of them a forward declaration of their own classes
		in addition to being variable declarations, (chronological order: declare -> use -> define)
		which ensures that their classes will be recognized within the header file.
	*/

	// Sphere component used to make sure the weapon could be on the ground after threw away.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		class UBoxComponent* DisplayCase;

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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		class UPrimitiveComponent* AttackDetectComponent;

	// Movement component that may be necessary
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", Replicated)
		class UMovementComponent* MovementComponent;

	// Particle System that may be necessary(for instance, wind/fire released by the weapon)
	UPROPERTY(EditAnywhere, Category = "Effects")
		class UNiagaraComponent* AttackOnEffect;

	// Particle System that may be necessary(for instance, exposion)
	UPROPERTY(EditAnywhere, Category = "Effects")
		class UParticleSystem* AttackHitEffect;

	//The damage type and damage that will be done by this weapon
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
		TSubclassOf<class UDamageType> DamageType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
		TSubclassOf<class UDamageType> MiniGameDamageType;

	UPROPERTY(EditAnywhere)
		TSubclassOf<class ABaseProjectile> SpecificProjectileClass;

	//WeaponName
	FString WeaponName;

private:

};
