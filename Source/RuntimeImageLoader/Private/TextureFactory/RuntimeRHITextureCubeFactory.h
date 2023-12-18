// Copyright 2023 Petr Leontev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RuntimeImageData.h"
#include "Templates/RefCounting.h"

#if (ENGINE_MAJOR_VERSION >= 5) && (ENGINE_MINOR_VERSION > 0)
class FRHITexture;
typedef TRefCountPtr<FRHITexture> FTextureCubeRHIRef;
#else
class FRHITextureCube;
typedef TRefCountPtr<FRHITextureCube> FTextureCubeRHIRef;
#endif


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