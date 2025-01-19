// Copyright 2023 Petr Leontev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/LatentActionManager.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureCube.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Containers/Queue.h"
#include "Tickable.h"
#include "Materials/MaterialInterface.h"
#include "Subsystems/WorldSubsystem.h"
#include "RuntimeImageReader.h"
#include "RuntimeImageLoader.generated.h"

class UAnimatedTexture2D;
class URuntimeGifReader;

DECLARE_DELEGATE_OneParam(FOnRequestCompleted, const FImageReadResult&);

struct RUNTIMEIMAGELOADER_API FLoadImageRequest
{
public:
    void Invalidate()
    {
        Params = FImageReadRequest();
    }

    bool IsRequestValid() const 
    {
        return Params.InputImage.ImageFilename.Len() > 0 || Params.InputImage.ImageBytes.Num() > 0;
    }

public:
    FImageReadRequest Params;
    FOnRequestCompleted OnRequestCompleted;
};

/**
 * 
 */
UCLASS(BlueprintType)
class RUNTIMEIMAGELOADER_API URuntimeImageLoader : public UWorldSubsystem, public FTickableGameObject
{    
    GENERATED_BODY()

public:
    //------------------ Images --------------------
    UFUNCTION(BlueprintCallable, Category = "Runtime Image Loader", meta = (AutoCreateRefTerm = "TransformParams", Latent, LatentInfo = "LatentInfo", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
    void LoadImageAsync(const FString& ImageFilename, const FTransformImageParams& TransformParams, UTexture2D*& OutTexture, bool& bSuccess, FString& OutError, FLatentActionInfo LatentInfo, UObject* WorldContextObject = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Runtime Image Loader | Bytes", meta = (AutoCreateRefTerm = "TransformParams", Latent, LatentInfo = "LatentInfo", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
    void LoadImageFromBytesAsync(UPARAM(ref) TArray<uint8>& ImageBytes, const FTransformImageParams& TransformParams, UTexture2D*& OutTexture, bool& bSuccess, FString& OutError, FLatentActionInfo LatentInfo, UObject* WorldContextObject = nullptr);
    
    UFUNCTION(BlueprintCallable, Category = "Runtime Image Loader | Cubemap", meta = (AutoCreateRefTerm = "TransformParams", Latent, LatentInfo = "LatentInfo", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
    void LoadHDRIAsCubemapAsync(const FString& ImageFilename, const FTransformImageParams& TransformParams, UTextureCube*& OutTextureCube, bool& bSuccess, FString& OutError, FLatentActionInfo LatentInfo, UObject* WorldContextObject = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Runtime Image Loader", meta = (AutoCreateRefTerm = "TransformParams"))
    void LoadImageSync(const FString& ImageFilename, const FTransformImageParams& TransformParams, UTexture2D*& OutTexture, bool& bSuccess, FString& OutError);

    UFUNCTION(BlueprintCallable, Category = "Runtime Image Loader | Bytes", meta = (AutoCreateRefTerm = "TransformParams"))
    void LoadImageFromBytesSync(UPARAM(ref) TArray<uint8>& ImageBytes, const FTransformImageParams& TransformParams, UTexture2D*& OutTexture, bool& bSuccess, FString& OutError);

    UFUNCTION(BlueprintCallable, Category = "Runtime Image Loader", meta = (Latent, LatentInfo = "LatentInfo", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
    void LoadImagePixels(const FInputImageDescription& InputImage, const FTransformImageParams& TransformParams, TArray<FColor>& OutImagePixels, bool& bSuccess, FString& OutError, FLatentActionInfo LatentInfo, UObject* WorldContextObject = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Runtime Image Loader")
    void GetImageResolution(const FString& ImageFilename, int32& OutWidth, int32& OutHeight, int32& OutChannels, bool& bSuccess, FString& OutError);

    UFUNCTION(BlueprintCallable, Category = "Runtime Image Loader | Bytes")
    void GetImageResolutionFromBytes(UPARAM(ref) TArray<uint8>& ImageBytes, int32& OutWidth, int32& OutHeight, int32& OutChannels, bool& bSuccess, FString& OutError);
    
    /** Utilities */
    UFUNCTION(BlueprintCallable, Category = "Runtime Image Loader | Utilities")
    void CancelAll();

    UFUNCTION(BlueprintCallable, Category = "Runtime Image Loader | Utilities")
    bool LoadImageToByteArray(const FString& ImageFilename, TArray<uint8>& OutImageBytes, FString& OutError);

    UFUNCTION(BlueprintCallable, Category = "Runtime Image Loader | Utilities")
    void FindImagesInDirectory(const FString& Directory, bool bIsRecursive, TArray<FString>& OutImageFilenames, bool& bSuccess, FString& OutError);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Runtime Image Loader | Utilities")
    static FString GetThisPluginResourcesDirectory();

protected:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;

private:
    void Tick(float DeltaTime) override;
    TStatId GetStatId() const override;
    virtual bool IsAllowedToTick() const override;

    URuntimeImageReader* InitializeImageReader();

private:
    UPROPERTY()
    URuntimeImageReader* ImageReader = nullptr;

    TQueue<FLoadImageRequest> Requests;
    FLoadImageRequest ActiveRequest;
};
