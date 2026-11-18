// Copyright (c) 2026 Age-1024. Licensed under the MIT License.

using UnrealBuildTool;
using System.Collections.Generic;

public class CalamityHuntTarget : TargetRules
{
    public CalamityHuntTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
        ExtraModuleNames.Add("CalamityHunt");
    }
}
