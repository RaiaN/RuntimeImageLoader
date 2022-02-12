// Copyright Peter Leontev

#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "HAL/ThreadSafeBool.h"

#include "RuntimeImageData.h"

#include "RuntimeImageReader.generated.h"


class FRunnableThread;
class FEvent;


USTRUCT()
struct FImageReadRequest
{
    GENERATED_BODY()

    FString ImageFilename = TEXT("");
};

USTRUCT()
struct FImageReadResult
{
    GENERATED_BODY()

    FString ImageFilename = TEXT("");
    UTexture2D* OutTexture = nullptr;
    FString OutError = TEXT("");
};


UCLASS()
class RUNTIMEIMAGEIMPORTER_API URuntimeImageReader : public UObject, public FRunnable
{
    GENERATED_BODY()

public:
    void Initialize();
    void Cleanup();

public:
    /* FRunnable interface */
    bool Init() override;
    uint32 Run() override;
    void Exit() override;
    /* ~FRunnable interface */

public:
    void AddRequest(const FImageReadRequest& Request);
    void GetResult(FImageReadResult& OutResult);
    void Clear();
    bool IsWorkCompleted() const;

    void Trigger();
    void BlockTillAllRequestsFinished();

private:
    UTexture2D* CreateTexture(const FString& TextureName, const FRuntimeImageData& ImageData);

private:
    TQueue<FImageReadRequest> Requests;

    UPROPERTY()
    TArray<FImageReadResult> Results;

private:
    // UPROPERTY()
    // TMap<FString, UTexture2D*> CachedTextures;

private:
    FRunnableThread* Thread = nullptr;
    FEvent* ThreadSemaphore = nullptr;

    FThreadSafeCounter AsyncTextureReallocation;

    FThreadSafeBool bCompletedWork = true; 

    bool bStopThread = false;
};
