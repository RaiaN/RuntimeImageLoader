// Copyright 2023 Unreal Solutions Ltd. All Rights Reserved.

#include "CoreMinimal.h"
#include "ImageCore.h"


extern void GenerateBaseCubeMipFromLongitudeLatitude2D(FImage* OutMip, const FImage& SrcImage, const uint32 MaxCubemapTextureResolution, uint8 SourceEncodingOverride);