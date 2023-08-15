// Copyright 2023 Peter Leontev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Async/Future.h"
#include "UObject/ObjectMacros.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Texture2DAnimation/AnimatedTexture2D.h"
#include "AsyncGIFLoader.generated.h"

class UAnimatedTexture2D;

UCLASS()
class UAsyncGIFLoader : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	DECLARE_DELEGATE_TwoParams(FOnGifLoaded, UAnimatedTexture2D*, const FString&);
	FOnGifLoaded OnGifLoaded;

	/** Handles GIF Texture requests coming from the Raw Data */
	void Init(const FString& InGifFilename);
	void Cancel();

protected:
	void Activate() override;

private:
	UFUNCTION()
	void OnGifDecodedHandler(bool bRes);

private:
	FString GifFilename;

	TUniquePtr<class FRuntimeGIFLoaderHelper> Decoder;

	TFuture<void> CurrentTask;

	DECLARE_DELEGATE_OneParam(FOnGifDecoded, bool);
	FOnGifDecoded OnGifDecoded;
};