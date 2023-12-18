// Copyright 2023 Petr Leontev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputImageDescription.generated.h"

USTRUCT(BlueprintType)
struct RUNTIMEIMAGELOADER_API FInputImageDescription
{
    GENERATED_BODY()

    FInputImageDescription(){ ImageBytes.Empty(); }
    FInputImageDescription(const FString& InputImageFilename): ImageFilename(InputImageFilename) {}
    FInputImageDescription(TArray<uint8>&& InputImageBytes) : ImageBytes(InputImageBytes) {}

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Runtime Image Loader"))
    FString ImageFilename = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Runtime Image Loader"))
    TArray<uint8> ImageBytes;
};