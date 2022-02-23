// Copyright 2022 Peter Leontev. All Rights Reserved.

using UnrealBuildTool;

public class RuntimeImageLoader : ModuleRules
{
	public RuntimeImageLoader(ReadOnlyTargetRules Target) : base(Target)
	{
        {
            bEnforceIWYU = true;
            bUseUnity = false;

            PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        }

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
				"ImageWrapper",
				"RenderCore",

				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
