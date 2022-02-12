// Copyright Peter Leontev

#include "RuntimeImageReader.h"

#include "HAL/RunnableThread.h"
#include "HAL/Event.h"
#include "RHI.h"
#include "RHIResources.h"
#include "RHICommandList.h"
#include "RHIDefinitions.h"
#include "RenderUtils.h"
#include "RenderCommandFence.h"
#include "Engine/Texture.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#include "PixelFormat.h"
#include "Containers/ResourceArray.h"
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

void URuntimeImageReader::GetResult(FImageReadResult& OutResult)
{
    ensure(Results.Num() > 0);

    OutResult = Results.Pop();
}

void URuntimeImageReader::Clear()
{
    Requests.Empty();
    Results.Empty();
    // CachedTextures.Empty();
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

class FTextureMips : public FResourceBulkDataInterface
{

public:
    FTextureMips(TArray<uint8>&& InMipsData, int32 InSizeX, int32 InSizeY)
    : MipsData(MoveTemp(InMipsData)), SizeX(InSizeX), SizeY(InSizeY)
    {}

    const void* GetResourceBulkData() const override
    {
        return MipsData.GetData();
    }

    uint32 GetResourceBulkDataSize() const override
    {
        return MipsData.Num();
    }

    void Discard() override
    {
        MipsData.Empty();
    }

private:
    TArray<uint8> MipsData;
    int32 SizeX;
    int32 SizeY;
};

UTexture2D* URuntimeImageReader::CreateTexture(const FString& TextureName, FRuntimeImageData& ImageData)
{
    check (IsInGameThread());
    
    UTexture2D* NewTexture = NewObject<UTexture2D>(this, NAME_None, RF_Transient);
    NewTexture->NeverStream = true;

    // TODO: notify cache? use method?
    // CachedTextures.Add(TextureName, NewTexture);

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
        NewTexture->PlatformData->SizeX = 1;
        NewTexture->PlatformData->SizeY = 1;
        NewTexture->PlatformData->PixelFormat = PixelFormat;

        FTexture2DMipMap* Mip = new FTexture2DMipMap();
        NewTexture->PlatformData->Mips.Add(Mip);
        Mip->SizeX = 1;
        Mip->SizeY = 1;

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

        ENQUEUE_RENDER_COMMAND(CreateRHITexture)(
            [NewTexture, &ImageData, PixelFormat](FRHICommandListImmediate& RHICmdList)
            {
                FTextureMips TextureMips(CopyTemp(ImageData.RawData), ImageData.SizeY, ImageData.SizeY);
                FRHIResourceCreateInfo RHICreateInfo(&TextureMips);

                /*FTexture2DRHIRef RHITexture2D = RHICreateTexture2D(
                    ImageData.SizeX, ImageData.SizeY,
                    PF_R8G8B8A8, 
                    NumMips, NumSamples, 
                    TexCreate_ShaderResource, RHICreateInfo
                );*/

                FTexture2DRHIRef RHITexture2D;

                auto Result = Async(
                    EAsyncExecution::Thread,
                    [&ImageData, NewTexture, &RHITexture2D]()
                    {
                        uint32 NumMips = 1;
                        uint32 NumSamples = 1;
                        void* Mip0Data = ImageData.RawData.GetData();

                        RHITexture2D = RHIAsyncCreateTexture2D(
                            ImageData.SizeX, ImageData.SizeY,
                            PF_R8G8B8A8,
                            NumMips,
                            TexCreate_ShaderResource, &Mip0Data, 1
                        );
                    }
                );

                Result.Wait();

                RHIUpdateTextureReference(NewTexture->TextureReference.TextureReferenceRHI, RHITexture2D);
                NewTexture->RefreshSamplerStates();
            }
        );

        FRenderCommandFence Fence;
        Fence.BeginFence();
        Fence.Wait(true);
    }

    return NewTexture;
}
