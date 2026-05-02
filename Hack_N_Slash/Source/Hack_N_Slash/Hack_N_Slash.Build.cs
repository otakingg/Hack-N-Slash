// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Hack_N_Slash : ModuleRules
{
	public Hack_N_Slash(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"GameplayTags",
			"NavigationSystem",
			"MotionWarping",
			"RootMovement"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"Hack_N_Slash",
			"Hack_N_Slash/Variant_Platforming",
			"Hack_N_Slash/Variant_Platforming/Animation",
			"Hack_N_Slash/Variant_Combat",
			"Hack_N_Slash/Variant_Combat/AI",
			"Hack_N_Slash/Variant_Combat/Animation",
			"Hack_N_Slash/Variant_Combat/Gameplay",
			"Hack_N_Slash/Variant_Combat/Interfaces",
			"Hack_N_Slash/Variant_Combat/UI",
			"Hack_N_Slash/Variant_SideScrolling",
			"Hack_N_Slash/Variant_SideScrolling/AI",
			"Hack_N_Slash/Variant_SideScrolling/Gameplay",
			"Hack_N_Slash/Variant_SideScrolling/Interfaces",
			"Hack_N_Slash/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
