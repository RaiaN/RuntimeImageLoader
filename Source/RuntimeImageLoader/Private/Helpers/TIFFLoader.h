// Copyright Peter Leontev

#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture.h"

class UTexture2D;

struct FIBITMAP;
struct FIMEMORY;

class FRuntimeTiffLoadHelper
{
public:

	FRuntimeTiffLoadHelper();

	~FRuntimeTiffLoadHelper();

	bool Load(const uint8 * Buffer, uint32 Length);

	void SetError(const FString& InErrorMessage);

	int32 GetBitDepth() const;
	FString GetError();

	bool IsValid();

public:
	// Resulting image data and properties
	TArray<uint8> RawData;
	int32 Width;
	int32 Height;
	ETextureSourceFormat TextureSourceFormat = TSF_Invalid;
	TextureCompressionSettings CompressionSettings = TC_Default;
	bool bSRGB = true;

private:
	bool ConvertToRGBA16();
	bool ConvertHighBPPGrayscaleToBGRA();

private:
	bool bIsValid = false;
	FIBITMAP* Bitmap = nullptr;
	FIMEMORY* Memory = nullptr;

	int32 BitDepth;

	FString ErrorMessage;
};
