// Copyright 2022 Peter Leontev. All Rights Reserved.

#include "ImageReaderLocal.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"

bool FImageReaderLocal::ReadImage(const FString& ImageURI, TArray<uint8>& OutImageData)
{
    QUICK_SCOPE_CYCLE_COUNTER(STAT_RuntimeImageUtils_ImportFileAsTexture);

    // TODO:
    const int64 MAX_FILESIZE_BYTES = 999999999;

    IFileManager& FileManager = IFileManager::Get();
    if (!FileManager.FileExists(*ImageURI))
    {
        OutError = FString::Printf(TEXT("Image does not exist: %s"), *ImageURI);
        return false;
    }

    const int64 ImageFileSizeBytes = FileManager.FileSize(*ImageURI);
    check(ImageFileSizeBytes != INDEX_NONE);

    // check filesize
    if (ImageFileSizeBytes > MAX_FILESIZE_BYTES)
    {
        OutError = FString::Printf(TEXT("Image filesize > %d MBs): %s"), MAX_FILESIZE_BYTES, *ImageURI);
        return false;
    }

    QUICK_SCOPE_CYCLE_COUNTER(STAT_FImageReaderLocal_LoadFileToArray);
    if (!FFileHelper::LoadFileToArray(OutImageData, *ImageURI))
    {
        OutError = FString::Printf(TEXT("Image loading I/O error: %s"), *ImageURI);
        return false;
    }
    OutImageData.Add(0);

    return true;
}

FString FImageReaderLocal::GetLastError() const
{
    return OutError;
}
