// Copyright 2023 Unreal Solutions Ltd. All Rights Reserved.

#include "RuntimeGifReader.h"
#include "Async/Async.h"
#include "Texture2DAnimation/AnimatedTexture2D.h"
#include "ImageReaders/ImageReaderFactory.h"
#include "ImageReaders/IImageReader.h"
#include "RuntimeImageLoaderLog.h"

DEFINE_LOG_CATEGORY(RuntimeGifReader);

#include UE_INLINE_GENERATED_CPP_BY_NAME(RuntimeGifReader)


URuntimeGifReader* URuntimeGifReader::LoadGIFAsync(const FString& GIFFilename)
{
	FGifReadRequest Request;
	{
		Request.InputGif.ImageFilename = GIFFilename;
	}

	URuntimeGifReader* GifReader = NewObject<URuntimeGifReader>();
	GifReader->SubmitRequest(MoveTemp(Request), false);

	return GifReader;
}

URuntimeGifReader* URuntimeGifReader::LoadGIFSync(const FString& GIFFilename)
{
	FGifReadRequest Request;
	{
		Request.InputGif.ImageFilename = GIFFilename;
	}

	URuntimeGifReader* GifReader = NewObject<URuntimeGifReader>();
	GifReader->SubmitRequest(MoveTemp(Request), true);

	return GifReader;
}

URuntimeGifReader* URuntimeGifReader::LoadGIFFromBytesAsync(TArray<uint8>& GifBytes)
{
	FGifReadRequest Request;
	{
		Request.InputGif.ImageBytes = MoveTemp(GifBytes);
	}

	URuntimeGifReader* GifReader = NewObject<URuntimeGifReader>();
	GifReader->SubmitRequest(MoveTemp(Request), false);

	return GifReader;
}


URuntimeGifReader* URuntimeGifReader::LoadGIFFromBytesSync(TArray<uint8>& GifBytes)
{
	FGifReadRequest Request;
	{
		Request.InputGif.ImageBytes = MoveTemp(GifBytes);
	}

	URuntimeGifReader* GifReader = NewObject<URuntimeGifReader>();
	GifReader->SubmitRequest(MoveTemp(Request), true);

	return GifReader;
}

// ------------------------------------------------------

void URuntimeGifReader::SubmitRequest(FGifReadRequest&& InRequest, bool bSynchronous)
{
	Request = MoveTemp(InRequest);
	
	CurrentTask = Async(
		EAsyncExecution::ThreadPool,
		[this]()
		{
			ProcessRequest();

			AsyncTask(
				ENamedThreads::GameThread, [this]()
				{
					if (ReadResult.OutError.IsEmpty())
					{
						OnSuccess.Broadcast(ReadResult.OutTexture);
					}
					else
					{
						OnFail.Broadcast(ReadResult.OutError);
					}
				}
			);
		}
	);

	if (bSynchronous)
	{
		CurrentTask.Wait();
	}
}

void URuntimeGifReader::ProcessRequest()
{
	Decoder = MakeUnique<FRuntimeGIFLoaderHelper>();
	check(Decoder.IsValid());

	TArray<uint8> ImageBuffer;

	// read image data from using URI
	// if not then read from bytes

	const FString& GifFilename = Request.InputGif.ImageFilename;

	if (GifFilename.Len() > 0)
	{
		ImageReader = FImageReaderFactory::CreateReader(GifFilename);
		{
			ImageBuffer = ImageReader->ReadImage(GifFilename);
			if (ImageBuffer.Num() == 0)
			{
				ReadResult.OutError = FString::Printf(TEXT("Failed to read GIF: %s. Error: %s"), *GifFilename, *ImageReader->GetLastError());
				return;
			}

		}

		ImageReader = nullptr;
	}
	else if (Request.InputGif.ImageBytes.Num() > 0)
	{
		ImageBuffer = MoveTemp(Request.InputGif.ImageBytes);
	}

	check (ImageBuffer.Num() > 0);

	bool bResult = Decoder->DecodeGIF(MoveTemp(ImageBuffer));
	if (!bResult)
	{
		ReadResult.OutError = FString::Printf(TEXT("Error: Failed to decode GIF: %s"), *Decoder->GetDecodeError());
		return;
	}

	UAnimatedTexture2D* Texture = UAnimatedTexture2D::Create(Decoder->GetWidth(), Decoder->GetHeight());
	if (!IsValid(Texture))
	{
		ReadResult.OutError = FString::Printf(TEXT("Error: Failed to Create Animated Texture Gif."));
		UE_LOG(RuntimeGifReader, Error, TEXT("Error: Failed to Create Animated Texture Gif. Please check logs for any decoding related errors"));
		return;
	}

	Texture->SetDecoder(MoveTemp(Decoder));

	Texture->SRGB = true;
	Texture->UpdateResource();

	ReadResult.OutTexture = Texture;
}
