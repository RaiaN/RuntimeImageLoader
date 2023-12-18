// Copyright 2023 Petr Leontev. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "RuntimeTextureResource.h"
#include "RHIResources.h"
#include "RHIDefinitions.h"
#include "Engine/Texture.h"
#include "Runtime/Launch/Resources/Version.h"
#if ENGINE_MAJOR_VERSION == 5
#include "Engine/TextureDefines.h"
#endif

class UTexture2D;


class FRuntimeTexture2DResource : public FRuntimeTextureResource
{
public:
	FRuntimeTexture2DResource(UTexture2D* InOwner, FTexture2DRHIRef InRHITexture2D, TextureFilter InFilterMode);
	~FRuntimeTexture2DResource();
	
	/**
	 * Called when the resource is initialized. This is only called by the rendering thread.
	 */
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION < 3
	virtual void InitRHI() override;
#else
	virtual void InitRHI(FRHICommandListBase& RHICmdList) override;
#endif

private:
	TextureFilter FilterMode;
};