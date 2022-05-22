// Copyright 2022 Peter Leontev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"
#include "Async/Future.h"

class FImageHttpDownload
{
public:
    FImageHttpDownload(const FString& ImageURI, TArray<uint8>& OutImageData);
    void Wait();

private:
    /** Handles image requests coming from the web */
    void HandleImageRequest(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);

private:
    FString ImageURI;
    TArray<uint8>& ImageData;

    TSharedPtr<TFutureState<bool>, ESPMode::ThreadSafe> DownloadFuture;
};