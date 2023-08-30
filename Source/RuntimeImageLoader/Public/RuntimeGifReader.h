// Copyright 2023 Unreal Solutions Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Async/Future.h"
#include "Templates/SharedPointerFwd.h"
#include "Helpers/GIFLoader.h"
#include "InputImageDescription.h"
#include "RuntimeGifReader.generated.h"

class UAnimatedTexture2D;
class IImageReader;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGifLoadingSuccessDelegate, UAnimatedTexture2D*, OutGifTexture);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGifLoadingFailureDelegate, FString, OutError);

struct RUNTIMEIMAGELOADER_API FGifReadRequest
{
	FInputImageDescription InputGif;
};

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
class RUNTIMEIMAGELOADER_API URuntimeGifReader : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/** GIF */
    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Runtime Gif Reader")
    static URuntimeGifReader* LoadGIFAsync(const FString& GIFFilename);

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Runtime Gif Reader")
    static URuntimeGifReader* LoadGIFSync(const FString& GIFFilename);

    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Runtime Gif Reader")
	static URuntimeGifReader* LoadGIFFromBytesAsync(UPARAM(ref) TArray<uint8>& GifBytes);

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Runtime Gif Reader")
	static URuntimeGifReader* LoadGIFFromBytesSync(UPARAM(ref) TArray<uint8>& GifBytes);

public:
	// Bind to these events when you want to use async API from C++
	UPROPERTY(BlueprintAssignable)
	FGifLoadingSuccessDelegate OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FGifLoadingFailureDelegate OnFail;

public:
	/* Native API */
	void SubmitRequest(FGifReadRequest&& InRequest, bool bSynchronous = false);

private:
	void ProcessRequest();

private:
	FGifReadRequest Request;

	TFuture<void> CurrentTask;
	TSharedPtr<IImageReader, ESPMode::ThreadSafe> ImageReader;
	TUniquePtr<FRuntimeGIFLoaderHelper> Decoder;

    UPROPERTY()
    FGifReadResult ReadResult;
};