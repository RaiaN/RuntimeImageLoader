// Copyright Peter Leontev

#pragma once

#include "CoreMinimal.h"

#if WITH_LIBNSGIF

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#endif // PLATFORM_WINDOWS

THIRD_PARTY_INCLUDES_START
    #include "nsgif.h"
THIRD_PARTY_INCLUDES_END

#define BYTES_PER_PIXEL 4

class FRuntimeGIFLoaderHelper
{
public:
	FRuntimeGIFLoaderHelper();
	~FRuntimeGIFLoaderHelper();

public:
	static void* bitmap_create(int width, int height);
	static unsigned char* bitmap_get_buffer(void* bitmap);
	static void bitmap_destroy(void* bitmap);
	uint8* Load_File(const FString& FilePath, uint32& DataSize);
	void Warning(FString context, nsgif_error err);
	void Decode(FILE* ppm, const FString& name, nsgif_t* gif, bool first);
	void GIFDecoding(const FString& FilePath);

private:
	uint32 Size;
	nsgif_t* Gif;
	uint8* Data;
	nsgif_error Error;
	FILE* PortablePixMap = nullptr;

private:
	const nsgif_bitmap_cb_vt bitmap_callbacks = {
		bitmap_create,
		bitmap_destroy,
		bitmap_get_buffer,
	};
};

#endif //WITH_LIBNSGIF