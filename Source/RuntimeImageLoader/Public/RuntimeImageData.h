// Copyright 2022 Peter Leontev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PixelFormat.h"
#include "ImageCore.h"
#include "Launch/Resources/Version.h"

#if ENGINE_MAJOR_VERSION < 5
#include "Engine/Texture.h"
#else
#include "Engine/TextureDefines.h"
#endif

enum TextureFilter : int;

struct RUNTIMEIMAGELOADER_API FRuntimeImageData : public FImage
{
    void Init2D(int32 InSizeX, int32 InSizeY, ETextureSourceFormat InFormat, const void* InData = nullptr);

    int32 NumMips = 1;
    bool SRGB = true;
    TextureFilter FilteringMode = TextureFilter::TF_Default;
    ETextureSourceFormat TextureSourceFormat = TSF_Invalid;
    TextureCompressionSettings CompressionSettings;
    EPixelFormat PixelFormat = PF_B8G8R8A8;
};