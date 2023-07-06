// Copyright Peter Leontev

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
	uint8* Load_File(const FString& FilePath, uint32& DataSize);
	void Warning(FString context, nsgif_error err);
	void Decode(FILE* ppm, const FString& name, nsgif_t* gif, bool first);
	void GIFDecoding(const FString& FilePath);

public:
	UTexture2D* ConvertGifToTexture2D(const FString& FilePath);
	nsgif_error DecodeFrames(nsgif_t* Gif, TArray<FColor>& OutPixels);
	void LoadGif(const FString& FilePath, UTexture2D*& OutTexture, bool bUseAsync);

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

#endif //WITH_LIBNSGIF
};