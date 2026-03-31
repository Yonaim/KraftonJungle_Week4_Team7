#pragma once
struct FCameraInfo
{
    FVector  Location;
    FRotator Rotation;
    float    FOV = 60.f;
    float    NearClip = 0.1f;
    float    FarClip = 1000.f;
};