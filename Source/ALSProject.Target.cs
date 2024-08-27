

using UnrealBuildTool;
using System.Collections.Generic;

public class ALSProjectTarget : TargetRules
{
	public ALSProjectTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;

		ExtraModuleNames.AddRange( new string[] { "ALSProject" } );
	}
}
