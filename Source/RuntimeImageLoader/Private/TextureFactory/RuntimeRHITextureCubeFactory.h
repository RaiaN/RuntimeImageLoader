// Copyright 2022 Peter Leontev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Async/Future.h"
#include "RHIResources.h"
#include "RuntimeImageData.h"


class UTextureCube;
struct FRuntimeImageData;

class FRuntimeRHITextureCubeFactory
{
public:
    FRuntimeRHITextureCubeFactory(UTextureCube* InTextureCube, const FRuntimeImageData& InImageData);

    FTextureCubeRHIRef Create();

private:
    FTextureCubeRHIRef CreateTextureCubeRHI_Windows();
    void FinalizeRHITexture2D();

    void CopySrcDataToLockedDestData(uint8* Src, uint8* Dest, uint32 DestPitch, uint32 MipSize);

private:
    UTextureCube* NewTextureCube;
    const FRuntimeImageData& ImageData;

    FTextureCubeRHIRef RHITextureCube;
};