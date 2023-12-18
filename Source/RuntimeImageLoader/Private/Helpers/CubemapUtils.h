// Copyright 2023 Petr Leontev. All Rights Reserved.

#include "CoreMinimal.h"
#include "ImageCore.h"


extern void GenerateBaseCubeMipFromLongitudeLatitude2D(FImage* OutMip, const FImage& SrcImage, const uint32 MaxCubemapTextureResolution, uint8 SourceEncodingOverride);