// Copyright 2023 Petr Leontev. All Rights Reserved.

#include "ImageReaderLocal.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Stats/Stats.h"

TArray<uint8> FImageReaderLocal::ReadImage(const FString& ImageURI)
{
    QUICK_SCOPE_CYCLE_COUNTER(STAT_RuntimeImageUtils_ImportFileAsTexture);

    // TODO:
    const int64 MAX_FILESIZE_BYTES = 999999999;

    IFileManager& FileManager = IFileManager::Get();
    if (!FileManager.FileExists(*ImageURI))
    {
        OutError = FString::Printf(TEXT("Image does not exist: %s"), *ImageURI);
        return TArray<uint8>();
    }

    const int64 ImageFileSizeBytes = FileManager.FileSize(*ImageURI);
    check(ImageFileSizeBytes != INDEX_NONE);

    // check filesize
    if (ImageFileSizeBytes > MAX_FILESIZE_BYTES)
    {
        OutError = FString::Printf(TEXT("Image filesize > %d MBs): %s"), MAX_FILESIZE_BYTES, *ImageURI);
        return TArray<uint8>();
    }

    QUICK_SCOPE_CYCLE_COUNTER(STAT_FImageReaderLocal_LoadFileToArray);
    if (!FFileHelper::LoadFileToArray(OutImageData, *ImageURI))
    {
        OutError = FString::Printf(TEXT("Image loading I/O error: %s"), *ImageURI);
        return TArray<uint8>();
    }
    OutImageData.Add(0);

    return OutImageData;
}

FString FImageReaderLocal::GetLastError() const
{
    return OutError;
}

void FImageReaderLocal::Flush()
{
    // TODO: need to implement async file reading first
}

void FImageReaderLocal::Cancel()
{
    // TODO: need to implement async file reading first
}
