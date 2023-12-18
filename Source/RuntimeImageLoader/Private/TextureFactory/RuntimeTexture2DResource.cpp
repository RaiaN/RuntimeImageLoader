// Copyright 2023 Petr Leontev. All Rights Reserved.

#include "RuntimeTexture2DResource.h"
#include "Engine/Texture2D.h"

DEFINE_LOG_CATEGORY_STATIC(LogRuntimeTexture2DResource, Log, All);

FRuntimeTexture2DResource::FRuntimeTexture2DResource(UTexture2D* InOwner, FTexture2DRHIRef InRHITexture2D, TextureFilter InFilterMode)
: FRuntimeTextureResource(CastChecked<UTexture>(InOwner), TRefCountPtr<FRHITexture>(InRHITexture2D)),
    FilterMode(InFilterMode)
{
    UE_LOG(LogRuntimeTexture2DResource, Verbose, TEXT("RuntimeTexture2DResource has been created!"))
}

FRuntimeTexture2DResource::~FRuntimeTexture2DResource()
{
    UE_LOG(LogRuntimeTexture2DResource, Verbose, TEXT("RuntimeTexture2DResource has been destroyed!"))
}

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION < 3
void FRuntimeTexture2DResource::InitRHI()
#else
void FRuntimeTexture2DResource::InitRHI(FRHICommandListBase& RHICmdList)
#endif
{
    // Default to point filtering.
    ESamplerFilter Filter = ESamplerFilter::SF_Trilinear;

    switch (FilterMode)
    {
        case TF_Nearest:
            Filter = ESamplerFilter::SF_Point;
            break;
        case TF_Bilinear:
            Filter = ESamplerFilter::SF_Bilinear;
            break;
        case TF_Trilinear:
            Filter = ESamplerFilter::SF_Trilinear;
            break;
        default:
            break;
    }

    // Create the sampler state RHI resource.
    FSamplerStateInitializerRHI SamplerStateInitializer(Filter);
    SamplerStateRHI = GetOrCreateSamplerState(SamplerStateInitializer);

    // Create a custom sampler state for using this texture in a deferred pass, where ddx / ddy are discontinuous
    FSamplerStateInitializerRHI DeferredPassSamplerStateInitializer(
        Filter,
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
