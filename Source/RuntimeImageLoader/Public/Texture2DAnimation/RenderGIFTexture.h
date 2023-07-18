#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Kismet/BlueprintAsyncActionBase.h"

#include "RenderGIFTexture.generated.h"

class UTexture2DDynamic;

UCLASS()
class URenderGIFTexture : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/** Handles GIF Texture requests coming from the Raw Data */
	UTexture2DDynamic* HandleGIFRequest(const FString& GIFFilename);
};