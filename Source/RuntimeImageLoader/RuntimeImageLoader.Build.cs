// Copyright 2023 Unreal Solutions Ltd. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class RuntimeImageLoader : ModuleRules
{
	public RuntimeImageLoader(ReadOnlyTargetRules Target) : base(Target)
	{
		var EngineDir = Path.GetFullPath(Target.RelativeEnginePath);

		{
            bEnforceIWYU = true;
            bUseUnity = false;

            PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        }
		
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "../", "ThirdParty", "RuntimeGifLibrary", "include"));

		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"RHI",
				"RenderCore",
				"Renderer",
				"ImageWrapper",
				"ImageCore",
				"FreeImage",
				"HTTP",
                "RuntimeGifLibrary",
				"Projects"
				// ... add private dependencies that you statically link with here ...	
			}
			);

        PrivateIncludePaths.AddRange(new string[]
        {
			Path.Combine(EngineDir, @"Source/Runtime/Renderer/Private")
        });

        DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
		);
    }
}
