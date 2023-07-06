// Copyright Peter Leontev

#include "GIFLoader.h"
#include "RuntimeImageLoaderLog.h"
#include "RuntimeImageLoaderLibHandler.h"
#include "Misc/FileHelper.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/Paths.h"
#include "Async/Async.h"

DEFINE_LOG_CATEGORY(LibNsgifHelper);

#if WITH_LIBNSGIF

URuntimeGIFLoaderHelper::URuntimeGIFLoaderHelper()
{
	/** Ain't Require Implementation At The Moment */
}

URuntimeGIFLoaderHelper::~URuntimeGIFLoaderHelper()
{
	/** Ain't Require Implementation At The Moment */
}

void* URuntimeGIFLoaderHelper::bitmap_create(int width, int height)
{
	if (width > 4096 || height > 4096) {
		return NULL;
	}

	return calloc(width * height, BYTES_PER_PIXEL);
}

unsigned char* URuntimeGIFLoaderHelper::bitmap_get_buffer(void* bitmap)
{
	return (unsigned char*)bitmap;
}

void URuntimeGIFLoaderHelper::bitmap_destroy(void* bitmap)
{
	free(bitmap);
}

uint8* URuntimeGIFLoaderHelper::Load_File(const FString& FilePath, uint32& DataSize)
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

void URuntimeGIFLoaderHelper::Warning(FString context, nsgif_error err)
{
	FString ErrorMessage = ANSI_TO_TCHAR(FLibnsgifHandler::FunctionPointerNsgifStrError()(err));
	const TCHAR* ContextStr = *context;
	const TCHAR* ErrorMessageStr = *ErrorMessage;

	UE_LOG(LibNsgifHelper, Warning, TEXT("%s: %s"), ContextStr, ErrorMessageStr);
}

void URuntimeGIFLoaderHelper::Decode(FILE* ppm, const FString& name, nsgif_t* gif, bool first)
{
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

		Error = FLibnsgifHandler::FunctionPointerNsgifFramePrepare()(gif, &Area, &DelayCs, &FrameNew);
		if (Error != NSGIF_OK)
		{
			Warning(TEXT("nsgif_frame_prepare"), Error);
			return;
		}

		if (FrameNew < FramePrev)
		{
			// Must be an animation that loops. We only care about decoding each frame once in this utility.
			return;
		}
		FramePrev = FrameNew;

		Error = FLibnsgifHandler::FunctionPointerNsgifFrameDecode()(gif, FrameNew, &Bitmap);
		if (Error != NSGIF_OK)
		{
			FString ErrorString = FString::Printf(TEXT("Frame %u: nsgif_decode_frame failed: %s\n"), FrameNew, FLibnsgifHandler::FunctionPointerNsgifStrError()(Error));
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

void URuntimeGIFLoaderHelper::GIFDecoding(const FString& FilePath)
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

UTexture2D* URuntimeGIFLoaderHelper::ConvertGifToTexture2D(const FString& FilePath)
{
	// Create the gif animation
	Error = FLibnsgifHandler::FunctionPointerNsgifCreate()(&bitmap_callbacks, NSGIF_BITMAP_FMT_R8G8B8A8, &Gif);
	if (Error != NSGIF_OK)
	{
		UE_LOG(LogTemp, Error, TEXT("nsgif_create error: %d"), Error);
		return nullptr;
	}

	// Load file into memory
	Data = Load_File(TCHAR_TO_UTF8(*FilePath), Size);
	if (!Data)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load file: %s"), *FilePath);
		FLibnsgifHandler::FunctionPointerNsgifDestroy()(Gif);
		return nullptr;
	}
	
	// Scan the raw data
	Error = FLibnsgifHandler::FunctionPointerNsgifDataScan()(Gif, Size, Data);
	if (Error != NSGIF_OK)
	{
		// Not fatal; some GIFs are nasty. Can still try to decode
		// any frames that were decoded successfully.
		UE_LOG(LogTemp, Warning, TEXT("nsgif_data_scan warning: %d"), Error);
	}

	FLibnsgifHandler::FunctionPointerNsgifDataComplete()(Gif);

	// Decode the frames
	TArray<FColor> Pixels;
	Error = DecodeFrames(Gif, Pixels);
	if (Error != NSGIF_OK)
	{
		UE_LOG(LogTemp, Error, TEXT("DecodeFrames error: %d"), Error);
		FLibnsgifHandler::FunctionPointerNsgifDestroy()(Gif);
		free(Data);
		return nullptr;
	}

	// Create UTexture2D and initialize it with the pixel data
	int32 Width = FLibnsgifHandler::FunctionPointerNsgifGetInfo()(Gif)->width;
	int32 Height = FLibnsgifHandler::FunctionPointerNsgifGetInfo()(Gif)->height;
	UTexture2D* Texture = UTexture2D::CreateTransient(Width, Height, PF_R8G8B8A8);

	if(!Texture)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create UTexture2D"));
		FLibnsgifHandler::FunctionPointerNsgifDestroy()(Gif);
		free(Data);
		return nullptr;
	}

	// Lock the texture and copy the pixel data
	FTexture2DMipMap& Mip = Texture->PlatformData->Mips[0];
	void* TextureData = Mip.BulkData.Lock(LOCK_READ_WRITE);
	if (TextureData)
	{
		FMemory::Memcpy(TextureData, Pixels.GetData(), Pixels.Num() * sizeof(FColor));
		Mip.BulkData.Unlock();
		Texture->UpdateResource();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to lock UTexture2D for writing"));
		Texture->ConditionalBeginDestroy();
		FLibnsgifHandler::FunctionPointerNsgifDestroy()(Gif);
		free(Data);
		return nullptr;
	}

	// Clean up
	FLibnsgifHandler::FunctionPointerNsgifDestroy()(Gif);
	free(Data);

	return Texture;
}

nsgif_error URuntimeGIFLoaderHelper::DecodeFrames(nsgif_t* Gif_, TArray<FColor>& OutPixels)
{
	uint32_t FramePrev = 0;
	const nsgif_info_t* Info = FLibnsgifHandler::FunctionPointerNsgifGetInfo()(Gif_);

	// Prepare the output pixel array
	const uint32_t NumPixels = Info->width * Info->height * Info->frame_count;
	OutPixels.Empty(NumPixels);
	OutPixels.AddUninitialized(NumPixels);

	// Decode the frames
	nsgif_bitmap_t* Bitmap;
	uint32_t FrameNew;
	nsgif_rect_t Area;
	uint32_t Delay_CS;
	uint32_t FrameIndex = 0;
	while (true)
	{
		Error = FLibnsgifHandler::FunctionPointerNsgifFramePrepare()(Gif_, &Area, &Delay_CS, &FrameNew);
		if (Error != NSGIF_OK)
		{
			UE_LOG(LogTemp, Warning, TEXT("nsgif_frame_prepare warning: %d"), Error);
		}

		if (FrameNew < FramePrev)
		{
			// Must be an animation that loops.
			// We only care about decoding each frame once in this utility.
			break;
		}
		FramePrev = FrameNew;

		Error = FLibnsgifHandler::FunctionPointerNsgifFrameDecode()(Gif_, FrameNew, &Bitmap);
		if (Error != NSGIF_OK)
		{
			UE_LOG(LogTemp, Warning, TEXT("nsgif_frame_decode warning: %d"), Error);
		}

		// Copy the decoded frame to the output pixel array
		const uint8_t* Image = (const uint8_t*)Bitmap;
		const uint32_t FrameStartIndex = FrameIndex * Info->width * Info->height;
		for (uint32_t Y = 0; Y < Info->height; Y++)
		{
			for (uint32_t X = 0; X < Info->width; X++)
			{
				const size_t SrcIndex = (Y * Info->width + X) * 4;
				const size_t DstIndex = FrameStartIndex + (Y * Info->width + X);
				OutPixels[DstIndex] = FColor(Image[SrcIndex + 2], Image[SrcIndex + 1], Image[SrcIndex], Image[SrcIndex + 3]);
			}
		}

		FrameIndex++;

		if (Delay_CS == NSGIF_INFINITE)
		{
			// This frame is the last.
			break;
		}
	}

	return NSGIF_OK;
}

void URuntimeGIFLoaderHelper::LoadGif(const FString& FilePath, UTexture2D*& OutTexture, bool bUseAsync)
{
	// Synchronous loading
	if (!bUseAsync)
	{
		UTexture2D* Texture = ConvertGifToTexture2D(FilePath);
		OutTexture = Texture;
		return;
	}

	// Asynchronous loading
	FString AsyncFilePath = FilePath;
	FSimpleDelegateGraphTask::CreateAndDispatchWhenReady(
		FSimpleDelegateGraphTask::FDelegate::CreateLambda(
			[&, AsyncFilePath]()
			{
				UTexture2D* Texture = ConvertGifToTexture2D(AsyncFilePath);
				AsyncTask(ENamedThreads::GameThread, [Texture, &OutTexture]()
					{
						OutTexture = Texture;
					}
				);
			}),
		TStatId(), nullptr, ENamedThreads::AnyThread
	);
}

#endif //WITH_LIBNSGIF