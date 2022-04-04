// Copyright 2022 Peter Leontev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture.h"
#include "ImageCore.h"
#include "RuntimeImageData.h"

class UTexture2D;


namespace FRuntimeImageUtils
{
    bool ImportBufferAsImage(const uint8* Buffer, int32 Length, FRuntimeImageData& OutImage, FString& OutError);
    void ImportFileAsImage(const FString& ImageFilename, FRuntimeImageData& OutImage, FString& OutError);

    UTexture2D* CreateDummyTexture(const FString& ImageFilename, EPixelFormat PixelFormat);
}