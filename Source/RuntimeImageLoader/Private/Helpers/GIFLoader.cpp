// Copyright 2023 Peter Leontev and Muhammad Ahmed Saleem. All Rights Reserved.

#include "GIFLoader.h"
#include "RuntimeImageLoaderLog.h"
#include "RuntimeImageLoaderLibHandler.h"

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

uint8* FRuntimeGIFLoaderHelper::LoadFile(const char* FilePath, size_t& DataSize)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	IFileHandle* FileHandle = PlatformFile.OpenRead(ANSI_TO_TCHAR(FilePath));

	if (!FileHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("Unable to open file: %s"), ANSI_TO_TCHAR(FilePath));
		return nullptr;
	}

	DataSize = FileHandle->Size();
	uint8* Buffer = reinterpret_cast<uint8*>(FMemory::Malloc(DataSize));

	if (!Buffer)
	{
		UE_LOG(LogTemp, Error, TEXT("Unable to allocate memory: %d bytes"), DataSize);
		FileHandle->~IFileHandle(); /** @See GenericPlatformFile.h. Destructor, also the only way to close the file handle **/
		return nullptr;
	}

	if (!FileHandle->Read(Buffer, DataSize))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to read file: %s"), ANSI_TO_TCHAR(FilePath));
		FMemory::Free(Buffer);
		FileHandle->~IFileHandle(); /** @See GenericPlatformFile.h. Destructor, also the only way to close the file handle **/
		return nullptr;
	}

	FileHandle->~IFileHandle(); /** @See GenericPlatformFile.h. Destructor, also the only way to close the file handle **/
	return Buffer;
}

void FRuntimeGIFLoaderHelper::Warning(const char* context, nsgif_error err)
{
	FString ErrorMessage = ANSI_TO_TCHAR(FLibnsgifHandler::FunctionPointerNsgifStrError()(err));
	const TCHAR* ContextStr = ANSI_TO_TCHAR(context);
	const TCHAR* ErrorMessageStr = *ErrorMessage;

	UE_LOG(LibNsgifHelper, Warning, TEXT("%s: %s"), ContextStr, ErrorMessageStr);
}

void FRuntimeGIFLoaderHelper::GIFDecoding(const char* FilePath)
{
	/* create our gif animation */
	Error = FLibnsgifHandler::FunctionPointerNsgifCreate()(&bitmap_callbacks, NSGIF_BITMAP_FMT_R8G8B8A8, &Gif);
	if (Error != NSGIF_OK) {
		Warning("nsgif_create", Error);
		return;
	}

	/* load file into memory */
	Data = LoadFile(FilePath, Size);

	Error = FLibnsgifHandler::FunctionPointerNsgifDataScan()(Gif, Size, Data);
	if (Error != NSGIF_OK)
	{
		/* Not fatal; some GIFs are nasty. Can still try to decode
		 * any frames that were decoded successfully. */
		Warning("nsgif_data_scan", Error);
	}

	FLibnsgifHandler::FunctionPointerNsgifDataComplete()(Gif);

	int32 LoopMax = FLibnsgifHandler::FunctionPointerNsgifGetInfo()(Gif)->loop_max;
	if (LoopMax == 0) LoopMax = 1;

	for (uint64 i = 0; i < LoopMax; i++)
	{
		Decode(Gif, i==0);
		
		/* We want to ignore any loop limit in the GIF. */
		FLibnsgifHandler::FunctionPointerNsgifReset()(Gif);
	}

	/* clean up */
	FLibnsgifHandler::FunctionPointerNsgifDestroy()(Gif);
	FMemory::Free(Data);
}

const int32 FRuntimeGIFLoaderHelper::GetWidth() const
{
	return FLibnsgifHandler::FunctionPointerNsgifGetInfo()(Gif)->width;
}

const int32 FRuntimeGIFLoaderHelper::GetHeight() const
{
	return FLibnsgifHandler::FunctionPointerNsgifGetInfo()(Gif)->height;
}

const int32 FRuntimeGIFLoaderHelper::GetTotalFrames() const
{
	return FLibnsgifHandler::FunctionPointerNsgifGetInfo()(Gif)->frame_count;
}

void FRuntimeGIFLoaderHelper::Decode(nsgif_t* gif, bool first)
{
	nsgif_error err;
	uint32_t frame_prev = 0;
	const nsgif_info_t* info;
	info = FLibnsgifHandler::FunctionPointerNsgifGetInfo()(gif);

	// Calculate the total number of pixels (frame_count * width * height)
	int32 TotalPixels = info->frame_count * info->width * info->height;
	TextureData.Empty(TotalPixels);
	TextureData.AddUninitialized(TotalPixels);

	// Decode the frames
	while (true) {
		nsgif_bitmap_t* bitmap;
		const uint8* image;
		uint32_t frame_new;
		uint32_t delay_cs;
		nsgif_rect_t area;

		err = FLibnsgifHandler::FunctionPointerNsgifFramePrepare()(gif, &area, &delay_cs, &frame_new);
		if (err != NSGIF_OK) {
			Warning("nsgif_frame_prepare", err);
			return;
		}

		if (frame_new < frame_prev) {
			// Must be an animation that loops. We only care about
			// decoding each frame once in this utility.
			return;
		}
		frame_prev = frame_new;

		err = FLibnsgifHandler::FunctionPointerNsgifFrameDecode()(gif, frame_new, &bitmap);
		if (err != NSGIF_OK) {
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
			return;
		}
	}
}

const FColor* FRuntimeGIFLoaderHelper::GetNextFrame(int32 FrameIndex)
{
	if (FrameIndex > GetTotalFrames() - 1) FrameIndex = 0;
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