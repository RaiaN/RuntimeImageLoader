
#include "Texture2DAnimation/RenderGIFTexture.h"
#include "Helpers/GifLoader.h"
#include "Engine/Texture2D.h"
#include "Engine/Texture2DDynamic.h"

#if !UE_SERVER

static void WriteRawToTexture_RenderThread(FTexture2DDynamicResource* TextureResource, const FColor* RawData, bool bUseSRGB = true)
{
	check(IsInRenderingThread());

	if (TextureResource)
	{
		FRHITexture2D* TextureRHI = TextureResource->GetTexture2DRHI();

		int32 Width = TextureRHI->GetSizeX();
		int32 Height = TextureRHI->GetSizeY();

		uint32 DestStride = 0;
		uint8* DestData = reinterpret_cast<uint8*>(RHILockTexture2D(TextureRHI, 0, RLM_WriteOnly, DestStride, false, false));

		for (int32 y = 0; y < Height; y++)
		{
			uint8* DestPtr = &DestData[((int64)Height - 1 - y) * DestStride];

			const FColor* SrcPtr = &((FColor*)(RawData))[((int64)Height - 1 - y) * Width];
			for (int32 x = 0; x < Width; x++)
			{
				*DestPtr++ = SrcPtr->B;
				*DestPtr++ = SrcPtr->G;
				*DestPtr++ = SrcPtr->R;
				*DestPtr++ = SrcPtr->A;
				SrcPtr++;
			}
		}

		RHIUnlockTexture2D(TextureRHI, 0, false, false);
	}
}

#endif

UTexture2DDynamic* URenderGIFTexture::HandleGIFRequest(const FString& GIFFilename)
{
	TSharedPtr<FRuntimeGIFLoaderHelper, ESPMode::ThreadSafe> Decoder = 
		MakeShared<FRuntimeGIFLoaderHelper, ESPMode::ThreadSafe>();

	if (Decoder)
	{
		Decoder->GIFDecoding(TCHAR_TO_UTF8(*GIFFilename));

		const FColor* RawData = Decoder->GetFrameBuffer();
		UTexture2DDynamic* Texture = UTexture2DDynamic::Create(Decoder->GetWidth(), Decoder->GetHeight());
		if (Texture)
		{
			Texture->SRGB = true;
			Texture->UpdateResource();

			FTexture2DDynamicResource* TextureResource = static_cast<FTexture2DDynamicResource*>(Texture->Resource);
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

						RHIUpdateTexture2D(Texture2DRHI, 0, Region, SrcPitch, (const uint8*)RawData);

						WriteRawToTexture_RenderThread(TextureResource, RawData);
					});
			}
		}
		return Texture;
	}
	return nullptr;
}
