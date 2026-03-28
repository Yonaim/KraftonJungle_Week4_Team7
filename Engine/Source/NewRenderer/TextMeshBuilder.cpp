#include "NewRenderer/TextMeshBuilder.h"
#include "NewRenderer/Shader.h"
#include "NewRenderer/Material.h"
#include "NewRenderer/ShaderMap.h"
#include "NewRenderer/Renderer.h"
#include "NewRenderer/RenderStateManager.h"
#include "Primitive/PrimitiveBase.h"
#include "Core/Path.h"
#include <cstring>

#include "Renderer/RenderAsset/FontResource.h"

CTextMeshBuilder::~CTextMeshBuilder()
{
    Release();
}

bool CTextMeshBuilder::Initialize(CRenderer* InRenderer)
{
    Release();

    if (!InRenderer)
    {
        return false;
    }

    Device = InRenderer->GetDevice();
    DeviceContext = InRenderer->GetDeviceContext();
    RenderStateManager = InRenderer->GetRenderStateManager().get();
    if (!Device || !DeviceContext)
    {
        return false;
    }

    // 폰트 아틀라스 초기화
    // 임시 Dir 경로
    std::wstring FontPath = (FPaths::AppRoot() / "Fonts/NotoSansKR_Atlas.png").wstring();
    // if (!Atlas.Initialize(Device, DeviceContext, FontPath))
    // {
    // 	return false;
    // }

    // 전용 머티리얼 구성
    const std::wstring ShaderDir = FPaths::AppRoot(); // 임시 Dir 경로
    const std::wstring VSPath = ShaderDir + L"FontVertexShader.hlsl";
    const std::wstring PSPath = ShaderDir + L"FontPixelShader.hlsl";

    auto VS = FShaderMap::Get().GetOrCreateVertexShader(Device, VSPath, L"main");
    auto PS = FShaderMap::Get().GetOrCreatePixelShader(Device, PSPath, L"main");

    FontMaterial = std::make_shared<FMaterial>();
    FontMaterial->SetOriginName("M_Font");
    FontMaterial->SetVertexShader(VS);
    FontMaterial->SetPixelShader(PS);

    // 래스터라이저 설정
    FRasterizerStateOption rasterizerOption;
    rasterizerOption.FillMode = D3D11_FILL_SOLID;
    rasterizerOption.CullMode = D3D11_CULL_NONE;
    auto RS = InRenderer->GetRenderStateManager()->GetOrCreateRasterizerState(rasterizerOption);
    FontMaterial->SetRasterizerOption(rasterizerOption);
    FontMaterial->SetRasterizerState(RS);

    // 깊이 설정: 현재는 디버깅용이므로 깊이 테스트 무시
    // TODO: 추후 World UI로 활용 시 DepthEnable = true, DepthWriteMask = ZERO 고려
    FDepthStencilStateOption depthOption;
    depthOption.DepthEnable = false;
    depthOption.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    auto DSS = InRenderer->GetRenderStateManager()->GetOrCreateDepthStencilState(depthOption);
    FontMaterial->SetDepthStencilOption(depthOption);
    FontMaterial->SetDepthStencilState(DSS);

    // 알파 블렌딩 설정
    FBlendStateOption blendOption;
    blendOption.BlendEnable = true;
    blendOption.SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendOption.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendOption.BlendOp = D3D11_BLEND_OP_ADD;
    blendOption.SrcBlendAlpha = D3D11_BLEND_ONE;
    blendOption.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    blendOption.BlendOpAlpha = D3D11_BLEND_OP_ADD;
    auto BS = InRenderer->GetRenderStateManager()->GetOrCreateBlendState(blendOption);
    FontMaterial->SetBlendOption(blendOption);
    FontMaterial->SetBlendState(BS);

    // b2: TextConstantBuffer (TextColor)
    int32 SlotIndex = FontMaterial->CreateConstantBuffer(Device, 16);
    if (SlotIndex >= 0)
    {
        FontMaterial->RegisterParameter("TextColor", SlotIndex, 0, 16);
        FVector4 White(1.0f, 1.0f, 1.0f, 1.0f);
        FontMaterial->SetParameterData("TextColor", &White, 16);
    }

    return true;
}

void CTextMeshBuilder::Release()
{
    Atlas.Reset();
    FontMaterial.reset();
    Device = nullptr;
    DeviceContext = nullptr;
    RenderStateManager = nullptr;
}

bool CTextMeshBuilder::BuildTextMesh(const FString& Text, FMeshData& OutMesh) const
{
    if (Text.empty())
    {
        return false;
    }

    const TArray<uint32> Codepoints = DecodeToCodepoints(Text);
    if (Codepoints.empty())
    {
        return false;
    }

    float TotalWidth = 0.0f;
    for (uint32 Cp : Codepoints)
    {
        const FFontGlyph* Glyph = Atlas.FindGlyph(Cp);
        TotalWidth += Glyph->XAdvance;
    }

    float PenX = -TotalWidth * 0.5f;

    OutMesh.Vertices.clear();
    OutMesh.Indices.clear();
    OutMesh.Topology = EMeshTopology::EMT_TriangleList;

    for (uint32 Cp : Codepoints)
    {
        const FFontGlyph* Glyph = Atlas.FindGlyph(Cp);

        if (Glyph->Width > 0.0f && Glyph->Height > 0.0f)
        {
            // TODO: 0~1로 매핑되어 있는 값 정수값으로, 혹은 반대로 통일해야 함
            const float X0 = PenX;
            const float X1 = PenX + Glyph->Width;
            const float Y0 = 0.0f;
            const float Y1 = Glyph->Height;

            const uint32 BaseIndex = static_cast<uint32>(OutMesh.Vertices.size());

            FPrimitiveVertex V0, V1, V2, V3;
            V0.Position = FVector(0.0f, X0, Y1);
            V0.UV = FVector2(Glyph->X, Glyph->Y);
            V1.Position = FVector(0.0f, X1, Y1);
            V1.UV = FVector2(Glyph->X + Glyph->Width, Glyph->Y);
            V2.Position = FVector(0.0f, X1, Y0);
            V2.UV = FVector2(Glyph->X + Glyph->Width, Glyph->Y + Glyph->Height);
            V3.Position = FVector(0.0f, X0, Y0);
            V3.UV = FVector2(Glyph->X, Glyph->Y + Glyph->Height);

            V0.Color = V1.Color = V2.Color = V3.Color = FVector4(1, 1, 1, 1);
            V0.Normal = V1.Normal = V2.Normal = V3.Normal = FVector(0, 0, 1);

            OutMesh.Vertices.push_back(V0);
            OutMesh.Vertices.push_back(V1);
            OutMesh.Vertices.push_back(V2);
            OutMesh.Vertices.push_back(V3);

            OutMesh.Indices.push_back(BaseIndex + 0);
            OutMesh.Indices.push_back(BaseIndex + 1);
            OutMesh.Indices.push_back(BaseIndex + 2);
            OutMesh.Indices.push_back(BaseIndex + 0);
            OutMesh.Indices.push_back(BaseIndex + 2);
            OutMesh.Indices.push_back(BaseIndex + 3);
        }

        PenX += Glyph->XAdvance;
    }

    return !OutMesh.Vertices.empty();
}

void CTextMeshBuilder::SetFillMode(D3D11_FILL_MODE InFillMode)
{
    if (!FontMaterial)
        return;
    FRasterizerStateOption Option = FontMaterial->GetRasterizerOption();
    if (Option.FillMode == InFillMode)
        return;
    Option.FillMode = InFillMode;
    auto RS = RenderStateManager->GetOrCreateRasterizerState(Option);
    FontMaterial->SetRasterizerOption(Option);
    FontMaterial->SetRasterizerState(RS);
}

TArray<uint32> CTextMeshBuilder::DecodeToCodepoints(const FString& Text) const
{
    TArray<uint32> Result;

    if (Text.empty())
    {
        return Result;
    }

    const int WideLength = MultiByteToWideChar(CP_UTF8, 0, Text.c_str(), -1, nullptr, 0);
    if (WideLength <= 0)
    {
        return Result;
    }

    std::wstring WideText;
    WideText.resize(static_cast<size_t>(WideLength - 1));
    MultiByteToWideChar(CP_UTF8, 0, Text.c_str(), -1, WideText.data(), WideLength);

    Result.reserve(WideText.size());
    for (size_t i = 0; i < WideText.size(); ++i)
    {
        const uint32 W1 = static_cast<uint32>(WideText[i]);
        if (W1 >= 0xD800 && W1 <= 0xDBFF)
        {
            if (i + 1 < WideText.size())
            {
                const uint32 W2 = static_cast<uint32>(WideText[i + 1]);
                if (W2 >= 0xDC00 && W2 <= 0xDFFF)
                {
                    const uint32 Codepoint = 0x10000 + (((W1 - 0xD800) << 10) | (W2 - 0xDC00));
                    Result.push_back(Codepoint);
                    ++i;
                    continue;
                }
            }
        }
        Result.push_back(W1);
    }

    return Result;
}