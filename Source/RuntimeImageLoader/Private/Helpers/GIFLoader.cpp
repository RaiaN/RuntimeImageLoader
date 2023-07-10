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

uint8_t* URuntimeGIFLoaderHelper::Load_File(const char* path, size_t* data_size)
{
	FILE* fd;
	struct stat sb;
	unsigned char* buffer;
	size_t size;
	size_t n;

	fd = fopen(path, "rb");
	if (!fd) {
		perror(path);
		exit(EXIT_FAILURE);
	}

	if (stat(path, &sb)) {
		perror(path);
		exit(EXIT_FAILURE);
	}
	size = sb.st_size;

	buffer = (unsigned char*)malloc(size);
	if (!buffer) {
		fprintf(stderr, "Unable to allocate %lld bytes\n",
			(long long)size);
		exit(EXIT_FAILURE);
	}

	n = fread(buffer, 1, size, fd);
	if (n != size) {
		perror(path);
		exit(EXIT_FAILURE);
	}

	fclose(fd);

	*data_size = size;
	return buffer;
}

void URuntimeGIFLoaderHelper::Warning(const char* context, nsgif_error err)
{
	FString ErrorMessage = ANSI_TO_TCHAR(FLibnsgifHandler::FunctionPointerNsgifStrError()(err));
	const TCHAR* ErrorMessageStr = *ErrorMessage;

	UE_LOG(LibNsgifHelper, Warning, TEXT("%s: %s\n"), context, ErrorMessageStr);
}

void URuntimeGIFLoaderHelper::Decode(FILE* ppm, const char* name, nsgif_t* gif, bool first)
{
	nsgif_error err;
	uint32_t frame_prev = 0;
	const nsgif_info_t* info;
	info = FLibnsgifHandler::FunctionPointerNsgifGetInfo()(gif);

	if (ppm != NULL) {
		fprintf(ppm, "P3\n");
		fprintf(ppm, "# %s\n", name);
		fprintf(ppm, "# width                %u \n", info->width);
		fprintf(ppm, "# height               %u \n", info->height);
		fprintf(ppm, "# frame_count          %u \n", info->frame_count);
		fprintf(ppm, "# loop_max             %u \n", info->loop_max);
		fprintf(ppm, "%u %u 256\n", info->width, info->height * info->frame_count);
		fprintf(ppm, "*");
	}

	/* decode the frames */
	while (true) {
		nsgif_bitmap_t* bitmap;
		const uint8_t* image;
		uint32_t frame_new;
		uint32_t delay_cs;
		nsgif_rect_t area;

		err = FLibnsgifHandler::FunctionPointerNsgifFramePrepare()(gif, &area, &delay_cs, &frame_new);
		if (err != NSGIF_OK) {
			Warning("nsgif_frame_prepare", err);
			return;
		}

		if (frame_new < frame_prev) {
			/* Must be an animation that loops. We only care about
			* decoding each frame once in this utility. */
			return;
		}
		frame_prev = frame_new;

		err = FLibnsgifHandler::FunctionPointerNsgifFrameDecode()(gif, frame_new, &bitmap);
		if (err != NSGIF_OK) {
			/* Continue decoding the rest of the frames. */

		}
		else if (ppm != NULL) {
			/*fprintf(ppm, "# frame %u:\n", frame_new);*/
			image = (const uint8_t*)bitmap;
			for (uint32_t y = 0; y != info->height; y++) {
				for (uint32_t x = 0; x != info->width; x++) {
					size_t z = (y * info->width + x) * 4;
					fprintf(ppm, "%u %u %u ",
						image[z],
						image[z + 1],
						image[z + 2]);
				}
				fprintf(ppm, "\n");
			}
		}

		if (delay_cs == NSGIF_INFINITE) {
			/** This frame is the last. */
			return;
		}
	}
}

void URuntimeGIFLoaderHelper::GIFDecoding(const char* FilePath)
{
	fopen_s(&PortablePixMap, TCHAR_TO_UTF8(*ppmFile), "w+");
	if (!PortablePixMap) return;

	/* create our gif animation */
	Error = FLibnsgifHandler::FunctionPointerNsgifCreate()(&bitmap_callbacks, NSGIF_BITMAP_FMT_R8G8B8A8, &Gif);
	if (Error != NSGIF_OK) {
		Warning("nsgif_create", Error);
		return;
	}

	/* load file into memory */
	Data = Load_File(FilePath, &Size);

	Error = FLibnsgifHandler::FunctionPointerNsgifDataScan()(Gif, Size, Data);
	if (Error != NSGIF_OK)
	{
		/* Not fatal; some GIFs are nasty. Can still try to decode
		 * any frames that were decoded successfully. */
		Warning("nsgif_data_scan", Error);
	}

	FLibnsgifHandler::FunctionPointerNsgifDataComplete()(Gif);

	auto LoopMax = FLibnsgifHandler::FunctionPointerNsgifGetInfo()(Gif)->loop_max;
	if (FLibnsgifHandler::FunctionPointerNsgifGetInfo()(Gif)->loop_max == 0)
		LoopMax = 1;

	for (uint64 i = 0; i < LoopMax; i++)
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

UTexture2D* URuntimeGIFLoaderHelper::ConvertPPMToTexture2D()
{
	const FString& FilePath = ppmFile;
	// Load PPM file into memory
	TArray<uint8> ImageData;
	if (!FFileHelper::LoadFileToArray(ImageData, *FilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load PPM file: %s"), *FilePath);
		return nullptr;
	}

	// Parse PPM header to extract image dimensions and pixel data
	int32 Width = 0;
	int32 Height = 0;
	int32 MaxValue = 0;
	TArray<uint8> Pixels;

	FString ImageDataAsString(reinterpret_cast<const ANSICHAR*>(ImageData.GetData()));
	TArray<FString> Lines;
	ImageDataAsString.ParseIntoArrayLines(Lines);

	// Sherading an image array to Remove Header
	int32 RemoveIndex = ImageData.Find('*');
	if (RemoveIndex != INDEX_NONE)
	{
		ImageData.RemoveAt(0, RemoveIndex + 1);
	}

	// Validating PPM File
	if (Lines.Num() < 3)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid PPM file format: %s"), *FilePath);
		return nullptr;
	}

	// Check if it's a valid PPM file
	if (Lines[0] != "P3")
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid PPM file format: %s"), *FilePath);
		return nullptr;
	}

	// Extracting Height &  Width Of The Frame
	Width = ExtractDimensions(Lines[2]);
	Height = ExtractDimensions(Lines[3]);

	if (Width == -1 && Height == -1)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid PPM file format: %s"), *FilePath);
		return nullptr;
	}

	// Copy pixel data from PPM image
	int32 PixelIndex = 0; 
	for (int32 i = 0; i < ImageData.Num(); ++i)
	{
		FString PixelValueString;
		while (i < ImageData.Num() && ImageData[i] != ' ' && ImageData[i] != '\n')
		{
			PixelValueString.AppendChar(ImageData[i]);
			++i;
		}

		uint8 PixelValue = FCString::Atoi(*PixelValueString);
		
		Pixels.Add(PixelValue);
	}

	// Create UTexture2D and initialize it with the pixel data
	UTexture2D* Texture = UTexture2D::CreateTransient(Width, Height, PF_R8G8B8A8);

	// Lock the texture and copy the pixel data
	uint8* TextureData = static_cast<uint8*>(Texture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE));
	FMemory::Memcpy(TextureData, Pixels.GetData(), Width * Height * 4);

	// Unlock the texture and update it
	Texture->PlatformData->Mips[0].BulkData.Unlock();
	Texture->UpdateResource();

	return Texture;
}

int32 URuntimeGIFLoaderHelper::ExtractDimensions(FString PPMDimension)
{
	TArray<FString> Substrings;
	PPMDimension.ParseIntoArrayWS(Substrings); // Split the string by whitespace

	for (const FString& Substring : Substrings)
	{
		int32 Number = FCString::Atoi(*Substring);
		if (Number != 0)
		{
			return Number;
		}
	}

	return -1;
}

#endif //WITH_LIBNSGIF