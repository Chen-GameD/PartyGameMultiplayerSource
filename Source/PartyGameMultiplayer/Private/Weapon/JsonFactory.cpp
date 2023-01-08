// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/JsonFactory.h"

#include "Serialization/JsonSerializer.h" // Json
#include "JsonObjectConverter.h"	// JsonUtilities

TSharedPtr<FJsonObject> UJsonFactory::JsonObject_1 = nullptr;

void UJsonFactory::dosomething()
{
	FString JsonString = "";
	TSharedPtr<FJsonObject> RetJsonObject;
	if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(JsonString), RetJsonObject))
	{
		return;
	}
	else
	{
		return;
	}
}


FString UJsonFactory::LoadFileToString(FString Filename)
{
	IPlatformFile& file = FPlatformFileManager::Get().GetPlatformFile();
	FString directory = FPaths::ProjectContentDir();
	FString result;

	if (file.CreateDirectory(*directory))
	{		
		FString myFile = directory + "/" + Filename;
		// check if the file exists
		if (!file.FileExists(*myFile))
		{
			return "";
		}
		FFileHelper::LoadFileToString(result, *myFile);
	}

	return result;
}


TArray<FString> UJsonFactory::LoadFileToStringArray(FString Filename)
{
	FString directory = FPaths::ProjectContentDir();
	TArray<FString> result;
	IPlatformFile& file = FPlatformFileManager::Get().GetPlatformFile();

	if (file.CreateDirectory(*directory))
	{
		FString myFile = directory + "/" + Filename;
		FFileHelper::LoadFileToStringArray(result, *myFile);
	}

	return result;
}


bool UJsonFactory::WriteStringToFile(FString FileName, FString String)
{
	FString directory = FPaths::ProjectContentDir();
	FString myFile = directory + "/" + FileName;
	// Try to write the string into the file
	if (!FFileHelper::SaveStringToFile(String, *myFile))
	{
		return false;
	}
	return true;
}

TSharedPtr<FJsonObject> UJsonFactory::ReadJson(FString JsonFilePath)
{
	FString JsonString = LoadFileToString(JsonFilePath);
	if (JsonString == "")
		return nullptr;

	TSharedPtr<FJsonObject> RetJsonObject;
	// Try to convert string to json object. Output goes in RetJsonObject
	if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(JsonString), RetJsonObject))
	{
		return nullptr;
	}
	
	return RetJsonObject;
}

bool UJsonFactory::WriteJson(FString JsonFilePath, TSharedPtr<FJsonObject> JsonObject)
{
	FString JsonString;

	// Try to convert json object to string. Output goes in JsonString
	if (!FJsonSerializer::Serialize(JsonObject.ToSharedRef(), TJsonWriterFactory<>::Create(&JsonString, 0)))
	{
		return false;
	}

	// Try to write json string to file
	if (WriteStringToFile(JsonFilePath, JsonString))
	{
		return false;
	}
	return true;
}


bool UJsonFactory::InitJsonObject_1()
{
	JsonObject_1 = ReadJson("DataFiles/test.json");
	return (JsonObject_1 != nullptr);
}


TSharedPtr<FJsonObject> UJsonFactory::GetJsonObject_1()
{
	if (!JsonObject_1)
	{
		JsonObject_1 = ReadJson("DataFiles/test.json");
	}
	return JsonObject_1;	
}