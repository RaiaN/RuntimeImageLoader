// Copyright 2023 Petr Leontev. All Rights Reserved.

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

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION < 3
void FRuntimeTextureCubeResource::InitRHI()
#else
void FRuntimeTextureCubeResource::InitRHI(FRHICommandListBase& RHICmdList)
#endif
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
