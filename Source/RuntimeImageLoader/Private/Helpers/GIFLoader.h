// Copyright 2023 Peter Leontev and Muhammad Ahmed Saleem. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#endif // PLATFORM_WINDOWS

THIRD_PARTY_INCLUDES_START
#include "nsgif.h"
THIRD_PARTY_INCLUDES_END

#define BYTES_PER_PIXEL 4

class FRuntimeGIFLoaderHelper
{
#if WITH_LIBNSGIF
public:
	FRuntimeGIFLoaderHelper() = default;
	~FRuntimeGIFLoaderHelper() {}

public: /** Bitmap Callbacks Methods */
	static void* bitmap_create(int width, int height);
	static unsigned char* bitmap_get_buffer(void* bitmap);
	static void bitmap_destroy(void* bitmap);

public: /** Gif Decoding Methods */
	uint8* LoadFile(const char* FilePath, size_t& DataSize);
	void Warning(const char* context, nsgif_error err);
	void Decode(nsgif_t* gif, bool first);
	void GIFDecoding(const char* FilePath);

public: /** Gif Data Method */
	const int32 GetWidth() const;
	const int32 GetHeight() const;
	const int32 GetTotalFrames() const;
	const int32 GetFramePixels() const { return GetWidth() * GetHeight(); }
	
public: /** Get Next Frame Texture Data*/
	const FColor* GetNextFrame(int32 FrameIndex);

private: /** Gif Data*/
	size_t Size;
	nsgif_t* Gif;
	uint8* Data;
	nsgif_error Error;
	TArray<FColor> TextureData;

private:
	const nsgif_bitmap_cb_vt bitmap_callbacks = {
		bitmap_create,
		bitmap_destroy,
		bitmap_get_buffer,
	};

public:
	FRuntimeGIFLoaderHelper(const FRuntimeGIFLoaderHelper&) = delete;
	FRuntimeGIFLoaderHelper& operator=(const FRuntimeGIFLoaderHelper&) = delete;

#endif //WITH_LIBNSGIF
};