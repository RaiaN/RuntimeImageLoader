// Copyright 2022 Peter Leontev. All Rights Reserved.

#include "RuntimeImageReader.h"

#include "GenericPlatform/GenericPlatformProcess.h"
#include "HAL/RunnableThread.h"
#include "HAL/Event.h"
#include "RHI.h"
#include "RHIResources.h"
#include "RHICommandList.h"
#include "RHIDefinitions.h"
#include "RenderUtils.h"
#include "Engine/Texture.h"
#include "Engine/Texture2D.h"
#include "PixelFormat.h"
#include "TextureResource.h"

#include "RuntimeImageUtils.h"

DEFINE_LOG_CATEGORY_STATIC(LogRuntimeImageReader, Log, All);

void URuntimeImageReader::Initialize()
{
    ThreadSemaphore = FPlatformProcess::GetSynchEventFromPool(false);
    TextureConstructedSemaphore = FPlatformProcess::GetSynchEventFromPool(false);
    Thread = FRunnableThread::Create(this, TEXT("RuntimeImageReader"), 0, TPri_SlightlyBelowNormal);

    UE_LOG(LogRuntimeImageReader, Log, TEXT("Image reader thread started!"))
}

void URuntimeImageReader::Deinitialize()
{
    Clear();
    Stop();

    UE_LOG(LogRuntimeImageReader, Log, TEXT("Image reader thread exited!"))
}

bool URuntimeImageReader::Init()
{
    return true;
}

uint32 URuntimeImageReader::Run()
{
    while (!bStopThread)
    {
        ThreadSemaphore->Wait();
        
        BlockTillAllRequestsFinished();
    }

    return 0;
}

void URuntimeImageReader::Exit()
{
    // 
}


void URuntimeImageReader::AddRequest(const FImageReadRequest& Request)
{
    Requests.Enqueue(Request);

    bCompletedWork.AtomicSet(false);
}

void URuntimeImageReader::GetResult(FImageReadResult& OutResult)
{
    ensure(Results.Num() > 0);

    OutResult = Results.Pop();
}

void URuntimeImageReader::Clear()
{
    Requests.Empty();
    Results.Empty();
}

void URuntimeImageReader::Stop()
{
    bStopThread = true;
    Trigger();

    Thread->WaitForCompletion();

    FPlatformProcess::ReturnSynchEventToPool(ThreadSemaphore);
}

bool URuntimeImageReader::IsWorkCompleted() const
{
    return bCompletedWork;
}

void URuntimeImageReader::Trigger()
{
    ThreadSemaphore->Trigger();
}

void URuntimeImageReader::BlockTillAllRequestsFinished()
{
    while (!bCompletedWork && !bStopThread)
    {
        FImageReadRequest Request;
        while (Requests.Dequeue(Request))
        {
            FImageReadResult& ReadResult = Results.Emplace_GetRef();
            ReadResult.ImageFilename = Request.ImageFilename;

            FRuntimeImageData ImageData;
            FRuntimeImageUtils::ImportFileAsImage(Request.ImageFilename, ImageData, ReadResult.OutError);

            if (ReadResult.OutError.Len() > 0)
            {
                continue;
            }

            EPixelFormat PixelFormat = DeterminePixelFormat(ImageData.Format, Request.TransformParams);
            if (PixelFormat == PF_Unknown)
            {
                ReadResult.OutError = TEXT("Image data is corrupted. Please contact devs");
                continue;
            }

            if (IsInGameThread())
            {
                ConstructedTextures.Add(FRuntimeImageUtils::CreateDummyTexture(Request.ImageFilename, PixelFormat));
            }
            else
            {
                FConstructTextureTask Task;
                {
                    Task.ImageFilename = Request.ImageFilename;
                    Task.PixelFormat = PixelFormat;
                }

                ConstructTasks.Enqueue(Task);
                while (!TextureConstructedSemaphore->Wait(100) && !bStopThread) {}
            }

            if (ConstructedTextures.Num() == 0)
            {
                return;
            }

            ReadResult.OutTexture = ConstructedTextures.Pop();

            // set up texture platform data
            FTexturePlatformData* PlatformData = nullptr;
            {
#if ENGINE_MAJOR_VERSION < 5
                PlatformData = ReadResult.OutTexture->PlatformData;
#else
                PlatformData = ReadResult.OutTexture->GetPlatformData();
#endif
            }

            ApplyTransformations(ImageData, Request.TransformParams);

            AsyncReallocateTexture(ReadResult.OutTexture, ImageData, PixelFormat);

            PlatformData->SizeX = ImageData.SizeX;
            PlatformData->SizeY = ImageData.SizeY;
        }

        bCompletedWork.AtomicSet(Requests.IsEmpty());
    }
}

/**
 * The rendering resource which represents a runtime texture.
 */
class FRuntimeTextureResource : public FTextureResource
{
public:
    FRuntimeTextureResource(FTexture2DRHIRef RHITexture2D)
        : SizeX(RHITexture2D->GetSizeX()), SizeY(RHITexture2D->GetSizeY())
    {
        TextureRHI = RHITexture2D;
        bSRGB = (TextureRHI->GetFlags() & TexCreate_SRGB) != TexCreate_None;
        bIgnoreGammaConversions = !bSRGB;
        bGreyScaleFormat = (TextureRHI->GetFormat() == PF_G8) || (TextureRHI->GetFormat() == PF_BC4);
    }

    virtual ~FRuntimeTextureResource() {}

    uint32 GetSizeX() const override
    {
        return SizeX;
    }

    uint32 GetSizeY() const override
    {
        return SizeY;
    }

    void InitRHI() override
    {
        FSamplerStateInitializerRHI SamplerStateInitializer(SF_Trilinear, AM_Wrap, AM_Wrap, AM_Wrap);
        SamplerStateRHI = RHICreateSamplerState(SamplerStateInitializer);
        DeferredPassSamplerStateRHI = RHICreateSamplerState(SamplerStateInitializer);
    }

    void ReleaseRHI() override
    {
        // RHIUpdateTextureReference(Owner->TextureReference.TextureReferenceRHI, nullptr);
        FTextureResource::ReleaseRHI();
    }

private:
    uint32 SizeX;
    uint32 SizeY;
};

EPixelFormat URuntimeImageReader::DeterminePixelFormat(ERawImageFormat::Type ImageFormat, const FTransformImageParams& Params) const
{
    EPixelFormat PixelFormat;
    
    // determine pixel format
    switch (ImageFormat)
    {
        case ERawImageFormat::G8:            PixelFormat = (Params.bForUI) ? PF_B8G8R8A8 : PF_G8; break;
        case ERawImageFormat::G16:           PixelFormat = PF_G16; break;
        case ERawImageFormat::BGRA8:         PixelFormat = PF_B8G8R8A8; break;
        case ERawImageFormat::BGRE8:         PixelFormat = PF_B8G8R8A8; break;
        case ERawImageFormat::RGBA16:        PixelFormat = (Params.bForUI) ? PF_B8G8R8A8 : PF_R16G16B16A16_SINT; break;
        case ERawImageFormat::RGBA16F:       PixelFormat = PF_FloatRGBA; break;
        default:                             PixelFormat = PF_Unknown; break;
    }

    return PixelFormat;
}

void URuntimeImageReader::AsyncReallocateTexture(UTexture2D* NewTexture, FRuntimeImageData& ImageData, EPixelFormat PixelFormat)
{
    uint32 NumMips = 1;
    uint32 NumSamples = 1;
    void* Mip0Data = ImageData.RawData.GetData();

    ETextureCreateFlags TextureFlags = TexCreate_ShaderResource;
    if (ImageData.SRGB)
    {
        TextureFlags |= TexCreate_SRGB;
    }

    FTexture2DRHIRef RHITexture2D = RHIAsyncCreateTexture2D(
        ImageData.SizeX, ImageData.SizeY,
        PixelFormat,
        NumMips,
        TextureFlags, 
        &Mip0Data, 
        1
    );

    FGraphEventRef UpdateTextureReferenceTask = FFunctionGraphTask::CreateAndDispatchWhenReady(
        [NewTexture, RHITexture2D]()
        {
            RHIUpdateTextureReference(NewTexture->TextureReference.TextureReferenceRHI, RHITexture2D);

        }, TStatId(), nullptr, ENamedThreads::ActualRenderingThread
    );
    UpdateTextureReferenceTask->Wait();

    // Create proper texture resource so UMG can display runtime texture
    FRuntimeTextureResource* NewTextureResource = new FRuntimeTextureResource(RHITexture2D);

    FGraphEventRef InitTextureResourceTask = FFunctionGraphTask::CreateAndDispatchWhenReady(
        [&NewTextureResource]()
        {
            NewTextureResource->InitRHI();

        }, TStatId(), nullptr, ENamedThreads::ActualRenderingThread
    );
    InitTextureResourceTask->Wait();

    NewTexture->SetResource(NewTextureResource);
}

void URuntimeImageReader::ApplyTransformations(FRuntimeImageData& ImageData, FTransformImageParams TransformParams)
{
    if (TransformParams.SizeX <= 0 || TransformParams.SizeX > 8192 || TransformParams.SizeY <= 0 || TransformParams.SizeY > 8192)
    {
        UE_LOG(LogRuntimeImageReader, Log, TEXT("Supplied transform params are not valid! SizeX, SizeY: (%d, %d)"), TransformParams.SizeX, TransformParams.SizeY);
        TransformParams.SizeX = -1;
        TransformParams.SizeY = -1;
    }
    
    if (TransformParams.SizeX > 0 && TransformParams.SizeY > 0)
    {
        FImage TransformedImage;
        TransformedImage.Init(TransformParams.SizeX, TransformParams.SizeY, ImageData.Format);

        ImageData.ResizeTo(TransformedImage, TransformedImage.SizeX, TransformedImage.SizeY, ImageData.Format, EGammaSpace::Linear);

        ImageData.RawData = MoveTemp(TransformedImage.RawData);
        ImageData.SizeX = TransformParams.SizeX;
        ImageData.SizeY = TransformParams.SizeY;
    }

    if (TransformParams.bForUI)
    {
        FImage BGRAImage;
        BGRAImage.Init(ImageData.SizeX, ImageData.SizeY, ERawImageFormat::BGRA8);
        ImageData.CopyTo(BGRAImage, ERawImageFormat::BGRA8, EGammaSpace::Linear);

        ImageData.RawData = MoveTemp(BGRAImage.RawData);
        ImageData.SRGB = true;
    }
}

void URuntimeImageReader::Tick(float DeltaTime)
{
    FConstructTextureTask Task;
    while (!bStopThread && ConstructTasks.Dequeue(Task))
    {
        ConstructedTextures.Add(FRuntimeImageUtils::CreateDummyTexture(Task.ImageFilename, Task.PixelFormat));
        TextureConstructedSemaphore->Trigger();
    }
}


TStatId URuntimeImageReader::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(URuntimeImageReader, STATGROUP_Tickables);
}
