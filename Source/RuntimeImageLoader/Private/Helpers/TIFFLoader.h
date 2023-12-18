// Copyright 2023 Petr Leontev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture.h"

struct FIBITMAP;
struct FIMEMORY;

class FRuntimeTiffLoadHelper
{
public:
	FRuntimeTiffLoadHelper();
	~FRuntimeTiffLoadHelper();

	bool Load(const uint8* Buffer, uint32 Length);

	void SetError(const FString& InErrorMessage);
	FString GetError();

	void Reset();

	bool IsValid();

public:
	// Resulting image data and properties
	TArray<uint8> RawData;
	int32 Width;
	int32 Height;
	ETextureSourceFormat TextureSourceFormat = TSF_Invalid;
	TextureCompressionSettings CompressionSettings = TC_Default;
	bool bSRGB = true;
	int32 BitDepth;

private:
	bool ConvertToRGBA16();

private:
	bool bIsValid = false;
	FIBITMAP* Bitmap = nullptr;
	FIMEMORY* Memory = nullptr;

	FString ErrorMessage;
};
