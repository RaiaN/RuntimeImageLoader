// Copyright 2023 Peter Leontev. All Rights Reserved.

#include "Texture2DAnimation/AsyncGIFLoader.h"
#include "Async/Async.h"
#include "RuntimeImageLoaderLog.h"

DEFINE_LOG_CATEGORY(GifLoaderAsyncTask);

void UAsyncGIFLoader::Init(const FString& InGifFilename)
{
	GifFilename = InGifFilename;

	Decoder = MakeUnique<FRuntimeGIFLoaderHelper>();
	check (Decoder.IsValid());

	OnGifDecoded.BindUObject(this, &UAsyncGIFLoader::OnGifDecodedHandler);
	
	Activate();
}


void UAsyncGIFLoader::Activate()
{
    Super::Activate();

	CurrentTask = Async(
		EAsyncExecution::Thread,
		[this]()
		{
			bool bRes = Decoder->DecodeGIF(GifFilename);

			OnGifDecoded.Execute(bRes);
		}
	);
}


void UAsyncGIFLoader::OnGifDecodedHandler(bool bRes)
{
	TFuture<UAnimatedTexture2D*> Texture = Async(
		EAsyncExecution::TaskGraphMainThread,
		[bRes, this]()
		{
			if (!bRes)
			{
				return (UAnimatedTexture2D*)nullptr;
			}

			UAnimatedTexture2D* Texture = UAnimatedTexture2D::Create(Decoder->GetWidth(), Decoder->GetHeight());
			if (!IsValid(Texture))
			{
				UE_LOG(GifLoaderAsyncTask, Error, TEXT("Gif Decoder Unable to Decode Gif"));
				return (UAnimatedTexture2D*)nullptr;
			}

			Texture->SetDecoder(MoveTemp(Decoder));

			Texture->SRGB = true;
			Texture->UpdateResource();

			return Texture;
		}
	);

	OnGifLoaded.Execute(Texture.Get(), Decoder->GetDecodeError());

	SetReadyToDestroy();
}

void UAsyncGIFLoader::Cancel()
{
	CurrentTask.Reset();
}
