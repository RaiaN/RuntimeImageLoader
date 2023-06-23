// Copyright 2022 Peter Leontev. All Rights Reserved.

#include "RuntimeImageLoaderModule.h"
#include "Core.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FRuntimeImageLoaderModule"

void FRuntimeImageLoaderModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	// Get the base directory of this plugin
	FString BaseDir = IPluginManager::Get().FindPlugin("RuntimeImageLoader")->GetBaseDir();

	// Add on the relative location of the third party dll and load it
	FString LibraryPath;
#if PLATFORM_WINDOWS
	LibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/RuntimeImageLoaderLibrary/Win64/libnsgif.dll"));
#endif // PLATFORM_WINDOWS

	LibnsgifHandle = !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;

	// Throwaway code just check weather Dll get loaded or not. ---- Start ----
	if (LibnsgifHandle)
	{
		UE_LOG(LogTemp, Display, TEXT("libnsgif.dll Loaded"));
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ThirdPartyLibraryError", "Failed to load example third party library"));
	}
	// ---- End ----
}

void FRuntimeImageLoaderModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	// Free the dll handle
	FPlatformProcess::FreeDllHandle(LibnsgifHandle);
	LibnsgifHandle = nullptr;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FRuntimeImageLoaderModule, RuntimeImageLoader)