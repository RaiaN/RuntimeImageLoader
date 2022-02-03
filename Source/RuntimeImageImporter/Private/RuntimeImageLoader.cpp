// Copyright Peter Leontev

#include "RuntimeImageLoader.h"

URuntimeImageLoader* URuntimeImageLoader::GetRuntimeImageLoader()
{
    URuntimeImageLoader* RuntimeImageLoader = GEngine->GetEngineSubsystem<URuntimeImageLoader>();
    ensure(IsValid(RuntimeImageLoader));

    return RuntimeImageLoader;
}

void URuntimeImageLoader::Initialize(FSubsystemCollectionBase& Collection)
{
    ImageReader = NewObject<URuntimeImageReader>();
}

void URuntimeImageLoader::Deinitialize()
{
    ImageReader = nullptr;
}

void URuntimeImageLoader::LoadImageAsync(const FName& ImageName, bool& Success, UTexture2D*& OutLoadedImage, FString& OutError, FLatentActionInfo LatentInfo, UObject* WorldContextObject /*= nullptr*/)
{

}

void URuntimeImageLoader::LoadImageSync(const FName& ImageName, bool& Success, UTexture2D*& OutLoadedImage, FString& OutError)
{
    
}

void URuntimeImageLoader::LoadImagesAsync(const TArray<FName>& ImageNames, TMap<FName, UTexture2D*>& OutLoadedImages, TArray<FString>& OutErrors, FLatentActionInfo LatentInfo, UObject* WorldContextObject /*= nullptr*/)
{
    
}

void URuntimeImageLoader::Tick(float DeltaTime)
{
    
}

TStatId URuntimeImageLoader::GetStatId() const
{
    return TStatId();
}
