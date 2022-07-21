// Copyright 2022 Peter Leontev. All Rights Reserved.

#include "RuntimeRHITextureCubeFactory.h"
#include "DynamicRHI.h"
#include "RHI.h"

FRuntimeRHITextureCubeFactory::FRuntimeRHITextureCubeFactory(UTextureCube* InTextureCube, const FRuntimeImageData& InImageData)
: NewTextureCube(InTextureCube), ImageData(InImageData)
{}


/* Faces ordered this way
enum ECubeFace
{
    CubeFace_PosX = 0,
    CubeFace_NegX,
    CubeFace_PosY,
    CubeFace_NegY,
    CubeFace_PosZ,
    CubeFace_NegZ,
    CubeFace_MAX
};
*/

struct FTextureCubeDataResource : public FResourceBulkDataInterface
{
public:
    FTextureCubeDataResource(void* InMipData, int32 InDataSize)
        : MipData(InMipData), DataSize(InDataSize)
    {}

    const void* GetResourceBulkData() const override { return MipData; }
    uint32 GetResourceBulkDataSize() const override { return DataSize; }
    void Discard() override {}

private:
    void* MipData;
    int32 DataSize;
};

FTextureCubeRHIRef FRuntimeRHITextureCubeFactory::Create()
{
    FTextureCubeRHIRef TextureCubeRHI = nullptr;

    uint32 NumMips = 1;
    uint32 NumSamples = 1;

    ensureMsgf(ImageData.SizeX > 0, TEXT("ImageData.SizeX must be > 0"));
    ensureMsgf(ImageData.SizeY > 0, TEXT("ImageData.SizeY must be > 0"));

    ETextureCreateFlags TextureFlags = TexCreate_ShaderResource;
    TextureFlags |= ImageData.SRGB ? TexCreate_SRGB : TexCreate_None;

    FRHIResourceCreateInfo CreateInfo;
    {
        void* Mip0Data = (void*)ImageData.RawData.GetData();
        FTextureCubeDataResource TextureData(Mip0Data, ImageData.RawData.Num());

        CreateInfo.BulkData = &TextureData;
    }


    ERHIAccess ResourceState = RHIGetDefaultResourceState(TextureFlags, true);

    TextureCubeRHI = GDynamicRHI->RHICreateTextureCube(ImageData.SizeX, ImageData.PixelFormat, 1, TextureFlags, ResourceState, CreateInfo);
    
    return TextureCubeRHI;
}

