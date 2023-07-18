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
	//~ End FTextureResource Interface.

private:
	int32 GetDefaultMipMapBias() const;

	void CreateSamplerStates(float MipMapBias);

private:
	UAnimatedTexture2D* Owner;

};