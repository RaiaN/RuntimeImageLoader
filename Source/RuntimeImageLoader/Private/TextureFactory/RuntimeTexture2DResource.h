// Copyright 2023 Unreal Solutions Ltd. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "RuntimeTextureResource.h"
#include "RHIResources.h"
#include "RHIDefinitions.h"
#include "Engine/Texture.h"
#include "Launch/Resources/Version.h"
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
	void InitRHI() override;

private:
	TextureFilter FilterMode;
};