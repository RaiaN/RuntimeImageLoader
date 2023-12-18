// Copyright 2023 Petr Leontev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class IImageReader
{
public:
    virtual TArray<uint8> ReadImage(const FString& ImageURI) = 0;
    virtual FString GetLastError() const { return TEXT(""); };
    virtual void Flush() = 0;
    virtual void Cancel() = 0;
};

class IGIFReader
{
public:
    virtual uint8* ReadGIF(const FString& ImageURI) = 0;
    virtual FString GetLastError() const { return TEXT(""); };
    virtual void Flush() = 0;
    virtual void Cancel() = 0;
};
