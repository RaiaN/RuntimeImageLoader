// Copyright 2022 Peter Leontev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture.h"
#include "PixelFormat.h"
#include "ImageCore.h"

struct RUNTIMEIMAGELOADER_API FRuntimeImageData : public FImage
{
    void Init2D(int32 InSizeX, int32 InSizeY, ETextureSourceFormat InFormat, const void* InData = nullptr);

    int32 NumMips = 1;
    bool SRGB = true;
    ETextureSourceFormat TextureSourceFormat = TSF_Invalid;
    TextureCompressionSettings CompressionSettings;
    FDateTime ModificationTime;
    EPixelFormat PixelFormat = PF_B8G8R8A8;
};