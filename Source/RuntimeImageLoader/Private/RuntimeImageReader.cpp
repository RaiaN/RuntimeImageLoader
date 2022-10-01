// Copyright 2022 Peter Leontev. All Rights Reserved.

#include "RuntimeImageReader.h"

#include "GenericPlatform/GenericPlatformProcess.h"
#include "HAL/RunnableThread.h"
#include "HAL/Event.h"
#include "RenderUtils.h"
#include "Engine/Texture.h"
#include "Engine/Texture2D.h"
#include "PixelFormat.h"
#include "Launch/Resources/Version.h"

#include "ImageReaders/ImageReaderFactory.h"
#include "ImageReaders/IImageReader.h"
#include "TextureFactory/RuntimeTextureResource.h"
#include "TextureFactory/RuntimeRHITexture2DFactory.h"
#include "TextureFactory/RuntimeRHITextureCubeFactory.h"
#include "TextureFactory/RuntimeTextureFactory.h"
#include "RuntimeImageUtils.h"
#include "Helpers/CubemapUtils.h"


DEFINE_LOG_CATEGORY_STATIC(LogRuntimeImageReader, Log, All);


void URuntimeImageReader::Initialize()
{
    TextureFactory = NewObject<URuntimeTextureFactory>(GetTransientPackage());

    ThreadSemaphore = FPlatformProcess::GetSynchEventFromPool(false);
    Thread = FRunnableThread::Create(this, TEXT("RuntimeImageReader"), 0, TPri_SlightlyBelowNormal);

    UE_LOG(LogRuntimeImageReader, Log, TEXT("Image reader thread started!"))
}

void URuntimeImageReader::Deinitialize()
{
    Clear();
    Stop();

    TextureFactory = nullptr;

    UE_LOG(LogRuntimeImageReader, Log, TEXT("Image reader thread exited!"))
}

bool URuntimeImageReader::Init()
{
    return true;
}

uint32 URuntimeImageReader::Run()
{
    while (!bStopThread)
    {
        ThreadSemaphore->Wait();
        
        BlockTillAllRequestsFinished();
    }

    return 0;
}

void URuntimeImageReader::Exit()
{
    // 
}

void URuntimeImageReader::AddRequest(const FImageReadRequest& Request)
{
    Requests.Enqueue(Request);

    bCompletedWork.AtomicSet(false);
}

bool URuntimeImageReader::GetResult(FImageReadResult& OutResult)
{
    FScopeLock ResultsLock(&ResultsMutex);

    if (Results.Num() > 0)
    {
        OutResult = Results.Pop();

        return true;
    }

    return false;
}

void URuntimeImageReader::Clear()
{
    Requests.Empty();

    {
        FScopeLock ResultsLock(&ResultsMutex);
        Results.Empty();
    }

    if (ImageReader.IsValid())
    {
        ImageReader->Cancel();
    }

    if (IsValid(TextureFactory))
    {
        TextureFactory->Cancel();
    }

    bCompletedWork = true;
}

void URuntimeImageReader::Stop()
{
    bStopThread = true;

    Trigger();
    Thread->WaitForCompletion();

    if (ImageReader.IsValid())
    {
        ImageReader->Cancel();
    }

    FPlatformProcess::ReturnSynchEventToPool(ThreadSemaphore);
}

bool URuntimeImageReader::IsWorkCompleted() const
{
    return bCompletedWork;
}

void URuntimeImageReader::Trigger()
{
    ThreadSemaphore->Trigger();
}

void URuntimeImageReader::BlockTillAllRequestsFinished()
{
    while (!bCompletedWork && !bStopThread)
    {
        FImageReadRequest Request;
        while (Requests.Dequeue(Request) && !bStopThread)
        {
            PendingReadResult.ImageFilename = Request.ImageFilename;

            UE_LOG(LogRuntimeImageReader, Log, TEXT("Reading image %s"), *PendingReadResult.ImageFilename);

            if (!ProcessRequest(Request))
            {
                UE_LOG(LogRuntimeImageReader, Warning, TEXT("Failed to process image %s"), *PendingReadResult.ImageFilename);
            }

            FScopeLock ResultsLock(&ResultsMutex);
            Results.Add(PendingReadResult);

            PendingReadResult = FImageReadResult();
        }

        bCompletedWork.AtomicSet(Requests.IsEmpty());
    }
}

bool URuntimeImageReader::ProcessRequest(FImageReadRequest& Request)
{
    const FString& InputImageFilename = Request.ImageFilename;
    
    // read image data from using URI
    TArray<uint8> ImageBuffer;
    ImageReader = FImageReaderFactory::CreateReader(Request.ImageFilename);
    {
        ImageBuffer = ImageReader->ReadImage(Request.ImageFilename);
        if (ImageBuffer.Num() == 0)
        {
            PendingReadResult.OutError = FString::Printf(TEXT("Failed to read %s image. Error: %s"), *Request.ImageFilename, *ImageReader->GetLastError());
            return false;
        }
        
    }
    ImageReader = nullptr;

    // sanity check
    check(ImageBuffer.Num() > 0);


    FRuntimeImageData ImageData;
    if (!FRuntimeImageUtils::ImportBufferAsImage(ImageBuffer.GetData(), ImageBuffer.Num(), ImageData, PendingReadResult.OutError))
    {
        return false;
    }

    if (PendingReadResult.OutError.Len() > 0)
    {
        return false;
    }

    // sanity checks
    check(ImageData.RawData.Num() > 0);
    check(ImageData.TextureSourceFormat != TSF_Invalid);

    ImageData.PixelFormat = DeterminePixelFormat(ImageData.Format, Request.TransformParams);
    if (ImageData.PixelFormat == PF_Unknown)
    {
        PendingReadResult.OutError = FString::Printf(TEXT("Pixel format is not supported: %d"), (int32)ImageData.PixelFormat);
        return false;
    }

    // TODO: Below code should be unified and texture source format should be respected by transformation layers
    if (ImageData.TextureSourceFormat == TSF_BGRE8)
    {
        PendingReadResult.OutTextureCube = TextureFactory->CreateTextureCube({ Request.ImageFilename, &ImageData });

        // TODO: Split into multiple transformation layers, which can be stacked?
        // FIXME: this transformation should be done after texture cube is created
        // FIXME: this is not exactly compatible with transform params
        ApplySizeFormatTransformations(ImageData, Request.TransformParams);

        FRuntimeRHITextureCubeFactory RHITextureCubeFactory(PendingReadResult.OutTextureCube, ImageData);
        if (!RHITextureCubeFactory.Create())
        {
            PendingReadResult.OutError = FString::Printf(TEXT("Failed to create RHI texture cube, pixel format: %d"), (int32)ImageData.PixelFormat);
            return false;
        }
    }
    else
    {
        // TODO: Split into multiple transformation layers, which can be stacked?
        ApplySizeFormatTransformations(ImageData, Request.TransformParams);

        PendingReadResult.OutTexture = TextureFactory->CreateTexture2D({ Request.ImageFilename, &ImageData });

        FRuntimeRHITexture2DFactory RHITexture2DFactory(PendingReadResult.OutTexture, ImageData);
        if (!RHITexture2DFactory.Create())
        {
            PendingReadResult.OutError = FString::Printf(TEXT("Failed to create RHI texture 2D, pixel format: %d"), (int32)ImageData.PixelFormat);
            return false;
        }
    }

    return true;
}

EPixelFormat URuntimeImageReader::DeterminePixelFormat(ERawImageFormat::Type ImageFormat, const FTransformImageParams& Params) const
{
    EPixelFormat PixelFormat;
    
    // determine pixel format
    switch (ImageFormat)
    {
        case ERawImageFormat::G8:            PixelFormat = PF_G8; break;
        case ERawImageFormat::G16:           PixelFormat = PF_G16; break;
        case ERawImageFormat::BGRA8:         PixelFormat = PF_B8G8R8A8; break;
        case ERawImageFormat::BGRE8:         PixelFormat = PF_B8G8R8A8; break;
        case ERawImageFormat::RGBA16:        PixelFormat = PF_R16G16B16A16_SINT; break;
        case ERawImageFormat::RGBA16F:       PixelFormat = PF_FloatRGBA; break;
        case ERawImageFormat::RGBA32F:       PixelFormat = PF_A32B32G32R32F; break;
        default:                             PixelFormat = PF_Unknown; break;
    }

    return PixelFormat;
}

void URuntimeImageReader::ApplySizeFormatTransformations(FRuntimeImageData& ImageData, FTransformImageParams TransformParams)
{
    if (TransformParams.IsPercentSizeValid())
    {
        const int32 TransformedSizeX = FMath::Floor(ImageData.SizeX * TransformParams.PercentSizeX * 0.01f);
        const int32 TransformedSizeY = FMath::Floor(ImageData.SizeY * TransformParams.PercentSizeY * 0.01f);
        
        FImage TransformedImage;
        TransformedImage.Init(TransformedSizeX, TransformedSizeY, ImageData.Format);

        ImageData.ResizeTo(TransformedImage, TransformedImage.SizeX, TransformedImage.SizeY, ImageData.Format, ImageData.GammaSpace);

        ImageData.RawData = MoveTemp(TransformedImage.RawData);
        ImageData.SizeX = TransformedImage.SizeX;
        ImageData.SizeY = TransformedImage.SizeY;
    }
    else
    {
        UE_LOG(LogRuntimeImageReader, Verbose, TEXT("Supplied transform params are not valid! PercentSizeX, PercentSizeX: (%d, %d)"), TransformParams.PercentSizeX, TransformParams.PercentSizeY);
    }

    if (TransformParams.bForUI)
    {
        // no need to convert float RGBA and HDR
        if (ImageData.TextureSourceFormat != TSF_RGBA16F && ImageData.TextureSourceFormat != TSF_BGRE8)
        {
            FImage BGRAImage;
            BGRAImage.Init(ImageData.SizeX, ImageData.SizeY, ERawImageFormat::BGRA8);
            ImageData.CopyTo(BGRAImage, ERawImageFormat::BGRA8, EGammaSpace::sRGB);

            ImageData.RawData = MoveTemp(BGRAImage.RawData);
            ImageData.SRGB = true;
            ImageData.GammaSpace = EGammaSpace::sRGB;

            // modify pixel format
            switch (ImageData.TextureSourceFormat)
            {
                case ERawImageFormat::G8:
                case ERawImageFormat::RGBA16:
                {
                    ImageData.PixelFormat = PF_B8G8R8A8;
                    break;
                }
                default:
                    break;
            }
        }
    }
    
    if (ImageData.TextureSourceFormat == TSF_BGRE8)
    {
        FImage CubemapMip;
        GenerateBaseCubeMipFromLongitudeLatitude2D(&CubemapMip, ImageData, 8192, 0);

        ImageData.RawData = MoveTemp(CubemapMip.RawData);
        ImageData.SizeX = CubemapMip.SizeX;
        ImageData.SizeY = CubemapMip.SizeY;
        ImageData.PixelFormat = DeterminePixelFormat(CubemapMip.Format, TransformParams);
        ImageData.GammaSpace = CubemapMip.GammaSpace;
    }
}

