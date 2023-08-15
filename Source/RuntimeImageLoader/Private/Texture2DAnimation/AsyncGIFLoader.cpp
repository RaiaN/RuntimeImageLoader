// Copyright 2023 Peter Leontev. All Rights Reserved.

#include "Texture2DAnimation/AsyncGIFLoader.h"
#include "Async/Async.h"
#include "RuntimeImageLoaderLog.h"

DEFINE_LOG_CATEGORY(GifLoaderAsyncTask);

void UAsyncGIFLoader::Init(const FString& InGifFilename)
{
	Decoder = MakeUnique<FRuntimeGIFLoaderHelper>();
	check (Decoder.IsValid());

	CurrentTask = Async(
		EAsyncExecution::Thread,
		[this, InGifFilename]()
		{
			bool bRes = Decoder->DecodeGIF(InGifFilename);

			OnPostGifDecode(bRes);
		}
	);
}

void UAsyncGIFLoader::OnPostGifDecode(bool bRes)
{
	Async(
		EAsyncExecution::TaskGraphMainThread,
		[bRes, this]()
		{
			UAnimatedTexture2D* Texture = UAnimatedTexture2D::Create(Decoder->GetWidth(), Decoder->GetHeight());
			if (!bRes || !IsValid(Texture))
			{
				UE_LOG(GifLoaderAsyncTask, Error, TEXT("Gif Decoder Unable to Decode Gif"));
				OnGifLoaded.Execute(Texture, Decoder->GetDecodeError());
			}

			Texture->SetDecoder(MoveTemp(Decoder));

			Texture->SRGB = true;
			Texture->UpdateResource();

			OnGifLoaded.Execute(Texture, TEXT(""));
		}
	).Wait();

	SetReadyToDestroy();
}

void UAsyncGIFLoader::Cancel()
{
	CurrentTask.Reset();
}
