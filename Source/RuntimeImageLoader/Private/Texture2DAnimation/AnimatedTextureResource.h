// Copyright 2023 Petr Leontev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TextureResource.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Templates/RefCounting.h"


#if (ENGINE_MAJOR_VERSION >= 5) && (ENGINE_MINOR_VERSION > 0)
class FRHITexture;
typedef TRefCountPtr<FRHITexture> FTexture2DRHIRef;
#else
class FRHITexture2D;
typedef TRefCountPtr<FRHITexture2D> FTexture2DRHIRef;
#endif

class UAnimatedTexture2D;

/**
 * FTextureResource implementation for animated 2D textures
 * @see class FTexture2DDynamicResource
 */
class FAnimatedTextureResource : public FTextureResource
{
public:
	FAnimatedTextureResource(UAnimatedTexture2D* InOwner);

	//~ Begin FTextureResource Interface.
	virtual uint32 GetSizeX() const override;
	virtual uint32 GetSizeY() const override;
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION < 3
	virtual void InitRHI() override;
#else
	virtual void InitRHI(FRHICommandListBase& RHICmdList) override;
#endif
	virtual void ReleaseRHI() override;
	/** Returns the Texture2DRHI, which can be used for locking/unlocking the mips. */
	FTexture2DRHIRef GetTexture2DRHI();
	//~ End FTextureResource Interface.

private:
	int32 GetDefaultMipMapBias() const;

	void CreateSamplerStates(float MipMapBias);

private:
	UAnimatedTexture2D* Owner;
	/** Texture2D reference, used for locking/unlocking the mips. */
	FTexture2DRHIRef Texture2DRHI;

};