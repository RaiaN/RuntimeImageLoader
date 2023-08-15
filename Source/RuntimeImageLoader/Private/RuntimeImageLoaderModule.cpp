// Copyright 2022 Peter Leontev. All Rights Reserved.

#include "RuntimeImageLoaderModule.h"

#define LOCTEXT_NAMESPACE "FRuntimeImageLoaderModule"

void FRuntimeImageLoaderModule::StartupModule()
{
}

void FRuntimeImageLoaderModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FRuntimeImageLoaderModule, RuntimeImageLoader)