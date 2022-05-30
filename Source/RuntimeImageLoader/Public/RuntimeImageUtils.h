// Copyright 2022 Peter Leontev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture.h"
#include "RuntimeImageData.h"

class UTexture2D;


namespace FRuntimeImageUtils
{
    bool ImportBufferAsImage(const uint8* Buffer, int32 Length, FRuntimeImageData& OutImage, FString& OutError);

    UTexture2D* CreateTexture(const FString& ImageFilename, const FRuntimeImageData& ImageData);
}