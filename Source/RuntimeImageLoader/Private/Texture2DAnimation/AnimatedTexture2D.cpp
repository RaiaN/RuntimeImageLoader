// Copyright 2023 Petr Leontev. All Rights Reserved.

#include "Texture2DAnimation/AnimatedTexture2D.h"
#include "RenderingThread.h"
#include "RHICommandList.h"
#include "RHI.h"
#include "AnimatedTextureResource.h"

DEFINE_LOG_CATEGORY_STATIC(LogAnimatedTexture, Log, All);

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

	UE_LOG(LogAnimatedTexture, Log, TEXT("CreateResource: Creating FAnimatedTextureResource. Decoder valid=%d, Size=%dx%d"), 
		Decoder.IsValid(), SizeX, SizeY);

	// create RHI resource object
	FTextureResource* NewResource = new FAnimatedTextureResource(this);
	return NewResource;
}

void UAnimatedTexture2D::Tick(float DeltaTime)
{
	// DEBUG: Uncomment to verify Tick is being called
	// UE_LOG(LogAnimatedTexture, Verbose, TEXT("Tick called - bPlaying=%d, Decoder=%d"), bPlaying, Decoder.IsValid());
	
	if (!bPlaying)
	{
		return;
	}

	if (!Decoder)
	{
		UE_LOG(LogAnimatedTexture, Warning, TEXT("Tick: Decoder is null but bPlaying is true!"));
		return;
	}

	const int32 TotalFrames = Decoder->GetTotalFrames();
	bool bIsOutsideBounds = (CurrentFrame < 0) || (CurrentFrame >= TotalFrames - 1);
	
	UE_LOG(LogAnimatedTexture, Verbose, TEXT("Tick: CurrentFrame=%d, TotalFrames=%d, bIsOutsideBounds=%d, bLooping=%d"), 
		CurrentFrame, TotalFrames, bIsOutsideBounds, bLooping);
	
	if (bIsOutsideBounds && bLooping)
	{
		CurrentFrame = 0;
		UE_LOG(LogAnimatedTexture, Log, TEXT("Tick: Looping back to frame 0"));
	}

	FrameTime += DeltaTime * PlayRate;
	const float FrameDelaySec = Decoder->GetNextFrameDelay(CurrentFrame);
	
	if (FrameTime < FrameDelaySec)
	{
		return;
	}

	UE_LOG(LogAnimatedTexture, Verbose, TEXT("Tick: Advancing frame. FrameTime=%.4f, FrameDelay=%.4f, CurrentFrame=%d -> %d"), 
		FrameTime, FrameDelaySec, CurrentFrame, CurrentFrame + 1);

	FrameTime = 0;

	RenderFrameToTexture();

	CurrentFrame++;
}

UAnimatedTexture2D* UAnimatedTexture2D::Create(int32 InSizeX, int32 InSizeY, const FAnimatedTexture2DCreateInfo& InCreateInfo)
{
	EPixelFormat DesiredFormat = EPixelFormat(InCreateInfo.Format);
	if (InSizeX > 0 && InSizeY > 0)
	{
		UAnimatedTexture2D* NewTexture = NewObject<UAnimatedTexture2D>(
			(UObject*)GetTransientPackage(),
			MakeUniqueObjectName((UObject*)GetTransientPackage(), UAnimatedTexture2D::StaticClass()),
			RF_Public | RF_Transient
		);

		check(IsValid(NewTexture));
		{
			NewTexture->Filter = InCreateInfo.Filter;
			NewTexture->SamplerAddressMode = InCreateInfo.SamplerAddressMode;
			NewTexture->SRGB = InCreateInfo.bSRGB;

			// Disable compression
			NewTexture->CompressionSettings = TC_Default;
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
		UE_LOG(LogAnimatedTexture, Warning, TEXT("Invalid parameters specified for UTexture2DDynamic::Create()"));
		return NULL;
	}
}

void UAnimatedTexture2D::SetDecoder(TUniquePtr<IGIFLoader> DecoderState)
{
	Decoder = MoveTemp(DecoderState);
	
	if (Decoder.IsValid())
	{
		UE_LOG(LogAnimatedTexture, Log, TEXT("SetDecoder: Decoder set successfully. TotalFrames=%d, Size=%dx%d"), 
			Decoder->GetTotalFrames(), Decoder->GetWidth(), Decoder->GetHeight());
	}
	else
	{
		UE_LOG(LogAnimatedTexture, Error, TEXT("SetDecoder: Decoder is invalid after move!"));
	}
}

void UAnimatedTexture2D::RenderFrameToTexture()
{
	FScopeLock ResultsLock(&ResultsMutex);

	if (CurrentFrame > Decoder->GetTotalFrames() - 1 && bLooping) CurrentFrame = 0;

	FRenderCommandData CommandData;

	CommandData.RHIResource = GetResource();
	CommandData.RawData = (const uint8*)Decoder->GetNextFrame(CurrentFrame);

	UE_LOG(LogAnimatedTexture, Verbose, TEXT("RenderFrameToTexture: Frame=%d, RHIResource=%p, RawData=%p"), 
		CurrentFrame, CommandData.RHIResource, CommandData.RawData);

	if (!CommandData.RHIResource)
	{
		UE_LOG(LogAnimatedTexture, Error, TEXT("RenderFrameToTexture: RHIResource is null!"));
		return;
	}

	if (!CommandData.RawData)
	{
		UE_LOG(LogAnimatedTexture, Error, TEXT("RenderFrameToTexture: RawData is null for frame %d!"), CurrentFrame);
		return;
	}

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
	UE_LOG(LogAnimatedTexture, Log, TEXT("Play: Started playback. Decoder valid=%d, CurrentFrame=%d"), 
		Decoder.IsValid(), CurrentFrame);
}

void UAnimatedTexture2D::PlayFromStart()
{
	FrameTime = 0;
	FrameDelay = 0;
	bPlaying = true;
	CurrentFrame = 0;
	UE_LOG(LogAnimatedTexture, Log, TEXT("PlayFromStart: Started playback from beginning. Decoder valid=%d"), 
		Decoder.IsValid());
}

void UAnimatedTexture2D::StepForward()
{
	CurrentFrame++;
	RenderFrameToTexture();
}

void UAnimatedTexture2D::StepBackward()
{
	CurrentFrame--;
	RenderFrameToTexture();
}

void UAnimatedTexture2D::GotoFrame(const int32 NewFrame)
{
	CurrentFrame = NewFrame;
	RenderFrameToTexture();
}


void UAnimatedTexture2D::Stop()
{
	bPlaying = false;
}

void UAnimatedTexture2D::Init(int32 InSizeX, int32 InSizeY, EPixelFormat InFormat/*=2*/, bool InIsResolveTarget/*=false*/)
{
	FScopeLock ResultsLock(&ResultsMutex);

	SizeX = InSizeX;
	SizeY = InSizeY;
	Format = (EPixelFormat)InFormat;
	NumMips = 1;
	bIsResolveTarget = InIsResolveTarget;

	UE_LOG(LogAnimatedTexture, Log, TEXT("Init: Size=%dx%d, Format=%d, bIsResolveTarget=%d, Decoder valid=%d"), 
		SizeX, SizeY, (int32)Format, bIsResolveTarget, Decoder.IsValid());

	// Initialize the resource.
	// NOTE: This is called BEFORE SetDecoder(), so Decoder will be null here.
	// GetFirstFrameData() will be called during resource init and will crash if Decoder is null.
	UpdateResource();
}

const uint8* UAnimatedTexture2D::GetFirstFrameData() const
{
	if (!Decoder.IsValid())
	{
		UE_LOG(LogAnimatedTexture, Warning, TEXT("GetFirstFrameData: Decoder is not valid! Returning nullptr."));
		return nullptr;
	}

	const uint8* FrameData = (const uint8*)Decoder->GetNextFrame(CurrentFrame);
	UE_LOG(LogAnimatedTexture, Log, TEXT("GetFirstFrameData: Frame=%d, FrameData=%p"), CurrentFrame, FrameData);
	return FrameData;
}

uint32 UAnimatedTexture2D::GetFrameSize() const
{
	return CalculateImageBytes(SizeX, SizeY, 1, Format);
}
