// Copyright 2023 Peter Leontev. All Rights Reserved.

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
	FScopeLock ResultsLock(&ResultsMutex);

	// create RHI resource object
	FTextureResource* NewResource = new FAnimatedTextureResource(this);
	return NewResource;
}

void UAnimatedTexture2D::Tick(float DeltaTime)
{
	if (!bPlaying)
		return;

	if (!Decoder) return;

	FrameTime += DeltaTime * PlayRate;
	if (FrameTime < DefaultFrameDelay)
		return;

	FrameTime = 0;

	RenderFrameToTexture();

	CurrentFrame++;
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
	FScopeLock ResultsLock(&ResultsMutex);

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

void UAnimatedTexture2D::RenderFrameToTexture()
{
	FScopeLock ResultsLock(&ResultsMutex);

	if (CurrentFrame > Decoder->GetTotalFrames() - 1 && bLooping) CurrentFrame = 0;

	FRenderCommandData CommandData;

	CommandData.RHIResource = GetResource();
	CommandData.RawData = (const uint8*)Decoder->GetNextFrame(CurrentFrame);

	/** @See AsyncTaskDownloadImage Class, How to Pass Texture Content Data To Render QUEUE at Runtime */
	ENQUEUE_RENDER_COMMAND(AnimTexture2D_RenderFrame)(
		[CommandData](FRHICommandListImmediate& RHICmdList)
		{
			if (!CommandData.RHIResource || !CommandData.RHIResource->TextureRHI)
			return;

			FTexture2DRHIRef Texture2DRHI = CommandData.RHIResource->TextureRHI->GetTexture2D();
			if (!Texture2DRHI)
				return;

			uint32 TexWidth = Texture2DRHI->GetSizeX();
			uint32 TexHeight = Texture2DRHI->GetSizeY();
			uint32 SrcPitch = TexWidth * sizeof(FColor);

			FUpdateTextureRegion2D Region;
			Region.SrcX = Region.SrcY = Region.DestX = Region.DestY = 0;
			Region.Width = TexWidth;
			Region.Height = TexHeight;

			RHIUpdateTexture2D(Texture2DRHI, 0, Region, SrcPitch, CommandData.RawData);
		}
	);
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
	CurrentFrame = 0;
}

void UAnimatedTexture2D::Stop()
{
	bPlaying = false;
}