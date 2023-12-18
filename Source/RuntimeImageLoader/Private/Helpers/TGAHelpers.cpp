// Copyright 2023 Petr Leontev. All Rights Reserved.

#include "TGAHelpers.h"


namespace FTGAHelpers
{
    void DecompressTGA_RLE_32bpp(const FTGAFileHeader* TGA, uint32* TextureData)
    {
        uint8* IdData = (uint8*)TGA + sizeof(FTGAFileHeader);
        uint8* ColorMap = IdData + TGA->IdFieldLength;
        uint8* ImageData = (uint8*)(ColorMap + (TGA->ColorMapEntrySize + 4) / 8 * TGA->ColorMapLength);
        uint32 Pixel = 0;
        int32 RLERun = 0;
        int32 RAWRun = 0;

        for (int32 Y = TGA->Height - 1; Y >= 0; Y--) // Y-flipped.
        {
            for (int32 X = 0; X < TGA->Width; X++)
            {
                if (RLERun > 0)
                {
                    RLERun--;  // reuse current Pixel data.
                }
                else if (RAWRun == 0) // new raw pixel or RLE-run.
                {
                    uint8 RLEChunk = *(ImageData++);
                    if (RLEChunk & 0x80)
                    {
                        RLERun = (RLEChunk & 0x7F) + 1;
                        RAWRun = 1;
                    }
                    else
                    {
                        RAWRun = (RLEChunk & 0x7F) + 1;
                    }
                }
                // Retrieve new pixel data - raw run or single pixel for RLE stretch.
                if (RAWRun > 0)
                {
                    Pixel = *(uint32*)ImageData; // RGBA 32-bit dword.
                    ImageData += 4;
                    RAWRun--;
                    RLERun--;
                }
                // Store.
                *((TextureData + Y * TGA->Width) + X) = Pixel;
            }
        }
    }

    void DecompressTGA_RLE_24bpp(const FTGAFileHeader* TGA, uint32* TextureData)
    {
        uint8* IdData = (uint8*)TGA + sizeof(FTGAFileHeader);
        uint8* ColorMap = IdData + TGA->IdFieldLength;
        uint8* ImageData = (uint8*)(ColorMap + (TGA->ColorMapEntrySize + 4) / 8 * TGA->ColorMapLength);
        uint8 Pixel[4] = {};
        int32 RLERun = 0;
        int32 RAWRun = 0;

        for (int32 Y = TGA->Height - 1; Y >= 0; Y--) // Y-flipped.
        {
            for (int32 X = 0; X < TGA->Width; X++)
            {
                if (RLERun > 0)
                    RLERun--;  // reuse current Pixel data.
                else if (RAWRun == 0) // new raw pixel or RLE-run.
                {
                    uint8 RLEChunk = *(ImageData++);
                    if (RLEChunk & 0x80)
                    {
                        RLERun = (RLEChunk & 0x7F) + 1;
                        RAWRun = 1;
                    }
                    else
                    {
                        RAWRun = (RLEChunk & 0x7F) + 1;
                    }
                }
                // Retrieve new pixel data - raw run or single pixel for RLE stretch.
                if (RAWRun > 0)
                {
                    Pixel[0] = *(ImageData++);
                    Pixel[1] = *(ImageData++);
                    Pixel[2] = *(ImageData++);
                    Pixel[3] = 255;
                    RAWRun--;
                    RLERun--;
                }
                // Store.
                *((TextureData + Y * TGA->Width) + X) = *(uint32*)&Pixel;
            }
        }
    }

    void DecompressTGA_RLE_16bpp(const FTGAFileHeader* TGA, uint32* TextureData)
    {
        uint8* IdData = (uint8*)TGA + sizeof(FTGAFileHeader);
        uint8* ColorMap = IdData + TGA->IdFieldLength;
        uint16* ImageData = (uint16*)(ColorMap + (TGA->ColorMapEntrySize + 4) / 8 * TGA->ColorMapLength);
        uint16 FilePixel = 0;
        uint32 TexturePixel = 0;
        int32 RLERun = 0;
        int32 RAWRun = 0;

        for (int32 Y = TGA->Height - 1; Y >= 0; Y--) // Y-flipped.
        {
            for (int32 X = 0; X < TGA->Width; X++)
            {
                if (RLERun > 0)
                    RLERun--;  // reuse current Pixel data.
                else if (RAWRun == 0) // new raw pixel or RLE-run.
                {
                    uint8 RLEChunk = *((uint8*)ImageData);
                    ImageData = (uint16*)(((uint8*)ImageData) + 1);
                    if (RLEChunk & 0x80)
                    {
                        RLERun = (RLEChunk & 0x7F) + 1;
                        RAWRun = 1;
                    }
                    else
                    {
                        RAWRun = (RLEChunk & 0x7F) + 1;
                    }
                }
                // Retrieve new pixel data - raw run or single pixel for RLE stretch.
                if (RAWRun > 0)
                {
                    FilePixel = *(ImageData++);
                    RAWRun--;
                    RLERun--;
                }
                // Convert file format A1R5G5B5 into pixel format B8G8R8B8
                TexturePixel = (FilePixel & 0x001F) << 3;
                TexturePixel |= (FilePixel & 0x03E0) << 6;
                TexturePixel |= (FilePixel & 0x7C00) << 9;
                TexturePixel |= (FilePixel & 0x8000) << 16;
                // Store.
                *((TextureData + Y * TGA->Width) + X) = TexturePixel;
            }
        }
    }

    void DecompressTGA_32bpp(const FTGAFileHeader* TGA, uint32* TextureData)
    {

        uint8* IdData = (uint8*)TGA + sizeof(FTGAFileHeader);
        uint8* ColorMap = IdData + TGA->IdFieldLength;
        uint32* ImageData = (uint32*)(ColorMap + (TGA->ColorMapEntrySize + 4) / 8 * TGA->ColorMapLength);

        for (int32 Y = 0; Y < TGA->Height; Y++)
        {
            FMemory::Memcpy(TextureData + Y * TGA->Width, ImageData + (TGA->Height - Y - 1) * TGA->Width, TGA->Width * 4);
        }
    }

    void DecompressTGA_16bpp(const FTGAFileHeader* TGA, uint32* TextureData)
    {
        uint8* IdData = (uint8*)TGA + sizeof(FTGAFileHeader);
        uint8* ColorMap = IdData + TGA->IdFieldLength;
        uint16* ImageData = (uint16*)(ColorMap + (TGA->ColorMapEntrySize + 4) / 8 * TGA->ColorMapLength);
        uint16 FilePixel = 0;
        uint32 TexturePixel = 0;

        for (int32 Y = TGA->Height - 1; Y >= 0; Y--)
        {
            for (int32 X = 0; X < TGA->Width; X++)
            {
                FilePixel = *ImageData++;
                // Convert file format A1R5G5B5 into pixel format B8G8R8A8
                TexturePixel = (FilePixel & 0x001F) << 3;
                TexturePixel |= (FilePixel & 0x03E0) << 6;
                TexturePixel |= (FilePixel & 0x7C00) << 9;
                TexturePixel |= (FilePixel & 0x8000) << 16;
                // Store.
                *((TextureData + Y * TGA->Width) + X) = TexturePixel;
            }
        }
    }

    void DecompressTGA_24bpp(const FTGAFileHeader* TGA, uint32* TextureData)
    {
        uint8* IdData = (uint8*)TGA + sizeof(FTGAFileHeader);
        uint8* ColorMap = IdData + TGA->IdFieldLength;
        uint8* ImageData = (uint8*)(ColorMap + (TGA->ColorMapEntrySize + 4) / 8 * TGA->ColorMapLength);
        uint8 Pixel[4];

        for (int32 Y = 0; Y < TGA->Height; Y++)
        {
            for (int32 X = 0; X < TGA->Width; X++)
            {
                Pixel[0] = *((ImageData + (TGA->Height - Y - 1)*TGA->Width * 3) + X * 3 + 0);
                Pixel[1] = *((ImageData + (TGA->Height - Y - 1)*TGA->Width * 3) + X * 3 + 1);
                Pixel[2] = *((ImageData + (TGA->Height - Y - 1)*TGA->Width * 3) + X * 3 + 2);
                Pixel[3] = 255;
                *((TextureData + Y * TGA->Width) + X) = *(uint32*)&Pixel;
            }
        }
    }

    void DecompressTGA_8bpp(const FTGAFileHeader* TGA, uint8* TextureData)
    {
        const uint8*  const IdData = (uint8*)TGA + sizeof(FTGAFileHeader);
        const uint8*  const ColorMap = IdData + TGA->IdFieldLength;
        const uint8*  const ImageData = (uint8*)(ColorMap + (TGA->ColorMapEntrySize + 4) / 8 * TGA->ColorMapLength);

        int32 RevY = 0;
        for (int32 Y = TGA->Height - 1; Y >= 0; --Y)
        {
            const uint8* ImageCol = ImageData + (Y * TGA->Width);
            uint8* TextureCol = TextureData + (RevY++ * TGA->Width);
            FMemory::Memcpy(TextureCol, ImageCol, TGA->Width);
        }
    }


    bool DecompressTGA_helper(const FTGAFileHeader* TGA, uint32*& TextureData, const int32 TextureDataSize, FString& OutError)
    {
        if (TGA->ImageTypeCode == 10) // 10 = RLE compressed 
        {
            // RLE compression: CHUNKS: 1 -byte header, high bit 0 = raw, 1 = compressed
            // bits 0-6 are a 7-bit count; count+1 = number of raw pixels following, or rle pixels to be expanded. 
            if (TGA->BitsPerPixel == 32)
            {
                DecompressTGA_RLE_32bpp(TGA, TextureData);
            }
            else if (TGA->BitsPerPixel == 24)
            {
                DecompressTGA_RLE_24bpp(TGA, TextureData);
            }
            else if (TGA->BitsPerPixel == 16)
            {
                DecompressTGA_RLE_16bpp(TGA, TextureData);
            }
            else
            {
                OutError = FString::Printf(TEXT("TGA uses an unsupported rle-compressed bit-depth: %u"), TGA->BitsPerPixel);
                return false;
            }
        }
        else if (TGA->ImageTypeCode == 2) // 2 = Uncompressed RGB
        {
            if (TGA->BitsPerPixel == 32)
            {
                DecompressTGA_32bpp(TGA, TextureData);
            }
            else if (TGA->BitsPerPixel == 16)
            {
                DecompressTGA_16bpp(TGA, TextureData);
            }
            else if (TGA->BitsPerPixel == 24)
            {
                DecompressTGA_24bpp(TGA, TextureData);
            }
            else
            {
                OutError = FString::Printf(TEXT("TGA uses an unsupported bit-depth: %u"), TGA->BitsPerPixel);
                return false;
            }
        }
        // Support for alpha stored as pseudo-color 8-bit TGA
        else if (TGA->ColorMapType == 1 && TGA->ImageTypeCode == 1 && TGA->BitsPerPixel == 8)
        {
            DecompressTGA_8bpp(TGA, (uint8*)TextureData);
        }
        // standard grayscale
        else if (TGA->ColorMapType == 0 && TGA->ImageTypeCode == 3 && TGA->BitsPerPixel == 8)
        {
            DecompressTGA_8bpp(TGA, (uint8*)TextureData);
        }
        else
        {
            OutError = FString::Printf(TEXT("TGA is an unsupported type: %u"), TGA->ImageTypeCode);
            return false;
        }

        // Flip the image data if the flip bits are set in the TGA header.
        bool FlipX = (TGA->ImageDescriptor & 0x10) ? 1 : 0;
        bool FlipY = (TGA->ImageDescriptor & 0x20) ? 1 : 0;
        if (FlipY || FlipX)
        {
            TArray<uint8> FlippedData;
            FlippedData.AddUninitialized(TextureDataSize);

            int32 NumBlocksX = TGA->Width;
            int32 NumBlocksY = TGA->Height;
            int32 BlockBytes = TGA->BitsPerPixel == 8 ? 1 : 4;

            uint8* MipData = (uint8*)TextureData;

            for (int32 Y = 0; Y < NumBlocksY; Y++)
            {
                for (int32 X = 0; X < NumBlocksX; X++)
                {
                    int32 DestX = FlipX ? (NumBlocksX - X - 1) : X;
                    int32 DestY = FlipY ? (NumBlocksY - Y - 1) : Y;
                    FMemory::Memcpy(
                        &FlippedData[(DestX + DestY * NumBlocksX) * BlockBytes],
                        &MipData[(X + Y * NumBlocksX) * BlockBytes],
                        BlockBytes
                    );
                }
            }
            FMemory::Memcpy(MipData, FlippedData.GetData(), FlippedData.Num());
        }

        return true;
    }

    bool DecompressTGA(const FTGAFileHeader* TGA, FRuntimeImageData& OutImage, FString& OutError)
    {
        if (TGA->ColorMapType == 1 && TGA->ImageTypeCode == 1 && TGA->BitsPerPixel == 8)
        {
            // Notes: The Scaleform GFx exporter (dll) strips all font glyphs into a single 8-bit texture.
            // The targa format uses this for a palette index; GFx uses a palette of (i,i,i,i) so the index
            // is also the alpha value.
            //
            // We store the image as PF_G8, where it will be used as alpha in the Glyph shader.
            OutImage.Init2D(
                TGA->Width,
                TGA->Height,
                TSF_G8);
            OutImage.CompressionSettings = TC_Grayscale;
        }
        else if (TGA->ColorMapType == 0 && TGA->ImageTypeCode == 3 && TGA->BitsPerPixel == 8)
        {
            // standard grayscale images
            OutImage.Init2D(
                TGA->Width,
                TGA->Height,
                TSF_G8);
            OutImage.CompressionSettings = TC_Grayscale;
        }
        else
        {
            if (TGA->ImageTypeCode == 10) // 10 = RLE compressed 
            {
                if (TGA->BitsPerPixel != 32 &&
                    TGA->BitsPerPixel != 24 &&
                    TGA->BitsPerPixel != 16)
                {
                    OutError = FString::Printf(TEXT("TGA uses an unsupported rle-compressed bit-depth: %u"), TGA->BitsPerPixel);
                    return false;
                }
            }
            else
            {
                if (TGA->BitsPerPixel != 32 &&
                    TGA->BitsPerPixel != 16 &&
                    TGA->BitsPerPixel != 24)
                {
                    OutError = FString::Printf(TEXT("TGA uses an unsupported bit-depth: %u"), TGA->BitsPerPixel);
                    return false;
                }
            }

            OutImage.Init2D(
                TGA->Width,
                TGA->Height,
                TSF_BGRA8
            );
        }

        int32 TextureDataSize = OutImage.RawData.Num();
        uint32* TextureData = (uint32*)OutImage.RawData.GetData();

        return DecompressTGA_helper(TGA, TextureData, TextureDataSize, OutError);
    }
}