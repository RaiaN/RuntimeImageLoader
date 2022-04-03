// Copyright 2022 Peter Leontev. All Rights Reserved.

#include "RuntimeImageLoader.h"
#include "Subsystems/SubsystemBlueprintLibrary.h"

void URuntimeImageLoader::Initialize(FSubsystemCollectionBase& Collection)
{
    InitializeImageReader();
}

void URuntimeImageLoader::Deinitialize()
{
    ImageReader->Deinitialize();
    ImageReader = nullptr;
}

bool URuntimeImageLoader::DoesSupportWorldType(EWorldType::Type WorldType) const
{
    return WorldType == EWorldType::PIE || WorldType == EWorldType::Game;
}

void URuntimeImageLoader::LoadImageAsync(const FString& ImageFilename, bool bForUI, UTexture2D*& OutTexture, bool& bSuccess, FString& OutError, FLatentActionInfo LatentInfo, UObject* WorldContextObject /*= nullptr*/)
{
    if (!IsValid(WorldContextObject))
    {
        return;
    }

    FLoadImageRequest Request;
    {
        Request.Params.ImageFilename = ImageFilename;
        Request.Params.bForUI = bForUI;

        Request.OnRequestCompleted.BindLambda(
            [this, &OutTexture, &bSuccess, &OutError, LatentInfo](const FImageReadResult& ReadResult)
            {
                if (UObject* CallbackTarget = LatentInfo.CallbackTarget)
                {
                    if (UFunction* ExecutionFunction = CallbackTarget->FindFunction(LatentInfo.ExecutionFunction))
                    {
                        int32 Linkage = LatentInfo.Linkage;

#if (UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT)
                        // Make sure the texture was not destroyed by GC 
                        if (ReadResult.OutError.IsEmpty())
                        {
                            ensure(IsValid(ReadResult.OutTexture));
                        }
#endif

                        bSuccess = ReadResult.OutError.IsEmpty();
                        OutTexture = ReadResult.OutTexture;
                        OutError = ReadResult.OutError;

                        CallbackTarget->ProcessEvent(ExecutionFunction, &Linkage);
                    }
                }
            }
        );
    }

    Requests.Enqueue(Request);
}

void URuntimeImageLoader::LoadImageSync(const FString& ImageFilename, bool bForUI, UTexture2D*& OutTexture, bool& bSuccess, FString& OutError)
{
    FImageReadRequest ReadRequest;
    {
        ReadRequest.ImageFilename = ImageFilename;
        ReadRequest.bForUI = bForUI;
    }

    ImageReader->BlockTillAllRequestsFinished();
    ImageReader->AddRequest(ReadRequest);
    ImageReader->BlockTillAllRequestsFinished();

    FImageReadResult ReadResult;
    ImageReader->GetResult(ReadResult);

    bSuccess = ReadResult.OutError.IsEmpty();
    OutTexture = ReadResult.OutTexture;
    OutError = ReadResult.OutError;
}

void URuntimeImageLoader::Tick(float DeltaTime)
{
    ensure(IsValid(ImageReader));
    
    if (!ActiveRequest.IsRequestValid() && !Requests.IsEmpty())
    {
        Requests.Dequeue(ActiveRequest);

        FImageReadRequest ReadRequest(ActiveRequest.Params);

        ImageReader->AddRequest(ReadRequest);
        ImageReader->Trigger();
    }

    if (ActiveRequest.IsRequestValid() && ImageReader->IsWorkCompleted())
    {
        FImageReadResult ReadResult;
        ImageReader->GetResult(ReadResult);

        ensure(ActiveRequest.OnRequestCompleted.IsBound());

        ActiveRequest.OnRequestCompleted.Execute(ReadResult);

        ActiveRequest.Invalidate();
    }
}

TStatId URuntimeImageLoader::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(URuntimeImageLoader, STATGROUP_Tickables);
}

bool URuntimeImageLoader::IsAllowedToTick() const
{
    return !IsTemplate();
}

URuntimeImageReader* URuntimeImageLoader::InitializeImageReader()
{
    if (!IsValid(ImageReader))
    {
        ImageReader = NewObject<URuntimeImageReader>(this);
        ImageReader->Initialize();
    }

    ensure(IsValid(ImageReader));
    return ImageReader;
}
