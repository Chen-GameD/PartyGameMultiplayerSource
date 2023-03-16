// Fill out your copyright notice in the Description page of Project Settings.

#include "LevelInteraction/TriggerBoxJumpPad.h"

#include "NiagaraComponent.h"
#include "DrawDebugHelpers.h"

#include "GameFramework/Character.h"
#include "Character/MCharacter.h"


ATriggerBoxJumpPad::ATriggerBoxJumpPad()
{
    LaunchVelocity = FVector(0.0f, 0.0f, 1000.0f);
    bXYOverride = false;
    bZOverride = true;
}

void ATriggerBoxJumpPad::BeginPlay()
{
	Super::BeginPlay();

    DrawDebugBox(GetWorld(), GetActorLocation(), GetComponentsBoundingBox().GetExtent(), FColor::Purple, true, -1, 0, 5);

    /* 
        Interestingly, if you name the 2 overlap function just OnOverlapBegin & OnOverlapEnd,
        then you actually are overriding the existing functions! which means that even if you don't call the AddDynamic,
        They will still be active because they have been added already! And if you call the AddDynamic by yourself, it will
        actually register Overlap function again. So what is in OnOverlapBegin & OnOverlapEnd will happen twice... which is not what you want...

        So, if you want to achieve certain effects(like make things different on server and client), 
        you have to create your own Overlap functions with other names.
    */
    if (GetLocalRole() == ROLE_Authority)
    {
        OnActorBeginOverlap.AddDynamic(this, &ATriggerBoxJumpPad::OnJumpPadOverlapBegin);
        OnActorEndOverlap.AddDynamic(this, &ATriggerBoxJumpPad::OnJumpPadOverlapEnd);
    }
}

void ATriggerBoxJumpPad::OnJumpPadOverlapBegin(class AActor* OverlappedActor, class AActor* OtherActor)
{
    AMCharacter* pCharacter = Cast<AMCharacter>(OtherActor);
    if (OtherActor && OtherActor != this && pCharacter) 
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Character enters the Jump Pad"));
        pCharacter->LaunchCharacter(LaunchVelocity, bXYOverride, bZOverride);
        NetMulticast_WhenCharacterEnterTriggerBoxJumpPad();
    }
}

void ATriggerBoxJumpPad::OnJumpPadOverlapEnd(class AActor* OverlappedActor, class AActor* OtherActor)
{
    AMCharacter* pCharacter = Cast<AMCharacter>(OtherActor);

    if (OtherActor && OtherActor != this && pCharacter) 
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Character exits the Jump Pad"));
    }
}


void ATriggerBoxJumpPad::NetMulticast_WhenCharacterEnterTriggerBoxJumpPad_Implementation()
{
    //if(GetLocalRole() == ROLE_AutonomousProxy)
    WhenCharacterEnterTriggerBoxJumpPad();
}