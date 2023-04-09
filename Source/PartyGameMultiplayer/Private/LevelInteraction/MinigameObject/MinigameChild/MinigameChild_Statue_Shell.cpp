// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelInteraction/MinigameObject/MinigameChild/MinigameChild_Statue_Shell.h"
#include "LevelInteraction/MinigameObject/MinigameObj_Statue.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

// Sets default values
AMinigameChild_Statue_Shell::AMinigameChild_Statue_Shell()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ShellMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShellMesh"));
	ShellMeshComponent->SetCollisionProfileName(TEXT("NoCollision"));
	ShellMeshComponent->SetupAttachment(RootComponent);

	ShellFly_NC = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ShellFlyVfx"));
	ShellFly_NC->SetupAttachment(ShellMeshComponent);
	ShellFly_NC->bAutoActivate = true;

	ShellInsert_NC = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ShellInsertVfx"));
	ShellInsert_NC->SetupAttachment(ShellMeshComponent);
	ShellInsert_NC->bAutoActivate = false;
	
	bReplicates = true;
	bFinishInsert = false;
}

void AMinigameChild_Statue_Shell::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMinigameChild_Statue_Shell, TargetSocketName);
	DOREPLIFETIME(AMinigameChild_Statue_Shell, TartgetStatue);
}

// Called when the game starts or when spawned
void AMinigameChild_Statue_Shell::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMinigameChild_Statue_Shell::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (TargetSocketName != "None" && TartgetStatue && !bFinishInsert)
	{
		// Update shell position
		// TODO

		// if (GetNetMode() == NM_Client)
		// {
		// 	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, TEXT("Client: Shell Position Update"));
		// 	this->SetActorLocation(FVector(1000, -1250, 100));
		// 	
		// }
		// 	
		// else if (GetNetMode() == NM_ListenServer)
		// {
		// 	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, TEXT("Server: Shell Position Update"));
		// 	this->SetActorLocation(FVector(1000, -1000, 100));
		// }

		AMinigameObj_Statue* Target = Cast<AMinigameObj_Statue>(TartgetStatue);
		if (Target)
		{
			bool IsCloseEnough = false;
			FTransform SocketTransform = Target->GetSkeletalMesh()->GetSocketTransform(TargetSocketName);
			if (FVector::Dist(GetActorLocation(), SocketTransform.GetLocation()) < 5.0f)
				IsCloseEnough = true;
			if (TimeElapsed < LerpDuration && !IsCloseEnough)
			{
				FVector NewPosition = FMath::Lerp(GetActorLocation(), SocketTransform.GetLocation(), TimeElapsed / LerpDuration);
				SetActorLocation(NewPosition);
				FQuat NewRotation = FMath::Lerp(GetActorRotation().Quaternion(), SocketTransform.GetRotation(), TimeElapsed / LerpDuration);
				SetActorRotation(NewRotation);
				FVector NewScale = FMath::Lerp(GetActorScale3D(), SocketTransform.GetScale3D(), TimeElapsed / LerpDuration);
				SetActorScale3D(NewScale);
				TimeElapsed += DeltaTime;
			}
			else
			{
				AttachToComponent(Target->GetSkeletalMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, TargetSocketName);
				SetActorLocation(SocketTransform.GetLocation());
				SetActorRotation(SocketTransform.GetRotation());
				SetActorScale3D(SocketTransform.GetScale3D());

				// Vfx
				if (ShellFly_NC && ShellFly_NC->IsActive())
				{
					ShellFly_NC->Deactivate();
					ShellFly_NC->SetVisibility(false);
				}
				if (ShellInsert_NC && !ShellInsert_NC->IsActive())
				{
					ShellInsert_NC->SetWorldRotation(FRotator::ZeroRotator);
					ShellInsert_NC->SetWorldScale3D(FVector::OneVector);
					ShellInsert_NC->Activate();
				}
				// Sfx
				CallShellInsertSfx();

				bFinishInsert = true;
			}
		}
				
	}

}

