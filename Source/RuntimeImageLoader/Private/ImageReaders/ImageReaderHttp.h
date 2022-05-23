// Copyright 2022 Peter Leontev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"
#include "Async/Future.h"
#include "ImageReaders/IImageReader.h"

class FImageReaderHttp : public IImageReader
{
public:
    virtual ~FImageReaderHttp() {}

    virtual bool ReadImage(const FString& ImageURI, TArray<uint8>& OutImageData) override;

private:
    /** Handles image requests coming from the web */
    void HandleImageRequest(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);

private:
    TArray<uint8>* ImageData;
    TSharedPtr<TFutureState<bool>, ESPMode::ThreadSafe> DownloadFuture;

    FString OutError;
};