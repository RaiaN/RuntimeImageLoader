// Copyright Peter Leontev
#pragma once

#include "CoreMinimal.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#endif // PLATFORM_WINDOWS

THIRD_PARTY_INCLUDES_START
#include "nsgif.h"
THIRD_PARTY_INCLUDES_END
//#include "GIFLoader.generated.h"

#define BYTES_PER_PIXEL 4

//UCLASS()
class FRuntimeGIFLoaderHelper/* : public UObject*/
{
	/*GENERATED_BODY()*/

#if WITH_LIBNSGIF
public:
	FRuntimeGIFLoaderHelper() = default;
	~FRuntimeGIFLoaderHelper() {}

public:
	static void* bitmap_create(int width, int height);
	static unsigned char* bitmap_get_buffer(void* bitmap);
	static void bitmap_destroy(void* bitmap);
	uint8_t* Load_File(const char* path, size_t* data_size);
	void Warning(const char* context, nsgif_error err);
	void Decode(FILE* ppm, const char* name, nsgif_t* gif, bool first);
	void GIFDecoding(const char* FilePath);

public:
	void ConvertPPMToTextureData();
	int32 ExtractDimensions(FString PPMDimension);
	TArray<TArray<FColor>> ReadPPMFile(const FString& FilePath, int32& Width, int32& Height);
	const FColor* GetFrameBuffer() const;
public:
	nsgif_t* GetGif() const { return Gif; }
	int32 GetWidth() const;
	int32 GetHeight() const;

private:
	size_t Size;
	nsgif_t* Gif;
	uint8* Data;
	nsgif_error Error;
	FILE* PortablePixMap = nullptr;
	FString ppmFile = FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("RuntimeImageLoader/Content/Output.ppm"));
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