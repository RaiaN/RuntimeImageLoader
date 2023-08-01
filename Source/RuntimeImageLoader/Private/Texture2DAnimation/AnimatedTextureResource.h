// Copyright 2023 Peter Leontev and Muhammad Ahmed Saleem. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TextureResource.h"	// Engine

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
	virtual void InitRHI() override;
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