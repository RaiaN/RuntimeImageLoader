// Copyright 2022 Peter Leontev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RuntimeImageData.h"

class UTexture2D;
class UTextureCube;

namespace FRuntimeImageUtils
{
    bool ImportBufferAsImage(const uint8* Buffer, int32 Length, FRuntimeImageData& OutImage, FString& OutError);

    UTexture2D* CreateTexture(const FString& ImageFilename, const FRuntimeImageData& ImageData);
    UTextureCube* CreateTextureCube(const FString& ImageFilename, const FRuntimeImageData& ImageData);
}