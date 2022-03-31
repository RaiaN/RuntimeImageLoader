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

void URuntimeImageReader::Initialize()
{
    ThreadSemaphore = FPlatformProcess::GetSynchEventFromPool(false);
    TextureConstructedSemaphore = FPlatformProcess::GetSynchEventFromPool(false);
    Thread = FRunnableThread::Create(this, TEXT("RuntimeImageReader"), 0, TPri_SlightlyBelowNormal);

    UE_LOG(LogTemp, Log, TEXT("Image reader thread started!"))
}

void URuntimeImageReader::Deinitialize()
{
    Clear();
    Stop();

    UE_LOG(LogTemp, Log, TEXT("Image reader thread exited!"))
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

    FGenericPlatformProcess::ReturnSynchEventToPool(ThreadSemaphore);
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

            EPixelFormat PixelFormat = PF_Unknown;

            // determine pixel format
            switch (ImageData.Format)
            {
                case TSF_G8:            PixelFormat = PF_G8; break;
                case TSF_G16:           PixelFormat = PF_G16; break;
                case TSF_BGRA8:         PixelFormat = PF_B8G8R8A8; break;
                case TSF_BGRE8:         PixelFormat = PF_B8G8R8A8; break;
                case TSF_RGBA16:        PixelFormat = PF_R16G16B16A16_SINT; break;
                case TSF_RGBA16F:       PixelFormat = PF_FloatRGBA; break;
                default:                PixelFormat = PF_Unknown; break;
            }

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

            ReadResult.OutTexture->PlatformData->SizeX = ImageData.SizeX;
            ReadResult.OutTexture->PlatformData->SizeY = ImageData.SizeY;

            AsyncReallocateTexture(ReadResult.OutTexture, ImageData, PixelFormat);
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
    FRuntimeTextureResource(UTexture2D* InOwner)
        : Owner(InOwner), SizeX(InOwner->GetSizeX()), SizeY(InOwner->GetSizeY())
    {
        bSRGB = true;
        bGreyScaleFormat = false;
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
        RHIUpdateTextureReference(Owner->TextureReference.TextureReferenceRHI, nullptr);
        FTextureResource::ReleaseRHI();
    }

private:
    UTexture2D* Owner;
    uint32 SizeX;
    uint32 SizeY;
};

void URuntimeImageReader::AsyncReallocateTexture(UTexture2D* NewTexture, FRuntimeImageData& ImageData, EPixelFormat PixelFormat)
{
    uint32 NumMips = 1;
    uint32 NumSamples = 1;
    void* Mip0Data = ImageData.RawData.GetData();

    FTexture2DRHIRef RHITexture2D = RHIAsyncCreateTexture2D(
        ImageData.SizeX, ImageData.SizeY,
        PixelFormat,
        NumMips,
        TexCreate_ShaderResource | TexCreate_SRGB, &Mip0Data, 1
    );

    FGraphEventRef UpdateTextureReferenceTask = FFunctionGraphTask::CreateAndDispatchWhenReady(
        [NewTexture, RHITexture2D]()
        {
            RHIUpdateTextureReference(NewTexture->TextureReference.TextureReferenceRHI, RHITexture2D);

        }, TStatId(), nullptr, ENamedThreads::ActualRenderingThread
    );
    UpdateTextureReferenceTask->Wait();

    // Create proper texture resource so UMG can display runtime texture
    FRuntimeTextureResource* NewTextureResource = new FRuntimeTextureResource(NewTexture);
    NewTextureResource->TextureRHI = RHITexture2D;

    FGraphEventRef InitTextureResourceTask = FFunctionGraphTask::CreateAndDispatchWhenReady(
        [&NewTextureResource]()
        {
            NewTextureResource->InitRHI();

        }, TStatId(), nullptr, ENamedThreads::ActualRenderingThread
    );
    InitTextureResourceTask->Wait();

    NewTexture->SetResource(NewTextureResource);
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