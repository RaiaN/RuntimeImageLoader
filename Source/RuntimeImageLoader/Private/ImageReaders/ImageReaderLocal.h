// Copyright 2022 Peter Leontev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ImageReaders/IImageReader.h"

class FImageReaderLocal : public IImageReader
{
public:
    virtual ~FImageReaderLocal() {}

    virtual bool ReadImage(const FString& ImageURI, TArray<uint8>& OutImageData) override;
    virtual FString GetLastError() const override;
    virtual void Flush() override;

private:
    FString OutError;
};