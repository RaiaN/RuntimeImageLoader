// Copyright 2023 Peter Leontev and Muhammad Ahmed Saleem. All Rights Reserved.

#include "Texture2DAnimation/GIFTexture.h"
#include "Texture2DAnimation/AnimatedTexture2D.h"
#include "RuntimeImageLoaderLog.h"

DEFINE_LOG_CATEGORY(GifTexture);

UAnimatedTexture2D* UGIFTexture::Init(const FString& GIFFilename)
{
	TSharedPtr<FRuntimeGIFLoaderHelper, ESPMode::ThreadSafe> Decoder = MakeShared<FRuntimeGIFLoaderHelper, ESPMode::ThreadSafe>();

	if (Decoder)
	{
		Decoder->GIFDecoding(TCHAR_TO_UTF8(*GIFFilename));
		
		Width = Decoder->GetWidth();
		Height = Decoder->GetHeight();

		UAnimatedTexture2D* Texture = UAnimatedTexture2D::Create(Width, Height);
		Texture->SetDecoder(Decoder);

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