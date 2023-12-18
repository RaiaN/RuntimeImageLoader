// Copyright 2023 Petr Leontev. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Engine/Texture.h"

class FQOILoader
{
public:
    bool IsValidImage(const uint8* Buffer, uint32 Length) const;
    bool Load(const uint8* Buffer, uint32 Length);

    FString GetLastError();

public:
    // Resulting image data and properties
    TArray<uint8> RawData;
    int32 Width;
    int32 Height;
    ETextureSourceFormat TextureSourceFormat = TSF_Invalid;
    TextureCompressionSettings CompressionSettings = TC_Default;
    bool bSRGB = true;

private:
    FString ErrorMessage;
};