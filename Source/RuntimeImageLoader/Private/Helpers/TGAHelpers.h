// Copyright 2023 Petr Leontev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Serialization/Archive.h"

#include "RuntimeImageData.h"


namespace FTGAHelpers
{
    #pragma pack(push,1)

    struct FTGAFileHeader
    {
        uint8 IdFieldLength;
        uint8 ColorMapType;
        uint8 ImageTypeCode;		// 2 for uncompressed RGB format
        uint16 ColorMapOrigin;
        uint16 ColorMapLength;
        uint8 ColorMapEntrySize;
        uint16 XOrigin;
        uint16 YOrigin;
        uint16 Width;
        uint16 Height;
        uint8 BitsPerPixel;
        uint8 ImageDescriptor;
        friend FArchive& operator<<(FArchive& Ar, FTGAFileHeader& H)
        {
            Ar << H.IdFieldLength << H.ColorMapType << H.ImageTypeCode;
            Ar << H.ColorMapOrigin << H.ColorMapLength << H.ColorMapEntrySize;
            Ar << H.XOrigin << H.YOrigin << H.Width << H.Height << H.BitsPerPixel;
            Ar << H.ImageDescriptor;
            return Ar;
        }
    };

    #pragma pack(pop)

    bool DecompressTGA_helper(const FTGAFileHeader* TGA, uint32*& TextureData, const int32 TextureDataSize, FString& OutError);
    bool DecompressTGA(const FTGAFileHeader* TGA, FRuntimeImageData& OutImage, FString& OutError);

}