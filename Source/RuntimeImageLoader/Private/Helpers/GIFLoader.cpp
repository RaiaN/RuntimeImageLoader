// Copyright Peter Leontev

#include "GIFLoader.h"
#include "RuntimeImageLoaderLog.h"
#include "RuntimeImageLoaderLibHandler.h"
#include "Misc/FileHelper.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/Paths.h"

DEFINE_LOG_CATEGORY(LibNsgifHelper);

#if WITH_LIBNSGIF

FRuntimeGIFLoaderHelper::FRuntimeGIFLoaderHelper()
{
	/** Ain't Require Implementation At The Moment */
}

FRuntimeGIFLoaderHelper::~FRuntimeGIFLoaderHelper()
{
	/** Ain't Require Implementation At The Moment */
}

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

uint8* FRuntimeGIFLoaderHelper::Load_File(const FString& FilePath, uint32& DataSize)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	// Open the file for reading
	IFileHandle* FileHandle = PlatformFile.OpenRead(*FilePath);
	if (!FileHandle)
	{
		UE_LOG(LibNsgifHelper, Error, TEXT("Failed to open file: %s"), *FilePath);
		return nullptr;
	}

	// Get the file size
	DataSize = FileHandle->Size();

	// Allocate buffer for file data
	uint8* Buffer = new uint8[DataSize];

	// Read the file data into the buffer
	if (!FileHandle->Read(Buffer, DataSize))
	{
		UE_LOG(LibNsgifHelper, Error, TEXT("Failed to read file: %s"), *FilePath);
		delete[] Buffer;
		Buffer = nullptr;
	}

	// Close the file handle
	delete FileHandle;
	FileHandle = nullptr;

	return Buffer;
}

void FRuntimeGIFLoaderHelper::Warning(FString context, nsgif_error err)
{
	FString ErrorMessage = ANSI_TO_TCHAR(FLibnsgifHandler::FunctionPointerNsgifStrError()(err));
	const TCHAR* ContextStr = *context;
	const TCHAR* ErrorMessageStr = *ErrorMessage;

	UE_LOG(LibNsgifHelper, Warning, TEXT("%s: %s"), ContextStr, ErrorMessageStr);
}

void FRuntimeGIFLoaderHelper::Decode(FILE* ppm, const FString& name, nsgif_t* gif, bool first)
{
	nsgif_error Err;
	uint32 FramePrev = 0;
	const nsgif_info_t* Info;

	Info = FLibnsgifHandler::FunctionPointerNsgifGetInfo()(gif);

	FString PPM = FPaths::GetPath(FString::Printf(TEXT("%p"), ppm));

	if (first && ppm != nullptr)
	{
		FString PPMHeader = FString::Printf(TEXT("P3\n"));
		FString PPMComment = FString::Printf(TEXT("# %s\n"), *name);
		FString PPMWidth = FString::Printf(TEXT("# width                %u \n"), Info->width);
		FString PPMHeight = FString::Printf(TEXT("# height               %u \n"), Info->height);
		FString PPMFrameCount = FString::Printf(TEXT("# frame_count          %u \n"), Info->frame_count);
		FString PPMLoopMax = FString::Printf(TEXT("# loop_max             %u \n"), Info->loop_max);
		FString PPMImageSize = FString::Printf(TEXT("%u %u 256\n"), Info->width, Info->height * Info->frame_count);

		FFileHelper::SaveStringToFile(PPMHeader, *PPM);
		FFileHelper::SaveStringToFile(PPMComment, *PPM, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_Append);
		FFileHelper::SaveStringToFile(PPMWidth, *PPM, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_Append);
		FFileHelper::SaveStringToFile(PPMHeight, *PPM, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_Append);
		FFileHelper::SaveStringToFile(PPMFrameCount, *PPM, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_Append);
		FFileHelper::SaveStringToFile(PPMLoopMax, *PPM, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_Append);
		FFileHelper::SaveStringToFile(PPMImageSize, *PPM, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_Append);
	}

	while (true)
	{
		nsgif_bitmap_t* Bitmap;
		const uint8_t* Image;
		uint32 FrameNew;
		uint32 DelayCs;
		nsgif_rect_t Area;

		Err = FLibnsgifHandler::FunctionPointerNsgifFramePrepare()(gif, &Area, &DelayCs, &FrameNew);
		if (Err != NSGIF_OK)
		{
			Warning(TEXT("nsgif_frame_prepare"), Err);
			return;
		}

		if (FrameNew < FramePrev)
		{
			// Must be an animation that loops. We only care about decoding each frame once in this utility.
			return;
		}
		FramePrev = FrameNew;

		Err = FLibnsgifHandler::FunctionPointerNsgifFrameDecode()(gif, FrameNew, &Bitmap);
		if (Err != NSGIF_OK)
		{
			FString ErrorString = FString::Printf(TEXT("Frame %u: nsgif_decode_frame failed: %s\n"), FrameNew, FLibnsgifHandler::FunctionPointerNsgifStrError()(Err));
			UE_LOG(LibNsgifHelper, Error, TEXT("%s"), *ErrorString);
			// Continue decoding the rest of the frames.
		}
		else if (first && ppm != nullptr)
		{
			FString FrameComment = FString::Printf(TEXT("# frame %u:\n"), FrameNew);
			FFileHelper::SaveStringToFile(FrameComment, *PPM, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_Append);

			Image = reinterpret_cast<const uint8_t*>(Bitmap);
			for (uint32 y = 0; y != Info->height; y++)
			{
				for (uint32 x = 0; x != Info->width; x++)
				{
					size_t Z = (y * Info->width + x) * 4;
					FString ColorString = FString::Printf(TEXT("%u %u %u "), Image[Z], Image[Z + 1], Image[Z + 2]);
					FFileHelper::SaveStringToFile(ColorString, *PPM, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_Append);
				}
				FString NewLine = FString::Printf(TEXT("\n"));
				FFileHelper::SaveStringToFile(NewLine, *PPM, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_Append);
			}
		}

		if (DelayCs == NSGIF_INFINITE)
		{
			// This frame is the last.
			return;
		}
	}
}

void FRuntimeGIFLoaderHelper::GIFDecoding(const FString& FilePath)
{
	/* create our gif animation */
	Error = FLibnsgifHandler::FunctionPointerNsgifCreate()(&bitmap_callbacks, NSGIF_BITMAP_FMT_R8G8B8A8, &Gif);
	if (Error != NSGIF_OK) {
		Warning("nsgif_create", Error);
		return;
	}

	/* load file into memory */
	Data = Load_File(FilePath, Size);

	Error = FLibnsgifHandler::FunctionPointerNsgifDataScan()(Gif, Size, Data);
	if (Error != NSGIF_OK)
	{
		/* Not fatal; some GIFs are nasty. Can still try to decode
		 * any frames that were decoded successfully. */
		Warning("nsgif_data_scan", Error);
	}

	FLibnsgifHandler::FunctionPointerNsgifDataComplete()(Gif);

	for (uint64 i = 0; i < 1; i++)
	{
		Decode(PortablePixMap, FilePath, Gif, i == 0);
		
		/* We want to ignore any loop limit in the GIF. */
		FLibnsgifHandler::FunctionPointerNsgifReset()(Gif);
	}

	if (PortablePixMap != nullptr)
	{
		fclose(PortablePixMap);
	}

	/* clean up */
	FLibnsgifHandler::FunctionPointerNsgifDestroy()(Gif);
	free(Data);
}

#endif //WITH_LIBNSGIF