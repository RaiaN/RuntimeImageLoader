// Copyright Peter Leontev

#include "RuntimeImageLoader.h"
#include "Subsystems/SubsystemBlueprintLibrary.h"

PRAGMA_DISABLE_OPTIMIZATION

void URuntimeImageLoader::Initialize(FSubsystemCollectionBase& Collection)
{
    ImageReader = InitializeImageReader();
}

void URuntimeImageLoader::Deinitialize()
{
    ImageReader = nullptr;
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
    InitializeImageReader();
    
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

PRAGMA_ENABLE_OPTIMIZATION