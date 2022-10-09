// Copyright 2022 Peter Leontev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RuntimeImageData.h"


class UTexture2D;

class FRuntimeRHITexture2DFactory
{
public:
    FRuntimeRHITexture2DFactory(UTexture2D* InTexture2D, const FRuntimeImageData& InImageData);

    FTexture2DRHIRef Create();

private:
    FTexture2DRHIRef CreateRHITexture2D_Windows();
    FTexture2DRHIRef CreateRHITexture2D_Mobile();
    FTexture2DRHIRef CreateRHITexture2D_Other();
    void FinalizeRHITexture2D();

private:
    UTexture2D* NewTexture;
    const FRuntimeImageData& ImageData;

    FTexture2DRHIRef RHITexture2D;
};