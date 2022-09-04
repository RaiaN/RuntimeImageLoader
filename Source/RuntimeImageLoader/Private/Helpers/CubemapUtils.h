// Copyright 2022 Peter Leontev. All Rights Reserved.

#include "CoreMinimal.h"


namespace RuntimeCubemapUtils
{
    static void GenerateBaseCubeMipFromLongitudeLatitude2D(struct FImage* OutMip, const FImage& SrcImage, const uint32 MaxCubemapTextureResolution, uint8 SourceEncodingOverride);
};