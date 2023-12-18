// Copyright 2023 Petr Leontev. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Runtime/Launch/Resources/Version.h"
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
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION < 3
	virtual void InitRHI() override;
#else
	virtual void InitRHI(FRHICommandListBase& RHICmdList) override;
#endif
};