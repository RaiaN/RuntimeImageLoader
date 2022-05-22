// Copyright 2022 Peter Leontev. All Rights Reserved.

#include "HttpDownloader.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "HttpModule.h"

FImageHttpDownload::FImageHttpDownload(const FString& InImageURI, TArray<uint8>& OutImageData)
: ImageURI(InImageURI), ImageData(OutImageData)
{
    if (DownloadFuture.IsValid())
    {
        ensure(false);
        return;
    }

    DownloadFuture = MakeShared<TFutureState<bool>, ESPMode::ThreadSafe>();

    // Create the Http request and add to pending request list
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();

    HttpRequest->OnProcessRequestComplete().BindRaw(this, &FImageHttpDownload::HandleImageRequest);
    HttpRequest->SetURL(ImageURI);
    HttpRequest->SetVerb(TEXT("GET"));
    HttpRequest->ProcessRequest();
}

void FImageHttpDownload::Wait()
{
    DownloadFuture->GetResult();
}

void FImageHttpDownload::HandleImageRequest(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
    if (HttpResponse->GetResponseCode() == 200)
    {
        ImageData = HttpResponse->GetContent();
    }

    DownloadFuture->EmplaceResult(true);
}

