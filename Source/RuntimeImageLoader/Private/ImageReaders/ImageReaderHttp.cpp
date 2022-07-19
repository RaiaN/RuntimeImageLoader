// Copyright 2022 Peter Leontev. All Rights Reserved.

#include "ImageReaderHttp.h"
#include "Launch/Resources/Version.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "HTTPManager.h"
#include "HttpModule.h"

bool FImageReaderHttp::ReadImage(const FString& ImageURI, TArray<uint8>& OutImageData)
{
    ImageData = &OutImageData;
    
    if (DownloadFuture.IsValid())
    {
        ensure(false);
        return false;
    }

    DownloadFuture = MakeShared<TFutureState<bool>, ESPMode::ThreadSafe>();

    // Create the Http request and add to pending request list
    CurrentHttpRequest = FHttpModule::Get().CreateRequest();
    {
        CurrentHttpRequest->OnProcessRequestComplete().BindRaw(this, &FImageReaderHttp::HandleImageRequest);
        CurrentHttpRequest->SetURL(ImageURI);
        CurrentHttpRequest->SetVerb(TEXT("GET"));
        CurrentHttpRequest->SetTimeout(60.0f);
        CurrentHttpRequest->ProcessRequest();
    }

    if (IsInGameThread())
    {
        Flush();
    }

    bool bResult = DownloadFuture->GetResult();

    return bResult;
}

FString FImageReaderHttp::GetLastError() const
{
    return OutError;
}

void FImageReaderHttp::Flush()
{
#if ENGINE_MAJOR_VERSION < 5
    FHttpModule::Get().GetHttpManager().Flush(false);
#else
    FHttpModule::Get().GetHttpManager().Flush(EHttpFlushReason::Default);
#endif
}

void FImageReaderHttp::Cancel()
{
    if (CurrentHttpRequest.IsValid() && DownloadFuture.IsValid() && !DownloadFuture->IsComplete())
    {
        CurrentHttpRequest->CancelRequest();
    }
}

void FImageReaderHttp::HandleImageRequest(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
    if (HttpResponse->GetResponseCode() == 200)
    {
        ImageData->Append(HttpResponse->GetContent().GetData(), HttpResponse->GetContentLength());
        DownloadFuture->EmplaceResult(true);
    }
    else
    {
        OutError = FString::Printf(TEXT("Error code: %d, Content: %d"), HttpResponse->GetResponseCode(), *HttpResponse->GetContentAsString());
        DownloadFuture->EmplaceResult(false);
    }

    CurrentHttpRequest = nullptr;
    DownloadFuture = nullptr;
}