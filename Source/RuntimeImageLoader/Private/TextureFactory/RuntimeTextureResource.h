// Copyright 2023 Unreal Solutions Ltd. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "TextureResource.h"
#include "RHIDefinitions.h"

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

    virtual void InitRHI() override;
    virtual void ReleaseRHI() override;

protected:
    UTexture* Owner;
    uint32 SizeX;
    uint32 SizeY;
};