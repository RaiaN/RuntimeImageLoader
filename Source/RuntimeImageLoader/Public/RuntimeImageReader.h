// Copyright 2022 Peter Leontev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture.h"
#include "Tickable.h"
#include "Misc/ScopedEvent.h"
#include "HAL/Runnable.h"
#include "HAL/ThreadSafeBool.h"
#include "RuntimeImageData.h"
#include "RuntimeImageReader.generated.h"


class FRunnableThread;
class FEvent;

USTRUCT(BlueprintType)
struct RUNTIMEIMAGELOADER_API FTransformImageParams
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Runtime Image Reader"))
    bool bForUI = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Runtime Image Reader", UIMin = 0, UIMax = 100, ClampMin = 0, ClampMax = 100))
    int32 PercentSizeX = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Runtime Image Reader", UIMin = 0, UIMax = 100, ClampMin = 0, ClampMax = 100))
    int32 PercentSizeY = 0;

    bool IsPercentSizeValid() const
    {
        return PercentSizeX > 0 && PercentSizeX < 100 && PercentSizeY > 0 && PercentSizeY < 100;
    }
};

struct RUNTIMEIMAGELOADER_API FImageReadRequest
{
    FString ImageFilename = TEXT("");
    FTransformImageParams TransformParams;
};

USTRUCT()
struct RUNTIMEIMAGELOADER_API FImageReadResult
{
    GENERATED_BODY()

    FString ImageFilename = TEXT("");
    UPROPERTY()
    UTexture2D* OutTexture = nullptr;
    FString OutError = TEXT("");
};

struct RUNTIMEIMAGELOADER_API FConstructTextureTask
{
    FString ImageFilename;
    FRuntimeImageData* ImageData;
};

class UTexture2D;

UCLASS()
class RUNTIMEIMAGELOADER_API URuntimeImageReader : public UObject, public FRunnable, public FTickableGameObject
{
    GENERATED_BODY()

public:
    void Initialize();
    void Deinitialize();

public:
    void AddRequest(const FImageReadRequest& Request);
    void GetResult(FImageReadResult& OutResult);
    void Clear();
    void Stop();
    bool IsWorkCompleted() const;

    void Trigger();
    void BlockTillAllRequestsFinished();

protected:
    /* FRunnable interface */
    bool Init() override;
    uint32 Run() override;
    void Exit() override;
    /* ~FRunnable interface */

    // FTickableGameObject
    void Tick(float DeltaTime) override;
    TStatId GetStatId() const override;
    // ~FTickableGameObject

private:
    EPixelFormat DeterminePixelFormat(ERawImageFormat::Type ImageFormat, const FTransformImageParams& Params) const;
    void ApplyTransformations(FRuntimeImageData& ImageData, FTransformImageParams TransformParams);
    void AsyncReallocateTexture(UTexture2D* NewTexture, const FRuntimeImageData& ImageData);

private:
    TQueue<FImageReadRequest, EQueueMode::Mpsc> Requests;

    UPROPERTY()
    TArray<FImageReadResult> Results;

private:
    TQueue<FConstructTextureTask, EQueueMode::Mpsc> ConstructTasks;

    UPROPERTY()
    TArray<UTexture2D*> ConstructedTextures;

    FEvent* TextureConstructedSemaphore = nullptr;

private:
    FRunnableThread* Thread = nullptr;
    FEvent* ThreadSemaphore = nullptr;

    FThreadSafeBool bCompletedWork = true; 
    FThreadSafeBool bStopThread = false;
};
