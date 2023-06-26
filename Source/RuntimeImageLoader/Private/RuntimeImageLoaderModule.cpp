// Copyright 2022 Peter Leontev. All Rights Reserved.

#include "RuntimeImageLoaderModule.h"
#include "RuntimeImageLoaderLibHandler.h"

#define LOCTEXT_NAMESPACE "FRuntimeImageLoaderModule"

void FRuntimeImageLoaderModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FLibnsgifHandler::Initialize();
}

void FRuntimeImageLoaderModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FLibnsgifHandler::Shutdown();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FRuntimeImageLoaderModule, RuntimeImageLoader)