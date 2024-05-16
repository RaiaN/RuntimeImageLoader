// Copyright 2023 Petr Leontev. All Rights Reserved.

#include "RuntimeTextureFactory.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureCube.h"
#include "Async/Async.h"
#include "RuntimeImageUtils.h"

void URuntimeTextureFactory::Cancel()
{
    CurrentTask.Reset();
}

UTexture2D* URuntimeTextureFactory::CreateTexture2D(const FConstructTextureTask& Task)
{
    UTexture2D* OutResult = nullptr;

    if (IsInGameThread())
    {
        return FRuntimeImageUtils::CreateTexture(Task.ImageFilename, *Task.ImageData);
    }
    
    if (IsEngineExitRequested())
    {
        return nullptr;
    }

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
    UTextureCube* OutResult = nullptr;

    if (IsInGameThread())
    {
        return FRuntimeImageUtils::CreateTextureCube(Task.ImageFilename, *Task.ImageData);
    }

    CurrentTask = Async(
        EAsyncExecution::TaskGraphMainThread,
        [Task, &OutResult]()
        {
            OutResult = FRuntimeImageUtils::CreateTextureCube(Task.ImageFilename, *Task.ImageData);

            return IsValid(OutResult);
        }
    );

    bool bResult = CurrentTask.Get();

    return OutResult;
}
