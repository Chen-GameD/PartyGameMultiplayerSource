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
	Flamethrower,
	Flamefork,
	Taser,
	Alarm,
	Alarmgun,
	Bomb,
	Cannon
};

enum EnumAttackType
{
	OneHit,
	Constant
};

enum EnumAttackBuff
{
	Burning,
	Paralysis,
	Blowing,
	Knockback
};

/*
	UClasses cannot really be abstract in the C++ sense, because the UObject sub-system requires that each class can be instantiated
	(it creates at least one instance of each class as a so called Class Default Object [CDO] that holds the default properties of that class).
	Therefore, every class method must have an implementation, even if it does nothing.
	That being said, you can get similar behavior by decorating your member methods with the PURE_VIRTUAL macro. This will tell
	the UObject sub-system that your intent is to declare a pure virtual method. So even though the method is not pure virtual
	in the C++ sense - it has a (possibly empty) function body - the compiler can still ensure that all child classes do supply an actual implementation.

	Summary:
	If you don't want a class to be instantiated, use the "Abstract" class specifier, although it isn't exactly abstract in the C++ sense
	We can't make a UClass abstract by putting a pure virtual function into it, because Unreal doesn't allow pure virtual functions in a UClass.
*/
UCLASS(Abstract)
class PARTYGAMEMULTIPLAYER_API ABaseWeapon : public AActor
{
	GENERATED_BODY()

// MEMBER METHODS
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

	/*
		Confusion: the macro doesn't keep the child class from instantiating when it doesn't implement the interface...
		If return type is not void, you have write something like "ABaseWeapon::AttackStart, return false;" inside the macro
	
		For example, the flamethrower starts to throw the flame
		virtual void AttackStart() PURE_VIRTUAL(ABaseWeapon::AttackStart, );
		For example, the flamethrower stops throwing the flame
		virtual void AttackStop() PURE_VIRTUAL(ABaseWeapon::AttackStop, );
	*/

	//Get weapon name
	virtual FString GetWeaponName() const;

	// Get Weapon Holder
	UFUNCTION(BlueprintCallable)
	ACharacter* GetHoldingPlayer() const;

protected:
	virtual void CheckInitilization();
	virtual void BeginPlay() override;
	//virtual void Destroyed() override; 
	virtual void PlayAnimationWhenNotBeingPickedUp(float DeltaTime);
	//virtual void GenerateAttackHitEffect() PURE_VIRTUAL(ABaseWeapon::GenerateAttackHitEffect, );
	virtual void GenerateAttackHitEffect();
	//virtual void GenerateDamage(class AActor* DamagedActor) PURE_VIRTUAL(ABaseWeapon::GenerateDamage, );
	virtual void GenerateDamage(class AActor* DamagedActor);
	//virtual void GenerateBuff(class AActor* DamagedActor);

	UFUNCTION()
		virtual void OnRep_Transform();
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
	// only is called on server(For test use, will not be used in the actual project)
	UFUNCTION(Category = "Weapon")
		virtual void OnDisplayCaseOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
			class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
private:


// MEMBER VARIABLES
public:
	EnumWeaponType WeaponType;
	EnumAttackType AttackType;
	bool IsCombined;
	UPROPERTY(EditAnywhere, Category = "Effects")
	UTexture2D* textureUI;
	// Ele is short for Element
	ABaseWeapon* EleWeapon_1;
	ABaseWeapon* EleWeapon_2;

	// TODO: Assgin them one time by an external class(like damage manager)
	UPROPERTY(EditAnywhere, Category = "Damage")
		float Damage;
	UPROPERTY(EditAnywhere, Category = "Damage")
		float MiniGameDamage;
	UPROPERTY(EditAnywhere, Category = "Damage")
		float AccumulatedTimeToGenerateDamage;
	UPROPERTY(EditAnywhere, Category = "Damage")
		float MiniGameAccumulatedTimeToGenerateDamage;

protected:
	// Might be necessary if there are multiple weapons of the same type
	size_t ID;

	// don't replicate pointers
	ACharacter* HoldingPlayer;

	UPROPERTY(ReplicatedUsing = OnRep_Transform)
		FVector RootLocation;
	UPROPERTY(ReplicatedUsing = OnRep_Transform)
		FRotator RootRotation;
	UPROPERTY(ReplicatedUsing = OnRep_Transform)
		FVector RootScale;

	UPROPERTY(ReplicatedUsing = OnRep_IsPickedUp)
		bool IsPickedUp;

	UPROPERTY(ReplicatedUsing = OnRep_bAttackOn)
		bool bAttackOn;

	UPROPERTY(ReplicatedUsing = OnRep_bAttackOverlap)
		bool bAttackOverlap;

	UPROPERTY(ReplicatedUsing = OnRep_DamageGenerationCounter)
		unsigned int DamageGenerationCounter;

	// record what is being attacked and how long they have been attacked
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

	//WeaponName
	FString WeaponName;

private:

};
