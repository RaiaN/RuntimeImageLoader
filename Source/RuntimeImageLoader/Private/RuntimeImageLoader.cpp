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

void URuntimeImageLoader::LoadImageAsync(const FString& ImageFilename, UTexture2D*& OutTexture, FString& OutError, FLatentActionInfo LatentInfo, UObject* WorldContextObject /*= nullptr*/)
{
    if (!IsValid(WorldContextObject))
    {
        return;
    }

    FLoadImageRequest Request;
    {
        Request.Params.ImageFilename = ImageFilename;
        Request.OnRequestCompleted.BindLambda(
            [this, &OutTexture, &OutError, LatentInfo](const FImageReadResult& ReadResult)
            {
                if (UObject* CallbackTarget = LatentInfo.CallbackTarget)
                {
                    if (UFunction* ExecutionFunction = CallbackTarget->FindFunction(LatentInfo.ExecutionFunction))
                    {
                        int32 Linkage = LatentInfo.Linkage;

                        // Make sure the texture was not destroyed by GC 
                        if (ReadResult.OutError.Len() == 0)
                        {
                            ensure(IsValid(ReadResult.OutTexture));
                        }

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

void URuntimeImageLoader::LoadImageSync(const FString& ImageFilename, UTexture2D*& OutTexture, FString& OutError)
{
    FImageReadRequest ReadRequest;
    ReadRequest.ImageFilename = ImageFilename;

    ImageReader->BlockTillAllRequestsFinished();
    ImageReader->AddRequest(ReadRequest);
    ImageReader->BlockTillAllRequestsFinished();

    FImageReadResult ReadResult;
    ImageReader->GetResult(ReadResult);

    OutTexture = ReadResult.OutTexture;
    OutError = ReadResult.OutError;
}

void URuntimeImageLoader::Tick(float DeltaTime)
{
    ensure(IsValid(ImageReader));
    
    if (!ActiveRequest.IsRequestValid() && !Requests.IsEmpty())
    {
        Requests.Dequeue(ActiveRequest);

        FImageReadRequest ReadRequest;
        ReadRequest.ImageFilename = ActiveRequest.Params.ImageFilename;

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
