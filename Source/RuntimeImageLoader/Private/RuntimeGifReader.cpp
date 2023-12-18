// Copyright 2023 Petr Leontev. All Rights Reserved.

#include "RuntimeGifReader.h"
#include "Async/Async.h"
#include "Texture2DAnimation/AnimatedTexture2D.h"
#include "ImageReaders/ImageReaderFactory.h"
#include "ImageReaders/IImageReader.h"
#include "RuntimeImageLoaderLog.h"

DEFINE_LOG_CATEGORY(RuntimeGifReader);


URuntimeGifReader* URuntimeGifReader::LoadGIF(const FString& GIFFilename, TEnumAsByte<enum TextureFilter> InFilterMode, bool bSynchronous)
{
	FGifReadRequest Request;
	{
		Request.InputGif.ImageFilename = GIFFilename;
		Request.FilterMode = InFilterMode;
	}

	URuntimeGifReader* GifReader = NewObject<URuntimeGifReader>();
	GifReader->SubmitRequest(MoveTemp(Request), bSynchronous);

	return GifReader;
}

URuntimeGifReader* URuntimeGifReader::LoadGIFFromBytes(TArray<uint8>& GifBytes, TEnumAsByte<enum TextureFilter> InFilterMode, bool bSynchronous)
{
	FGifReadRequest Request;
	{
		Request.InputGif.ImageBytes = MoveTemp(GifBytes);
		Request.FilterMode = InFilterMode;
	}

	URuntimeGifReader* GifReader = NewObject<URuntimeGifReader>();
	GifReader->SubmitRequest(MoveTemp(Request), bSynchronous);

	return GifReader;
}

// ------------------------------------------------------

void URuntimeGifReader::SubmitRequest(FGifReadRequest&& InRequest, bool bSynchronous)
{
	Request = MoveTemp(InRequest);

	bool bIsCallerGameThread = IsInGameThread();
	if (bIsCallerGameThread && bSynchronous)
	{
		ProcessRequest();
		OnPostProcessRequest();

		return;
	}
	
	CurrentTask = Async(
		EAsyncExecution::ThreadPool,
		[this, bIsCallerGameThread]()
		{
			ProcessRequest();
			OnPostProcessRequest();
		}
	);

	if (bSynchronous)
	{
		CurrentTask.Wait();
	}
}

void URuntimeGifReader::ProcessRequest()
{
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

	Decoder = FGIFLoaderFactory::CreateLoader(GifFilename, ImageBuffer);
	check(Decoder.IsValid());

	bool bResult = Decoder->DecodeGIF(MoveTemp(ImageBuffer));
	if (!bResult)
	{
		ReadResult.OutError = FString::Printf(TEXT("Error: Failed to decode GIF: %s"), *Decoder->GetDecodeError());
		return;
	}

	FAnimatedTexture2DCreateInfo CreateInfo;
	CreateInfo.Filter = Request.FilterMode;

	ReadResult.OutTexture = UAnimatedTexture2D::Create(Decoder->GetWidth(), Decoder->GetHeight(), CreateInfo);
	if (!IsValid(ReadResult.OutTexture))
	{
		ReadResult.OutError = FString::Printf(TEXT("Error: Failed to Create Animated Texture Gif."));
		UE_LOG(RuntimeGifReader, Error, TEXT("Error: Failed to Create Animated Texture Gif. Please check logs for any decoding related errors"));
		return;
	}

	ReadResult.OutTexture->SetDecoder(MoveTemp(Decoder));

	ReadResult.OutTexture->SRGB = true;
	ReadResult.OutTexture->UpdateResource();
}

void URuntimeGifReader::OnPostProcessRequest()
{
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
