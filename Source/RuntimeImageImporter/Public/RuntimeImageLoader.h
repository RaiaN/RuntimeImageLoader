// Copyright Peter Leontev

#pragma once

#include "CoreMinimal.h"
#include "Engine/LatentActionManager.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Containers/Queue.h"
#include "Tickable.h"
#include "Materials/MaterialInterface.h"
#include "Subsystems/EngineSubsystem.h"

#include "RuntimeImageLoader.generated.h"


class URuntimeImageReader;

/**
 * 
 */
UCLASS(BlueprintType)
class RUNTIMEIMAGEIMPORTER_API URuntimeImageLoader : public UEngineSubsystem, public FTickableGameObject
{    
    GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "RuntimeImageLoader")
	static URuntimeImageLoader* GetRuntimeImageLoader();

    void Initialize(FSubsystemCollectionBase& Collection) override;
    void Deinitialize() override;

public:
    //------------------ Images --------------------
    UFUNCTION(BlueprintCallable, Category = "RuntimeImageImporter", meta = (Latent, LatentInfo = "LatentInfo", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
    void LoadImageAsync(const FName& ImageName, bool& Success, UTexture2D*& OutLoadedImage, FString& OutError, FLatentActionInfo LatentInfo, UObject* WorldContextObject = nullptr);
    
    UFUNCTION(BlueprintCallable, Category = "RuntimeImageImporter")
    void LoadImageSync(const FName& ImageName, bool& Success, UTexture2D*& OutLoadedImage, FString& OutError);
 
    UFUNCTION(BlueprintCallable, Category = "RuntimeImageImporter", meta = (Latent, LatentInfo = "LatentInfo", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
    void LoadImagesAsync(const TArray<FName>& ImageNames, TMap<FName, UTexture2D*>& OutLoadedImages, TArray<FString>& OutErrors, FLatentActionInfo LatentInfo, UObject* WorldContextObject = nullptr);

public:
    // DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRequestCompleted);
    // UPROPERTY(BlueprintAssignable, Category = "EvoAssetManager")
    // FOnRequestCompleted OnRequestCompleted;

private:
    void Tick(float DeltaTime) override;
    TStatId GetStatId() const override;

private:
    TMap<FName, TWeakObjectPtr<UTexture2D>> LoadedImages;

    UPROPERTY()
    URuntimeImageReader* ImageReader = nullptr;
};
