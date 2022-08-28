// Copyright 2022 Peter Leontev. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "RuntimeTextureResource.h"
#include "RHIResources.h"
#include "RHIDefinitions.h"

class UTexture2D;


class FRuntimeTexture2DResource : public FRuntimeTextureResource
{
public:
	FRuntimeTexture2DResource(UTexture2D* InOwner, FTexture2DRHIRef RHITextureCube);
	~FRuntimeTexture2DResource();
	
	/**
	 * Called when the resource is initialized. This is only called by the rendering thread.
	 */
	void InitRHI() override;
};