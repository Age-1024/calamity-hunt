// Copyright (c) 2026 Age-1024. Licensed under the MIT License.

using UnrealBuildTool;

public class CalamityHunt : ModuleRules
{
    public CalamityHunt(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core", "CoreUObject", "Engine", "InputCore", "AIModule"
        });
    }
}
