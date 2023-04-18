// Fill out your copyright notice in the Description page of Project Settings.


#include "Matchmaking/EOSGameInstance.h"
#include <string>

#include "M_PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "OnlineSessionSettings.h"
#include "Blueprint/UserWidget.h"
#include "GameBase/MGameState.h"
#include "GameFramework/GameState.h"

const FName SESSION_NAME = FName("MAINSESSION");

void UEOSGameInstance::Init()
{
	Super::Init();
	ReturnGameState = NewObject<UReturnGameState>();
	Login("", "", "accountportal");
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
	if(GetIsLoggedIn())
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
	}
	return PlayerName;
}

void UEOSGameInstance::CreateSession(bool IsDedicatedServer, bool IsLanServer, int32 NumberOfPublicConnections,
	bool IsPrivate = false, FString RoomName = FString(), int32 MapReference = -1)
{
	isLoading = true;
	if(bIsLoggedIn)
	{
		if(IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(this->GetWorld()))
		{
			if(IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface())
			{
				FOnlineSessionSettings SessionSettings;
				SessionSettings.bIsDedicated = IsDedicatedServer;
				SessionSettings.bUsesPresence = true;
				SessionSettings.bAllowInvites = true;
				SessionSettings.bIsLANMatch = IsLanServer;
				SessionSettings.NumPublicConnections = IsPrivate ? 0 : NumberOfPublicConnections;
				SessionSettings.NumPrivateConnections = !IsPrivate ? 0 : NumberOfPublicConnections;
				SessionSettings.bAllowJoinViaPresence = !IsPrivate;
				SessionSettings.bAllowJoinViaPresenceFriendsOnly = IsPrivate;
				SessionSettings.bUseLobbiesIfAvailable = true;
				SessionSettings.bShouldAdvertise = true;
				SessionSettings.bAllowJoinInProgress = true;
				SessionSettings.Set(SEARCH_KEYWORDS, FString("CBLobby"), EOnlineDataAdvertisementType::ViaOnlineService);
				SessionSettings.Set(SETTING_MAPNAME, MapReference, EOnlineDataAdvertisementType::ViaOnlineService);
				SessionSettings.Set(SETTING_SESSIONKEY, RoomName, EOnlineDataAdvertisementType::ViaOnlineService);
				DebugLevelSelect = MapReference;

				SessionPtr->OnCreateSessionCompleteDelegates.AddUObject(this, &UEOSGameInstance::OnCreateSessionComplete);
				SessionPtr->CreateSession(0, SESSION_NAME, SessionSettings);
			}
		}
	}
	else
	{
		isLoading = false;
		UE_LOG(LogTemp, Error, TEXT("Not logged IN"));
	}
}

void UEOSGameInstance::FindSession()
{
	isLoading = true;
	IsSessionsListAvailable = false;
	if(bIsLoggedIn)
	{
		if(IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(this->GetWorld()))
		{
			if(IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface())
			{
				SearchSettings = MakeShareable(new FOnlineSessionSearch());
				SearchSettings->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
				SearchSettings->QuerySettings.Set(SEARCH_KEYWORDS, FString("CBLobby"), EOnlineComparisonOp::Equals);
				SearchSettings->MaxSearchResults = 100;
				
				SessionPtr->OnFindSessionsCompleteDelegates.AddUObject(this, &UEOSGameInstance::OnFindSessionComplete);
				SessionPtr->FindSessions(0, SearchSettings.ToSharedRef());
			}
		}
	}
	else
	{
		isLoading = false;
		UE_LOG(LogTemp, Error, TEXT("Not logged IN"));
	}
}

void UEOSGameInstance::DestroySession()
{
	isLoading = true;
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
	else
	{
		isLoading = false;
	}
}

UReturnGameState* UEOSGameInstance::GetReturnGameStateRef()
{
	return ReturnGameState;
}

void UEOSGameInstance::SaveEndGameState()
{
	if(const auto gameState = Cast<AMGameState>(GetWorld()->GetGameState()))
	{
		ReturnGameState->UpdateTeamScore(true, gameState->Team_1_Score);
		ReturnGameState->UpdateTeamScore(false, gameState->Team_2_Score);
		for(auto player : gameState->PlayerArray)
		{
			if(const auto playerState = Cast<AM_PlayerState>(player))
			{
				ReturnGameState->AddPlayerData(playerState->TeamIndex == 1 ? true : false, playerState->PlayerNameString, playerState->kill, playerState->death);
			}
		}
	}
}

void UEOSGameInstance::ClearEndGameState()
{
	ReturnGameState = nullptr;
	ReturnGameState = NewObject<UReturnGameState>();
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
			const bool isSessionPrivate = session.Session.SessionSettings.bAllowJoinViaPresenceFriendsOnly;
			int minPlayers, maxPlayers;
			if(isSessionPrivate)
			{
				maxPlayers = session.Session.SessionSettings.NumPrivateConnections;
				minPlayers = session.Session.SessionSettings.NumPrivateConnections-session.Session.NumOpenPrivateConnections;
			}
			else
			{
				maxPlayers = session.Session.SessionSettings.NumPublicConnections;
				minPlayers = session.Session.SessionSettings.NumPublicConnections-session.Session.NumOpenPublicConnections;
			}
			UE_LOG(LogTemp, Warning, TEXT("Sessions maxP : %d"), maxPlayers);
			UE_LOG(LogTemp, Warning, TEXT("Sessions minP : %d"), minPlayers);
			FString sessionNameString = FString();
			if(session.Session.SessionSettings.Settings.Contains(SETTING_SESSIONKEY))
			{
				const auto val = session.Session.SessionSettings.Settings.Find(SETTING_SESSIONKEY);
				UE_LOG(LogTemp, Warning, TEXT("strRoomName : %s"), *val->ToString());
				sessionNameString = val->Data.ToString();
			}
			if(sessionNameString.IsEmpty())
			{
				sessionNameString = session.GetSessionIdStr();
			}
			
			newEntry->SetSessionData(sessionNameString, maxPlayers, minPlayers, isSessionPrivate);
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Sessions in search result"));
			UE_LOG(LogTemp, Warning, TEXT("Sessions in search result : %s"), *sessionNameString);
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
	isLoading = true;
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
					CurrentlyJoiningSessionIndex = index;  //set index of search result to join
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
		isLoading = false;
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
		const int mapIndex = DebugLevelSelect != -1 ? DebugLevelSelect : FMath::RandRange(0, LevelText.Num()-1);
		GetWorld()->ServerTravel(LevelText[mapIndex] +"?listen", true);
	}
	isLoading = false;
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
	isLoading = false;
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
	isLoading = false;
	OnFindSessionsDelegate.Broadcast();
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
						if(SearchSettings->SearchResults[index].Session.SessionSettings.Settings.Contains(SETTING_MAPNAME))
						{
							auto val = SearchSettings->SearchResults[index].Session.SessionSettings.Settings.Find(SETTING_MAPNAME);
							int32 mapIndex = -999;
							val->Data.GetValue(mapIndex);
							UE_LOG(LogTemp, Warning, TEXT("index MapKeys : %d"), mapIndex);
						}
						if(SearchSettings->SearchResults[index].Session.SessionSettings.Settings.Contains(SETTING_SESSIONKEY))
						{
							auto val = SearchSettings->SearchResults[index].Session.SessionSettings.Settings.Find(SETTING_SESSIONKEY);
							UE_LOG(LogTemp, Warning, TEXT("strRoomName : %s"), *val->ToString());
						}
					}
					OnlineSessionPtr->ClearOnJoinSessionCompleteDelegates(this);
				}
			}
			if(!JoinURL.IsEmpty())
			{
				PlayerController->ClientTravel(JoinURL, ETravelType::TRAVEL_Absolute);
				CurrentlyJoiningSessionIndex = -1;  //reset as joining process finished
			}
		}
	}
	isLoading = false;
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
