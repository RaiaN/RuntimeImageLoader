// Copyright 2023 Unreal Solutions Ltd. All Rights Reserved.

#include "GIFLoader.h"
#include "NSGIFLoader.h"
#include "WEBPGIFLoader.h"


TUniquePtr<IGIFLoader> FGIFLoaderFactory::CreateLoader(const FString& GifURI, const TArray<uint8>& GifData)
{
    if (GifURI.EndsWith(".gif"))
    {
        return MakeUnique<FNSGIFLoader>();
    }
    else if (GifURI.EndsWith(".webp"))
    {
        return MakeUnique<FWEBPGIFLoader>();
    }

    // figure out GIF format using libs API

    return nullptr;
}