// Copyright 2023 Peter Leontev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

THIRD_PARTY_INCLUDES_START
extern "C" {
	#include "nsgif.h"
}
THIRD_PARTY_INCLUDES_END

#define BYTES_PER_PIXEL 4

class FRuntimeGIFLoaderHelper
{
#if WITH_LIBNSGIF
public:
	FRuntimeGIFLoaderHelper() = default;
	~FRuntimeGIFLoaderHelper() {}

public: /** Gif Data Method */
	const int32 GetWidth() const;
	const int32 GetHeight() const;
	const int32 GetTotalFrames() const;
	const int32 GetFramePixels() const { return GetWidth() * GetHeight(); }

public: /** Get Next Frame Texture Data*/
    const FColor* GetNextFrame(int32 FrameIndex);
    const TArray<FColor>& GetTextureData() const { return TextureData; }

	FString GetDecodeError() const;

	bool DecodeGIF(const FString& FilePath);

protected: /** Bitmap Callbacks Methods */
	static void* bitmap_create(int width, int height);
	static unsigned char* bitmap_get_buffer(void* bitmap);
	static void bitmap_destroy(void* bitmap);
	
	bool DecodeInternal(nsgif_t* gif, bool first);

	void Warning(const char* context);

private: /** Gif Data*/
	nsgif_t* Gif;
	TArray<uint8> Data;
	TArray<FColor> TextureData;
	const nsgif_info_t* Info;

	int32 Width = -1, Height = -1, TotalFrameCount = -1;

	nsgif_error LastError;
	FString LastContext;

private:
	const nsgif_bitmap_cb_vt bitmap_callbacks = {
		bitmap_create,
		bitmap_destroy,
		bitmap_get_buffer,
	};

#endif //WITH_LIBNSGIF
};