Texture2D MaterialTexture : register(t0);
SamplerState NormalSampler : register(s0);

cbuffer FrameConstants : register(b0)
{
    row_major float4x4 ViewMatrix;
    row_major float4x4 ProjectionMatrix;
}

cbuffer ObjectConstants : register(b1)
{
    row_major float4x4 WorldMatrix;
    uint ObjectId;
    float2 UVOffset;
    uint Padding2;
};

struct VSInput
{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float4 Color    : COLOR;
    float2 UV       : TEXCOORD;
};

struct PSInput
{
    float4 Position : SV_POSITION;
    float3 Normal   : NORMAL;
    float4 Color    : COLOR;
    float2 UV       : TEXCOORD;
};

PSInput VSMain(VSInput In)
{
    PSInput Out;
    float4x4 MVP = mul(mul(WorldMatrix, ViewMatrix), ProjectionMatrix);
    Out.Position = mul(float4(In.Position, 1.0f), MVP);
    Out.Normal = mul(float4(In.Normal, 0.0f), WorldMatrix);
    Out.Color = In.Color;
    Out.UV = In.UV + UVOffset;
    return Out;
}

float4 PSMain(PSInput In) : SV_TARGET
{   
    float4 TexColor = MaterialTexture.Sample(NormalSampler, In.UV);
    
    return TexColor * In.Color;
}
