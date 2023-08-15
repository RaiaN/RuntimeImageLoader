// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using UnrealBuildTool;

public class RuntimeGifLibrary : ModuleRules
{
	public RuntimeGifLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		//if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PublicDefinitions.Add("WITH_LIBNSGIF=1");

			string IncPath = Path.Combine(ModuleDirectory, "include");
			PublicSystemIncludePaths.Add(IncPath);

			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "x64", "Release", "libnsgif.lib"));
		}
		if (Target.Platform == UnrealTargetPlatform.Linux)
		{
            // TODO
        }
        else if (Target.Platform == UnrealTargetPlatform.Android)
		{
			// TODO
		}
    }
}
