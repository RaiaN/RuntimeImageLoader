// Copyright 2022 Peter Leontev. All Rights Reserved.

#include "RuntimeTextureResource.h"
#include "PixelFormat.h"
#include "Engine/Texture.h"
#include "Engine/Texture2D.h"
#include "RenderResource.h"

DEFINE_LOG_CATEGORY_STATIC(LogRuntimeTextureResource, Log, All);

FRuntimeTextureResource::FRuntimeTextureResource(UTexture2D* InTexture, FTexture2DRHIRef RHITexture2D)
: Owner(InTexture), SizeX(RHITexture2D->GetSizeX()), SizeY(RHITexture2D->GetSizeY())
{
    TextureRHI = RHITexture2D;
    bSRGB = (TextureRHI->GetFlags() & TexCreate_SRGB) != TexCreate_None;
    bIgnoreGammaConversions = !bSRGB;
    bGreyScaleFormat = (TextureRHI->GetFormat() == PF_G8) || (TextureRHI->GetFormat() == PF_BC4);

    UE_LOG(LogRuntimeTextureResource, Verbose, TEXT("RuntimeTextureResource has been created!"))
}

FRuntimeTextureResource::~FRuntimeTextureResource()
{
    Owner->SetResource(nullptr);

    UE_LOG(LogRuntimeTextureResource, Verbose, TEXT("RuntimeTextureResource has been destroyed!"))
}

void FRuntimeTextureResource::InitRHI()
{
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

    UE_LOG(LogRuntimeTextureResource, Verbose, TEXT("RuntimeTextureResource RHI has been created!"))
}

void FRuntimeTextureResource::ReleaseRHI()
{
    RHIUpdateTextureReference(Owner->TextureReference.TextureReferenceRHI, nullptr);
    FTextureResource::ReleaseRHI();

    UE_LOG(LogRuntimeTextureResource, Verbose, TEXT("RuntimeTextureResource RHI has been destroyed!"))
}
