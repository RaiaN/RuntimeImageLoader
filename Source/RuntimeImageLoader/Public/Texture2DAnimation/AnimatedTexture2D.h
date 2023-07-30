#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture.h"
#include "Tickable.h"	// Engine
#include "AnimatedTexture2D.generated.h"

/**
 * Animated Texutre
 * @see class UTexture2D
 */

class FRuntimeGIFLoaderHelper;

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

UCLASS(BlueprintType, Category = AnimatedTexture)
class RUNTIMEIMAGELOADER_API UAnimatedTexture2D : public UTexture, public FTickableGameObject
{
	GENERATED_BODY()

	/** The width of the texture. */
	int32 SizeX;

	/** The height of the texture. */
	int32 SizeY;

	/** The format of the texture. */
	UPROPERTY(transient)
	TEnumAsByte<enum EPixelFormat> Format;

	/** Whether the texture can be used as a resolve target. */
	uint8 bIsResolveTarget : 1;

	/** The number of mip-maps in the texture. */
	int32 NumMips;

	/** The sampler default address mode for this texture. */
	ESamplerAddressMode SamplerAddressMode;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AnimatedTexture, meta = (DisplayName = "X-axis Tiling Method"), AssetRegistrySearchable, AdvancedDisplay)
		TEnumAsByte<enum TextureAddress> AddressX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AnimatedTexture, meta = (DisplayName = "Y-axis Tiling Method"), AssetRegistrySearchable, AdvancedDisplay)
		TEnumAsByte<enum TextureAddress> AddressY;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AnimatedTexture)
		bool SupportsTransparency = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AnimatedTexture)
		float DefaultFrameDelay = 1.0f / 10;	// used while Frame.Delay==0

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AnimatedTexture)
		float PlayRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AnimatedTexture)
		bool bLooping = true;

public:	// Playback APIs
	UFUNCTION(BlueprintCallable, Category = AnimatedTexture)
		void Play();

	UFUNCTION(BlueprintCallable, Category = AnimatedTexture)
		void PlayFromStart();

	UFUNCTION(BlueprintCallable, Category = AnimatedTexture)
		void Stop();

	UFUNCTION(BlueprintCallable, Category = AnimatedTexture)
		bool IsPlaying() const { return bPlaying; }

	UFUNCTION(BlueprintCallable, Category = AnimatedTexture)
		void SetLooping(bool bNewLooping) { bLooping = bNewLooping; }

	UFUNCTION(BlueprintCallable, Category = AnimatedTexture)
		bool IsLooping() const { return bLooping; }

	UFUNCTION(BlueprintCallable, Category = AnimatedTexture)
		void SetPlayRate(float NewRate) { PlayRate = NewRate; }

	UFUNCTION(BlueprintCallable, Category = AnimatedTexture)
		float GetPlayRate() const { return PlayRate; }

	UFUNCTION(BlueprintCallable, Category = AnimatedTexture)
		float GetAnimationLength() const;

public:	// UTexture Interface
	virtual float GetSurfaceWidth() const override;
	virtual float GetSurfaceHeight() const override;

	virtual FTextureResource* CreateResource() override;
	virtual EMaterialValueType GetMaterialType() const override { return MCT_Texture2D; }

public:	// FTickableGameObject Interface
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
public:	// UObject Interface.
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

public: // Internal APIs
	void ImportFile(const FString& GIFFilename);
	float RenderFrameToTexture();

public:
	/** @See Texture2DDynamic Class
		Creates and initializes a new AnimatedTexture2D with the requested settings 
	*/
	static UAnimatedTexture2D* Create(int32 InSizeX, int32 InSizeY, const FAnimatedTexture2DCreateInfo& InCreateInfo = FAnimatedTexture2DCreateInfo());
	/**
	 * Initializes the texture with 1 mip-level and creates the render resource.
	 *
	 * @param InSizeX			- Width of the texture, in texels
	 * @param InSizeY			- Height of the texture, in texels
	 * @param InFormat			- Format of the texture, defaults to PF_B8G8R8A8
	 * @param InIsResolveTarget	- Whether the texture can be used as a resolve target
	 */
	void Init(int32 InSizeX, int32 InSizeY, EPixelFormat InFormat = PF_B8G8R8A8, bool InIsResolveTarget = false);

public:
	void SetDecoder(TSharedPtr<FRuntimeGIFLoaderHelper, ESPMode::ThreadSafe> DecoderState);

private:
	UPROPERTY()
		TArray<uint8> FileBlob;

private:
	TSharedPtr<FRuntimeGIFLoaderHelper, ESPMode::ThreadSafe> Decoder;

	float AnimationLength = 0.0f;
	float FrameDelay = 0.0f;
	float FrameTime = 0.0f;
	bool bPlaying = true;
};