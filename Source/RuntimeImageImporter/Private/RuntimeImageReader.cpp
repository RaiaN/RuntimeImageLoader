// Copyright Peter Leontev

#include "RuntimeImageReader.h"

#include "Engine/Texture.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#include "HAL/RunnableThread.h"
#include "HAL/Event.h"
#include "RHI.h"
#include "RHIResources.h"
#include "RHICommandList.h"
#include "RHIDefinitions.h"
#include "RenderUtils.h"
#include "Serialization/BulkData.h"
#include "TextureResource.h"

#include "RuntimeImageUtils.h"

void URuntimeImageReader::Initialize()
{
    Thread = FRunnableThread::Create(this, TEXT("RuntimeImageReader"), 0, TPri_SlightlyBelowNormal);
}

void URuntimeImageReader::Cleanup()
{
    bStopThread = true;
    Trigger();
    Thread->WaitForCompletion();
}

bool URuntimeImageReader::Init()
{
    ThreadSemaphore = FGenericPlatformProcess::GetSynchEventFromPool(false);
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
    bStopThread = true;
    bCompletedWork.AtomicSet(true);
}


void URuntimeImageReader::AddRequest(const FImageReadRequest& Request)
{
    Requests.Enqueue(Request);

    bCompletedWork.AtomicSet(false);
}

void URuntimeImageReader::GetResults(TArray<FImageReadResult>& OutResults) const
{
    OutResults.Append(Results);
}

void URuntimeImageReader::Clear()
{
    Requests.Empty();
    Results.Empty();
    CachedTextures.Empty();
}

void URuntimeImageReader::Reset()
{
    Clear();
}

bool URuntimeImageReader::HasRequests() const
{
    return !Requests.IsEmpty();
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
            FImageReadResult ReadResult;
            {
                ReadResult.ImageFilename = Request.ImageFilename;
            }

            FRuntimeImageData ImageData;
            FRuntimeImageUtils::ImportFileAsImage(Request.ImageFilename, ImageData, ReadResult.OutError);

            if (ReadResult.OutError.Len() > 0)
            {
                Results.Add(ReadResult);
                continue;
            }

            if (IsInGameThread())
            {
                ReadResult.OutTexture = CreateTexture(Request.ImageFilename, ImageData);
            }
            else
            {
                auto Result = Async(
                    EAsyncExecution::TaskGraphMainThread,
                    [this, &ReadResult, &ImageData]
                    {
                        ReadResult.OutTexture = CreateTexture(ReadResult.ImageFilename, ImageData);
                    }
                );

                Result.Wait();
            }

            Results.Add(ReadResult);
        }

        bCompletedWork.AtomicSet(Requests.IsEmpty());
    }
}


UTexture2D* URuntimeImageReader::CreateTexture(const FString& TextureName, const FRuntimeImageData& ImageData)
{
    UTexture2D* NewTexture = NewObject<UTexture2D>(this, NAME_None, RF_Transient);

    // TODO: notify cache? use method?
    CachedTextures.Add(TextureName, NewTexture);

    {
        QUICK_SCOPE_CYCLE_COUNTER(STAT_RuntimeImageReader_ImportFileAsTexture_NewTexture);

        check(IsValid(NewTexture));

        EPixelFormat PixelFormat;
        switch (ImageData.Format)
        {
            case TSF_G8:            PixelFormat = PF_G8; break;
            case TSF_G16:           PixelFormat = PF_G16; break;
            case TSF_BGRA8:         PixelFormat = PF_B8G8R8A8; break;
            case TSF_BGRE8:         PixelFormat = PF_B8G8R8A8; break;
            case TSF_RGBA16:        PixelFormat = PF_R16G16B16A16_SINT; break;
            case TSF_RGBA16F:       PixelFormat = PF_FloatRGBA; break;
            default:                PixelFormat = PF_B8G8R8A8; break;
        }

        // TODO: Rework & Optimize

        NewTexture->PlatformData = new FTexturePlatformData();
        NewTexture->PlatformData->SizeX = ImageData.SizeX;
        NewTexture->PlatformData->SizeY = ImageData.SizeX;
        NewTexture->PlatformData->PixelFormat = PixelFormat;

        FTexture2DMipMap* Mip = new FTexture2DMipMap();
        NewTexture->PlatformData->Mips.Add(Mip);
        Mip->SizeX = ImageData.SizeX;
        Mip->SizeY = ImageData.SizeY;

        {
            const uint32 MipBytes = Mip->SizeX * Mip->SizeY * GPixelFormats[PixelFormat].BlockBytes;

            Mip->BulkData.Lock(LOCK_READ_WRITE);
            {
                void* TextureData = Mip->BulkData.Realloc(MipBytes);
                FMemory::Memcpy(TextureData, ImageData.RawData.GetData(), MipBytes);
            }
            Mip->BulkData.Unlock();
        }

        NewTexture->UpdateResource();

        FlushRenderingCommands();
    }

    return NewTexture;
}
