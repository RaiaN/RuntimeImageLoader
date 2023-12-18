// Copyright 2023 Petr Leontev. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "TextureResource.h"
#include "RHIDefinitions.h"
#include "Runtime/Launch/Resources/Version.h"

class UTexture;

/**
 * The rendering resource which represents a runtime texture.
 */

class FRuntimeTextureResource : public FTextureResource
{
public:
    FRuntimeTextureResource(UTexture* InTexture, FTextureRHIRef InRHITexture);
    virtual ~FRuntimeTextureResource();

    uint32 GetSizeX() const override { return SizeX; }
    uint32 GetSizeY() const override { return SizeY; }

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION < 3
    virtual void InitRHI() override;
#else
    virtual void InitRHI(FRHICommandListBase& RHICmdList) override;
#endif
    virtual void ReleaseRHI() override;

protected:
    UTexture* Owner;
    uint32 SizeX;
    uint32 SizeY;
};