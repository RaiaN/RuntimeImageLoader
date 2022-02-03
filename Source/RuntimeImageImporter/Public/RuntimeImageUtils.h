// Copyright Peter Leontev

#pragma once

#include "CoreMinimal.h"

#include "RuntimeImageData.h"

class UTexture2D;


namespace FRuntimeImageUtils
{
    bool ImportBufferAsImage(const uint8* Buffer, int32 Length, FRuntimeImageData& OutImage, FString& OutError);
    void ImportFileAsImage(const FString& ImageFilename, FRuntimeImageData& OutImage, FString& OutError);
}