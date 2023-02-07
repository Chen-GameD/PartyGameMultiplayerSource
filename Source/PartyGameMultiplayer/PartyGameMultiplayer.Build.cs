// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PartyGameMultiplayer : ModuleRules
{
	public PartyGameMultiplayer(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "Niagara", "UMG", "OnlineSubsystem", "OnlineSubsystemEOS", "OnlineSubsystemUtils" ,"Json", "JsonUtilities" });

		//PublicIncludePaths.AddRange(new string[] { Path.Combine(ModuleDirectory, "Niagara") });
	}
}
