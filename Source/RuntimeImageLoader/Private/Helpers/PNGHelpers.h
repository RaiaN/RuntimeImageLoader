// Copyright 2023 Petr Leontev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture.h"

#include "RuntimeImageData.h"


namespace FPNGHelpers
{
    /**
     * This fills any pixels of a texture with have an alpha value of zero,
     * with an RGB from the nearest neighboring pixel which has non-zero alpha.
    */
    template<typename PixelDataType, typename ColorDataType, int32 RIdx, int32 GIdx, int32 BIdx, int32 AIdx> class PNGDataFill
    {
    public:

        PNGDataFill(int32 SizeX, int32 SizeY, uint8* SourceTextureData)
            : SourceData(reinterpret_cast<PixelDataType*>(SourceTextureData))
            , TextureWidth(SizeX)
            , TextureHeight(SizeY)
        {
        }

        void ProcessData()
        {
            int32 NumZeroedTopRowsToProcess = 0;
            int32 FillColorRow = -1;
            for (int32 Y = 0; Y < TextureHeight; ++Y)
            {
                if (!ProcessHorizontalRow(Y))
                {
                    if (FillColorRow != -1)
                    {
                        FillRowColorPixels(FillColorRow, Y);
                    }
                    else
                    {
                        NumZeroedTopRowsToProcess = Y;
                    }
                }
                else
                {
                    FillColorRow = Y;
                }
            }

            // Can only fill upwards if image not fully zeroed
            if (NumZeroedTopRowsToProcess > 0 && NumZeroedTopRowsToProcess + 1 < TextureHeight)
            {
                for (int32 Y = 0; Y <= NumZeroedTopRowsToProcess; ++Y)
                {
                    FillRowColorPixels(NumZeroedTopRowsToProcess + 1, Y);
                }
            }
        }

        /* returns False if requires further processing because entire row is filled with zeroed alpha values */
        bool ProcessHorizontalRow(int32 Y)
        {
            // only wipe out colors that are affected by png turning valid colors white if alpha = 0
            const uint32 WhiteWithZeroAlpha = FColor(255, 255, 255, 0).DWColor();

            // Left -> Right
            int32 NumLeftmostZerosToProcess = 0;
            const PixelDataType* FillColor = nullptr;
            for (int32 X = 0; X < TextureWidth; ++X)
            {
                PixelDataType* PixelData = SourceData + (Y * TextureWidth + X) * 4;
                ColorDataType* ColorData = reinterpret_cast<ColorDataType*>(PixelData);

                if (*ColorData == WhiteWithZeroAlpha)
                {
                    if (FillColor)
                    {
                        PixelData[RIdx] = FillColor[RIdx];
                        PixelData[GIdx] = FillColor[GIdx];
                        PixelData[BIdx] = FillColor[BIdx];
                    }
                    else
                    {
                        // Mark pixel as needing fill
                        *ColorData = 0;

                        // Keep track of how many pixels to fill starting at beginning of row
                        NumLeftmostZerosToProcess = X;
                    }
                }
                else
                {
                    FillColor = PixelData;
                }
            }

            if (NumLeftmostZerosToProcess == 0)
            {
                // No pixels left that are zero
                return true;
            }

            if (NumLeftmostZerosToProcess + 1 >= TextureWidth)
            {
                // All pixels in this row are zero and must be filled using rows above or below
                return false;
            }

            // Fill using non zero pixel immediately to the right of the beginning series of zeros
            FillColor = SourceData + (Y * TextureWidth + NumLeftmostZerosToProcess + 1) * 4;

            // Fill zero pixels found at beginning of row that could not be filled during the Left to Right pass
            for (int32 X = 0; X <= NumLeftmostZerosToProcess; ++X)
            {
                PixelDataType* PixelData = SourceData + (Y * TextureWidth + X) * 4;
                PixelData[RIdx] = FillColor[RIdx];
                PixelData[GIdx] = FillColor[GIdx];
                PixelData[BIdx] = FillColor[BIdx];
            }

            return true;
        }

        void FillRowColorPixels(int32 FillColorRow, int32 Y)
        {
            for (int32 X = 0; X < TextureWidth; ++X)
            {
                const PixelDataType* FillColor = SourceData + (FillColorRow * TextureWidth + X) * 4;
                PixelDataType* PixelData = SourceData + (Y * TextureWidth + X) * 4;
                PixelData[RIdx] = FillColor[RIdx];
                PixelData[GIdx] = FillColor[GIdx];
                PixelData[BIdx] = FillColor[BIdx];
            }
        }

        PixelDataType* SourceData;
        int32 TextureWidth;
        int32 TextureHeight;
    };

    void FillZeroAlphaPNGData(int32 SizeX, int32 SizeY, ETextureSourceFormat SourceFormat, uint8* SourceData);
}