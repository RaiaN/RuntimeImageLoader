#pragma once

#include "CoreMinimal.h"
#include "ImageReaders/IImageReader.h"


class FImageReaderFactory
{
public:
    static TSharedPtr<IImageReader, ESPMode::ThreadSafe> CreateReader(const FString& ImageURI);
};