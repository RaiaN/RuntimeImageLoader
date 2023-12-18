// Copyright 2023 Petr Leontev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ImageReaders/IImageReader.h"

class FImageReaderLocal : public IImageReader
{
public:
    virtual ~FImageReaderLocal() {}

    virtual TArray<uint8> ReadImage(const FString& ImageURI) override;
    virtual FString GetLastError() const override;
    virtual void Flush() override;
    virtual void Cancel() override;

private:
    TArray<uint8> OutImageData;
    FString OutError;
};