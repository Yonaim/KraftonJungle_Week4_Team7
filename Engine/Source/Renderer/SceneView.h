#pragma once

#include "Core/HAL/PlatformTypes.h"
#include "Core/Math/Matrix.h"
#include "Core/Math/Vector.h"

#include "Renderer/D3D11/D3D11Common.h"
#include <d3dcompiler.h>

// TODO: Reverse-Z 적용

class FEditorViewportClient;

enum class ELevelViewportType : uint8
{
    LVT_Perspective,
    LVT_OrthoTop,
    LVT_OrthoBottom,
    LVT_OrthoLeft,
    LVT_OrthoRight,
    LVT_OrthoFront,
    LVT_OrthoBack,
};

struct FViewportRect
{
    int32 X = 0;
    int32 Y = 0;
    int32 Width = 0;
    int32 Height = 0;
};

class FSceneView
{
  public:
    void SetViewMatrix(const FMatrix& InViewMatrix)
    {
        ViewMatrix = InViewMatrix;
        RebuildViewProjectionMatrix();
    }

    void SetProjectionMatrix(const FMatrix& InProjectionMatrix)
    {
        ProjectionMatrix = InProjectionMatrix;
        RebuildViewProjectionMatrix();
    }

    void SetViewRect(const FViewportRect& InViewRect) { ViewRect = InViewRect; }

    void SetViewLocation(const FVector& InViewLocation) { ViewLocation = InViewLocation; }

    void SetClipPlanes(float InNearZ, float InFarZ)
    {
        NearZ = InNearZ;
        FarZ = InFarZ;
    }

    const FMatrix&       GetViewMatrix() const { return ViewMatrix; }
    const FMatrix&       GetProjectionMatrix() const { return ProjectionMatrix; }
    const FMatrix&       GetViewProjectionMatrix() const { return ViewProjectionMatrix; }
    const FVector&       GetViewLocation() const { return ViewLocation; }
    const FViewportRect& GetViewRect() const { return ViewRect; }

    float GetCameraWidth() const { return static_cast<float>(ViewRect.Width); }
    float GetCameraHeight() const { return static_cast<float>(ViewRect.Height); }

    void SetViewportClient(FEditorViewportClient* newViewportClient) { ViewportClient = newViewportClient; }
    FEditorViewportClient* const& GetViewportClient() { return ViewportClient; }
    void RemoveViewportClient() { ViewportClient = nullptr; }
    bool IsValid() { return ViewportClient != nullptr; }

  private:
    void RebuildViewProjectionMatrix() { ViewProjectionMatrix = ViewMatrix * ProjectionMatrix; }

  private:
    FMatrix ViewMatrix;
    FMatrix ProjectionMatrix;
    FMatrix ViewProjectionMatrix;

    FVector ViewLocation;

    float NearZ = 0.1f;
    float FarZ = 1000.0f;

    FViewportRect ViewRect;
    FEditorViewportClient* ViewportClient = nullptr;
};