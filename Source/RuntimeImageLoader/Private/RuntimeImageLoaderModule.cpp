// Copyright 2022 Peter Leontev. All Rights Reserved.

#include "RuntimeImageLoaderModule.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/MessageDialog.h"
#include "Misc/Paths.h"

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

	if (LibnsgifHandle)
	{
		UE_LOG(LogTemp, Display, TEXT("libnsgif.dll Loaded."));

		Fn_nsgif_create = (nsgif_create_FnPtr)FPlatformProcess::GetDllExport(LibnsgifHandle, TEXT("nsgif_create"));
		Fn_nsgif_destroy = (nsgif_destroy_FnPtr)FPlatformProcess::GetDllExport(LibnsgifHandle, TEXT("nsgif_destroy"));
		Fn_nsgif_data_scan = (nsgif_data_scan_FnPtr)FPlatformProcess::GetDllExport(LibnsgifHandle, TEXT("nsgif_data_scan"));
		Fn_nsgif_data_complete = (nsgif_data_complete_FnPtr)FPlatformProcess::GetDllExport(LibnsgifHandle, TEXT("nsgif_data_complete"));
		Fn_nsgif_frame_prepare = (nsgif_frame_prepare_FnPtr)FPlatformProcess::GetDllExport(LibnsgifHandle, TEXT("nsgif_frame_prepare"));
		Fn_nsgif_frame_decode = (nsgif_frame_decode_FnPtr)FPlatformProcess::GetDllExport(LibnsgifHandle, TEXT("nsgif_frame_decode"));
		Fn_nsgif_reset = (nsgif_reset_FnPtr)FPlatformProcess::GetDllExport(LibnsgifHandle, TEXT("nsgif_reset"));
		Fn_nsgif_get_info = (nsgif_get_info_FnPtr)FPlatformProcess::GetDllExport(LibnsgifHandle, TEXT("nsgif_get_info"));

		// Throwaway code just check weather Dll get loaded or not. ---- Start ----
		if (Fn_nsgif_create && Fn_nsgif_destroy && Fn_nsgif_data_scan && Fn_nsgif_data_complete && Fn_nsgif_frame_prepare && Fn_nsgif_frame_decode && Fn_nsgif_reset && Fn_nsgif_get_info)
			UE_LOG(LogTemp, Display, TEXT("libnsgif methods Loaded."));
		// ---- End ----
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ThirdPartyLibraryError", "Failed to load Libnsgif library"));
	}
}

void FRuntimeImageLoaderModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	// Free the dll handle
	FPlatformProcess::FreeDllHandle(LibnsgifHandle);
	LibnsgifHandle = nullptr;
}

nsgif_create_FnPtr FRuntimeImageLoaderModule::FunctionPointerNsgifCreate()
{
	return Fn_nsgif_create;
}

nsgif_destroy_FnPtr FRuntimeImageLoaderModule::FunctionPointerNsgifDestroy()
{
	return Fn_nsgif_destroy;
}

nsgif_data_scan_FnPtr FRuntimeImageLoaderModule::FunctionPointerNsgifDataScan()
{
	return Fn_nsgif_data_scan;
}

nsgif_data_complete_FnPtr FRuntimeImageLoaderModule::FunctionPointerNsgifDataComplete()
{
	return Fn_nsgif_data_complete;
}

nsgif_frame_prepare_FnPtr FRuntimeImageLoaderModule::FunctionPointerNsgifFramePrepare()
{
	return Fn_nsgif_frame_prepare;
}

nsgif_frame_decode_FnPtr FRuntimeImageLoaderModule::FunctionPointerNsgifFrameDecode()
{
	return Fn_nsgif_frame_decode;
}

nsgif_reset_FnPtr FRuntimeImageLoaderModule::FunctionPointerNsgifReset()
{
	return Fn_nsgif_reset;
}

nsgif_get_info_FnPtr FRuntimeImageLoaderModule::FunctionPointerNsgifGetInfo()
{
	return Fn_nsgif_get_info;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FRuntimeImageLoaderModule, RuntimeImageLoader)