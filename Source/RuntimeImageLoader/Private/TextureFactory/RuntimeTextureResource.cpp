// Copyright 2023 Petr Leontev. All Rights Reserved.

#include "RuntimeTextureResource.h"
#include "PixelFormat.h"
#include "Engine/Texture.h"
#include "RenderResource.h"
#include "RHICommandList.h"

DEFINE_LOG_CATEGORY_STATIC(LogRuntimeTextureResource, Log, All);

FRuntimeTextureResource::FRuntimeTextureResource(UTexture* InTexture, FTextureRHIRef InRHITexture)
: Owner(InTexture), SizeX(InRHITexture->GetSizeXYZ().X), SizeY(InRHITexture->GetSizeXYZ().Y)
{
    TextureRHI = InRHITexture;
    bSRGB = (TextureRHI->GetFlags() & TexCreate_SRGB) != TexCreate_None;
    bIgnoreGammaConversions = !bSRGB;
    bGreyScaleFormat = (TextureRHI->GetFormat() == PF_G8) || (TextureRHI->GetFormat() == PF_BC4);

    UE_LOG(LogRuntimeTextureResource, Verbose, TEXT("RuntimeTextureResource has been created!"))
}

FRuntimeTextureResource::~FRuntimeTextureResource()
{
    /*if (IsValid(Owner))
    {
        Owner->SetResource(nullptr);
    }*/
    TextureRHI.SafeRelease();

    UE_LOG(LogRuntimeTextureResource, Verbose, TEXT("RuntimeTextureResource has been destroyed!"))
}

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION < 3
void FRuntimeTextureResource::InitRHI()
#else
void FRuntimeTextureResource::InitRHI(FRHICommandListBase& RHICmdList)
#endif
{
    // implement in child classes
    check(false);
}

void FRuntimeTextureResource::ReleaseRHI()
{
    if (IsValid(Owner))
    {
        RHIUpdateTextureReference(Owner->TextureReference.TextureReferenceRHI, nullptr);
    }
    FTextureResource::ReleaseRHI();

    UE_LOG(LogRuntimeTextureResource, Verbose, TEXT("RuntimeTextureResource RHI has been destroyed!"))
}
