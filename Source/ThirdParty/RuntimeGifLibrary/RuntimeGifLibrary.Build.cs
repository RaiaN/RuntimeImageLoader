// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using UnrealBuildTool;

public class RuntimeGifLibrary : ModuleRules
{
	public RuntimeGifLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		if (Target.Platform == UnrealTargetPlatform.Win64 || Target.Platform == UnrealTargetPlatform.Android)
		{
			PublicDefinitions.Add("WITH_LIBNSGIF=1");

			string IncPath = Path.Combine(ModuleDirectory, "include");
			PublicSystemIncludePaths.Add(IncPath);

			// Add the import library
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "x64", "Release", "libnsgif.lib"));

			// Delay-load the DLL, so we can load it from the right place first
			PublicDelayLoadDLLs.Add("libnsgif.dll");
			
			// Ensure that the DLL is staged along with the executable
			RuntimeDependencies.Add(
                "$(BinaryOutputDir)/ThirdParty/RuntimeGifLibrary/Win64/libnsgif.dll",
				Path.Combine(ModuleDirectory, "x64", "Release", "libnsgif.dll")
				);
		}
		else
        {
			PublicDefinitions.Add("WITH_LIBNSGIF=0");
		}
	}
}
