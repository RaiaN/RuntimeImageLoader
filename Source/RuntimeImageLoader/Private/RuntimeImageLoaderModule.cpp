// Copyright 2023 Petr Leontev. All Rights Reserved.

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