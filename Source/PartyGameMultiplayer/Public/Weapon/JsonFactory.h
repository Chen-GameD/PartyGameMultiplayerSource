// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Templates/SharedPointer.h"
#include "JsonFactory.generated.h"

class FJsonObject;

UCLASS()
class PARTYGAMEMULTIPLAYER_API UJsonFactory : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static void dosomething();

	UFUNCTION(BlueprintCallable, Category = "File I/O")
		static FString LoadFileToString(FString Filename);
	UFUNCTION(BlueprintCallable, Category = "File I/O")
		static TArray<FString> LoadFileToStringArray(FString FIlename);
	UFUNCTION(BlueprintCallable, Category = "File I/O")
		static bool WriteStringToFile(FString FileName, FString String);

	// Note: TSharedPtr cannot be used associated with a U* macro
	static TSharedPtr<FJsonObject> ReadJson(FString JsonFilePath);
	static bool WriteJson(FString JsonFilePath, TSharedPtr<FJsonObject> JsonObject);

	static bool InitJsonObject_1();
	static TSharedPtr<FJsonObject> GetJsonObject_1();

protected:
private:

public:
protected:
	static TSharedPtr<FJsonObject> JsonObject_1;
private:

};
