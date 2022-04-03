// Copyright 2022 Peter Leontev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/LatentActionManager.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Containers/Queue.h"
#include "Tickable.h"
#include "Materials/MaterialInterface.h"
#include "Subsystems/WorldSubsystem.h"

#include "RuntimeImageReader.h"

#include "RuntimeImageLoader.generated.h"


class URuntimeImageReader;


DECLARE_DELEGATE_OneParam(FOnRequestCompleted, const FImageReadResult&);

struct FLoadImageRequest
{
public:
    void Invalidate()
    {
        Params = FImageReadRequest();
    }

    bool IsRequestValid() const 
    {
        return Params.ImageFilename.Len() > 0;
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
    UFUNCTION(BlueprintCallable, Category = "RuntimeImageImporter", meta = (Latent, LatentInfo = "LatentInfo", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
    void LoadImageAsync(const FString& ImageFilename, bool bForUI, UTexture2D*& OutTexture, bool& bSuccess, FString& OutError, FLatentActionInfo LatentInfo, UObject* WorldContextObject = nullptr);
    
    UFUNCTION(BlueprintCallable, Category = "RuntimeImageImporter")
    void LoadImageSync(const FString& ImageFilename, bool bForUI, UTexture2D*& OutTexture, bool& bSuccess, FString& OutError);

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
