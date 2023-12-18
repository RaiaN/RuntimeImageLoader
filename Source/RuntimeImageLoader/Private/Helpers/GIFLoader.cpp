// Copyright 2023 Petr Leontev. All Rights Reserved.

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

    if (FWEBPGIFLoader::HasValidWebpHeader(GifData))
    {
        return MakeUnique<FWEBPGIFLoader>();
    }

    // construct .gif loader as it is more popular than .webp
    return MakeUnique<FNSGIFLoader>();
}