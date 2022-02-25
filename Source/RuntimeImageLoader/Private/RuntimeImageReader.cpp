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
                Results.Add(ReadResult);
                continue;
            }

            if (IsInGameThread())
            {
                ConstructedTextures.Add(FRuntimeImageUtils::CreateDummyTexture(Request.ImageFilename, ImageData.Format));
            }
            else
            {
                FConstructTextureTask Task;
                {
                    Task.ImageFilename = Request.ImageFilename;
                    Task.TextureFormat = ImageData.Format;
                }

                ConstructTasks.Enqueue(Task);
                while (!TextureConstructedSemaphore->Wait(100) && !bStopThread) {}
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

void URuntimeImageReader::AsyncReallocateTexture(UTexture2D* NewTexture, FRuntimeImageData& ImageData)
{
    uint32 NumMips = 1;
    uint32 NumSamples = 1;
    void* Mip0Data = ImageData.RawData.GetData();

    FTexture2DRHIRef RHITexture2D = RHIAsyncCreateTexture2D(
        ImageData.SizeX, ImageData.SizeY,
        PF_R8G8B8A8,
        NumMips,
        TexCreate_ShaderResource, &Mip0Data, 1
    );

    FGraphEventRef UpdateTextureReferenceTask = FFunctionGraphTask::CreateAndDispatchWhenReady(
        [NewTexture, RHITexture2D]()
        {
            RHIUpdateTextureReference(NewTexture->TextureReference.TextureReferenceRHI, RHITexture2D);
            NewTexture->RefreshSamplerStates();
        }, TStatId(), nullptr, ENamedThreads::ActualRenderingThread
    );

    UpdateTextureReferenceTask->Wait();
}


void URuntimeImageReader::Tick(float DeltaTime)
{
    FConstructTextureTask Task;
    while (!bStopThread && ConstructTasks.Dequeue(Task))
    {
        ConstructedTextures.Add(FRuntimeImageUtils::CreateDummyTexture(Task.ImageFilename, Task.TextureFormat));
        TextureConstructedSemaphore->Trigger();
    }
}


TStatId URuntimeImageReader::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(URuntimeImageReader, STATGROUP_Tickables);
}