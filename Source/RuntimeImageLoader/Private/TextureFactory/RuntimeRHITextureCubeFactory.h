// Copyright 2022 Peter Leontev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RuntimeImageData.h"

class FRHITextureCube;
typedef TRefCountPtr<FRHITextureCube> FTextureCubeRHIRef;


class UTextureCube;

class FRuntimeRHITextureCubeFactory
{
public:
    FRuntimeRHITextureCubeFactory(UTextureCube* InTextureCube, const FRuntimeImageData& InImageData);

    FTextureCubeRHIRef Create();

private:
    FTextureCubeRHIRef CreateTextureCubeRHI_Windows();
    void FinalizeRHITexture2D();

private:
    UTextureCube* NewTextureCube;
    const FRuntimeImageData& ImageData;

    FTextureCubeRHIRef RHITextureCube;
};