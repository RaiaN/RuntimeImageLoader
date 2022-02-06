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
    Thread = FRunnableThread::Create(this, TEXT("RuntimeImageReader"));
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
            ReadImage(Request, ReadResult);

            if (IsInGameThread())
            {
                InitializeTexture(ReadResult);
            }
            else
            {
                auto Result = Async(
                    EAsyncExecution::TaskGraphMainThread,
                    [this, &ReadResult]
                    {
                        InitializeTexture(ReadResult);
                    }
                );

                Result.Wait();
            }
        }

        bCompletedWork.AtomicSet(Requests.IsEmpty());
    }
}

void URuntimeImageReader::ReadImage(FImageReadRequest& Request, FImageReadResult& ReadResult)
{
    FString OutError;
    {
        FRuntimeImageUtils::ImportFileAsImage(Request.ImageFilename, ReadResult.OutImage, OutError);

        ReadResult.ImageFilename = Request.ImageFilename;
        ReadResult.OutError = OutError;
    }    
}


void URuntimeImageReader::InitializeTexture(FImageReadResult& ReadResult)
{
    if (ReadResult.OutError.Len() != 0)
    {
        Results.Add(ReadResult);
        return;
    }

    ReadResult.OutTexture = NewObject<UTexture2D>(this, *FPaths::GetBaseFilename(ReadResult.ImageFilename));

    // TODO: notify cache?
    CachedTextures.Add(ReadResult.ImageFilename, ReadResult.OutTexture);

    FRuntimeImageData& Image = ReadResult.OutImage;
    UTexture2D* NewTexture = ReadResult.OutTexture;

    {
        QUICK_SCOPE_CYCLE_COUNTER(STAT_RuntimeImageReader_ImportFileAsTexture_NewTexture);

        check(IsValid(NewTexture));

        EPixelFormat PixelFormat;
        switch (ReadResult.OutImage.Format)
        {
            case TSF_G8:            PixelFormat = PF_G8; break;
            case TSF_G16:           PixelFormat = PF_G16; break;
            case TSF_BGRA8:         PixelFormat = PF_B8G8R8A8; break;
            case TSF_BGRE8:         PixelFormat = PF_B8G8R8A8; break;
            case TSF_RGBA16:        PixelFormat = PF_R16G16B16A16_SINT; break;
            case TSF_RGBA16F:       PixelFormat = PF_FloatRGBA; break;
            default:                PixelFormat = PF_B8G8R8A8; break;
        }

        // REWORK THIS CODE

        NewTexture->PlatformData = new FTexturePlatformData();
        NewTexture->PlatformData->SizeX = ReadResult.OutImage.SizeX;
        NewTexture->PlatformData->SizeY = ReadResult.OutImage.SizeX;
        NewTexture->PlatformData->PixelFormat = PixelFormat;

        FTexture2DMipMap* Mip = new FTexture2DMipMap();
        NewTexture->PlatformData->Mips.Add(Mip);
        Mip->SizeX = ReadResult.OutImage.SizeX;
        Mip->SizeY = ReadResult.OutImage.SizeY;

        {
            const uint32 MipBytes = Mip->SizeX * Mip->SizeY * GPixelFormats[PixelFormat].BlockBytes;

            Mip->BulkData.Lock(LOCK_READ_WRITE);
            void* TextureData = Mip->BulkData.Realloc(MipBytes);
            FMemory::Memcpy(TextureData, Image.RawData.GetData(), MipBytes);
            Mip->BulkData.Unlock();
        }

        NewTexture->UpdateResource();

        FlushRenderingCommands();

        Results.Add(ReadResult);
    }
}
