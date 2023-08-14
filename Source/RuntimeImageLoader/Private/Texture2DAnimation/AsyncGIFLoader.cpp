// Copyright 2023 Peter Leontev and Muhammad Ahmed Saleem. All Rights Reserved.

#include "Texture2DAnimation/AsyncGIFLoader.h"
#include "Async/Async.h"
#include "RuntimeImageLoaderLog.h"

DEFINE_LOG_CATEGORY(GifTexture);

UAnimatedTexture2D* UAsyncGIFLoader::Init(const FString& GIFFilename)
{
	TUniquePtr<FRuntimeGIFLoaderHelper> Decoder = MakeUnique<FRuntimeGIFLoaderHelper>();

	if (Decoder)
	{
		CurrentTask = Async(
			EAsyncExecution::Thread,
			[GIFFilename, &Decoder]()
			{
				Decoder->GIFDecoding(TCHAR_TO_UTF8(*GIFFilename));
				
				return !Decoder->GetTextureData().IsEmpty();
			}
		);
		
		bool bResult = CurrentTask.Get();
		
		Width = Decoder->GetWidth();
		Height = Decoder->GetHeight();

		UAnimatedTexture2D* Texture = UAnimatedTexture2D::Create(Width, Height);
		Texture->SetDecoder(MoveTemp(Decoder));

		if (Texture)
		{
			Texture->SRGB = true;
			Texture->UpdateResource();
		}
		return Texture;
	}

	UE_LOG(GifTexture, Error, TEXT("Gif Texture isn't Initialized"));
	
	return nullptr;
}