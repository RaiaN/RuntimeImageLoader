// Copyright 2022 Peter Leontev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Async/Future.h"
#include "RuntimeTextureFactory.generated.h"

class UTexture2D;
class UTextureCube;
struct FRuntimeImageData;

struct FConstructTextureTask
{
    FString ImageFilename;
    FRuntimeImageData* ImageData;
};

UCLASS()
class URuntimeTextureFactory : public UObject
{
    GENERATED_BODY()

public:
    void Flush();

public:
    UTexture2D* CreateTexture2D(const FConstructTextureTask& Task);
    UTextureCube* CreateTextureCube(const FConstructTextureTask& Task);

private:
    TFuture<bool> CurrentTask;
};