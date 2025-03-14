// Copyright 2023 Petr Leontev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture.h"
#include "Misc/ScopedEvent.h"
#include "HAL/Runnable.h"
#include "HAL/ThreadSafeBool.h"
#include "ImageCore.h"
#include "Containers/Queue.h"
#include "RuntimeImageData.h"
#include "InputImageDescription.h"
#include "RuntimeImageReader.generated.h"


class URuntimeTextureFactory;
class FRunnableThread;
class FEvent;
class UTexture2D;
class UTextureCube;
class IImageReader;


USTRUCT(BlueprintType)
struct RUNTIMEIMAGELOADER_API FTransformImageParams
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Runtime Image Reader"))
    bool bForUI = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Runtime Image Reader"))
    TEnumAsByte<TextureFilter> FilterMode = TextureFilter::TF_Default;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Runtime Image Reader", UIMin = 0, UIMax = 100, ClampMin = 0, ClampMax = 100))
    int32 PercentSizeX = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Runtime Image Reader", UIMin = 0, UIMax = 100, ClampMin = 0, ClampMax = 100))
    int32 PercentSizeY = 100;

    // Hidden as there is method in RuntimeImageLoader that sets these flags
    bool bOnlyPixels = false;
    bool bOnlyBytes = false;

    bool IsPercentSizeValid() const
    {
        return PercentSizeX > 0 && PercentSizeX < 100 && PercentSizeY > 0 && PercentSizeY < 100;
    }
};

struct RUNTIMEIMAGELOADER_API FImageReadRequest
{
    FInputImageDescription InputImage;
    FTransformImageParams TransformParams;
};

USTRUCT()
struct RUNTIMEIMAGELOADER_API FImageReadResult
{
    GENERATED_BODY()

    FString ImageFilename = TEXT("");

    UPROPERTY()
    TArray<FColor> OutImagePixels;

    // For pure bytes only
    TArray<uint8> OutImageBytes;
    
    UPROPERTY()
    UTexture2D* OutTexture = nullptr;

    UPROPERTY()
    UTextureCube* OutTextureCube = nullptr;

    FString OutError = TEXT("");
};


UCLASS()
class RUNTIMEIMAGELOADER_API URuntimeImageReader : public UObject, public FRunnable
{
    GENERATED_BODY()

public:
    void Initialize();
    void Deinitialize();

public:
    void AddRequest(const FImageReadRequest& Request);
    bool GetResult(FImageReadResult& OutResult);
    void Clear();
    void Stop();
    bool IsWorkCompleted() const;

    void Trigger();
    void BlockTillAllRequestsFinished();
    bool ProcessRequest(FImageReadRequest& Request);

protected:
    /* FRunnable interface */
    bool Init() override;
    uint32 Run() override;
    void Exit() override;
    /* ~FRunnable interface */

private:
    EPixelFormat DeterminePixelFormat(ERawImageFormat::Type ImageFormat, const FTransformImageParams& Params) const;
    void ApplySizeFormatTransformations(FRuntimeImageData& ImageData, FTransformImageParams TransformParams);

private:
    TQueue<FImageReadRequest, EQueueMode::Mpsc> Requests;

    UPROPERTY()
    TArray<FImageReadResult> Results;

    UPROPERTY()
    FImageReadResult PendingReadResult;

    FCriticalSection ResultsMutex;

private:
    UPROPERTY()
    URuntimeTextureFactory* TextureFactory;

private:
    FRunnableThread* Thread = nullptr;
    FEvent* ThreadSemaphore = nullptr;

    TSharedPtr<IImageReader, ESPMode::ThreadSafe> ImageReader;

    FThreadSafeBool bCompletedWork = true; 
    FThreadSafeBool bStopThread = false;
};
