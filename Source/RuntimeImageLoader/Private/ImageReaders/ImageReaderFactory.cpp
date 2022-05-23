#include "ImageReaderFactory.h"
#include "ImageReaderLocal.h"
#include "ImageReaderHttp.h"

TSharedPtr<IImageReader> FImageReaderFactory::CreateReader(const FString& ImageURI)
{
    if (ImageURI.StartsWith("http://") || ImageURI.StartsWith("https://"))
    {
        return MakeShared<FImageReaderHttp>();
    }

    return MakeShared<FImageReaderLocal>();
}
