// Copyright 2023 Petr Leontev. All Rights Reserved.

#include "ImageReaderHttp.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "HttpManager.h"
#include "HttpModule.h"

FImageReaderHttp::~FImageReaderHttp()
{
    DownloadFuture = nullptr;
}

TArray<uint8> FImageReaderHttp::ReadImage(const FString& ImageURI)
{
    check (!DownloadFuture.IsValid());

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
    if (bResult)
    {
        return OutImageData;
    }
    return TArray<uint8>();
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
        CurrentHttpRequest->OnProcessRequestComplete().Unbind();
        CurrentHttpRequest->CancelRequest();
        DownloadFuture->EmplaceResult(false);
    }
}

void FImageReaderHttp::HandleImageRequest(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
    bool bSuccess = HttpResponse->GetResponseCode() == 200;
    
    if (bSuccess)
    {
        OutImageData.Append(HttpResponse->GetContent().GetData(), HttpResponse->GetContentLength());
    }
    else
    {
        OutError = FString::Printf(TEXT("Error code: %d, Content: %d"), HttpResponse->GetResponseCode(), *HttpResponse->GetContentAsString());
    }

    if (DownloadFuture && !DownloadFuture->IsComplete())
    {
        DownloadFuture->EmplaceResult(bSuccess);
    }
}