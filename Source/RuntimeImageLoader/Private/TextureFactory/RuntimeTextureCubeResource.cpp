// Copyright 2022 Peter Leontev. All Rights Reserved.

#include "RuntimeTextureCubeResource.h"
#include "Engine/TextureCube.h"
#include "Templates/RefCounting.h"

DEFINE_LOG_CATEGORY_STATIC(LogRuntimeTextureCubeResource, Log, All);

FRuntimeTextureCubeResource::FRuntimeTextureCubeResource(UTextureCube* InOwner, FTextureCubeRHIRef RHITextureCube)
: FRuntimeTextureResource(CastChecked<UTexture>(InOwner), TRefCountPtr<FRHITexture>(RHITextureCube))
{

    UE_LOG(LogRuntimeTextureCubeResource, Verbose, TEXT("RuntimeTextureCubeResource has been created!"))
}

FRuntimeTextureCubeResource::~FRuntimeTextureCubeResource()
{
    UE_LOG(LogRuntimeTextureCubeResource, Verbose, TEXT("RuntimeTextureCubeResource has been destroyed!"))
}

void FRuntimeTextureCubeResource::InitRHI()
{
    // Create the sampler state RHI resource.
    FSamplerStateInitializerRHI SamplerStateInitializer
    (
        SF_Trilinear,
        AM_Clamp,
        AM_Clamp,
        AM_Clamp
    );
    SamplerStateRHI = GetOrCreateSamplerState(SamplerStateInitializer);

    UE_LOG(LogRuntimeTextureCubeResource, Verbose, TEXT("RuntimeTextureCubeResource RHI has been created!"))
}
