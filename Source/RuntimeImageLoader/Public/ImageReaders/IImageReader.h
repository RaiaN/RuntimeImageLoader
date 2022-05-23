// Copyright 2022 Peter Leontev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class IImageReader
{
public:
    virtual bool ReadImage(const FString& ImageURI, TArray<uint8>& OutImageData) = 0;
    virtual FString GetLastError() const { return TEXT(""); };
};