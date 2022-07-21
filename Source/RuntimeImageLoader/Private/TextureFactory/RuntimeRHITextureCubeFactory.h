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
    UTextureCube* NewTextureCube;
    const FRuntimeImageData& ImageData;

    FTextureCubeRHIRef RHITextureCube;
};