// Copyright 2023 Peter Leontev and Muhammad Ahmed Saleem. All Rights Reserved.

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
	/** Handles GIF Texture requests coming from the Raw Data */
	UAnimatedTexture2D* Init(const FString& GIFFilename);

private:
	int32 Width = 0;
	int32 Height = 0;

private:
	TFuture<bool> CurrentTask;
};