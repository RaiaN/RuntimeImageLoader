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
#include "Async/Async.h"

#include "ImageReaders/ImageReaderFactory.h"
#include "ImageReaders/IImageReader.h"
#include "TextureFactory/RuntimeTextureResource.h"
#include "TextureFactory/RuntimeRHITexture2DFactory.h"
#include "TextureFactory/RuntimeRHITextureCubeFactory.h"
#include "TextureFactory/RuntimeTextureFactory.h"
#include "RuntimeImageUtils.h"


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
    Results.Empty();

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
            FImageReadResult& ReadResult = Results.Emplace_GetRef();
            ReadResult.ImageFilename = Request.ImageFilename;

            ImageReader = FImageReaderFactory::CreateReader(Request.ImageFilename);

            TArray<uint8> ImageBuffer;
            if (!ImageReader->ReadImage(Request.ImageFilename, ImageBuffer))
            {
                ReadResult.OutError = FString::Printf(TEXT("Failed to read %s image. Error: %s"), *Request.ImageFilename, *ImageReader->GetLastError());
                continue;
            }

            FRuntimeImageData ImageData;
            if (!FRuntimeImageUtils::ImportBufferAsImage(ImageBuffer.GetData(), ImageBuffer.Num(), ImageData, ReadResult.OutError))
            {
                continue;
            }

            if (ReadResult.OutError.Len() > 0)
            {
                continue;
            }

            // sanity checks
            check(ImageData.RawData.Num() > 0);
            check(ImageData.TextureSourceFormat != TSF_Invalid);

            ImageData.PixelFormat = DeterminePixelFormat(ImageData.Format, Request.TransformParams);
            if (ImageData.PixelFormat == PF_Unknown)
            {
                ReadResult.OutError = FString::Printf(TEXT("Pixel format is not supported: %d"), (int32)ImageData.PixelFormat);
                continue;
            }

            ApplyTransformations(ImageData, Request.TransformParams);

            if (ImageData.TextureSourceFormat == TSF_BGRE8)
            {
                ReadResult.OutTextureCube = TextureFactory->CreateTextureCube({ Request.ImageFilename, &ImageData });

                // TODO: Add cancel()?
                FRuntimeRHITextureCubeFactory RHITextureCubeFactory(ReadResult.OutTextureCube, ImageData);
                RHITextureCubeFactory.Create();
            }
            else
            {
                ReadResult.OutTexture = TextureFactory->CreateTexture2D({ Request.ImageFilename, &ImageData });

                // TODO: Add cancel()?
                FRuntimeRHITexture2DFactory RHITexture2DFactory(ReadResult.OutTexture, ImageData);
                RHITexture2DFactory.Create();
            }
        }

        bCompletedWork.AtomicSet(Requests.IsEmpty());
    }
}

EPixelFormat URuntimeImageReader::DeterminePixelFormat(ERawImageFormat::Type ImageFormat, const FTransformImageParams& Params) const
{
    EPixelFormat PixelFormat;
    
    // determine pixel format
    switch (ImageFormat)
    {
        case ERawImageFormat::G8:            PixelFormat = (Params.bForUI) ? PF_B8G8R8A8 : PF_G8; break;
        case ERawImageFormat::G16:           PixelFormat = PF_G16; break;
        case ERawImageFormat::BGRA8:         PixelFormat = PF_B8G8R8A8; break;
        case ERawImageFormat::BGRE8:         PixelFormat = PF_B8G8R8A8; break;
        case ERawImageFormat::RGBA16:        PixelFormat = (Params.bForUI) ? PF_B8G8R8A8 : PF_R16G16B16A16_SINT; break;
        case ERawImageFormat::RGBA16F:       PixelFormat = PF_FloatRGBA; break;
        default:                             PixelFormat = PF_Unknown; break;
    }

    return PixelFormat;
}

void URuntimeImageReader::ApplyTransformations(FRuntimeImageData& ImageData, FTransformImageParams TransformParams)
{
    if (!TransformParams.IsPercentSizeValid())
    {
        UE_LOG(LogRuntimeImageReader, Verbose, TEXT("Supplied transform params are not valid! PercentSizeX, PercentSizeX: (%d, %d)"), TransformParams.PercentSizeX, TransformParams.PercentSizeY);
    }
    
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

    if (TransformParams.bForUI)
    {
        // no need to convert float RGBA
        if (ImageData.TextureSourceFormat != TSF_RGBA16F)
        {
            FImage BGRAImage;
            BGRAImage.Init(ImageData.SizeX, ImageData.SizeY, ERawImageFormat::BGRA8);
            ImageData.CopyTo(BGRAImage, ERawImageFormat::BGRA8, EGammaSpace::sRGB);

            ImageData.RawData = MoveTemp(BGRAImage.RawData);
            ImageData.SRGB = true;
            ImageData.GammaSpace = EGammaSpace::sRGB;
        }
    }
}
