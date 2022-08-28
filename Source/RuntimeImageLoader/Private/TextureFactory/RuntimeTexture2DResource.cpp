// Copyright 2022 Peter Leontev. All Rights Reserved.

#include "RuntimeTexture2DResource.h"
#include "Engine/Texture2D.h"

DEFINE_LOG_CATEGORY_STATIC(LogRuntimeTexture2DResource, Log, All);

FRuntimeTexture2DResource::FRuntimeTexture2DResource(UTexture2D* InOwner, FTexture2DRHIRef InRHITexture2D)
: FRuntimeTextureResource(CastChecked<UTexture>(InOwner), TRefCountPtr<FRHITexture>(InRHITexture2D))
{
    UE_LOG(LogRuntimeTexture2DResource, Verbose, TEXT("RuntimeTexture2DResource has been created!"))
}

FRuntimeTexture2DResource::~FRuntimeTexture2DResource()
{
    UE_LOG(LogRuntimeTexture2DResource, Verbose, TEXT("RuntimeTexture2DResource has been destroyed!"))
}

void FRuntimeTexture2DResource::InitRHI()
{
    // TODO: this should be reevaluated as runtime textures do not have mips just yet

    // Create the sampler state RHI resource.
    FSamplerStateInitializerRHI SamplerStateInitializer(SF_Trilinear);
    SamplerStateRHI = GetOrCreateSamplerState(SamplerStateInitializer);

    // Create a custom sampler state for using this texture in a deferred pass, where ddx / ddy are discontinuous
    FSamplerStateInitializerRHI DeferredPassSamplerStateInitializer(
        SF_Trilinear,
        AM_Wrap,
        AM_Wrap,
        AM_Wrap,
        0,
        // Disable anisotropic filtering, since aniso doesn't respect MaxLOD
        1,
        0,
        // Prevent the less detailed mip levels from being used, which hides artifacts on silhouettes due to ddx / ddy being very large
        // This has the side effect that it increases minification aliasing on light functions
        2
    );

    DeferredPassSamplerStateRHI = GetOrCreateSamplerState(DeferredPassSamplerStateInitializer);

    UE_LOG(LogRuntimeTexture2DResource, Verbose, TEXT("RuntimeTexture2DResource RHI has been created!"))
}
