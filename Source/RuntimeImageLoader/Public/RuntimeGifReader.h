// Copyright 2023 Peter Leontev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RuntimeGifReader.generated.h"

class UAnimatedTexture2D;
class FRuntimeGIFLoaderHelper;

USTRUCT()
struct RUNTIMEIMAGELOADER_API FGifReadResult
{
	GENERATED_BODY()

public:
	FString GifFilename = TEXT("");

	UAnimatedTexture2D* OutTexture = nullptr;

	FString OutError = TEXT("");
};

UCLASS()
class URuntimeGifReader : public UObject, public FRunnable
{
	GENERATED_BODY()

public:
	URuntimeGifReader();
	~URuntimeGifReader();
	URuntimeGifReader(FVTableHelper& Helper);

public:
	void Initialize();
	void Deinitialize();

public:
	void AddRequest(const FString& Request);
	bool GetResult(FGifReadResult& OutResult);
	void Clear();
	void Stop();
	bool IsWorkCompleted() const;

	void Trigger();
	void BlockTillAllRequestsFinished();
	bool ProcessRequest(const FString& InGifFilename);

protected:
	/* FRunnable interface */
	bool Init() override;
	uint32 Run() override;
	void Exit() override;
	/* ~FRunnable interface */

private:
	/**
		@See UniquePtr method void operator()(T* Ptr) const
	*/
	TUniquePtr<class FRuntimeGIFLoaderHelper> Decoder;

private:
	TQueue<FString, EQueueMode::Mpsc> Requests;

	UPROPERTY()
	FGifReadResult PendingReadResult;

	UPROPERTY()
	TArray<FGifReadResult> Results;

private:
	FCriticalSection ResultsMutex;

private:
	FRunnableThread* Thread = nullptr;
	FEvent* ThreadSemaphore = nullptr;

	FThreadSafeBool bCompletedWork = true;
	FThreadSafeBool bStopThread = false;
};