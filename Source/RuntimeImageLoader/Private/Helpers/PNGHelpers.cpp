// Copyright 2023 Petr Leontev. All Rights Reserved.

#include "PNGHelpers.h"

namespace FPNGHelpers
{
    void FillZeroAlphaPNGData(int32 SizeX, int32 SizeY, ETextureSourceFormat SourceFormat, uint8* SourceData)
    {
        switch (SourceFormat)
        {
            case TSF_BGRA8:
            {
                PNGDataFill<uint8, uint32, 2, 1, 0, 3> PNGFill(SizeX, SizeY, SourceData);
                PNGFill.ProcessData();
                break;
            }

            case TSF_RGBA16:
            {
                PNGDataFill<uint16, uint64, 0, 1, 2, 3> PNGFill(SizeX, SizeY, SourceData);
                PNGFill.ProcessData();
                break;
            }
        }
    }
}