// Copyright Peter Leontev

#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture.h"
#include "Tickable.h"
#include "Misc/ScopedEvent.h"
#include "HAL/ThreadSafeBool.h"
#include "RuntimeImageData.h"
#include "RuntimeImageReader.generated.h"


class FRunnableThread;
class FEvent;


struct FImageReadRequest
{
    FString ImageFilename = TEXT("");
};

USTRUCT()
struct FImageReadResult
{
    GENERATED_BODY()

    FString ImageFilename = TEXT("");
    UPROPERTY()
    UTexture2D* OutTexture = nullptr;
    FString OutError = TEXT("");
};

struct FConstructTextureTask
{
    FString ImageFilename;
    ETextureSourceFormat TextureFormat;
};

UCLASS()
class RUNTIMEIMAGEIMPORTER_API URuntimeImageReader : public UObject, public FRunnable, public FTickableGameObject
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
    void AsyncReallocateTexture(UTexture2D* NewTexture, FRuntimeImageData& ImageData);

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
