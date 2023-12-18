// Copyright 2023 Petr Leontev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Launch/Resources/Version.h"
#include "RuntimeImageData.h"
#include "Templates/RefCounting.h"

#if (ENGINE_MAJOR_VERSION >= 5) && (ENGINE_MINOR_VERSION > 0)
class FRHITexture;
typedef TRefCountPtr<FRHITexture> FTexture2DRHIRef;
#else
class FRHITexture2D;
typedef TRefCountPtr<FRHITexture2D> FTexture2DRHIRef;
#endif


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