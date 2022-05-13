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
#include "Runtime/Launch/Resources/Version.h"
#include "Async/Async.h"

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

void URuntimeImageReader::Tick(float DeltaTime)
{
    FConstructTextureTask Task;
    while (!bStopThread && ConstructTasks.Dequeue(Task))
    {
        ConstructedTextures.Add(FRuntimeImageUtils::CreateTexture(Task.ImageFilename, *Task.ImageData));
        TextureConstructedSemaphore->Trigger();
    }
}

TStatId URuntimeImageReader::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(URuntimeImageReader, STATGROUP_Tickables);
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

            ImageData.PixelFormat = DeterminePixelFormat(ImageData.Format, Request.TransformParams);
            if (ImageData.PixelFormat == PF_Unknown)
            {
                ReadResult.OutError = TEXT("Image data is corrupted. Please contact devs");
                continue;
            }

            ApplyTransformations(ImageData, Request.TransformParams);

            if (IsInGameThread())
            {
                ConstructedTextures.Add(FRuntimeImageUtils::CreateTexture(Request.ImageFilename, ImageData));
            }
            else
            {
                FConstructTextureTask Task;
                {
                    Task.ImageFilename = Request.ImageFilename;
                    Task.ImageData = &ImageData;
                }
                ConstructTasks.Enqueue(Task);

                while (!TextureConstructedSemaphore->Wait(100) && !bStopThread);
            }

            if (ConstructedTextures.Num() == 0)
            {
                return;
            }

            ReadResult.OutTexture = ConstructedTextures.Pop();

            AsyncReallocateTexture(ReadResult.OutTexture, ImageData);
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
    FRuntimeTextureResource(UTexture2D* InTexture, FTexture2DRHIRef RHITexture2D)
        : Owner(InTexture), SizeX(RHITexture2D->GetSizeX()), SizeY(RHITexture2D->GetSizeY())
    {
        TextureRHI = RHITexture2D;
        bSRGB = (TextureRHI->GetFlags() & TexCreate_SRGB) != TexCreate_None;
        bIgnoreGammaConversions = !bSRGB;
        bGreyScaleFormat = (TextureRHI->GetFormat() == PF_G8) || (TextureRHI->GetFormat() == PF_BC4);
    }

    virtual ~FRuntimeTextureResource() 
    {
        Owner->SetResource(nullptr);
       
        UE_LOG(LogRuntimeImageReader, Log, TEXT("RuntimeTextureResource has been destroyed!"))
    }

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
    }

    void ReleaseRHI() override
    {
        RHIUpdateTextureReference(Owner->TextureReference.TextureReferenceRHI, nullptr);
        FTextureResource::ReleaseRHI();
    }

private:
    UTexture2D* Owner;
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

void URuntimeImageReader::AsyncReallocateTexture(UTexture2D* NewTexture, const FRuntimeImageData& ImageData)
{
    uint32 NumMips = 1;
    uint32 NumSamples = 1;
    void* Mip0Data = (void*)ImageData.RawData.GetData();

    ETextureCreateFlags TextureFlags = TexCreate_ShaderResource;
    if (ImageData.SRGB)
    {
        TextureFlags |= TexCreate_SRGB;
    }

    FTexture2DRHIRef RHITexture2D = nullptr;

    ensureMsgf(ImageData.SizeX > 0, TEXT("ImageData.SizeX must be > 0"));
    ensureMsgf(ImageData.SizeY > 0, TEXT("ImageData.SizeY must be > 0"));

    RHITexture2D = RHIAsyncCreateTexture2D(
        ImageData.SizeX, ImageData.SizeY,
        ImageData.PixelFormat,
        ImageData.NumMips,
        TextureFlags,
        &Mip0Data,
        1
    );

    // Create proper texture resource so UMG can display runtime texture
    FRuntimeTextureResource* NewTextureResource = new FRuntimeTextureResource(NewTexture, RHITexture2D);
    NewTexture->SetResource(NewTextureResource);

    FGraphEventRef UpdateResourceTask = FFunctionGraphTask::CreateAndDispatchWhenReady(
        [&NewTextureResource, &NewTexture, &RHITexture2D]()
        {
            NewTextureResource->InitResource();
            RHIUpdateTextureReference(NewTexture->TextureReference.TextureReferenceRHI, RHITexture2D);
            NewTextureResource->SetTextureReference(NewTexture->TextureReference.TextureReferenceRHI);

        }, TStatId(), nullptr, ENamedThreads::ActualRenderingThread
    );
    UpdateResourceTask->Wait();
}

void URuntimeImageReader::ApplyTransformations(FRuntimeImageData& ImageData, FTransformImageParams TransformParams)
{
    if (!TransformParams.IsPercentSizeValid())
    {
        UE_LOG(LogRuntimeImageReader, Verbose, TEXT("Supplied transform params are not valid! PercentSizeX, PercentSizeX: (%d, %d)"), TransformParams.PercentSizeX, TransformParams.PercentSizeY);
    }
    
    if (TransformParams.IsPercentSizeValid())
    {
        const int32 TransformedSizeX = FMath::Floor(ImageData.SizeX * TransformParams.PercentSizeX * 0.01f);
        const int32 TransformedSizeY = FMath::Floor(ImageData.SizeY * TransformParams.PercentSizeY * 0.01f);
        
        FImage TransformedImage;
        TransformedImage.Init(TransformedSizeX, TransformedSizeY, ImageData.Format);

        ImageData.ResizeTo(TransformedImage, TransformedImage.SizeX, TransformedImage.SizeY, ImageData.Format, ImageData.GammaSpace);

        ImageData.RawData = MoveTemp(TransformedImage.RawData);
        ImageData.SizeX = TransformedImage.SizeX;
        ImageData.SizeY = TransformedImage.SizeY;
    }

    if (TransformParams.bForUI)
    {
        FImage BGRAImage;
        BGRAImage.Init(ImageData.SizeX, ImageData.SizeY, ERawImageFormat::BGRA8);
        ImageData.CopyTo(BGRAImage, ERawImageFormat::BGRA8, EGammaSpace::sRGB);

        ImageData.RawData = MoveTemp(BGRAImage.RawData);
        ImageData.SRGB = true;
        ImageData.GammaSpace = EGammaSpace::sRGB;
    }
}
