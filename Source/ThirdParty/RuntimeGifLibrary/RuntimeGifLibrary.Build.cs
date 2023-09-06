// Copyright 2023 Unreal Solutions Ltd. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class RuntimeGifLibrary : ModuleRules
{
	public RuntimeGifLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

        PublicDefinitions.Add("WITH_LIBNSGIF=1");

        string IncPath = Path.Combine(ModuleDirectory, "include");
        PublicSystemIncludePaths.Add(IncPath);

        if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Win64", "libnsgif.lib"));
		}
		else if (Target.Platform == UnrealTargetPlatform.Linux)
		{
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Linux", "libnsgif.a"));
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Mac", "libnsgif.a"));
        }
        else if (Target.Platform == UnrealTargetPlatform.Android)
		{
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Android", "libnsgif.a"));
        }
    }
}
