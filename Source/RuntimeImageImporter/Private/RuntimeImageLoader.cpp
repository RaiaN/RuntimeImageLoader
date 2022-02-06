// Copyright Peter Leontev

#include "RuntimeImageLoader.h"

PRAGMA_DISABLE_OPTIMIZATION

URuntimeImageLoader* URuntimeImageLoader::GetRuntimeImageLoader()
{
    URuntimeImageLoader* RuntimeImageLoader = GEngine->GetEngineSubsystem<URuntimeImageLoader>();
    ensure(IsValid(RuntimeImageLoader));

    return RuntimeImageLoader;
}

void URuntimeImageLoader::Initialize(FSubsystemCollectionBase& Collection)
{
    ImageReader = InitializeImageReader();
}

void URuntimeImageLoader::Deinitialize()
{
    ImageReader = nullptr;
}

void URuntimeImageLoader::LoadImageAsync(const FString& ImageFilename, UTexture2D*& OutLoadedImage, FString& OutError, FLatentActionInfo LatentInfo, UObject* WorldContextObject /*= nullptr*/)
{
    if (!IsValid(WorldContextObject))
    {
        return;
    }
    
    FLoadImageRequestParams RequestParams;
    RequestParams.ImageFilename = ImageFilename;

    FLoadImageRequest Request;
    {
        Request.Params = RequestParams;
        Request.OnRequestCompleted.BindLambda(
            [this, &OutLoadedImage, &OutError, LatentInfo](const FImageReadResult& ReadResult)
            {
                if (UObject* CallbackTarget = LatentInfo.CallbackTarget)
                {
                    if (UFunction* ExecutionFunction = CallbackTarget->FindFunction(LatentInfo.ExecutionFunction))
                    {
                        int32 Linkage = LatentInfo.Linkage;

                        OutLoadedImage = ReadResult.OutTexture;
                        OutError = ReadResult.OutError;

                        CallbackTarget->ProcessEvent(ExecutionFunction, &Linkage);
                    }
                }
            }
        );
    }

    Requests.Enqueue(Request);
}

void URuntimeImageLoader::LoadImageSync(const FString& ImageFilename, UTexture2D*& OutTexture)
{
    
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
        TArray<FImageReadResult> OutResults;
        ImageReader->GetResults(OutResults);

        if (OutResults.Num() == 0)
        {
            return;
        }

        ensure(ActiveRequest.OnRequestCompleted.IsBound());

        ActiveRequest.OnRequestCompleted.Execute(OutResults[0]);

        ActiveRequest.Invalidate();
    }
}

TStatId URuntimeImageLoader::GetStatId() const
{
    return TStatId();
}

URuntimeImageReader* URuntimeImageLoader::InitializeImageReader()
{
    if (!IsValid(ImageReader))
    {
        ImageReader = NewObject<URuntimeImageReader>();
        ImageReader->Initialize();
    }

    ensure(IsValid(ImageReader));
    return ImageReader;
}

PRAGMA_ENABLE_OPTIMIZATION