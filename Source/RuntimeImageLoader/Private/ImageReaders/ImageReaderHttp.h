// Copyright 2023 Petr Leontev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"
#include "Async/Future.h"
#include "ImageReaders/IImageReader.h"

class FImageReaderHttp : public IImageReader
{
public:
    virtual ~FImageReaderHttp();

    virtual TArray<uint8> ReadImage(const FString& ImageURI) override;
    virtual FString GetLastError() const override;
    virtual void Flush() override;
    virtual void Cancel() override;

private:
    /** Handles image requests coming from the web */
    void HandleImageRequest(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);

private:
    TSharedPtr<TFutureState<bool>, ESPMode::ThreadSafe> DownloadFuture;

    TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> CurrentHttpRequest;

    TArray<uint8> OutImageData;
    FString OutError;
};