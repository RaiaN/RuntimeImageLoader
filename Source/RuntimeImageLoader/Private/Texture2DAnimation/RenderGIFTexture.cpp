// Copyright 2023 Peter Leontev and Muhammad Ahmed Saleem. All Rights Reserved.

#include "Texture2DAnimation/RenderGIFTexture.h"
#include "Texture2DAnimation/AnimatedTexture2D.h"
#include "Texture2DAnimation/AnimatedTextureResource.h"
#include "Helpers/GifLoader.h"
#include "Engine/Texture2D.h"
#include "Engine/Texture2DDynamic.h"

UAnimatedTexture2D* URenderGIFTexture::RenderGIFData(const FString& GIFFilename, int32 Current_Frame)
{
	TSharedPtr<FRuntimeGIFLoaderHelper, ESPMode::ThreadSafe> Decoder = 
		MakeShared<FRuntimeGIFLoaderHelper, ESPMode::ThreadSafe>();

	if (Decoder)
	{
		Decoder->GIFDecoding(TCHAR_TO_UTF8(*GIFFilename));
		
		TArray<FColor> NextFramePixels;
		int32 Width = Decoder->GetWidth();
		int32 Height = Decoder->GetHeight();
		int32 TotalFrames = Decoder->GetTotalFrames();

		if (Current_Frame >= TotalFrames) Current_Frame = 0;

		Decoder->GetNextFrame(NextFramePixels, Current_Frame, Width, Height, TotalFrames);
 
		const uint8* RawData = (const uint8*)NextFramePixels.GetData();

		UAnimatedTexture2D* Texture = UAnimatedTexture2D::Create(Width, Height);
		Texture->SetDecoder(Decoder);

		if (Texture)
		{
			Texture->SRGB = true;
			Texture->UpdateResource();

			FAnimatedTextureResource* TextureResource = static_cast<FAnimatedTextureResource*>(Texture->Resource);
			if (TextureResource)
			{
				ENQUEUE_RENDER_COMMAND(FWriteRawDataToTexture)(
					[TextureResource, RawData](FRHICommandListImmediate& RHICmdList)
					{
						FTexture2DRHIRef Texture2DRHI = TextureResource->TextureRHI->GetTexture2D();
						if (!Texture2DRHI)
							return;

						uint32 TexWidth = Texture2DRHI->GetSizeX();
						uint32 TexHeight = Texture2DRHI->GetSizeY();
						uint32 SrcPitch = TexWidth * sizeof(FColor);

						FUpdateTextureRegion2D Region;
						Region.SrcX = Region.SrcY = Region.DestX = Region.DestY = 0;
						Region.Width = TexWidth;
						Region.Height = TexHeight;

						RHIUpdateTexture2D(Texture2DRHI, 0, Region, SrcPitch, RawData);
					}
				);
			}
		}
		return Texture;
	}
	return nullptr;
}
