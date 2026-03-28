cbuffer FrameConstants : register(b0)
{
    row_major float4x4 ViewMatrix;
    row_major float4x4 ProjectionMatrix;
}

cbuffer FMeshUnlitConstants : register(b1)
{
    row_major float4x4 WorldMatrix;
};

struct VSInput
{
    float3 Position : POSITION;
    float4 Color : COLOR;
    float3 Normal : NORMAL;
    float2 UV : TEXCOORD;
};

struct PSInput
{
    float4 Position : SV_POSITION;
    float4 Color    : COLOR;
};

PSInput VSMain(VSInput In)
{
    PSInput Out;
    float4x4 MVP = mul(mul(WorldMatrix, ViewMatrix), ProjectionMatrix);
    Out.Position = mul(float4(In.Position, 1.0f), MVP);
    Out.Color = In.Color;
    return Out;
}

float4 PSMain(PSInput In) : SV_TARGET
{
    return In.Color;
}