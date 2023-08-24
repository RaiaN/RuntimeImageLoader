#include "ImageReaderFactory.h"
#include "ImageReaderLocal.h"
#include "ImageReaderHttp.h"

TSharedPtr<IImageReader, ESPMode::ThreadSafe> FImageReaderFactory::CreateReader(const FString& ImageURI)
{
    if (ImageURI.StartsWith("http://") || ImageURI.StartsWith("https://"))
    {
        return MakeShared<FImageReaderHttp, ESPMode::ThreadSafe>();
    }

    return MakeShared<FImageReaderLocal, ESPMode::ThreadSafe>();
}
