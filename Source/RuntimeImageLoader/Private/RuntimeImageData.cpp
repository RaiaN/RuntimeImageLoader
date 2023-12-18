// Copyright 2023 Petr Leontev. All Rights Reserved.

#include "RuntimeImageData.h"

// Duplicate the code in "int32 FTextureSource::GetBytesPerPixel(ETextureSourceFormat Format)"
// Because that was Editor only code
int32 GetBytesPerPixel(ETextureSourceFormat Format)
{
	int32 BytesPerPixel = 0;
	switch (Format)
	{
	    case TSF_G8:		BytesPerPixel = 1; break;
	    case TSF_G16:		BytesPerPixel = 2; break;
	    case TSF_BGRA8:		BytesPerPixel = 4; break;
	    case TSF_BGRE8:		BytesPerPixel = 4; break;
	    case TSF_RGBA16:	BytesPerPixel = 8; break;
	    case TSF_RGBA16F:	BytesPerPixel = 8; break;
	    default:			BytesPerPixel = 0; break;
	}
	return BytesPerPixel;
}


ERawImageFormat::Type ToRawImageFormat(ETextureSourceFormat SourceFormat)
{
    check (SourceFormat != TSF_Invalid);
    
    switch (SourceFormat)
    {
        case TSF_G8:		return ERawImageFormat::G8;
        case TSF_G16:		return ERawImageFormat::G16;
        case TSF_BGRA8:		return ERawImageFormat::BGRA8;
        case TSF_BGRE8:		return ERawImageFormat::BGRE8;
        case TSF_RGBA16:	return ERawImageFormat::RGBA16;
        case TSF_RGBA16F:	return ERawImageFormat::RGBA16F;
    }
    checkNoEntry();
	return ERawImageFormat::BGRA8;
}

void FRuntimeImageData::Init2D(int32 InSizeX, int32 InSizeY, ETextureSourceFormat InFormat, const void* InData)
{
    SizeX = InSizeX;
    SizeY = InSizeY;
    NumSlices = 1;
    NumMips = 1;
    TextureSourceFormat = InFormat;
    Format = ToRawImageFormat(InFormat);

    const int32 RawDataSize = SizeX * SizeY * GetBytesPerPixel();

    RawData.AddUninitialized(RawDataSize);

    if (InData)
    {
        FMemory::Memcpy(RawData.GetData(), InData, RawData.Num());
    }
}
