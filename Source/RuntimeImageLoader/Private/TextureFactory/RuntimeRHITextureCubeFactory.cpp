// Copyright 2022 Peter Leontev. All Rights Reserved.

#include "RuntimeRHITextureCubeFactory.h"
#include "Engine/TextureCube.h"
#include "DynamicRHI.h"
#include "RHI.h"
#include "RHIDefinitions.h"
#include "RHIResources.h"
#include "RenderUtils.h"
#include "RHICommandList.h"
#include "Containers/ResourceArray.h"
#include "HAL/Platform.h"
#include "TextureResource.h"
#include "Async/TaskGraphInterfaces.h"

#include "RuntimeTextureCubeResource.h"
#include "RuntimeImageData.h"

PRAGMA_ENABLE_OPTIMIZATION

FRuntimeRHITextureCubeFactory::FRuntimeRHITextureCubeFactory(UTextureCube* InTextureCube, const FRuntimeImageData& InImageData)
: NewTextureCube(InTextureCube), ImageData(InImageData)
{}


/* Faces ordered this way
enum ECubeFace
{
    CubeFace_PosX = 0,
    CubeFace_NegX,
    CubeFace_PosY,
    CubeFace_NegY,
    CubeFace_PosZ,
    CubeFace_NegZ,
    CubeFace_MAX
};
*/

struct FTextureCubeDataResource : public FResourceBulkDataInterface
{
public:
    FTextureCubeDataResource(void* InMipData, int32 InDataSize)
        : MipData(InMipData), DataSize(InDataSize)
    {}

    const void* GetResourceBulkData() const override { return MipData; }
    uint32 GetResourceBulkDataSize() const override { return DataSize; }
    void Discard() override {}

private:
    void* MipData;
    int32 DataSize;
};

FTextureCubeRHIRef FRuntimeRHITextureCubeFactory::Create()
{
    RHITextureCube = CreateTextureCubeRHI_Windows();

    FinalizeRHITexture2D();
    
    return RHITextureCube;
}

FTextureCubeRHIRef FRuntimeRHITextureCubeFactory::CreateTextureCubeRHI_Windows()
{
    FTextureCubeRHIRef TextureCubeRHI = nullptr;
    
    int32 NumMips = 1;
    uint32 NumSamples = 1;

    ensureMsgf(ImageData.SizeX > 0, TEXT("ImageData.SizeX must be > 0"));
    ensureMsgf(ImageData.SizeY > 0, TEXT("ImageData.SizeY must be > 0"));

    ETextureCreateFlags TextureFlags = TexCreate_ShaderResource | (ImageData.SRGB ? TexCreate_SRGB : TexCreate_None);
    ERHIAccess ResourceState = RHIGetDefaultResourceState(TextureFlags, true);

    FRHIResourceCreateInfo CreateInfo(TEXT("RuntimeImageReader_TextureCubeData"));

    FTextureCubeDataResource TextureCubeData((void*)ImageData.RawData.GetData(), ImageData.RawData.Num());
    CreateInfo.BulkData = &TextureCubeData;

    FGraphEventRef CreateTextureTask = FFunctionGraphTask::CreateAndDispatchWhenReady(
        [this, &TextureCubeRHI, &CreateInfo, ResourceState, TextureFlags]()
        {
            TextureCubeRHI = RHICreateTextureCube(
                ImageData.SizeX, ImageData.PixelFormat, 1, TextureFlags, CreateInfo
            );
        }, TStatId(), nullptr, ENamedThreads::ActualRenderingThread
    );
    CreateTextureTask->Wait();

    return TextureCubeRHI;
}

void FRuntimeRHITextureCubeFactory::FinalizeRHITexture2D()
{
    // Create texture resource that returns actual texture size so that UMG can display the texture
    FRuntimeTextureResource* NewTextureResource = new FRuntimeTextureCubeResource(NewTextureCube, RHITextureCube);
    NewTextureCube->SetResource(NewTextureResource);

    FGraphEventRef UpdateResourceTask = FFunctionGraphTask::CreateAndDispatchWhenReady(
        [this, &NewTextureResource]()
        {
            NewTextureResource->InitResource();
            RHIUpdateTextureReference(NewTextureCube->TextureReference.TextureReferenceRHI, RHITextureCube);
            NewTextureResource->SetTextureReference(NewTextureCube->TextureReference.TextureReferenceRHI);

        }, TStatId(), nullptr, ENamedThreads::ActualRenderingThread
    );
    UpdateResourceTask->Wait();
}

void FRuntimeRHITextureCubeFactory::CopySrcDataToLockedDestData(uint8* Src, uint8* Dest, uint32 DestPitch, uint32 MipSize)
{
    EPixelFormat PixelFormat = NewTextureCube->GetPixelFormat();
    uint32 NumRows = 0;
    uint32 SrcPitch = 0;

    CopyTextureData2D(Src, Dest, ImageData.SizeY, NewTextureCube->GetPixelFormat(), ImageData.SizeY, DestPitch);
}

PRAGMA_DISABLE_OPTIMIZATION