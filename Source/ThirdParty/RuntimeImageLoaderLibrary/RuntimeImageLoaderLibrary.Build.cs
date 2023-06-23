// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using UnrealBuildTool;

public class RuntimeImageLoaderLibrary : ModuleRules
{
	public RuntimeImageLoaderLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			string IncPath = Path.Combine(ModuleDirectory, "include");
			PublicSystemIncludePaths.Add(IncPath);

			// Add the import library
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "x64", "Release", "libnsgif.lib"));

			// Delay-load the DLL, so we can load it from the right place first
			PublicDelayLoadDLLs.Add("libnsgif.dll");

			// Ensure that the DLL is staged along with the executable
			RuntimeDependencies.Add("$(PluginDir)/Binaries/ThirdParty/RuntimeImageLoaderLibrary/Win64/libnsgif.dll");
        }
  //      else if (Target.Platform == UnrealTargetPlatform.Mac)
  //      {
  //          PublicDelayLoadDLLs.Add(Path.Combine(ModuleDirectory, "Mac", "Release", "libExampleLibrary.dylib"));
  //          RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/RuntimeImageLoaderLibrary/Mac/Release/libExampleLibrary.dylib");
  //      }
  //      else if (Target.Platform == UnrealTargetPlatform.Linux)
		//{
		//	string ExampleSoPath = Path.Combine("$(PluginDir)", "Binaries", "ThirdParty", "RuntimeImageLoaderLibrary", "Linux", "x86_64-unknown-linux-gnu", "libExampleLibrary.so");
		//	PublicAdditionalLibraries.Add(ExampleSoPath);
		//	PublicDelayLoadDLLs.Add(ExampleSoPath);
		//	RuntimeDependencies.Add(ExampleSoPath);
		//}
	}
}
