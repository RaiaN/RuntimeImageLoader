// Copyright 2023 Petr Leontev. All Rights Reserved.

#include "AnimatedTextureResource.h"
#include "Containers/ResourceArray.h"
#include "Engine/Texture2D.h"
#include "Texture2DAnimation/AnimatedTexture2D.h"

struct FGifDataResource : public FResourceBulkDataInterface
{
public:
	FGifDataResource(void* InMipData, int32 InDataSize)
		: MipData(InMipData), DataSize(InDataSize)
	{}

	const void* GetResourceBulkData() const override { return MipData; }
	uint32 GetResourceBulkDataSize() const override { return DataSize; }
	void Discard() override {}

private:
	void* MipData;
	int32 DataSize;
};


FAnimatedTextureResource::FAnimatedTextureResource(UAnimatedTexture2D* InOwner) :Owner(InOwner)
{
}

uint32 FAnimatedTextureResource::GetSizeX() const
{
	return Owner->GetSurfaceWidth();
}

uint32 FAnimatedTextureResource::GetSizeY() const
{
	return Owner->GetSurfaceHeight();
}

static ESamplerAddressMode ConvertAddressMode(enum TextureAddress addr)
{
	ESamplerAddressMode ret = AM_Wrap;
	switch (addr)
	{
	case TA_Wrap:
		ret = AM_Wrap;
		break;
	case TA_Clamp:
		ret = AM_Clamp;
		break;
	case TA_Mirror:
		ret = AM_Mirror;
		break;
	}
	return ret;
}

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION < 3
void FAnimatedTextureResource::InitRHI()
#else
void FAnimatedTextureResource::InitRHI(FRHICommandListBase& RHICmdList)
#endif
{
	// Create the sampler state RHI resource.
	ESamplerAddressMode AddressU = ConvertAddressMode(Owner->AddressX);
	ESamplerAddressMode AddressV = ConvertAddressMode(Owner->AddressY);
	ESamplerAddressMode AddressW = AM_Wrap;

	// Default to point filtering.
	ESamplerFilter Filter = ESamplerFilter::SF_Trilinear;

	switch (Owner->Filter)
	{
	case TF_Nearest:
		Filter = ESamplerFilter::SF_Point;
		break;
	case TF_Bilinear:
		Filter = ESamplerFilter::SF_Bilinear;
		break;
	case TF_Trilinear:
		Filter = ESamplerFilter::SF_Trilinear;
		break;
	default:
		break;
	}

	FSamplerStateInitializerRHI SamplerStateInitializer
	(
		Filter,
		AddressU,
		AddressV,
		AddressW
	);
	SamplerStateRHI = GetOrCreateSamplerState(SamplerStateInitializer);

	ETextureCreateFlags  Flags = TexCreate_None;
	if (!Owner->SRGB)
		bIgnoreGammaConversions = true;

	if (Owner->SRGB)
		Flags |= TexCreate_SRGB;
	if (Owner->bNoTiling)
		Flags |= TexCreate_NoTiling;

	const uint32 NumMips = 1;
	const FString& Name = Owner->GetName();
	const EPixelFormat ImageFormat = PF_B8G8R8A8;
	
	FRHIResourceCreateInfo CreateInfo(*Name);

	FGifDataResource GifBulkData((void*)Owner->GetFirstFrameData(), Owner->GetFrameSize());
	CreateInfo.BulkData = &GifBulkData;
	
#if (ENGINE_MAJOR_VERSION >= 5) && (ENGINE_MINOR_VERSION > 0)
    TextureRHI = RHICreateTexture(
        FRHITextureCreateDesc::Create2D(CreateInfo.DebugName)
        .SetExtent(GetSizeX(), GetSizeY())
        .SetFormat(ImageFormat)
        .SetNumMips(NumMips)
        .SetNumSamples(1)
        .SetFlags(Flags)
        .SetInitialState(ERHIAccess::Unknown)
        .SetExtData(CreateInfo.ExtData)
		.SetBulkData(CreateInfo.BulkData)
        .SetGPUMask(CreateInfo.GPUMask)
        .SetClearValue(CreateInfo.ClearValueBinding)
    );
#else
    TextureRHI = RHICreateTexture2D(GetSizeX(), GetSizeY(), ImageFormat, NumMips, 1, Flags, CreateInfo);
#endif

	TextureRHI->SetName(Owner->GetFName());
	RHIUpdateTextureReference(Owner->TextureReference.TextureReferenceRHI, TextureRHI);
}

void FAnimatedTextureResource::ReleaseRHI()
{
	RHIUpdateTextureReference(Owner->TextureReference.TextureReferenceRHI, nullptr);
	FTextureResource::ReleaseRHI();
}

FTexture2DRHIRef FAnimatedTextureResource::GetTexture2DRHI()
{
	return Texture2DRHI;
}

int32 FAnimatedTextureResource::GetDefaultMipMapBias() const
{
	return 0;
}

void FAnimatedTextureResource::CreateSamplerStates(float MipMapBias)
{
	FSamplerStateInitializerRHI SamplerStateInitializer
	(
		SF_Bilinear,
		Owner->AddressX == TA_Wrap ? AM_Wrap : (Owner->AddressX == TA_Clamp ? AM_Clamp : AM_Mirror),
		Owner->AddressY == TA_Wrap ? AM_Wrap : (Owner->AddressY == TA_Clamp ? AM_Clamp : AM_Mirror),
		AM_Wrap,
		MipMapBias
	);
	SamplerStateRHI = RHICreateSamplerState(SamplerStateInitializer);

	FSamplerStateInitializerRHI DeferredPassSamplerStateInitializer
	(
		SF_Bilinear,
		Owner->AddressX == TA_Wrap ? AM_Wrap : (Owner->AddressX == TA_Clamp ? AM_Clamp : AM_Mirror),
		Owner->AddressY == TA_Wrap ? AM_Wrap : (Owner->AddressY == TA_Clamp ? AM_Clamp : AM_Mirror),
		AM_Wrap,
		MipMapBias,
		1,
		0,
		2
	);

	DeferredPassSamplerStateRHI = RHICreateSamplerState(DeferredPassSamplerStateInitializer);
}