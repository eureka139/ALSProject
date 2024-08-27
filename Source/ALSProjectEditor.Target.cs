

using UnrealBuildTool;
using System.Collections.Generic;

public class ALSProjectEditorTarget : TargetRules
{
	public ALSProjectEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;

		ExtraModuleNames.AddRange( new string[] { "ALSProject" } );
	}
}
