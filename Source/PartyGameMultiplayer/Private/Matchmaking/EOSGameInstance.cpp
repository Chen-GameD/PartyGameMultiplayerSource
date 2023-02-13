// Fill out your copyright notice in the Description page of Project Settings.


#include "Matchmaking/EOSGameInstance.h"
#include <string>
#include "Kismet/GameplayStatics.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "OnlineSessionSettings.h"
#include "Blueprint/UserWidget.h"

const FName SESSION_NAME = FName("MAINSESSION");

void UEOSGameInstance::Init()
{
	Super::Init();
	
	// Login();
}

void UEOSGameInstance::Login(FString ID, FString Token, FString LoginType)
{
	if(IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(this->GetWorld()))
	{
		IOnlineIdentityPtr IdentityPtr = OnlineSubsystem->GetIdentityInterface();
		if(IdentityPtr)
		{
			FOnlineAccountCredentials Credentials;
			Credentials.Id = ID;
			Credentials.Token = Token;
			Credentials.Type = LoginType;
			IdentityPtr->OnLoginCompleteDelegates->AddUObject(this, &UEOSGameInstance::OnLoginComplete);
			IdentityPtr->Login(0, Credentials);
		}
	}
}

FString UEOSGameInstance::GetPlayerUsername()
{
	if(IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(this->GetWorld()))
	{
		if(IOnlineIdentityPtr IdentityPtr = OnlineSubsystem->GetIdentityInterface())
		{
			if(IdentityPtr->GetLoginStatus(0) == ELoginStatus::LoggedIn)
			{
				return IdentityPtr->GetPlayerNickname(0);
			}
		}
	}
	return FString();
}

void UEOSGameInstance::CreateSession(bool IsDedicatedServer, bool IsLanServer, int32 NumberOfPublicConnections)
{
	if(bIsLoggedIn)
	{
		if(IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(this->GetWorld()))
		{
			if(IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface())
			{
				FOnlineSessionSettings SessionSettings;
				SessionSettings.bIsDedicated = IsDedicatedServer;
				SessionSettings.bAllowInvites = true;
				SessionSettings.bIsLANMatch = IsLanServer;
				SessionSettings.NumPublicConnections = NumberOfPublicConnections;
				SessionSettings.bUsesPresence = true;
				SessionSettings.bUseLobbiesIfAvailable = true;
				SessionSettings.bShouldAdvertise = true;
				SessionSettings.Set(SETTING_MAPNAME, FString("/Game/Level/SessionTest"), EOnlineDataAdvertisementType::ViaOnlineService);
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
	IsSessionsListAvailable = false;
	if(bIsLoggedIn)
	{
		if(IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(this->GetWorld()))
		{
			if(IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface())
			{
				SearchSettings = MakeShareable(new FOnlineSessionSearch());
				SearchSettings->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
				SearchSettings->MaxSearchResults = 100;
				SearchSettings->QuerySettings.Set(SEARCH_KEYWORDS, FString("CBLobby"), EOnlineComparisonOp::Equals);
				
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
		if(IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(this->GetWorld()))
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
		if(IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(this->GetWorld()))
		{
			if(IOnlineExternalUIPtr UIPtr = OnlineSubsystem->GetExternalUIInterface())
			{
				UIPtr->ShowInviteUI(0, SESSION_NAME);
			}
		}
	}
}

TArray<USessionEntry*> UEOSGameInstance::GetSessionsList()
{
	TArray<USessionEntry*> sessions = TArray<USessionEntry*>();
	if(IsSessionsListAvailable)
	{
		for (auto session : SearchSettings->SearchResults)
		{
			USessionEntry *newEntry = NewObject<USessionEntry>();
			newEntry->SetSessionData(session.GetSessionIdStr(), session.Session.SessionSettings.NumPublicConnections,
			              session.Session.SessionSettings.NumPublicConnections-session.Session.NumOpenPublicConnections);
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Sessions in search result"));
			UE_LOG(LogTemp, Warning, TEXT("Sessions in search result : %s"), *session.GetSessionIdStr());
			sessions.Add(newEntry);
		}
	}
	return sessions;
}

void UEOSGameInstance::UI_ShowMainMenu()
{
	if (WB_MainMenuClass)
	{
		if (!WB_MainMenu)
		{
			// Create menu
			APlayerController* currentPlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
			if (currentPlayerController != nullptr)
			{
				WB_MainMenu = CreateWidget<UUserWidget>(currentPlayerController, WB_MainMenuClass);
				//CreateWidget(GetFirstLocalPlayerController(), WB_MainMenuClass->StaticClass());
                
				WB_MainMenu->AddToViewport();
				FInputModeUIOnly inputMode;
				inputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
				currentPlayerController->SetInputMode(inputMode);
				currentPlayerController->SetShowMouseCursor(true);
			}
			
		}
	}
}

bool UEOSGameInstance::GetIsLoggedIn()
{
	return bIsLoggedIn;
}

void UEOSGameInstance::JoinSession(int32 index)
{
	if(bIsLoggedIn)
	{
		if(IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(this->GetWorld()))
		{
			if(IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface())
			{
				if(SearchSettings->SearchResults.Num() > 0)
				{
					GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("JOINING A SESSION NOW..."));
					SessionPtr->OnJoinSessionCompleteDelegates.AddUObject(this, &UEOSGameInstance::OnJoinSessionComplete);
					UE_LOG(LogTemp, Warning, TEXT("Joining session : %s"), *SearchSettings->SearchResults[index].Session.OwningUserName);
					CurrentlyJoiningSessionIndex = index;
					SessionPtr->JoinSession(0, SESSION_NAME, SearchSettings->SearchResults[index]);
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
		if(IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(this->GetWorld()))
		{
			if(IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface())
			{
				SessionPtr->ClearOnCreateSessionCompleteDelegates(this);
			}
		}
	}
	if(bWasSuccessful)
	{
		GetWorld()->ServerTravel(LevelText+"?listen", true);
	}
}

void UEOSGameInstance::OnDestroySessionComplete(FName sessionName, bool bWasSuccessful)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, TEXT("Destroy Session Complete"));
	UE_LOG(LogTemp, Warning, TEXT("Success destroy session: %d"), bWasSuccessful);
	if(bIsLoggedIn)
	{
		if(IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(this->GetWorld()))
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
		if(IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(this->GetWorld()))
		{
			if(IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface())
			{
				if(bWasSuccessful)
				{
					UE_LOG(LogTemp, Warning, TEXT("Count find session: %d"), SearchSettings->SearchResults.Num());
					if(SearchSettings->SearchResults.Num() > 0)
					{
						IsSessionsListAvailable = true;
					}
					else
					{
						GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("No sessions in search results"));
						UE_LOG(LogTemp, Warning, TEXT("No sessions in search results"));
					}
				}
				SessionPtr->ClearOnFindSessionsCompleteDelegates(this);
			}
		}
	}
}

void UEOSGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, TEXT("Join Complete"));
	UE_LOG(LogTemp, Warning, TEXT("Success Join: %d"), Result);

	if(Result == EOnJoinSessionCompleteResult::Success)
	{
		if(APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(),0))
		{
			FString JoinURL;
			if(IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(this->GetWorld()))
			{
				if(IOnlineSessionPtr OnlineSessionPtr = OnlineSubsystem->GetSessionInterface())
				{
					int32 index = (CurrentlyJoiningSessionIndex == -1) ? 0 : CurrentlyJoiningSessionIndex;
					OnlineSessionPtr->GetResolvedConnectString(SearchSettings->SearchResults[index], NAME_GamePort, JoinURL);
					UE_LOG(LogTemp, Warning, TEXT("JoinURL : %s"), *JoinURL);
					if(!JoinURL.IsEmpty())
					{
						//JoinURL.Append("/Game/Level/SessionTest");
						if(SearchSettings->SearchResults[index].Session.SessionSettings.Settings.Contains(SETTING_MAPNAME))
						{
							auto val = SearchSettings->SearchResults[index].Session.SessionSettings.Settings.Find(SETTING_MAPNAME);
							UE_LOG(LogTemp, Warning, TEXT("strMapKeys : %s"), *val->ToString());
							//JoinURL.Append(val->ToString());
						}
						PlayerController->ClientTravel(JoinURL, ETravelType::TRAVEL_Absolute);
						CurrentlyJoiningSessionIndex = -1;
					}
				}
			}
		}
	}
}

void UEOSGameInstance::OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, bWasSuccessful ? FColor::Green : FColor::Red, TEXT("Login Complete"));
	UE_LOG(LogTemp, Warning, TEXT("Success login: %d"), bWasSuccessful);
	if(IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(this->GetWorld()))
	{
		if(IOnlineIdentityPtr Identity = OnlineSubsystem->GetIdentityInterface())
		{
			Identity->ClearOnLoginCompleteDelegates(0, this);
		}
	}
	bIsLoggedIn = bWasSuccessful;
}
