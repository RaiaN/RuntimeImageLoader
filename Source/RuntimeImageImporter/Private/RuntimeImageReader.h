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
    FString AssetName = TEXT("");
    UTexture2D* InOutTexture = nullptr;
};

USTRUCT()
struct FImageReadResult
{
    GENERATED_BODY()

    FString ImageFilename = TEXT("");
    FString AssetName = TEXT("");
    FRuntimeImageData OutImage;
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

public:
    void AddRequest(const FImageReadRequest& Request);
    void GetResults(TArray<FImageReadResult>& OutResults) const;
    void Clear();
    void Reset();
    bool HasRequests() const;
    bool CompletedWork() const;

    void Trigger();
    void BlockTillAllRequestsFinished();

private:
    void ReadImage(FImageReadRequest& Request, FImageReadResult& ReadResult);
    void InitializeTexture(FImageReadResult& ReadResult);

private:
    TQueue<FImageReadRequest> Requests;

    UPROPERTY()
    TArray<FImageReadResult> Results;

private:
    UPROPERTY()
    TArray<UTexture2D*> PendingTextures;

private:
    FRunnableThread* Thread = nullptr;
    FEvent* ThreadSemaphore = nullptr;

    FThreadSafeCounter AsyncTextureReallocation;

    FThreadSafeBool bCompletedWork = true; 

    bool bStopThread = false;
};
