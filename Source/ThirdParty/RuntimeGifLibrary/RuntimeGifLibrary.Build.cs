// Copyright 2023 Unreal Solutions Ltd. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class RuntimeGifLibrary : ModuleRules
{
	public RuntimeGifLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

        PublicDefinitions.Add("WITH_LIBNSGIF=1");
        PublicDefinitions.Add("WITH_LIBWEBP=1");

        string IncPath = Path.Combine(ModuleDirectory, "include");
        PublicSystemIncludePaths.Add(IncPath);

        if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Win64", "libnsgif.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Win64", "libwebp.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Win64", "libsharpyuv.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Win64", "libwebpdecoder.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Win64", "libwebpdemux.lib"));
        }
		else if (Target.Platform == UnrealTargetPlatform.Linux)
		{
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Linux", "libnsgif.a"));

            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Linux", "libwebp.a"));
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Linux", "libsharpyuv.a"));
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Linux", "libwebpdecoder.a"));
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Linux", "libwebpdemux.a"));

            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Linux", "libwebp.la"));
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Linux", "libsharpyuv.la"));
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Linux", "libwebpdecoder.la"));
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Linux", "libwebpdemux.la"));
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Mac", "libnsgif.a"));
            // TODO:
        }
        else if (Target.Platform == UnrealTargetPlatform.Android)
		{
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Android", "libnsgif.a"));

            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Android", "libwebp.a"));
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Android", "libsharpyuv.a"));
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Android", "libwebpdecoder.a"));
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Android", "libwebpdemux.a"));

            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Android", "libwebp.la"));
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Android", "libsharpyuv.la"));
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Android", "libwebpdecoder.la"));
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Android", "libwebpdemux.la"));
        }
    }
}
