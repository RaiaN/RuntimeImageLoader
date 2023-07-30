// Copyright 2023 Peter Leontev and Muhammad Ahmed Saleem. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Kismet/BlueprintAsyncActionBase.h"

#include "RenderGIFTexture.generated.h"

class UTexture2DDynamic;
class UAnimatedTexture2D;

UCLASS()
class URenderGIFTexture : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/** Handles GIF Texture requests coming from the Raw Data */
	UAnimatedTexture2D* RenderGIFData(const FString& GIFFilename, int32 Current_Frame);

private:
	int32 CurrentFrame = 12;
};