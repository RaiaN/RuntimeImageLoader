// Copyright 2022 Peter Leontev. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "TextureResource.h"
#include "RHIResources.h"
#include "RHIDefinitions.h"

class UTexture2D;

/**
 * The rendering resource which represents a runtime texture.
 */

class FRuntimeTextureResource : public FTextureResource
{
public:
    FRuntimeTextureResource(UTexture2D* InTexture, FTexture2DRHIRef RHITexture2D);
    virtual ~FRuntimeTextureResource();

    uint32 GetSizeX() const override { return SizeX; }
    uint32 GetSizeY() const override { return SizeY; }

    void InitRHI() override;
    void ReleaseRHI() override;

private:
    UTexture2D* Owner;
    uint32 SizeX;
    uint32 SizeY;
};