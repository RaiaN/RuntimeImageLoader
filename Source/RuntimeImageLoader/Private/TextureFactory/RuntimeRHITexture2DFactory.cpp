// Copyright 2023 Petr Leontev. All Rights Reserved.

#include "RuntimeRHITexture2DFactory.h"
#include "Engine/Texture2D.h"
#include "RHI.h"
#include "DynamicRHI.h"
#include "RHIDefinitions.h"
#include "RHICommandList.h"
#include "Containers/ResourceArray.h"
#include "HAL/Platform.h"
#include "TextureResource.h"
#include "Async/TaskGraphInterfaces.h"

#include "RuntimeTexture2DResource.h"


FRuntimeRHITexture2DFactory::FRuntimeRHITexture2DFactory(UTexture2D* InTexture2D, const FRuntimeImageData& InImageData)
: NewTexture(InTexture2D), ImageData(InImageData), RHITexture2D(nullptr)
{
}


FTexture2DRHIRef FRuntimeRHITexture2DFactory::Create()
{
#if PLATFORM_WINDOWS
    RHITexture2D = CreateRHITexture2D_Windows();
#elif PLATFORM_ANDROID
    RHITexture2D = CreateRHITexture2D_Mobile();
#else
    RHITexture2D = CreateRHITexture2D_Other();
#endif

    FinalizeRHITexture2D();

    return RHITexture2D;
}

struct FTextureDataResource : public FResourceBulkDataInterface
{
public:
    FTextureDataResource(void* InMipData, int32 InDataSize)
        : MipData(InMipData), DataSize(InDataSize)
    {}

    const void* GetResourceBulkData() const override { return MipData; }
    uint32 GetResourceBulkDataSize() const override { return DataSize; }
    void Discard() override {}

private:
    void* MipData;
    int32 DataSize;
};

FTexture2DRHIRef FRuntimeRHITexture2DFactory::CreateRHITexture2D_Windows()
{
    uint32 NumMips = 1;
    uint32 NumSamples = 1;
    void* Mip0Data = (void*)ImageData.RawData.GetData();

    ETextureCreateFlags TextureFlags = TexCreate_ShaderResource;
    if (ImageData.SRGB)
    {
        TextureFlags |= TexCreate_SRGB;
    }

    ensureMsgf(ImageData.SizeX > 0, TEXT("ImageData.SizeX must be > 0"));
    ensureMsgf(ImageData.SizeY > 0, TEXT("ImageData.SizeY must be > 0"));

    if (GRHISupportsAsyncTextureCreation)
    {
        // TODO: Wait until completion?
#if (ENGINE_MAJOR_VERSION >= 5) && (ENGINE_MINOR_VERSION > 2)
        FGraphEventRef CompletionEvent;
#endif
        RHITexture2D = RHIAsyncCreateTexture2D(
            ImageData.SizeX, ImageData.SizeY,
            ImageData.PixelFormat,
            ImageData.NumMips,
            TextureFlags,
            &Mip0Data,
            1
#if (ENGINE_MAJOR_VERSION >= 5) && (ENGINE_MINOR_VERSION > 2)
            ,CompletionEvent
#endif
        );
    }
    else
    {
        FTextureDataResource TextureData(Mip0Data, ImageData.RawData.Num());

        FRHIResourceCreateInfo CreateInfo(TEXT("RuntimeImageReaderTextureData"));
        CreateInfo.BulkData = &TextureData;

        FGraphEventRef CreateTextureTask = FFunctionGraphTask::CreateAndDispatchWhenReady(
            [this, &CreateInfo, TextureFlags]()
            {
#if (ENGINE_MAJOR_VERSION >= 5) && (ENGINE_MINOR_VERSION > 0)
                RHITexture2D = RHICreateTexture(
                    FRHITextureCreateDesc::Create2D(CreateInfo.DebugName)
                    .SetExtent(ImageData.SizeX, ImageData.SizeY)
                    .SetFormat(ImageData.PixelFormat)
                    .SetNumMips(ImageData.NumMips)
                    .SetNumSamples(1)
                    .SetFlags(TextureFlags)
                    .SetInitialState(ERHIAccess::Unknown)
                    .SetExtData(CreateInfo.ExtData)
                    .SetBulkData(CreateInfo.BulkData)
                    .SetGPUMask(CreateInfo.GPUMask)
                    .SetClearValue(CreateInfo.ClearValueBinding)
                );
#else
                RHITexture2D = RHICreateTexture2D(
                    ImageData.SizeX, ImageData.SizeY,
                    ImageData.PixelFormat,
                    ImageData.NumMips,
                    1,
                    TextureFlags,
                    CreateInfo);
#endif

            }, TStatId(), nullptr, ENamedThreads::ActualRenderingThread
        );
        CreateTextureTask->Wait();
    }

    return RHITexture2D;
}

FTexture2DRHIRef FRuntimeRHITexture2DFactory::CreateRHITexture2D_Mobile()
{
    uint32 NumMips = 1;
    uint32 NumSamples = 1;
    void* Mip0Data = (void*)ImageData.RawData.GetData();

    ETextureCreateFlags TextureFlags = TexCreate_ShaderResource;
    if (ImageData.SRGB)
    {
        TextureFlags |= TexCreate_SRGB;
    }

    ensureMsgf(ImageData.SizeX > 0, TEXT("ImageData.SizeX must be > 0"));
    ensureMsgf(ImageData.SizeY > 0, TEXT("ImageData.SizeY must be > 0"));

    FGraphEventRef CreateTextureTask = FFunctionGraphTask::CreateAndDispatchWhenReady(
        [this, &TextureFlags]()
        {
            FRHIResourceCreateInfo DummyCreateInfo(TEXT("DummyCreateInfo"));
#if (ENGINE_MAJOR_VERSION >= 5) && (ENGINE_MINOR_VERSION > 0)
            RHITexture2D = RHICreateTexture(
                FRHITextureCreateDesc::Create2D(DummyCreateInfo.DebugName)
                .SetExtent(ImageData.SizeX, ImageData.SizeY)
                .SetFormat(ImageData.PixelFormat)
                .SetNumMips(ImageData.NumMips)
                .SetNumSamples(1)
                .SetFlags(TextureFlags)
                .SetInitialState(ERHIAccess::Unknown)
                .SetExtData(DummyCreateInfo.ExtData)
                .SetBulkData(DummyCreateInfo.BulkData)
                .SetGPUMask(DummyCreateInfo.GPUMask)
                .SetClearValue(DummyCreateInfo.ClearValueBinding)
            );
#else
            RHITexture2D = RHICreateTexture2D(
                ImageData.SizeX, ImageData.SizeY,
                ImageData.PixelFormat,
                ImageData.NumMips,
                1,
                TextureFlags,
                DummyCreateInfo);
#endif

            FUpdateTextureRegion2D TextureRegion2D;
            {
                TextureRegion2D.DestX = 0;
                TextureRegion2D.DestY = 0;
                TextureRegion2D.SrcX = 0;
                TextureRegion2D.SrcY = 0;
                TextureRegion2D.Width = ImageData.SizeX;
                TextureRegion2D.Height = ImageData.SizeY;
            }

            RHIUpdateTexture2D(
                RHITexture2D, 0, TextureRegion2D,
                TextureRegion2D.Width * GPixelFormats[ImageData.PixelFormat].BlockBytes,
                ImageData.RawData.GetData()
            );
        }, TStatId(), nullptr, ENamedThreads::ActualRenderingThread
    );
    CreateTextureTask->Wait();

    return RHITexture2D;
}

FTexture2DRHIRef FRuntimeRHITexture2DFactory::CreateRHITexture2D_Other()
{
    // TODO: Figure out the best way to support other graphics APIs and platfornms
    return CreateRHITexture2D_Windows();
}

void FRuntimeRHITexture2DFactory::FinalizeRHITexture2D()
{
    // Create texture resource that returns actual texture size so that UMG can display the texture
    FRuntimeTextureResource* NewTextureResource = new FRuntimeTexture2DResource(NewTexture, RHITexture2D, ImageData.FilterMode);
    NewTexture->SetResource(NewTextureResource);

    FGraphEventRef UpdateResourceTask = FFunctionGraphTask::CreateAndDispatchWhenReady(
        [this, &NewTextureResource]()
        {
            NewTextureResource->InitResource();
            RHIUpdateTextureReference(NewTexture->TextureReference.TextureReferenceRHI, RHITexture2D);
            NewTextureResource->SetTextureReference(NewTexture->TextureReference.TextureReferenceRHI);

        }, TStatId(), nullptr, ENamedThreads::ActualRenderingThread
    );
    UpdateResourceTask->Wait();
}
