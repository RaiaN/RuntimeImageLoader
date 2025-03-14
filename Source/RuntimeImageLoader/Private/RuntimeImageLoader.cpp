// Copyright 2023 Petr Leontev. All Rights Reserved.

#include "RuntimeImageLoader.h"
#include "Subsystems/SubsystemBlueprintLibrary.h"
#include "UObject/WeakObjectPtr.h"
#include "HAL/Platform.h"
#include "HAL/FileManager.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "Misc/FileHelper.h"
#include "Interfaces/IPluginManager.h"
#include "RuntimeImageUtils.h"
#include "InputImageDescription.h"

THIRD_PARTY_INCLUDES_START
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
THIRD_PARTY_INCLUDES_END


DEFINE_LOG_CATEGORY_STATIC(LogRuntimeImageLoader, Log, All);

void URuntimeImageLoader::Initialize(FSubsystemCollectionBase& Collection)
{
    InitializeImageReader();
}

void URuntimeImageLoader::Deinitialize()
{
    ImageReader->Deinitialize();
    ImageReader = nullptr;
}

bool URuntimeImageLoader::DoesSupportWorldType(EWorldType::Type WorldType) const
{
    return WorldType == EWorldType::PIE || WorldType == EWorldType::Game;
}

void URuntimeImageLoader::LoadImageAsync(const FString& ImageFilename, const FTransformImageParams& TransformParams, UTexture2D*& OutTexture, bool& bSuccess, FString& OutError, FLatentActionInfo LatentInfo, UObject* WorldContextObject /*= nullptr*/)
{
    if (!IsValid(WorldContextObject))
    {
        return;
    }

    FLoadImageRequest Request;
    {
        Request.Params.InputImage = FInputImageDescription(ImageFilename);
        Request.Params.TransformParams = TransformParams;

        Request.OnRequestCompleted.BindLambda(
            [this, &OutTexture, &bSuccess, &OutError, LatentInfo](const FImageReadResult& ReadResult)
            {
                FWeakObjectPtr CallbackTargetPtr = LatentInfo.CallbackTarget;
                if (UObject* CallbackTarget = CallbackTargetPtr.Get())
                {
                    UFunction* ExecutionFunction = CallbackTarget->FindFunction(LatentInfo.ExecutionFunction);
                    if (IsValid(ExecutionFunction))
                    {
                        int32 Linkage = LatentInfo.Linkage;

#if (UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT)
                        // Make sure the texture was not destroyed by GC 
                        if (ReadResult.OutError.IsEmpty())
                        {
                            ensure(IsValid(ReadResult.OutTexture));
                        }
#endif

                        if (!ReadResult.OutError.IsEmpty())
                        {
                            UE_LOG(LogRuntimeImageLoader, Error, TEXT("Failed to load image. Error: %s"), *ReadResult.OutError);
                        }

                        bSuccess = ReadResult.OutError.IsEmpty();
                        OutTexture = ReadResult.OutTexture;
                        OutError = ReadResult.OutError;

                        if (Linkage != -1)
                        {
                            CallbackTarget->ProcessEvent(ExecutionFunction, &Linkage);
                        }
                    }
                }
            }
        );
    }

    Requests.Enqueue(Request);
}

void URuntimeImageLoader::LoadImageFromBytesAsync(UPARAM(ref) TArray<uint8>& ImageBytes, const FTransformImageParams& TransformParams, UTexture2D*& OutTexture, bool& bSuccess, FString& OutError, FLatentActionInfo LatentInfo, UObject* WorldContextObject /*= nullptr*/)
{
    if (!IsValid(WorldContextObject))
    {
        return;
    }

    FLoadImageRequest Request;
    {
        Request.Params.InputImage = FInputImageDescription(MoveTemp(ImageBytes));
        Request.Params.TransformParams = TransformParams;

        Request.OnRequestCompleted.BindLambda(
            [this, &OutTexture, &bSuccess, &OutError, LatentInfo](const FImageReadResult& ReadResult)
            {
                FWeakObjectPtr CallbackTargetPtr = LatentInfo.CallbackTarget;
                if (UObject* CallbackTarget = CallbackTargetPtr.Get())
                {
                    UFunction* ExecutionFunction = CallbackTarget->FindFunction(LatentInfo.ExecutionFunction);
                    if (IsValid(ExecutionFunction))
                    {
                        int32 Linkage = LatentInfo.Linkage;

        #if (UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT)
                        // Make sure the texture was not destroyed by GC 
                        if (ReadResult.OutError.IsEmpty())
                        {
                            ensure(IsValid(ReadResult.OutTexture));
                        }
        #endif

                        if (!ReadResult.OutError.IsEmpty())
                        {
                            UE_LOG(LogRuntimeImageLoader, Error, TEXT("Failed to load image. Error: %s"), *ReadResult.OutError);
                        }

                        bSuccess = ReadResult.OutError.IsEmpty();
                        OutTexture = ReadResult.OutTexture;
                        OutError = ReadResult.OutError;

                        if (Linkage != -1)
                        {
                            CallbackTarget->ProcessEvent(ExecutionFunction, &Linkage);
                        }
                    }
                }
            }
        );
    }

    Requests.Enqueue(Request);
}

void URuntimeImageLoader::LoadHDRIAsCubemapAsync(const FString& ImageFilename, const FTransformImageParams& TransformParams, UTextureCube*& OutTextureCube, bool& bSuccess, FString& OutError, FLatentActionInfo LatentInfo, UObject* WorldContextObject /*= nullptr*/)
{
    if (!IsValid(WorldContextObject))
    {
        return;
    }

    // TODO: loading cubemaps is not supported on Android & Mac platforms! You can fix the behaviour at your own risk!
    // TODO: Pull requests are appreciated!
#if PLATFORM_ANDROID || PLATFORM_MAC
    OutError = TEXT("Loading cubemaps is not supported on Android & Mac platforms!");
    bSuccess = false;
    UE_LOG(LogRuntimeImageLoader, Warning, TEXT("%s"), *OutError);
    return;
#endif

    FLoadImageRequest Request;
    {
        Request.Params.InputImage = FInputImageDescription(ImageFilename);
        Request.Params.TransformParams = TransformParams;

        Request.OnRequestCompleted.BindLambda(
            [this, &OutTextureCube, &bSuccess, &OutError, LatentInfo](const FImageReadResult& ReadResult)
            {
                FWeakObjectPtr CallbackTargetPtr = LatentInfo.CallbackTarget;
                if (UObject* CallbackTarget = CallbackTargetPtr.Get())
                {
                    UFunction* ExecutionFunction = CallbackTarget->FindFunction(LatentInfo.ExecutionFunction);
                    if (IsValid(ExecutionFunction))
                    {
                        int32 Linkage = LatentInfo.Linkage;

#if (UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT)
                        // Make sure the texture was not destroyed by GC 
                        if (ReadResult.OutError.IsEmpty())
                        {
                            ensure(IsValid(ReadResult.OutTextureCube));
                        }
#endif

                        if (!ReadResult.OutError.IsEmpty())
                        {
                            UE_LOG(LogRuntimeImageLoader, Error, TEXT("Failed to load image. Error: %s"), *ReadResult.OutError);
                        }

                        bSuccess = ReadResult.OutError.IsEmpty();
                        OutTextureCube = ReadResult.OutTextureCube;
                        OutError = ReadResult.OutError;

                        if (Linkage != -1)
                        {
                            CallbackTarget->ProcessEvent(ExecutionFunction, &Linkage);
                        }
                    }
                }
            }
        );
    }

    Requests.Enqueue(Request);
}

void URuntimeImageLoader::LoadImageSync(const FString& ImageFilename, const FTransformImageParams& TransformParams, UTexture2D*& OutTexture, bool& bSuccess, FString& OutError)
{
    FImageReadRequest ReadRequest;
    {
        ReadRequest.InputImage = FInputImageDescription(ImageFilename);
        ReadRequest.TransformParams = TransformParams;
    }

    ImageReader->BlockTillAllRequestsFinished();
    ImageReader->AddRequest(ReadRequest);
    ImageReader->BlockTillAllRequestsFinished();

    FImageReadResult ReadResult;
    ImageReader->GetResult(ReadResult);

    bSuccess = ReadResult.OutError.IsEmpty();
    OutTexture = ReadResult.OutTexture;
    OutError = ReadResult.OutError;
}

void URuntimeImageLoader::LoadImageFromBytesSync(UPARAM(ref) TArray<uint8>& ImageBytes, const FTransformImageParams& TransformParams, UTexture2D*& OutTexture, bool& bSuccess, FString& OutError)
{
    FImageReadRequest ReadRequest;
    {
        ReadRequest.InputImage = FInputImageDescription(MoveTemp(ImageBytes));
        ReadRequest.TransformParams = TransformParams;
    }

    ImageReader->BlockTillAllRequestsFinished();
    ImageReader->AddRequest(ReadRequest);
    ImageReader->BlockTillAllRequestsFinished();

    FImageReadResult ReadResult;
    ImageReader->GetResult(ReadResult);

    bSuccess = ReadResult.OutError.IsEmpty();
    OutTexture = ReadResult.OutTexture;
    OutError = ReadResult.OutError;
}

void URuntimeImageLoader::LoadImagePixels(const FInputImageDescription& InputImage, const FTransformImageParams& TransformParams, TArray<FColor>& OutImagePixels, bool& bSuccess, FString& OutError, FLatentActionInfo LatentInfo, UObject* WorldContextObject /*= nullptr*/)
{
    if (!IsValid(WorldContextObject))
    {
        return;
    }

    FLoadImageRequest Request;
    {
        Request.Params.InputImage = InputImage;
        Request.Params.TransformParams = TransformParams;
        Request.Params.TransformParams.bOnlyPixels = true;

        Request.OnRequestCompleted.BindLambda(
        [this, &OutImagePixels, &bSuccess, &OutError, LatentInfo](const FImageReadResult& ReadResult)
        {
            FWeakObjectPtr CallbackTargetPtr = LatentInfo.CallbackTarget;
            if (UObject* CallbackTarget = CallbackTargetPtr.Get())
            {
                UFunction* ExecutionFunction = CallbackTarget->FindFunction(LatentInfo.ExecutionFunction);
                if (IsValid(ExecutionFunction))
                {
                    int32 Linkage = LatentInfo.Linkage;

    #if (UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT)
                    // Make sure the texture was not destroyed by GC 
                    if (ReadResult.OutError.IsEmpty())
                    {
                        ensure(IsValid(ReadResult.OutTexture) || ReadResult.OutImagePixels.Num() > 0);
                    }
    #endif

                    if (!ReadResult.OutError.IsEmpty())
                    {
                        UE_LOG(LogRuntimeImageLoader, Error, TEXT("Failed to load image. Error: %s"), *ReadResult.OutError);
                    }

                    bSuccess = ReadResult.OutError.IsEmpty();
                    OutImagePixels = ReadResult.OutImagePixels;
                    OutError = ReadResult.OutError;

                    if (Linkage != -1)
                    {
                        CallbackTarget->ProcessEvent(ExecutionFunction, &Linkage);
                    }
                }
            }
            }
        );
    }

    Requests.Enqueue(Request);
}

void URuntimeImageLoader::GetImageResolution(const FString& ImageFilename, int32& OutWidth, int32& OutHeight, int32& OutChannels, bool& bSuccess, FString& OutError)
{
    if (stbi_info(TCHAR_TO_ANSI(*ImageFilename), &OutWidth, &OutHeight, &OutChannels) == 1)
    {
        bSuccess = true;
        return;
    }

    OutError = TEXT("Failed to get image dimensions! Try to load an image via LoadImage or LoadImageAsync! stb error (if any): ");
    OutError += stbi_failure_reason();
}

void URuntimeImageLoader::GetImageResolutionFromBytes(UPARAM(ref) TArray<uint8>& ImageBytes, int32& OutWidth, int32& OutHeight, int32& OutChannels, bool& bSuccess, FString& OutError)
{
    if (stbi_info_from_memory(ImageBytes.GetData(), ImageBytes.Num(), &OutWidth, &OutHeight, &OutChannels) == 1)
    {
        bSuccess = true;
        return;
    }

    OutError = TEXT("Failed to get image dimensions! Try to load an image via LoadImage or LoadImageAsync! stb error (if any): ");
    OutError += stbi_failure_reason();
}

void URuntimeImageLoader::CancelAll()
{
    check (IsInGameThread());

    // TODO: Cancelling http request leads to crash on Android!
    // TODO: opportunity for a pull request!
#if PLATFORM_ANDROID
    UE_LOG(LogRuntimeImageLoader, Warning, TEXT("Cancelling http requests is not supported on Android platform!"));
    return;
#endif

    Requests.Empty();
    ActiveRequest.Invalidate();

    ImageReader->Clear();
}

bool URuntimeImageLoader::LoadImageToByteArray(const FString& ImageFilename, TArray<uint8>& OutImageBytes, FString& OutError)
{
    TArray<uint8> OutData;
    
    FImageReadRequest ReadRequest;
    {
        ReadRequest.InputImage = FInputImageDescription(ImageFilename);
        ReadRequest.TransformParams.bOnlyBytes = true;
    }

    ImageReader->BlockTillAllRequestsFinished();
    ImageReader->AddRequest(ReadRequest);
    ImageReader->BlockTillAllRequestsFinished();

    FImageReadResult ReadResult;
    ImageReader->GetResult(ReadResult);

    if (!ReadResult.OutError.IsEmpty())
    {
        OutError = FString::Printf(TEXT("Failed to load image. Error: %s"), *ReadResult.OutError);
        return false;
    }

    OutImageBytes = MoveTemp(ReadResult.OutImageBytes);

    return true;
}

class FImageFileVisitor : public IPlatformFile::FDirectoryVisitor
{
public:
    virtual bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory) override
    {
        if (!bIsDirectory)
        {
            const FString Filename(FilenameOrDirectory);

            for (const FString& ImageFormat : FRuntimeImageUtils::SupportedImageFormats)
            {
                if (Filename.EndsWith(ImageFormat))
                {
                    Files.Add(Filename);
                    break;
                }
            }
        }
        return true;
    }

    TArray<FString> Files;
};

void URuntimeImageLoader::FindImagesInDirectory(const FString& Directory, bool bIsRecursive, TArray<FString>& OutImageFilenames, bool& bSuccess, FString& OutError)
{
    if (Directory.IsEmpty() || !IFileManager::Get().DirectoryExists(*Directory))
    {
        bSuccess = false;
        OutError = FString::Printf(TEXT("Directory not found: %s"), *Directory);
        return;
    }

    FImageFileVisitor DirectoryVisitor;
    if (bIsRecursive)
    {
        IFileManager::Get().IterateDirectoryRecursively(*Directory, DirectoryVisitor);
    }
    else
    {
        IFileManager::Get().IterateDirectory(*Directory, DirectoryVisitor);
    }

    OutImageFilenames = DirectoryVisitor.Files;
    bSuccess = true;
}

FString URuntimeImageLoader::GetThisPluginResourcesDirectory()
{
    return IPluginManager::Get().FindPlugin(TEXT("RuntimeImageLoader"))->GetBaseDir() / TEXT("Resources");
}

void URuntimeImageLoader::Tick(float DeltaTime)
{
    ensure(IsValid(ImageReader));
    
    if (!ActiveRequest.IsRequestValid() && !Requests.IsEmpty())
    {
        Requests.Dequeue(ActiveRequest);

        FImageReadRequest ReadRequest(ActiveRequest.Params);

        ImageReader->AddRequest(ReadRequest);
        ImageReader->Trigger();
    }

    if (ActiveRequest.IsRequestValid() && ImageReader->IsWorkCompleted())
    {
        FImageReadResult ReadResult;
        ImageReader->GetResult(ReadResult);

        ensure(ActiveRequest.OnRequestCompleted.IsBound());

        ActiveRequest.OnRequestCompleted.Execute(ReadResult);

        ActiveRequest.Invalidate();
    }
}

TStatId URuntimeImageLoader::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(URuntimeImageLoader, STATGROUP_Tickables);
}

bool URuntimeImageLoader::IsAllowedToTick() const
{
    return !IsTemplate();
}

URuntimeImageReader* URuntimeImageLoader::InitializeImageReader()
{
    if (!IsValid(ImageReader))
    {
        ImageReader = NewObject<URuntimeImageReader>(this);
        ImageReader->Initialize();
    }

    ensure(IsValid(ImageReader));
    return ImageReader;
}
