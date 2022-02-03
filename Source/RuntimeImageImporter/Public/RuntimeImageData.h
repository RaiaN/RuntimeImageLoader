// Copyright Peter Leontev

#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture.h"
#include "Misc/SecureHash.h"


struct FRuntimeImageData
{
    TArray<uint8> RawData;
    ETextureSourceFormat Format = TSF_Invalid;
    TextureCompressionSettings CompressionSettings = TC_Default;
    int32 NumMips;
    int32 SizeX = 0;
    int32 SizeY = 0;
    bool SRGB = true;
    FDateTime ModificationTime;

    void Init2D(int32 InSizeX, int32 InSizeY, ETextureSourceFormat InFormat, const void* InData = nullptr);
    void Init2DWithMips(int32 InSizeX, int32 InSizeY, int32 InNumMips, ETextureSourceFormat InFormat, const void* InData = nullptr);

    int32 GetMipSize(int32 InMipIndex) const;
    void* GetMipData(int32 InMipIndex);
};