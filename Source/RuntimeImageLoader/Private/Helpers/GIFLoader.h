// Copyright Peter Leontev
#define _CRT_SECURE_NO_WARNINGS
#pragma once

#include "CoreMinimal.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#endif // PLATFORM_WINDOWS

THIRD_PARTY_INCLUDES_START
    #include "nsgif.h"
THIRD_PARTY_INCLUDES_END
#include "GIFLoader.generated.h"

#define BYTES_PER_PIXEL 4

UCLASS()
class URuntimeGIFLoaderHelper : public UObject
{
	GENERATED_BODY()

#if WITH_LIBNSGIF
public:
	URuntimeGIFLoaderHelper();
	~URuntimeGIFLoaderHelper();

public:
	static void* bitmap_create(int width, int height);
	static unsigned char* bitmap_get_buffer(void* bitmap);
	static void bitmap_destroy(void* bitmap);
	uint8_t* Load_File(const char* path, size_t* data_size);
	void Warning(const char* context, nsgif_error err);
	void Decode(FILE* ppm, const char* name, nsgif_t* gif, bool first);
	void GIFDecoding(const char* FilePath);

public:
	UTexture2D* ConvertPPMToTexture2D();
	int32 ExtractDimensions(FString PPMDimension);

private:
	size_t Size;
	nsgif_t* Gif;
	uint8* Data;
	nsgif_error Error;
	FILE* PortablePixMap = nullptr;
	FString ppmFile = FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("RuntimeImageLoader/Content/Output.ppm"));;

private:
	const nsgif_bitmap_cb_vt bitmap_callbacks = {
		bitmap_create,
		bitmap_destroy,
		bitmap_get_buffer,
	};

#endif //WITH_LIBNSGIF
};