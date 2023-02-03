// Fill out your copyright notice in the Description page of Project Settings.


#include "EOS_GameMode.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineIdentityInterface.h"

void AEOS_GameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if(NewPlayer)
	{
		FUniqueNetIdRepl UniqueNetIdRepl;
		if(NewPlayer->IsLocalController())
		{
			ULocalPlayer *LocalPlayer = NewPlayer->GetLocalPlayer();
			if(LocalPlayer)
			{
				UniqueNetIdRepl = LocalPlayer->GetPreferredUniqueNetId();
			}
			else
			{
				UNetConnection *NetConnectionRef = Cast<UNetConnection>(NewPlayer->Player);
				check(IsValid(NetConnectionRef));
				UniqueNetIdRepl = NetConnectionRef->PlayerId;
			}
		}
		else
		{
			UNetConnection *NetConnectionRef = Cast<UNetConnection>(NewPlayer->Player);
			check(IsValid(NetConnectionRef));
			UniqueNetIdRepl = NetConnectionRef->PlayerId;
		}
		
		TSharedPtr<const FUniqueNetId> UniqueNetId = UniqueNetIdRepl.GetUniqueNetId();
		if(UniqueNetId == nullptr)
			return;
		IOnlineSubsystem *OnlineSubsystemRef = Online::GetSubsystem(NewPlayer->GetWorld());
		IOnlineSessionPtr OnlineSessionRef = OnlineSubsystemRef->GetSessionInterface();
		bool bRegistrationSuccess = OnlineSessionRef->RegisterPlayer(FName("MAINSESSION"), *UniqueNetId, false);
		if(bRegistrationSuccess)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, TEXT("Success Registration"));
			UE_LOG(LogTemp, Warning, TEXT("Success registration: %d"), bRegistrationSuccess);
		}
	}
}
