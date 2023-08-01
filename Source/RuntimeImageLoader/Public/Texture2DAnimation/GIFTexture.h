// Copyright 2023 Peter Leontev and Muhammad Ahmed Saleem. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Texture2DAnimation/AnimatedTexture2D.h"
#include "GIFTexture.generated.h"

class UAnimatedTexture2D;

UCLASS()
class UGIFTexture : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/** Handles GIF Texture requests coming from the Raw Data */
	UAnimatedTexture2D* Init(const FString& GIFFilename);

private:
	int32 Width = 0;
	int32 Height = 0;
};