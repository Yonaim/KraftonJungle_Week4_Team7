#include "ViewerNavigationController.h"

void FViewerNavigationController::Tick(float /*DeltaTime*/)
{
    // 필요시 카메라 위치 보간 등 구현
}

void FViewerNavigationController::ResetView(const FVector& InLocation, const FVector& InPivot)
{
    OrbitPivot = InPivot;
    FVector Forward = (OrbitPivot - InLocation).GetSafeNormal();

    // 초기 각도 추출 (이건 딱 한 번만 수행)
    CurrentPitch = FMath::RadiansToDegrees(std::asin(FMath::Clamp(Forward.Z, -1.0f, 1.0f)));
    CurrentYaw = FMath::RadiansToDegrees(std::atan2(Forward.Y, Forward.X));
    OrbitRadius = (InLocation - OrbitPivot).Size();
}

void FViewerNavigationController::BeginOrbit(const FVector2& MousePos)
{
    bOrbiting = true;
    LastMousePos = MousePos;
}

void FViewerNavigationController::UpdateOrbit(const FVector2& MousePos)
{
    if (!bOrbiting || !ViewportCamera)
        return;

    float DeltaX = MousePos.X - LastMousePos.X;
    float DeltaY = MousePos.Y - LastMousePos.Y;
    LastMousePos = MousePos; // 현재 좌표를 저장해서 다음 프레임의 기준점으로 사용

    CurrentYaw += DeltaX * OrbitSensitivity;
    CurrentPitch = FMath::Clamp(CurrentPitch + DeltaY * OrbitSensitivity, -89.0f, 89.0f);

    ApplyOrbitTransform();
}

void FViewerNavigationController::EndOrbit() { bOrbiting = false; }

void FViewerNavigationController::BeginPan(const FVector2& MousePos)
{
    if (!ViewportCamera)
        return;

    bPanning = true;
    LastMousePos = MousePos;
}

void FViewerNavigationController::UpdatePan(const FVector2& MousePos)
{
    if (!bPanning || !ViewportCamera)
        return;

    float DeltaX = MousePos.X - LastMousePos.X;
    float DeltaY = MousePos.Y - LastMousePos.Y;
    LastMousePos = MousePos; 

    FVector Right = ViewportCamera->GetRightVector();
    FVector Up = ViewportCamera->GetUpVector();

    // PanSensitivity에 OrbitRadius를 곱해주면 거리감에 따른 속도가 자연스러워집니다.
    float   DynamicSensitivity = PanSensitivity * (OrbitRadius * 0.1f);
    FVector PanDelta = (Right * -DeltaX * DynamicSensitivity) + (Up * DeltaY * DynamicSensitivity);

    // 4. 핵심: 카메라와 피봇을 동시에 이동!
    OrbitPivot += PanDelta;
    ViewportCamera->SetLocation(ViewportCamera->GetLocation() + PanDelta);

}

void FViewerNavigationController::EndPan() { bPanning = false; }

void FViewerNavigationController::Zoom(float Delta)
{
    if (!ViewportCamera)
        return;
    FVector Forward = ViewportCamera->GetForwardVector();
    FVector NewLocation = ViewportCamera->GetLocation() + Forward * Delta * ZoomStep;
    ViewportCamera->SetLocation(NewLocation);
}

void FViewerNavigationController::ApplyOrbitTransform() 
{
    if (!ViewportCamera)
        return;

    float RadYaw = FMath::DegreesToRadians(CurrentYaw);
    float RadPitch = FMath::DegreesToRadians(CurrentPitch);

    FVector NewLocation;
    float   CosPitch = std::cos(RadPitch);

    NewLocation.X = OrbitPivot.X + OrbitRadius * CosPitch * std::cos(RadYaw);
    NewLocation.Y = OrbitPivot.Y + OrbitRadius * CosPitch * std::sin(RadYaw);
    NewLocation.Z = OrbitPivot.Z + OrbitRadius * std::sin(RadPitch);

    ViewportCamera->SetLocation(NewLocation);

    FVector Forward = (OrbitPivot - NewLocation).GetSafeNormal();

    FVector WorldUp = FVector::UpVector;
    if (FMath::Abs(FVector::DotProduct(Forward, WorldUp)) > 0.99f)
    {
        WorldUp = FVector(0.0f, 1.0f, 0.0f);
    }

    FVector Right = FVector::CrossProduct(WorldUp, Forward).GetSafeNormal();
    FVector Up = FVector::CrossProduct(Forward, Right);

    FMatrix RotationMatrix = FMatrix::Identity;
    RotationMatrix.SetAxes(Forward, Right, Up);

    FQuat NewQuat(RotationMatrix);
    NewQuat.Normalize();

    ViewportCamera->SetRotation(NewQuat);
}
