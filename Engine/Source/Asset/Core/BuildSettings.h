#pragma once

#include <sstream>

#include "Asset/Core/Types.h"

struct FTextureBuildSettings
{
    bool bSRGB = true;
    bool bGenerateMips = false;

    FString ToKeyString() const
    {
        std::ostringstream Oss;
        Oss << "SRGB=" << (bSRGB ? 1 : 0) << ";Mips=" << (bGenerateMips ? 1 : 0);
        return Oss.str();
    }
};

struct FStaticMeshBuildSettings
{
    bool bRecomputeNormals = false;
    bool bFlipV = false;

    FString ToKeyString() const
    {
        std::ostringstream Oss;
        Oss << "RecomputeNormals=" << (bRecomputeNormals ? 1 : 0) << ";FlipV=" << (bFlipV ? 1 : 0);
        return Oss.str();
    }
};
