// Copyright Peter Leontev

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



void FRuntimeImageData::Init2D(int32 InSizeX, int32 InSizeY, ETextureSourceFormat InFormat, const void* InData)
{
    SizeX = InSizeX;
    SizeY = InSizeY;
    NumMips = 1;
    Format = InFormat;
    RawData.AddUninitialized(SizeX * SizeY * GetBytesPerPixel(Format));
    if (InData)
    {
        FMemory::Memcpy(RawData.GetData(), InData, RawData.Num());
    }
}

void FRuntimeImageData::Init2DWithMips(int32 InSizeX, int32 InSizeY, int32 InNumMips, ETextureSourceFormat InFormat, const void* InData)
{
    SizeX = InSizeX;
    SizeY = InSizeY;
    NumMips = InNumMips;
    Format = InFormat;

    int32 TotalSize = 0;
    for (int32 MipIndex = 0; MipIndex < InNumMips; ++MipIndex)
    {
        TotalSize += GetMipSize(MipIndex);
    }
    RawData.AddUninitialized(TotalSize);

    if (InData)
    {
        FMemory::Memcpy(RawData.GetData(), InData, RawData.Num());
    }
}

int32 FRuntimeImageData::GetMipSize(int32 InMipIndex) const
{
    check(InMipIndex >= 0);
    check(InMipIndex < NumMips);
    const int32 MipSizeX = FMath::Max(SizeX >> InMipIndex, 1);
    const int32 MipSizeY = FMath::Max(SizeY >> InMipIndex, 1);
    return MipSizeX * MipSizeY * GetBytesPerPixel(Format);
}

void* FRuntimeImageData::GetMipData(int32 InMipIndex)
{
    int32 Offset = 0;
    for (int32 MipIndex = 0; MipIndex < InMipIndex; ++MipIndex)
    {
        Offset += GetMipSize(MipIndex);
    }
    return &RawData[Offset];
}
