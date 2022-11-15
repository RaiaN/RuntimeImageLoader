// Copyright 2022 Peter Leontev. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "RuntimeTextureResource.h"

class UTextureCube;


class FRuntimeTextureCubeResource : public FRuntimeTextureResource
{
public:
	FRuntimeTextureCubeResource(UTextureCube* InOwner, FTextureCubeRHIRef RHITextureCube);
	~FRuntimeTextureCubeResource();
	
	/**
	 * Called when the resource is initialized. This is only called by the rendering thread.
	 */
	void InitRHI() override;
};