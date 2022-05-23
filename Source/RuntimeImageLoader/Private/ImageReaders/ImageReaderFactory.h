#pragma once

#include "CoreMinimal.h"
#include "ImageReaders/IImageReader.h"


class FImageReaderFactory
{
public:
    static TSharedPtr<IImageReader> CreateReader(const FString& ImageURI);
};