// Copyright (c) 2026 Age-1024. Licensed under the MIT License.

using UnrealBuildTool;
using System.Collections.Generic;

public class CalamityHuntEditorTarget : TargetRules
{
    public CalamityHuntEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
        ExtraModuleNames.Add("CalamityHunt");
    }
}
