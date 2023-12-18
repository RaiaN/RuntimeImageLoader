// Copyright 2023 Petr Leontev. All Rights Reserved.

#include "CubemapUtils.h"
#include "ImageCore.h"
#include "Runtime/Launch/Resources/Version.h"

// transform world space vector to a space relative to the face
static FVector TransformSideToWorldSpace(uint32 CubemapFace, FVector InDirection)
{
    float x = InDirection.X, y = InDirection.Y, z = InDirection.Z;

    FVector Ret = FVector(0, 0, 0);

    // see http://msdn.microsoft.com/en-us/library/bb204881(v=vs.85).aspx
    switch (CubemapFace)
    {
        case 0: Ret = FVector(+z, -y, -x); break;
        case 1: Ret = FVector(-z, -y, +x); break;
        case 2: Ret = FVector(+x, +z, +y); break;
        case 3: Ret = FVector(+x, -z, -y); break;
        case 4: Ret = FVector(+x, -y, +z); break;
        case 5: Ret = FVector(-x, -y, -z); break;
        default:
            checkSlow(0);
    }

    // this makes it with the Unreal way (z and y are flipped)
    return FVector(Ret.X, Ret.Z, Ret.Y);
}

// transform vector relative to the face to world space
static FVector TransformWorldToSideSpace(uint32 CubemapFace, FVector InDirection)
{
    // undo Unreal way (z and y are flipped)
    float x = InDirection.X, y = InDirection.Z, z = InDirection.Y;

    FVector Ret = FVector(0, 0, 0);

    // see http://msdn.microsoft.com/en-us/library/bb204881(v=vs.85).aspx
    switch (CubemapFace)
    {
        case 0: Ret = FVector(-z, -y, +x); break;
        case 1: Ret = FVector(+z, -y, -x); break;
        case 2: Ret = FVector(+x, +z, +y); break;
        case 3: Ret = FVector(+x, -z, -y); break;
        case 4: Ret = FVector(+x, -y, +z); break;
        case 5: Ret = FVector(-x, -y, -z); break;
        default:
            checkSlow(0);
    }

    return Ret;
}

static FVector ComputeSSCubeDirectionAtTexelCenter(uint32 x, uint32 y, float InvSideExtent)
{
    // center of the texels
    FVector DirectionSS((x + 0.5f) * InvSideExtent * 2 - 1, (y + 0.5f) * InvSideExtent * 2 - 1, 1);
    DirectionSS.Normalize();
    return DirectionSS;
}

static FVector ComputeWSCubeDirectionAtTexelCenter(uint32 CubemapFace, uint32 x, uint32 y, float InvSideExtent)
{
    FVector DirectionSS = ComputeSSCubeDirectionAtTexelCenter(x, y, InvSideExtent);
    FVector DirectionWS = TransformSideToWorldSpace(CubemapFace, DirectionSS);
    return DirectionWS;
}

static uint32 ComputeLongLatCubemapExtents(const FImage& SrcImage, const uint32 MaxCubemapTextureResolution)
{
    return FMath::Clamp(1U << FMath::FloorLog2(SrcImage.SizeX / 2), 32U, MaxCubemapTextureResolution);
}

struct FImageView2D
{
    /** Pointer to colors in the slice. */
    FLinearColor* SliceColors;
    /** Width of the slice. */
    int32 SizeX;
    /** Height of the slice. */
    int32 SizeY;

    FImageView2D() : SliceColors(nullptr), SizeX(0), SizeY(0) {}

    /** Initialization constructor. */
    FImageView2D(FImage& Image, int32 SliceIndex)
    {
        SizeX = Image.SizeX;
        SizeY = Image.SizeY;
        SliceColors = (&Image.AsRGBA32F()[0]) + SliceIndex * SizeY * SizeX;
    }

    /** Access a single texel. */
    FLinearColor& Access(int32 X, int32 Y)
    {
        return SliceColors[X + Y * SizeX];
    }

    /** Const access to a single texel. */
    const FLinearColor& Access(int32 X, int32 Y) const
    {
        return SliceColors[X + Y * SizeX];
    }

    bool IsValid() const { return SliceColors != nullptr; }

    static const FImageView2D ConstructConst(const FImage& Image, int32 SliceIndex)
    {
        return FImageView2D(const_cast<FImage&>(Image), SliceIndex);
    }

};

struct FImageViewLongLat
{
    /** Image colors. */
    FLinearColor* ImageColors;
    /** Width of the image. */
    int32 SizeX;
    /** Height of the image. */
    int32 SizeY;

    /** Initialization constructor. */
    explicit FImageViewLongLat(FImage& Image, int32 SliceIndex)
    {
        SizeX = Image.SizeX;
        SizeY = Image.SizeY;
        ImageColors = (&Image.AsRGBA32F()[0]) + SliceIndex * SizeY * SizeX;
    }

    /** Wraps X around W. */
    static void WrapTo(int32& X, int32 W)
    {
        X = X % W;

        if (X < 0)
        {
            X += W;
        }
    }

    /** Const access to a texel. */
    FLinearColor Access(int32 X, int32 Y) const
    {
        return ImageColors[X + Y * SizeX];
    }

    /** Makes a filtered lookup. */
    FLinearColor LookupFiltered(float X, float Y) const
    {
        int32 X0 = (int32)floor(X);
        int32 Y0 = (int32)floor(Y);

        float FracX = X - X0;
        float FracY = Y - Y0;

        int32 X1 = X0 + 1;
        int32 Y1 = Y0 + 1;

        WrapTo(X0, SizeX);
        WrapTo(X1, SizeX);
        Y0 = FMath::Clamp(Y0, 0, (int32)(SizeY - 1));
        Y1 = FMath::Clamp(Y1, 0, (int32)(SizeY - 1));

        FLinearColor CornerRGB00 = Access(X0, Y0);
        FLinearColor CornerRGB10 = Access(X1, Y0);
        FLinearColor CornerRGB01 = Access(X0, Y1);
        FLinearColor CornerRGB11 = Access(X1, Y1);

        FLinearColor CornerRGB0 = FMath::Lerp(CornerRGB00, CornerRGB10, FracX);
        FLinearColor CornerRGB1 = FMath::Lerp(CornerRGB01, CornerRGB11, FracX);

        return FMath::Lerp(CornerRGB0, CornerRGB1, FracY);
    }

    /** Makes a filtered lookup using a direction. */
    FLinearColor LookupLongLat(FVector NormalizedDirection) const
    {
        // see http://gl.ict.usc.edu/Data/HighResProbes
        // latitude-longitude panoramic format = equirectangular mapping

        float X = (1 + atan2(NormalizedDirection.X, -NormalizedDirection.Z) / PI) / 2 * SizeX;
        float Y = acos(NormalizedDirection.Y) / PI * SizeY;

        return LookupFiltered(X, Y);
    }
};

void GenerateBaseCubeMipFromLongitudeLatitude2D(FImage* OutMip, const FImage& SrcImage, const uint32 MaxCubemapTextureResolution, uint8 SourceEncodingOverride)
{
    FImage LongLatImage;

#if ENGINE_MAJOR_VERSION < 5
    SrcImage.CopyTo(LongLatImage, ERawImageFormat::RGBA32F, EGammaSpace::Linear);
#else
    SrcImage.Linearize(SourceEncodingOverride, LongLatImage);
#endif

    // TODO_TEXTURE: Expose target size to user.
    uint32 Extent = ComputeLongLatCubemapExtents(LongLatImage, MaxCubemapTextureResolution);
    float InvExtent = 1.0f / Extent;
    OutMip->Init(Extent, Extent, SrcImage.NumSlices * 6, ERawImageFormat::RGBA32F, EGammaSpace::Linear);

    for (int32 Slice = 0; Slice < SrcImage.NumSlices; ++Slice)
    {
        FImageViewLongLat LongLatView(LongLatImage, Slice);
        for (uint32 Face = 0; Face < 6; ++Face)
        {
            FImageView2D MipView(*OutMip, Slice * 6 + Face);
            for (uint32 y = 0; y < Extent; ++y)
            {
                for (uint32 x = 0; x < Extent; ++x)
                {
                    FVector DirectionWS = ComputeWSCubeDirectionAtTexelCenter(Face, x, y, InvExtent);
                    MipView.Access(x, y) = LongLatView.LookupLongLat(DirectionWS);
                }
            }
        }
    }
}
