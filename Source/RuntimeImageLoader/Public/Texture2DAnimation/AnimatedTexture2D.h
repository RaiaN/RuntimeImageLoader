// Copyright 2023 Petr Leontev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture.h"
#include "Tickable.h"
#include "Helpers/GIFLoader.h"
#include "AnimatedTexture2D.generated.h"

/** Copy Frame TO RHITexture */
struct FRenderCommandData
{
	FTextureResource* RHIResource;
	const uint8* RawData;
};

/** @See Texture2DDynamic Class
	Helper to set properties on the UAnimatedTexture2D so it doesn't need to be reinitialized. */
struct FAnimatedTexture2DCreateInfo
{
	FAnimatedTexture2DCreateInfo(EPixelFormat InFormat = PF_B8G8R8A8, bool InIsResolveTarget = false, bool InSRGB = true, TextureFilter InFilter = TF_Default, ESamplerAddressMode InSamplerAddressMode = AM_Wrap)
		: Format(InFormat)
		, bIsResolveTarget(InIsResolveTarget)
		, bSRGB(InSRGB)
		, Filter(InFilter)
		, SamplerAddressMode(InSamplerAddressMode)
	{}

	EPixelFormat Format;
	bool bIsResolveTarget;
	bool bSRGB;
	TextureFilter Filter;
	ESamplerAddressMode SamplerAddressMode;
};

UCLASS(BlueprintType, Category = RuntimeAnimatedTexture)
class RUNTIMEIMAGELOADER_API UAnimatedTexture2D : public UTexture, public FTickableGameObject
{
	GENERATED_BODY()

public:
	// Playback APIs
	UFUNCTION(BlueprintCallable, Category = RuntimeAnimatedTexture)
	void Play();

	UFUNCTION(BlueprintCallable, Category = RuntimeAnimatedTexture)
	void PlayFromStart();
	
	UFUNCTION(BlueprintCallable, Category = RuntimeAnimatedTexture)
	void StepForward();
	
	UFUNCTION(BlueprintCallable, Category = RuntimeAnimatedTexture)
	void StepBackward();

	UFUNCTION(BlueprintCallable, Category = RuntimeAnimatedTexture)
	void GotoFrame(int32 NewFrame);

	UFUNCTION(BlueprintCallable, Category = RuntimeAnimatedTexture)
	void Stop();

	UFUNCTION(BlueprintCallable, Category = RuntimeAnimatedTexture)
	bool IsPlaying() const { return bPlaying; }

	UFUNCTION(BlueprintCallable, Category = RuntimeAnimatedTexture)
	void SetLooping(bool bNewLooping) { bLooping = bNewLooping; }

	UFUNCTION(BlueprintCallable, Category = RuntimeAnimatedTexture)
	bool IsLooping() const { return bLooping; }

	UFUNCTION(BlueprintCallable, Category = RuntimeAnimatedTexture)
	void SetPlayRate(float NewRate) { PlayRate = NewRate; }

	UFUNCTION(BlueprintCallable, Category = RuntimeAnimatedTexture)
	float GetPlayRate() const { return PlayRate; }

	UFUNCTION(BlueprintCallable, Category = RuntimeAnimatedTexture)
	int32 GetWidth() const { return (int32)GetSurfaceWidth(); }

	UFUNCTION(BlueprintCallable, Category = RuntimeAnimatedTexture)
	int32 GetHeight() const { return (int32)GetSurfaceHeight(); }

	UFUNCTION(BlueprintCallable, Category = RuntimeAnimatedTexture)
	int32 GetSizeX() const { return GetWidth(); }

	UFUNCTION(BlueprintCallable, Category = RuntimeAnimatedTexture)
	int32 GetSizeY() const { return GetHeight(); }

public:
	/** @See Texture2DDynamic Class
		Creates and initializes a new AnimatedTexture2D with the requested settings 
	*/
	static UAnimatedTexture2D* Create(int32 InSizeX, int32 InSizeY, const FAnimatedTexture2DCreateInfo& InCreateInfo = FAnimatedTexture2DCreateInfo());

public:
	void SetDecoder(TUniquePtr<IGIFLoader> DecoderState);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RuntimeAnimatedTexture, meta = (DisplayName = "X-axis Tiling Method"), AdvancedDisplay)
	TEnumAsByte<enum TextureAddress> AddressX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RuntimeAnimatedTexture, meta = (DisplayName = "Y-axis Tiling Method"), AdvancedDisplay)
	TEnumAsByte<enum TextureAddress> AddressY;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RuntimeAnimatedTexture)
	float DefaultFrameDelay = 1.0f / 10;	// used while Frame.Delay==0

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RuntimeAnimatedTexture)
	float PlayRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RuntimeAnimatedTexture)
	bool bLooping = true;

public:
	/* Used by AnimatedTextureResource when initializing the texture */
	const uint8* GetFirstFrameData() const;
	uint32 GetFrameSize() const;

	virtual float GetSurfaceWidth() const override;
	virtual float GetSurfaceHeight() const override;

protected:
	// FTickableGameObject Interface
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override
	{
		return true;
	}
	virtual TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(UAnimatedTexture2D, STATGROUP_Tickables);
	}
	virtual bool IsTickableInEditor() const
	{
		return true;
	}

	virtual UWorld* GetTickableGameObjectWorld() const
	{
		return GetWorld();
	}

	void RenderFrameToTexture();

protected:
	virtual FTextureResource* CreateResource() override;
	virtual EMaterialValueType GetMaterialType() const override { return MCT_Texture2D; }

private:
	void Init(int32 InSizeX, int32 InSizeY, EPixelFormat InFormat = PF_B8G8R8A8, bool InIsResolveTarget = false);

private:
	/** The width of the texture. */
	int32 SizeX;

	/** The height of the texture. */
	int32 SizeY;

	/** The format of the texture. */
	UPROPERTY(Transient)
	TEnumAsByte<enum EPixelFormat> Format;

	/** Whether the texture can be used as a resolve target. */
	uint8 bIsResolveTarget : 1;

	/** The number of mip-maps in the texture. */
	int32 NumMips;

	/** The sampler default address mode for this texture. */
	ESamplerAddressMode SamplerAddressMode;

private:
	TUniquePtr<IGIFLoader> Decoder;

	float FrameDelay = 0.0f;
	float FrameTime = 0.0f;
	bool bPlaying = false;

private:
	int32 CurrentFrame = 0;
	FCriticalSection ResultsMutex;
};