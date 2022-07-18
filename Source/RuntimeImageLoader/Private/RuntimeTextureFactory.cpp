// Copyright 2022 Peter Leontev. All Rights Reserved.

#include "RuntimeTextureFactory.h"
#include "RuntimeImageUtils.h"

void URuntimeTextureFactory::Flush()
{
    CurrentTask.Reset();
}

UTexture2D* URuntimeTextureFactory::CreateTexture2D(const FConstructTextureTask& Task)
{
    UTexture2D* OutResult = nullptr;
    
    CurrentTask = Async(
        EAsyncExecution::TaskGraphMainThread,
        [Task, &OutResult]()
        {
            OutResult = FRuntimeImageUtils::CreateTexture(Task.ImageFilename, *Task.ImageData);

            return IsValid(OutResult);
        }
    );

    bool bResult = CurrentTask.Get();

    return OutResult;
}

UTextureCube* URuntimeTextureFactory::CreateTextureCube(const FConstructTextureTask& Task)
{
    return nullptr;
}
