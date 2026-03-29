#pragma once

#include <memory>

#include "Asset/Data/MaterialCookedData.h"
#include "Engine/Asset/MaterialInterface.h"

class UMaterial : public UMaterialInterface
{
    DECLARE_RTTI(UMaterial, UMaterialInterface)

  public:
    void SetCookedData(const std::shared_ptr<FMaterialCookedData>& InCookedData)
    {
        CookedData = InCookedData;
    }
    std::shared_ptr<FMaterialCookedData> GetCookedData() const { return CookedData; }

  private:
    std::shared_ptr<FMaterialCookedData> CookedData;
};
