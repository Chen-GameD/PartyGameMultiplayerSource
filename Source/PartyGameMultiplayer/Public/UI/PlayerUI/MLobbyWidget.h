// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "MLobbyWidget.generated.h"

/**
 * 
 */
USTRUCT()
struct FLobbyInformationStruct
{
	GENERATED_BODY()

	// Member
	FString PlayerName;
	int TeamIndex;
	bool IsReady;
};

UCLASS()
class PARTYGAMEMULTIPLAYER_API UMLobbyWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UMLobbyWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	UFUNCTION()
	void UpdateLobbyInformation(TArray<FLobbyInformationStruct> i_Team1Arr, TArray<FLobbyInformationStruct> i_Team2Arr, TArray<FLobbyInformationStruct> i_UndecidedArr);
	UFUNCTION()
	void UpdateTeam1LobbyInformation(TArray<FLobbyInformationStruct> i_TeamArr);
	UFUNCTION()
	void UpdateTeam2LobbyInformation(TArray<FLobbyInformationStruct> i_TeamArr);
	UFUNCTION()
	void UpdateUndecidedLobbyInformation(TArray<FLobbyInformationStruct> i_TeamArr);
protected:
	// Team Information Element
	////////////////////////////////////////////////////
	// Team1
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Team_1_Player1;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Team_1_Player2;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Team_1_Player3;

	// Team2
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Team_2_Player1;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Team_2_Player2;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Team_2_Player3;

	// Undecided
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Undecided_Player1;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Undecided_Player2;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Undecided_Player3;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Undecided_Player4;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Undecided_Player5;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Undecided_Player6;

	// Information Container
	UPROPERTY()
	TMap<int, UTextBlock*> Team1TextContainer;
	UPROPERTY()
	TMap<int, UTextBlock*> Team2TextContainer;
	UPROPERTY()
	TMap<int, UTextBlock*> UndecidedTextContainer;
	///////////////////////////////////////////////////////////////

	// Button
	///////////////////////////////////////////////////////////////
	UPROPERTY(meta = (BindWidget))
	UButton* Btn_JoinTeam_1;
	UPROPERTY(meta = (BindWidget))
	UButton* Btn_JoinTeam_2;
	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Ready;
	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Invite;
	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Back;
	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Info;
};
