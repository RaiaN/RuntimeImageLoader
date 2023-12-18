// Copyright 2023 Petr Leontev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Engine/Texture.h"
#include "Async/Future.h"
#include "Templates/SharedPointer.h"
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
    TEnumAsByte<TextureFilter> FilterMode = TextureFilter::TF_Default;
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
    static URuntimeGifReader* LoadGIF(const FString& GIFFilename, TEnumAsByte<enum TextureFilter> InFilterMode = TextureFilter::TF_Trilinear, bool bSynchronous = false);

    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Runtime Gif Reader")
	static URuntimeGifReader* LoadGIFFromBytes(UPARAM(ref) TArray<uint8>& GifBytes, TEnumAsByte<enum TextureFilter> InFilterMode = TextureFilter::TF_Trilinear, bool bSynchronous = false);

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
	void OnPostProcessRequest();

private:
	FGifReadRequest Request;

	TFuture<void> CurrentTask;
	TSharedPtr<IImageReader, ESPMode::ThreadSafe> ImageReader;
	TUniquePtr<IGIFLoader> Decoder;

    UPROPERTY()
    FGifReadResult ReadResult;
};