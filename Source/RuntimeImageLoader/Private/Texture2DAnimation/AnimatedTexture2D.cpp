// Copyright 2023 Peter Leontev and Muhammad Ahmed Saleem. All Rights Reserved.

#include "Texture2DAnimation/AnimatedTexture2D.h"
#include "AnimatedTextureResource.h"

float UAnimatedTexture2D::GetSurfaceWidth() const
{
	if (Decoder) return Decoder->GetWidth();
	return 1.0f;
}

float UAnimatedTexture2D::GetSurfaceHeight() const
{
	if (Decoder) return Decoder->GetHeight();
	return 1.0f;
}

FTextureResource* UAnimatedTexture2D::CreateResource()
{
	// create RHI resource object
	FTextureResource* NewResource = new FAnimatedTextureResource(this);
	return NewResource;
}

void UAnimatedTexture2D::Tick(float DeltaTime)
{
	if (!Decoder) return;

	FrameTime += DeltaTime * PlayRate;
	if (FrameTime < DefaultFrameDelay)
		return;

	FrameTime = 0;

	if (CurrentFrame > Decoder->GetTotalFrames() - 1) CurrentFrame = 0;

	Decoder->GetNextFrame(NextFramePixels, CurrentFrame);

	const uint8* RawData = (const uint8*)NextFramePixels.GetData();
	/** @See AsyncTaskDownloadImage Class, How to Pass Texture Content Data To Render QUEUE at Runtime */
	FAnimatedTextureResource* TextureResource = static_cast<FAnimatedTextureResource*>(GetResource());
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

	if(bLooping) CurrentFrame++;
}

UAnimatedTexture2D* UAnimatedTexture2D::Create(int32 InSizeX, int32 InSizeY, const FAnimatedTexture2DCreateInfo& InCreateInfo)
{
	EPixelFormat DesiredFormat = EPixelFormat(InCreateInfo.Format);
	if (InSizeX > 0 && InSizeY > 0)
	{

		auto NewTexture = NewObject<UAnimatedTexture2D>(GetTransientPackage(), NAME_None, RF_Transient);
		if (NewTexture != NULL)
		{
			NewTexture->Filter = InCreateInfo.Filter;
			NewTexture->SamplerAddressMode = InCreateInfo.SamplerAddressMode;
			NewTexture->SRGB = InCreateInfo.bSRGB;

			// Disable compression
			NewTexture->CompressionSettings = TC_Default;
#if WITH_EDITORONLY_DATA
			NewTexture->CompressionNone = true;
			NewTexture->MipGenSettings = TMGS_NoMipmaps;
			NewTexture->CompressionNoAlpha = true;
			NewTexture->DeferCompression = false;
#endif // #if WITH_EDITORONLY_DATA
			if (InCreateInfo.bIsResolveTarget)
			{
				NewTexture->bNoTiling = false;
			}
			else
			{
				// Untiled format
				NewTexture->bNoTiling = true;
			}

			NewTexture->Init(InSizeX, InSizeY, DesiredFormat, InCreateInfo.bIsResolveTarget);
		}
		return NewTexture;
	}
	else
	{
		UE_LOG(LogTexture, Warning, TEXT("Invalid parameters specified for UTexture2DDynamic::Create()"));
		return NULL;
	}
}

void UAnimatedTexture2D::Init(int32 InSizeX, int32 InSizeY, EPixelFormat InFormat/*=2*/, bool InIsResolveTarget/*=false*/)
{
	SizeX = InSizeX;
	SizeY = InSizeY;
	Format = (EPixelFormat)InFormat;
	NumMips = 1;
	bIsResolveTarget = InIsResolveTarget;

	// Initialize the resource.
	UpdateResource();
}

void UAnimatedTexture2D::SetDecoder(TUniquePtr<FRuntimeGIFLoaderHelper> DecoderState)
{
	Decoder = MoveTemp(DecoderState);
}

void UAnimatedTexture2D::Play()
{
	bPlaying = true;
}

void UAnimatedTexture2D::PlayFromStart()
{
	FrameTime = 0;
	FrameDelay = 0;
	bPlaying = true;
	/*if (Decoder) Decoder->Reset();*/
}

void UAnimatedTexture2D::Stop()
{
	bPlaying = false;
}