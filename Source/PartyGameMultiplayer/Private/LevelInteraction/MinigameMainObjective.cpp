// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelInteraction/MinigameMainObjective.h"
#include "UObject/ConstructorHelpers.h"
#include "Net/UnrealNetwork.h"
#include "../PartyGameMultiplayerCharacter.h"
#include "Weapon/DamageTypeToCharacter.h"
//#include "Engine/TriggerBoxDamageTaker.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
//#include "GeometryCacheComponent.h"

#include "UI/MCharacterFollowWidget.h"
#include "Components/WidgetComponent.h"
#include "Character/MCharacter.h"
#include "GameBase/MGameMode.h"


AMinigameMainObjective::AMinigameMainObjective()
{
	RootMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RootMesh"));
	RootMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	RootMesh->SetupAttachment(RootComponent);
	/*static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("/Game/StarterContent/Shapes/Shape_Cube.Shape_Cube"));
	if (DefaultMesh.Succeeded())
	{
		RootMesh->SetStaticMesh(DefaultMesh.Object);
	}*/

	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(RootMesh);

	//ArrayUStaticMeshComponent.Emplace(CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh_1")));
	//ArrayUStaticMeshComponent.Emplace(CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh_2")));
	//for (auto& Mesh : ArrayUStaticMeshComponent)
	//{
	//	Mesh->SetupAttachment(RootMesh);
	//}

	MaxHealth = 1200.0f;
	CurrentHealth = MaxHealth;

	//Create HealthBar UI Widget
	HealthWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBar"));
	HealthWidget->SetupAttachment(RootMesh);	

	BlowUpEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("BlowUpEffect"));
	BlowUpEffect->SetupAttachment(RootMesh);
	BlowUpEffect->bAutoActivate = false;

	//BlowUpGeometryCacheComponent = CreateDefaultSubobject<UGeometryCache>(TEXT("BlowUpGeometryCacheComponent"));
	//BlowUpGeometryCacheComponent->SetupAttachment(RootMesh);

	bReplicates = true;
}


void AMinigameMainObjective::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMinigameMainObjective, CurrentHealth);
}

// server-only
float AMinigameMainObjective::TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!EventInstigator || !DamageCauser)
		return 0.0f;
	if (CurrentHealth <= 0)
		return 0.0f;

	CurrentHealth -= DamageTaken;
	if (CurrentHealth <= 0)
	{
		CurrentHealth = 0.0f;	
		if (SkeletalMesh)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Destroying MiniGameObjective's SkeletalMesh on Server"));
			SkeletalMesh->DestroyComponent();
		}
		if (AM_PlayerState* killerPS = EventInstigator->GetPlayerState<AM_PlayerState>())
		{
			killerPS->addScore(30);
		}

		// Set timer and respawn this actor
		FTimerHandle RespawnMinigameObjectTimerHandle;
		GetWorldTimerManager().SetTimer(RespawnMinigameObjectTimerHandle, this, &AMinigameMainObjective::StartToRespawnActor, 5, false);
	}		

	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("AMinigameMainObjective's current health is %f"), CurrentHealth));
	UDamageTypeToCharacter* DamageType = Cast<UDamageTypeToCharacter>(DamageEvent.DamageTypeClass->GetDefaultObject());

	if (DamageType)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Customized Damage Type is %s"), *(DamageType->TestString)));
	}
	
	auto pCharacter = EventInstigator->GetCharacter();
	// Get the info of the player who applies the damage
	if (auto pDefaultUECharacter = dynamic_cast<APartyGameMultiplayerCharacter*>(pCharacter))
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, 
		//	FString::Printf(TEXT("%f pts damage was applied by %s"), DamageTaken, *(pDefaultUECharacter->GetName()))
		//	);
	}
	
	return 0.0f;
}


void AMinigameMainObjective::BeginPlay()
{
	Super::BeginPlay();
	if (UMCharacterFollowWidget* healthBar = Cast<UMCharacterFollowWidget>(HealthWidget->GetUserWidgetObject()))
	{
		healthBar->SetHealthToProgressBar((MaxHealth - CurrentHealth) / MaxHealth);
		healthBar->HideTip();
	}
	BlowUpEffect->Deactivate();
}

// client-only
void AMinigameMainObjective::OnRep_CurrentHealth()
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("AMinigameMainObjective's current health is %f"), CurrentHealth));

	if (UMCharacterFollowWidget* healthBar = Cast<UMCharacterFollowWidget>(HealthWidget->GetUserWidgetObject()))
	{
		healthBar->SetHealthToProgressBar((MaxHealth - CurrentHealth) / MaxHealth);
	}

	if (CurrentHealth <= 0)
	{
		if (SkeletalMesh)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Destroying MiniGameObjective's SkeletalMesh on Client"));
			SkeletalMesh->DestroyComponent();
		}
		if (HealthWidget)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Destroying MiniGameObjective's HealthWidget on Client"));
			HealthWidget->DestroyComponent();
		}
		if (BlowUpEffect)
		{
			BlowUpEffect->Activate();
		}
		EnableBlowUpGeometryCacheComponent();
	}
}

void AMinigameMainObjective::StartToRespawnActor()
{
	AMGameMode* MyGameMode = Cast<AMGameMode>(GetWorld()->GetAuthGameMode());
	if (MyGameMode)
	{
		MyGameMode->Server_RespawnMinigameObject();
	}
	AMinigameMainObjective::Destroy(true, true);
}
