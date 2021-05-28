// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class RefreshAllNodes : ModuleRules
{
	public RefreshAllNodes(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(new string[] {
				"Core",	"CoreUObject", "Engine", "InputCore"});

		PrivateDependencyModuleNames.AddRange(new string[] {
				"UnrealEd", "Slate", "SlateCore", "EditorStyle"});
	
		if (Target.Version.MinorVersion >= 26 || Target.Version.MajorVersion > 4) {
			PrivateDependencyModuleNames.AddRange(new string[] {
				"DeveloperSettings"});
		}
	}
}
