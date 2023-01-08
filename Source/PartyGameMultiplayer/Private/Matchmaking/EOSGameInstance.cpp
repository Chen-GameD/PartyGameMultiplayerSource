// Fill out your copyright notice in the Description page of Project Settings.


#include "Matchmaking/EOSGameInstance.h"

#include <string>

#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Kismet/GameplayStatics.h"

const FName SESSION_NAME = FName("TEST SESSION");

UEOSGameInstance::UEOSGameInstance()
{
	
}

void UEOSGameInstance::Init()
{
	Super::Init();

	OnlineSubsystem = IOnlineSubsystem::Get();
	Login();
}

void UEOSGameInstance::Login()
{
	if(OnlineSubsystem)
	{
		if(IOnlineIdentityPtr Identity = OnlineSubsystem->GetIdentityInterface())
		{
			FOnlineAccountCredentials Credentials;
			Credentials.Id = FString("127.0.0.1:5666");
			Credentials.Token = FString("KD");
			Credentials.Type = FString("developer");

			Identity->OnLoginCompleteDelegates->AddUObject(this, &UEOSGameInstance::OnLoginComplete);
			Identity->Login(0, Credentials);
		}
	}
}

void UEOSGameInstance::CreateSession()
{
	if(bIsLoggedIn)
	{
		if(OnlineSubsystem)
		{
			if(IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface())
			{
				FOnlineSessionSettings SessionSettings;
				SessionSettings.bIsDedicated = false;
				SessionSettings.bShouldAdvertise = true;
				SessionSettings.bIsLANMatch = false;
				SessionSettings.NumPublicConnections = 8;
				SessionSettings.bAllowJoinInProgress = true;
				SessionSettings.bAllowJoinViaPresence = true;
				SessionSettings.bUsesPresence = true;
				SessionSettings.bUseLobbiesIfAvailable = true;
				SessionSettings.Set(SEARCH_KEYWORDS, FString("CBLobby"), EOnlineDataAdvertisementType::ViaOnlineService);

				SessionPtr->OnCreateSessionCompleteDelegates.AddUObject(this, &UEOSGameInstance::OnCreateSessionComplete);
				SessionPtr->CreateSession(0, SESSION_NAME, SessionSettings);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Not logged IN"));
	}
}

void UEOSGameInstance::FindSession()
{
	if(bIsLoggedIn)
	{
		if(OnlineSubsystem)
		{
			if(IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface())
			{
				SearchSettings = MakeShareable(new FOnlineSessionSearch());
				SearchSettings->MaxSearchResults = 5000;
				SearchSettings->QuerySettings.Set(SEARCH_KEYWORDS, FString("CBLobby"), EOnlineComparisonOp::Equals);
				SearchSettings->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
				
				SessionPtr->OnFindSessionsCompleteDelegates.AddUObject(this, &UEOSGameInstance::OnFindSessionComplete);
				SessionPtr->FindSessions(0, SearchSettings.ToSharedRef());
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Not logged IN"));
	}
}

void UEOSGameInstance::DestroySession()
{
	if(bIsLoggedIn)
	{
		if(OnlineSubsystem)
		{
			if(IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface())
			{
				SessionPtr->OnDestroySessionCompleteDelegates.AddUObject(this, &UEOSGameInstance::OnDestroySessionComplete);
				SessionPtr->DestroySession(SESSION_NAME);
			}
		}
	}
}

void UEOSGameInstance::ShowInviteUI()
{
	if(bIsLoggedIn)
	{
		if(OnlineSubsystem)
		{
			if(IOnlineExternalUIPtr UIPtr = OnlineSubsystem->GetExternalUIInterface())
			{
				UIPtr->ShowInviteUI(0, SESSION_NAME);
			}
		}
	}
}

void UEOSGameInstance::JoinSession()
{
	if(bIsLoggedIn)
	{
		if(OnlineSubsystem)
		{
			if(IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface())
			{
				if(SearchSettings->SearchResults.Num() > 0)
				{
					SessionPtr->OnJoinSessionCompleteDelegates.AddUObject(this, &UEOSGameInstance::OnJoinSessionComplete);
					SessionPtr->JoinSession(0, SESSION_NAME, SearchSettings->SearchResults[0]);
				}
				else
				{
					GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("No sessions in search results"));
					UE_LOG(LogTemp, Warning, TEXT("No sessions in search results"));
				}
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Not logged IN"));
	}
}

void UEOSGameInstance::OnCreateSessionComplete(FName sessionName, bool bWasSuccessful)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, TEXT("Create Session Complete"));
	UE_LOG(LogTemp, Warning, TEXT("Success: %d"), bWasSuccessful);
	if(bIsLoggedIn)
	{
		if(OnlineSubsystem)
		{
			if(IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface())
			{
				SessionPtr->ClearOnCreateSessionCompleteDelegates(this);
			}
		}
	}
	if(bWasSuccessful)
	{
		//UGameplayStatics::OpenLevel(GetWorld(), FName("WhiteboxLevel"));
	}
}

void UEOSGameInstance::OnDestroySessionComplete(FName sessionName, bool bWasSuccessful)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, TEXT("Destroy Session Complete"));
	UE_LOG(LogTemp, Warning, TEXT("Success destroy session: %d"), bWasSuccessful);
	if(bIsLoggedIn)
	{
		if(OnlineSubsystem)
		{
			if(IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface())
			{
				SessionPtr->ClearOnDestroySessionCompleteDelegates(this);
			}
		}
	}
}

void UEOSGameInstance::OnFindSessionComplete(bool bWasSuccessful)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, TEXT("Find Session Complete"));
	UE_LOG(LogTemp, Warning, TEXT("Success find session: %d"), bWasSuccessful);
	if(bIsLoggedIn)
	{
		if(OnlineSubsystem)
		{
			if(IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface())
			{
				if(bWasSuccessful)
				{
					UE_LOG(LogTemp, Warning, TEXT("Count find session: %d"), SearchSettings->SearchResults.Num());
					if(SearchSettings->SearchResults.Num() > 0)
					{
						//Join
						JoinSession();
						GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, SearchSettings->SearchResults[0].GetSessionIdStr());
					}
				}
				SessionPtr->ClearOnFindSessionsCompleteDelegates(this);
			}
		}
	}
}

void UEOSGameInstance::OnJoinSessionComplete(FName sessionName, EOnJoinSessionCompleteResult::Type result)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, TEXT("Join Complete"));
	UE_LOG(LogTemp, Warning, TEXT("Success Join: %d"), result);

	if(bIsLoggedIn)
	{
		if(OnlineSubsystem && result == EOnJoinSessionCompleteResult::Success)
		{
			if(IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface())
			{
				FString connectionInfo = FString();
				SessionPtr->GetResolvedConnectString(sessionName, connectionInfo);
				if(!connectionInfo.IsEmpty())
				{
					if(APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0))
					{
						PlayerController->ClientTravel(connectionInfo, TRAVEL_Absolute);
					}
				}
				SessionPtr->ClearOnJoinSessionCompleteDelegates(this);
			}
		}
	}
}

void UEOSGameInstance::OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId,
                                       const FString& Error)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, TEXT("Login Complete"));
	UE_LOG(LogTemp, Warning, TEXT("Success login: %d"), bWasSuccessful);
	if(OnlineSubsystem)
	{
		if(IOnlineIdentityPtr Identity = OnlineSubsystem->GetIdentityInterface())
		{
			Identity->ClearOnLoginCompleteDelegates(0, this);
		}
	}
	bIsLoggedIn = bWasSuccessful;
}
