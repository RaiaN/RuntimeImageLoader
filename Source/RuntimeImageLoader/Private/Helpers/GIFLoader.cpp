// Copyright 2023 Peter Leontev. All Rights Reserved.

#include "GIFLoader.h"
#include "Misc/FileHelper.h"
#include "RuntimeImageLoaderLog.h"

DEFINE_LOG_CATEGORY(LibNsgifHelper);

#if WITH_LIBNSGIF
void* FRuntimeGIFLoaderHelper::bitmap_create(int width, int height)
{
	if (width > 4096 || height > 4096) {
		return NULL;
	}

	return calloc(width * height, BYTES_PER_PIXEL);
}

unsigned char* FRuntimeGIFLoaderHelper::bitmap_get_buffer(void* bitmap)
{
	return (unsigned char*)bitmap;
}

void FRuntimeGIFLoaderHelper::bitmap_destroy(void* bitmap)
{
	free(bitmap);
}

void FRuntimeGIFLoaderHelper::Warning(const char* context)
{
	LastContext = ANSI_TO_TCHAR(context);

	UE_LOG(LibNsgifHelper, Warning, TEXT("%s"), *GetDecodeError());
}


FString FRuntimeGIFLoaderHelper::GetDecodeError() const
{
	return FString::Printf(TEXT("%s: %s"), *LastContext, nsgif_strerror(LastError));
}

bool FRuntimeGIFLoaderHelper::DecodeGIF(const FString& FilePath)
{
	/* create our gif animation */
	LastError = nsgif_create(&bitmap_callbacks, NSGIF_BITMAP_FMT_R8G8B8A8, &Gif);
	if (LastError != NSGIF_OK)
	{
		Warning("nsgif_create");
		return false;
	}

	/* load file into memory */
	if (!FFileHelper::LoadFileToArray(Data, *FilePath))
	{
		const FString FileError = FString::Printf(TEXT("LoadFileToArray: %s"), *FilePath);
		Warning(TCHAR_TO_ANSI(*FileError));
		return false;
	}

	LastError = nsgif_data_scan(Gif, Data.Num(), Data.GetData());
	if (LastError != NSGIF_OK)
	{
		/* Not fatal; some GIFs are nasty. Can still try to decode
		 * any frames that were decoded successfully. */
		Warning("nsgif_data_scan");
		return false;
	}

	nsgif_data_complete(Gif);

	int32 LoopMax = nsgif_get_info(Gif)->loop_max;
	if (LoopMax == 0) LoopMax = 1;

	for (uint64 i = 0; i < LoopMax; i++)
	{
		DecodeInternal(Gif, i==0);
		
		/* We want to ignore any loop limit in the GIF. */
		nsgif_reset(Gif);
	}

	/* clean up */
	nsgif_destroy(Gif);

	return !TextureData.IsEmpty();
}

const int32 FRuntimeGIFLoaderHelper::GetWidth() const
{
	return nsgif_get_info(Gif)->width;
}

const int32 FRuntimeGIFLoaderHelper::GetHeight() const
{
	return nsgif_get_info(Gif)->height;
}

const int32 FRuntimeGIFLoaderHelper::GetTotalFrames() const
{
	return nsgif_get_info(Gif)->frame_count;
}

bool FRuntimeGIFLoaderHelper::DecodeInternal(nsgif_t* gif, bool first)
{
	uint32_t frame_prev = 0;
	const nsgif_info_t* info;
	info = nsgif_get_info(gif);

	// Calculate the total number of pixels (frame_count * width * height)
	const int32 TotalPixels = info->frame_count * info->width * info->height;
	TextureData.Empty(TotalPixels);
	TextureData.AddUninitialized(TotalPixels);

	// Decode the frames
	while (true) {
		nsgif_bitmap_t* bitmap;
		const uint8* image;
		uint32_t frame_new;
		uint32_t delay_cs;
		nsgif_rect_t area;

		LastError = nsgif_frame_prepare(gif, &area, &delay_cs, &frame_new);
		if (LastError != NSGIF_OK) {
			Warning("nsgif_frame_prepare");
			return false;
		}

		if (frame_new < frame_prev) {
			// Must be an animation that loops. We only care about
			// decoding each frame once in this utility.
			return true;
		}
		frame_prev = frame_new;

		LastError = nsgif_frame_decode(gif, frame_new, &bitmap);
		if (LastError != NSGIF_OK) {
			// Continue decoding the rest of the frames.
		}
		else {
			image = (const uint8*)bitmap;
			for (uint32_t i = 0; i < info->height * info->width; i++) {
				uint32_t y = i / info->width;
				uint32_t x = i % info->width;

				size_t z = (y * info->width + x) * 4;
				uint8 Red = image[z];
				uint8 Green = image[z + 1];
				uint8 Blue = image[z + 2];
				uint8 Alpha = image[z + 3];

				int32 PixelIndex = ((frame_new * info->height + y) * info->width + x);
				TextureData[PixelIndex] = FColor(Red, Green, Blue, Alpha);
			}
		}

		if (delay_cs == NSGIF_INFINITE) {
			// This frame is the last.
			return true;
		}
	}

	return true;
}

const FColor* FRuntimeGIFLoaderHelper::GetNextFrame(int32 FrameIndex)
{
	if (FrameIndex > GetTotalFrames() - 1)
	{
		FrameIndex = 0;
	}

	// Calculate the starting index of the desired frame in the TextureData array
	int32 StartIndex = FrameIndex * GetWidth() * GetHeight();

	if (StartIndex >= 0 && StartIndex < TextureData.Num())
	{
		return &TextureData[StartIndex];
	}
	else
	{
		// Handling the case where the index is out of bounds
		return &FColor::Black; // return a default FColor value, like FColor::Black
	}
}
#endif //WITH_LIBNSGIF