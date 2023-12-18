// Copyright 2023 Petr Leontev. All Rights Reserved.

#include "QOIHelpers.h"

#define QOI_IMPLEMENTATION 1

#include "qoi.h"

bool FQOILoader::IsValidImage(const uint8* Buffer, uint32 Length) const
{
    if (Buffer == nullptr || 
        Length < QOI_HEADER_SIZE + (int)sizeof(qoi_padding))
    {
        return false;
    }

    const unsigned char* bytes;
    unsigned int header_magic;
    int p = 0, run = 0;

    bytes = (const unsigned char*)Buffer;
    qoi_desc desc;

    header_magic = qoi_read_32(bytes, &p);
    desc.width = qoi_read_32(bytes, &p);
    desc.height = qoi_read_32(bytes, &p);
    desc.channels = bytes[p++];
    desc.colorspace = bytes[p++];

    if (
        desc.width == 0 || desc.height == 0 ||
        desc.channels < 3 || desc.channels > 4 ||
        desc.colorspace > 1 ||
        header_magic != QOI_MAGIC ||
        desc.height >= QOI_PIXELS_MAX / desc.width
    ) 
    {
        return false;
    }

    return true;
}


bool FQOILoader::Load(const uint8* Buffer, uint32 Length)
{
    qoi_desc ImageDescr;
    uint8* DecodedPixels = reinterpret_cast<uint8*>(qoi_decode(Buffer, Length, &ImageDescr, 0));
    if (DecodedPixels == nullptr)
    {
        ErrorMessage = TEXT("Can't decode input QOI image! Make sure the image is valid!");
        return false;
    }

    Width = ImageDescr.width;
    Height = ImageDescr.height;
    TextureSourceFormat = TSF_BGRA8;
    bSRGB = (ImageDescr.colorspace == QOI_SRGB);

    // reserver enough memory for BGRA8 
    RawData.SetNumUninitialized(Width * Height * 4);

    bool bSourceHasAlpha = static_cast<uint8>(ImageDescr.channels) == 4;

    const int32 PixelsLen = Width * Height * ImageDescr.channels;
    const uint32 Step = ImageDescr.channels;
    int32 RawDataIndex = 0;

    // convert RGBA to BGRA
    for (int32 Index = 0; Index < PixelsLen; Index += Step)
    {
        RawData[RawDataIndex]     = DecodedPixels[Index + 2];
        RawData[RawDataIndex + 1] = DecodedPixels[Index + 1];
        RawData[RawDataIndex + 2] = DecodedPixels[Index];
        RawData[RawDataIndex + 3] = (bSourceHasAlpha) ? DecodedPixels[Index + 3] : 255;
        
        RawDataIndex += 4;
    }

    QOI_FREE(DecodedPixels);

    return true;
}

FString FQOILoader::GetLastError()
{
    return ErrorMessage;
}