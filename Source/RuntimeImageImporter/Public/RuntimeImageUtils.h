// Copyright Peter Leontev

#pragma once

#include "CoreMinimal.h"

#include "ImportImage.h"

class UTexture2D;


namespace FRuntimeImageUtils
{
    bool ImportBufferAsImage(const uint8* Buffer, int32 Length, FImportImage& OutImage, FString& OutError);
    void ImportFileAsImage(const FString& ImageFilename, FImportImage& OutImage, FString& OutError);
}