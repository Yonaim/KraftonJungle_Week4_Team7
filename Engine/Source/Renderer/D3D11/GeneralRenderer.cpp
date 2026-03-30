#include "Core/CoreMinimal.h"
#include "GeneralRenderer.h"

#include "Core/Misc/Paths.h"
#include "Renderer/Material/Material.h"
#include "Renderer/Material/MaterialManager.h"
#include "Renderer/Shader/ShaderMap.h"
#include "Renderer/Shader/ShaderType.h"

std::shared_ptr<FMaterial> FGeneralRenderer::DefaultMaterial;
std::shared_ptr<FMaterial> FGeneralRenderer::DefaultSpriteMaterial;

FGeneralRenderer::FGeneralRenderer(HWND InHwnd, int32 InWidth, int32 InHeight)
{
    Initialize(InHwnd, InWidth, InHeight);
}

FGeneralRenderer::~FGeneralRenderer()
{
    Release();
}

bool FGeneralRenderer::Initialize(HWND InHwnd, int32 Width, int32 Height)
{
    Hwnd = InHwnd;

    if (!RHI.Initialize(Hwnd))
        return false;

    RenderStateManager = std::make_unique<CRenderStateManager>(RHI.GetDevice(), RHI.GetDeviceContext());
    RenderStateManager->PrepareCommonStates();

    if (!CreateConstantBuffers())
        return false;
    SetConstantBuffers();

    if (!InitializeDefaultMaterial()) 
        return false;

    if (!CreatePickResources(Width, Height))
        return false;

    return true;
}

bool FGeneralRenderer::Pick(int32 MouseX, int32 MouseY, uint32& OutPickId)
{
    OutPickId = 0;

    ID3D11DeviceContext* DeviceContext = RHI.GetDeviceContext();
    if (DeviceContext == nullptr || PickRTV == nullptr || PickDSV == nullptr)
        return false;

    D3D11_VIEWPORT Viewport = RHI.GetViewport();
    if (MouseX < 0 || MouseY < 0 || MouseX >= Viewport.Width || MouseY >= Viewport.Height)
        return false;

    // 1. 현재 렌더 상태 저장
    ID3D11RenderTargetView* PrevRTV = nullptr;
    ID3D11DepthStencilView* PrevDSV = nullptr;
    DeviceContext->OMGetRenderTargets(1, &PrevRTV, &PrevDSV);
    
    UINT PrevVPCount = 1;
    D3D11_VIEWPORT PrevVP;
    DeviceContext->RSGetViewports(&PrevVPCount, &PrevVP);

    // 2. 픽킹용 렌더 타겟 설정
    DeviceContext->OMSetRenderTargets(1, &PickRTV, PickDSV);
    
    D3D11_VIEWPORT PickVP = Viewport;
    PickVP.TopLeftX = 0;
    PickVP.TopLeftY = 0;
    DeviceContext->RSSetViewports(1, &PickVP);

    static const float ClearColor[4] = {0, 0, 0, 0};
    DeviceContext->ClearRenderTargetView(PickRTV, ClearColor);
    DeviceContext->ClearDepthStencilView(PickDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

    // 3. 픽킹용 셰이더 및 파이프라인 바인딩
    DeviceContext->VSSetShader(PickVertexShader, nullptr, 0);
    DeviceContext->PSSetShader(PickPixelShader, nullptr, 0);
    DeviceContext->IASetInputLayout(PickInputLayout);
    
    RenderStateManager->RebindState();

    for (const auto& Cmd : CommandList)
    {
        if (!Cmd.MeshData || (Cmd.MeshData->Vertices.empty() && Cmd.MeshData->Indices.empty()))
            continue;

        Cmd.MeshData->Bind(&RHI);
        
        D3D11_PRIMITIVE_TOPOLOGY Topology = (D3D11_PRIMITIVE_TOPOLOGY)Cmd.MeshData->Topology;
        DeviceContext->IASetPrimitiveTopology(Topology);

        UpdateObjectConstantBuffer(Cmd.WorldMatrix, Cmd.ObjectId);

        if (!Cmd.MeshData->Indices.empty())
            DeviceContext->DrawIndexed(static_cast<UINT>(Cmd.MeshData->Indices.size()), 0, 0);
        else if (!Cmd.MeshData->Vertices.empty())
            DeviceContext->Draw(static_cast<UINT>(Cmd.MeshData->Vertices.size()), 0);
    }

    // 5. 이전 렌더 상태 복구
    DeviceContext->OMSetRenderTargets(1, &PrevRTV, PrevDSV);
    DeviceContext->RSSetViewports(1, &PrevVP);
    
    if (PrevRTV) PrevRTV->Release();
    if (PrevDSV) PrevDSV->Release();

    // 6. 결과 읽어오기
    return ReadBackMousePixel(MouseX, MouseY, OutPickId);
}

bool FGeneralRenderer::InitializeDefaultMaterial()
{
    ID3D11Device* Device = RHI.GetDevice();
    if (NormalSampler == nullptr)
    {
        D3D11_SAMPLER_DESC SamplerDesc = {};
        SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        SamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        SamplerDesc.MinLOD = 0;
        SamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
        Device->CreateSamplerState(&SamplerDesc, &NormalSampler);
    }

    // Create a 1x1 white texture for DefaultMaterial
    ID3D11ShaderResourceView* WhiteSRV = nullptr;
    {
        uint32_t WhiteData = 0xFFFFFFFF;
        D3D11_TEXTURE2D_DESC TexDesc = {};
        TexDesc.Width = 1;
        TexDesc.Height = 1;
        TexDesc.MipLevels = 1;
        TexDesc.ArraySize = 1;
        TexDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        TexDesc.SampleDesc.Count = 1;
        TexDesc.Usage = D3D11_USAGE_IMMUTABLE;
        TexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA InitData = {};
        InitData.pSysMem = &WhiteData;
        InitData.SysMemPitch = 4;

        ID3D11Texture2D* WhiteTex = nullptr;
        if (SUCCEEDED(Device->CreateTexture2D(&TexDesc, &InitData, &WhiteTex)))
        {
            Device->CreateShaderResourceView(WhiteTex, nullptr, &WhiteSRV);
            WhiteTex->Release();
        }
    }

    std::wstring ShaderDirW = FPaths::ShaderDir();
    std::wstring VSPath = ShaderDirW + L"\\NewShaderMesh.hlsl";
    std::wstring PSPath = ShaderDirW + L"\\NewShaderMesh.hlsl";

    if (!ShaderManager.LoadVertexShader(Device, VSPath, L"VSMain"))
        return false;
    if (!ShaderManager.LoadPixelShader(Device, PSPath, L"PSMain"))
        return false;

    /** 기본 Material 생성 */
    {
        auto VS = FShaderMap::Get().GetOrCreateVertexShader(Device, VSPath, L"VSMain");
        auto PS = FShaderMap::Get().GetOrCreatePixelShader(Device, PSPath, L"PSMain");
        DefaultMaterial = std::make_shared<FMaterial>();
        DefaultMaterial->SetOriginName("M_Default");
        DefaultMaterial->SetVertexShader(VS);
        DefaultMaterial->SetPixelShader(PS);

        if (WhiteSRV)
        {
            auto Tex = std::make_shared<FMaterialTexture>();
            Tex->TextureSRV = WhiteSRV;
            DefaultMaterial->SetMaterialTexture(Tex);
        }

        FRasterizerStateOption rasterizerOption;
        rasterizerOption.FillMode = D3D11_FILL_SOLID;
        rasterizerOption.CullMode = D3D11_CULL_BACK;
        auto RS = RenderStateManager->GetOrCreateRasterizerState(rasterizerOption);
        DefaultMaterial->SetRasterizerOption(rasterizerOption);
        DefaultMaterial->SetRasterizerState(RS);

        FDepthStencilStateOption depthStencilOption;
        depthStencilOption.DepthEnable = true;
        depthStencilOption.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        auto DSS = RenderStateManager->GetOrCreateDepthStencilState(depthStencilOption);
        DefaultMaterial->SetDepthStencilOption(depthStencilOption);
        DefaultMaterial->SetDepthStencilState(DSS);
        
        FBlendStateOption blendStateOption;
        auto BS = RenderStateManager->GetOrCreateBlendState(blendStateOption);
        DefaultMaterial->SetBlendOption(blendStateOption);
        DefaultMaterial->SetBlendState(BS);

        int32 SlotIndex = DefaultMaterial->CreateConstantBuffer(Device, 16);
        if (SlotIndex >= 0)
        {
            DefaultMaterial->RegisterParameter("BaseColor", SlotIndex, 0, 16);
            float White[4] = {1.0f, 1.0f, 1.0f, 1.0f};
            DefaultMaterial->GetConstantBuffer(SlotIndex)->SetData(White, sizeof(White));
        }
        FMaterialManager::Get().Register("M_Default", DefaultMaterial);
    }

    /** 기본 Sprite Material 생성 */
    {
        DefaultSpriteMaterial = DefaultMaterial->CreateDynamicMaterial();
        DefaultSpriteMaterial->SetOriginName("M_DefaultSprite");
        DefaultSpriteMaterial->SetInstanceName("M_DefaultSprite");

        FBlendStateOption blendStateOption;
        blendStateOption.BlendEnable = true;
        blendStateOption.SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blendStateOption.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        blendStateOption.BlendOp = D3D11_BLEND_OP_ADD;
        blendStateOption.SrcBlendAlpha = D3D11_BLEND_ONE;
        blendStateOption.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
        blendStateOption.BlendOpAlpha = D3D11_BLEND_OP_ADD;

        auto BS = RenderStateManager->GetOrCreateBlendState(blendStateOption);
        DefaultSpriteMaterial->SetBlendOption(blendStateOption);
        DefaultSpriteMaterial->SetBlendState(BS);

        FMaterialManager::Get().Register("M_DefaultSprite", DefaultSpriteMaterial);
    }
    
    InitializeAABBResources();

    return true;
}

void FGeneralRenderer::BeginFrame()
{
    RHI.BeginFrame();

    constexpr float ClearColor[4] = {0.17f, 0.17f, 0.17f, 1.0f};
    RHI.Clear(ClearColor, 1.0f, 0);
    
    ID3D11RenderTargetView* ActiveRTV = RHI.GetBackBufferRTV();
    ID3D11DepthStencilView* ActiveDSV = RHI.GetDepthStencilView();
    D3D11_VIEWPORT          ActiveVP = RHI.GetViewport();

    ID3D11DeviceContext* DeviceContext = RHI.GetDeviceContext();

    if (bUseSceneRenderTargetOverride)
    {
        ActiveRTV = SceneRenderTargetView;
        ActiveDSV = SceneDepthStencilView;
        ActiveVP = SceneViewport;
        if (ActiveRTV && ActiveRTV != RHI.GetBackBufferRTV())
            DeviceContext->ClearRenderTargetView(ActiveRTV, ClearColor);
        if (ActiveDSV && ActiveDSV != RHI.GetDepthStencilView())
            DeviceContext->ClearDepthStencilView(ActiveDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
                                                 1.0f, 0);
    }

    DeviceContext->OMSetRenderTargets(1, &ActiveRTV, ActiveDSV);
    DeviceContext->RSSetViewports(1, &ActiveVP);
    ClearCommandList();
}

void FGeneralRenderer::EndFrame()
{
    ID3D11DeviceContext* DeviceContext = RHI.GetDeviceContext();
    if (RHI.GetBackBufferRTV())
    {
        ID3D11RenderTargetView* RTV = RHI.GetBackBufferRTV();
        DeviceContext->OMSetRenderTargets(1, &RTV, RHI.GetDepthStencilView());
        D3D11_VIEWPORT VP = RHI.GetViewport();
        DeviceContext->RSSetViewports(1, &VP);
    }

    if (GUIRender)
        GUIRender();

    RHI.EndFrame();

    if (GUIPostPresent)
        GUIPostPresent();
}

void FGeneralRenderer::Release()
{
    ClearViewportCallbacks();
    ClearSceneRenderTarget();
    ShaderManager.Release();
    FShaderMap::Get().Clear();
    FMaterialManager::Get().Clear();
    if (NormalSampler)
        NormalSampler->Release();
    DefaultMaterial.reset();
    if (FrameConstantBuffer)
        FrameConstantBuffer->Release();
    if (ObjectConstantBuffer)
        ObjectConstantBuffer->Release();
    
    ReleasePickResources();
    RHI.Shutdown();
}

bool FGeneralRenderer::IsOccluded()
{
    if (bSwapChainOccluded && RHI.GetSwapChain()->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
        return true;
    bSwapChainOccluded = false;
    return false;
}

void FGeneralRenderer::OnResize(int32 NewWidth, int32 NewHeight)
{
    if (NewWidth == 0 || NewHeight == 0)
        return;
    ClearSceneRenderTarget();
    RHI.Resize(NewWidth, NewHeight);
}

void FGeneralRenderer::SetSceneRenderTarget(ID3D11RenderTargetView* InRenderTargetView,
                                            ID3D11DepthStencilView* InDepthStencilView,
                                            const D3D11_VIEWPORT&   InViewport)
{
    SceneRenderTargetView = InRenderTargetView;
    SceneDepthStencilView = InDepthStencilView;
    SceneViewport = InViewport;
    bUseSceneRenderTargetOverride = (SceneRenderTargetView != nullptr && SceneDepthStencilView !=
                                     nullptr);
}

void FGeneralRenderer::ClearSceneRenderTarget()
{
    SceneRenderTargetView = nullptr;
    SceneDepthStencilView = nullptr;
    SceneViewport = {};
    bUseSceneRenderTargetOverride = false;
}

void FGeneralRenderer::SubmitCommands(const FRenderCommandQueue& Queue)
{
    ViewMatrix = Queue.ViewMatrix;
    ProjectionMatrix = Queue.ProjectionMatrix;

    for (const auto& Cmd : Queue.Commands)
    {
        // if (Cmd.MeshData)
        //     Cmd.MeshData->UpdateVertexAndIndexBuffer(RHI.GetDevice());
        AddCommand(Cmd);
    }
}

void FGeneralRenderer::ExecuteCommands()
{
    std::sort(CommandList.begin(), CommandList.end(),
              [](const FRenderCommand& A, const FRenderCommand& B)
              {
                  if (A.RenderLayer != B.RenderLayer)
                      return A.RenderLayer < B.RenderLayer;
                  return A.SortKey < B.SortKey;
              });

    SetConstantBuffers();
    UpdateFrameConstantBuffer();

    ExecuteRenderPass(ERenderLayer::Default);
    DrawAllAABBLines(ERenderLayer::Default);
    
    ClearDepthBuffer();
    ExecuteRenderPass(ERenderLayer::Overlay);
    DrawAllAABBLines(ERenderLayer::Overlay);

    ClearCommandList();
}

void FGeneralRenderer::SetGUICallbacks(FGUICallback InInit, FGUICallback     InShutdown,
                                       FGUICallback InNewFrame, FGUICallback InRender,
                                       FGUICallback InPostPresent)
{
    GUIInit = std::move(InInit);
    GUIShutdown = std::move(InShutdown);
    GUINewFrame = std::move(InNewFrame);
    GUIRender = std::move(InRender);
    GUIPostPresent = std::move(InPostPresent);

    if (GUIInit)
    {
        GUIInit();
    }
}

void FGeneralRenderer::ClearViewportCallbacks()
{
    if (GUIShutdown)
    {
        GUIShutdown();
    }

    GUIInit = nullptr;
    GUIShutdown = nullptr;
    GUINewFrame = nullptr;
    GUIUpdate = nullptr;
    GUIRender = nullptr;
    GUIPostPresent = nullptr;
}

void FGeneralRenderer::SetGUIUpdateCallback(FGUICallback InUpdate)
{
    GUIUpdate = std::move(InUpdate);
}

FVector FGeneralRenderer::GetCameraPosition() const
{
    const FMatrix InvView = ViewMatrix.GetInverse();
    return FVector(InvView.M[3][0], InvView.M[3][1], InvView.M[3][2]);
}

void FGeneralRenderer::SetConstantBuffers()
{
    ID3D11Buffer* CBs[2] = {FrameConstantBuffer, ObjectConstantBuffer};
    RHI.GetDeviceContext()->VSSetConstantBuffers(0, 2, CBs);
}

void FGeneralRenderer::AddCommand(const FRenderCommand& Command)
{
    CommandList.push_back(Command);
    FRenderCommand& Added = CommandList.back();
    if (!Added.Material)
        Added.Material = DefaultMaterial.get();
    Added.SortKey = FRenderCommand::MakeSortKey(Added.Material, Added.MeshData);
}

void FGeneralRenderer::ClearCommandList()
{
    CommandList.clear();
}

bool FGeneralRenderer::CreateConstantBuffers()
{
    D3D11_BUFFER_DESC Desc = {};
    Desc.Usage = D3D11_USAGE_DYNAMIC;
    Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    Desc.ByteWidth = sizeof(FFrameConstantBuffer);
    if (FAILED(RHI.GetDevice()->CreateBuffer(&Desc, nullptr, &FrameConstantBuffer)))
        return false;

    Desc.ByteWidth = sizeof(FObjectConstantBuffer);
    return SUCCEEDED(RHI.GetDevice()->CreateBuffer(&Desc, nullptr, &ObjectConstantBuffer));
}

void FGeneralRenderer::UpdateFrameConstantBuffer()
{
    FFrameConstantBuffer CBData;
    CBData.View = ViewMatrix;
    CBData.Projection = ProjectionMatrix;
    D3D11_MAPPED_SUBRESOURCE Mapped;
    if (SUCCEEDED(RHI.GetDeviceContext()->Map(FrameConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Mapped)))
    {
        memcpy(Mapped.pData, &CBData, sizeof(CBData));
        RHI.GetDeviceContext()->Unmap(FrameConstantBuffer, 0);
    }
}

void FGeneralRenderer::UpdateObjectConstantBuffer(const FMatrix& WorldMatrix, uint32 ObjectId)
{
    FObjectConstantBuffer CBData;
    CBData.World = WorldMatrix;
    CBData.ObjectId = ObjectId;
    D3D11_MAPPED_SUBRESOURCE Mapped;
    if (SUCCEEDED(RHI.GetDeviceContext()->Map(ObjectConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Mapped)))
    {
        memcpy(Mapped.pData, &CBData, sizeof(CBData));
        RHI.GetDeviceContext()->Unmap(ObjectConstantBuffer, 0);
    }
}

void FGeneralRenderer::ClearDepthBuffer()
{
    if (!SceneDepthStencilView)
        return;
    RHI.GetDeviceContext()->ClearDepthStencilView(SceneDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void FGeneralRenderer::ExecuteRenderPass(ERenderLayer InRenderLayer)
{
    FMaterial*               CurrentMaterial = nullptr;
    FMeshData*               CurrentMesh = nullptr;
    D3D11_PRIMITIVE_TOPOLOGY CurrentMeshTopology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
    
    FRenderCommand toFind;
    toFind.RenderLayer = InRenderLayer;
    auto it = std::lower_bound(CommandList.begin(), CommandList.end(), toFind,
                               [](const FRenderCommand& A, const FRenderCommand& B)
                               {
                                   return A.RenderLayer < B.RenderLayer;
                               });

    ID3D11DeviceContext* DeviceContext = RHI.GetDeviceContext();
    RenderStateManager->RebindState();
    for (; it != CommandList.end(); it++)
    {
        auto Cmd = *it;
        if (Cmd.RenderLayer != InRenderLayer)
            return;
        if (!Cmd.MeshData || (Cmd.MeshData->Vertices.empty() && Cmd.MeshData->Indices.empty()))
            continue;

        if (Cmd.Material != CurrentMaterial)
        {
            Cmd.Material->Bind(DeviceContext);

            RenderStateManager->BindState(Cmd.Material->GetRasterizerState());
            RenderStateManager->BindState(Cmd.Material->GetDepthStencilState());
            RenderStateManager->BindState(Cmd.Material->GetBlendState());

            CurrentMaterial = Cmd.Material;
            DeviceContext->PSSetSamplers(0, 1, &NormalSampler);
        }

        if (Cmd.MeshData != CurrentMesh)
        {
            Cmd.MeshData->Bind(&RHI);
            CurrentMesh = Cmd.MeshData;
        }

        D3D11_PRIMITIVE_TOPOLOGY DesiredTopology = (D3D11_PRIMITIVE_TOPOLOGY)CurrentMesh->Topology;
        if (DesiredTopology != CurrentMeshTopology)
        {
            DeviceContext->IASetPrimitiveTopology(DesiredTopology);
            CurrentMeshTopology = DesiredTopology;
        }

        UpdateObjectConstantBuffer(Cmd.WorldMatrix, Cmd.ObjectId);

        if (!Cmd.MeshData->Indices.empty())
            DeviceContext->DrawIndexed(static_cast<UINT>(Cmd.MeshData->Indices.size()), 0, 0);
        else if (!Cmd.MeshData->Vertices.empty())
            DeviceContext->Draw(static_cast<UINT>(Cmd.MeshData->Vertices.size()), 0);
    }
}

bool FGeneralRenderer::CreatePickResources(int32 Width, int32 Height)
{
    ID3D11Device* Device = RHI.GetDevice();
    if (Device == nullptr) return false;

    D3D11_TEXTURE2D_DESC TexDesc = {};
    TexDesc.Width = Width;
    TexDesc.Height = Height;
    TexDesc.MipLevels = 1;
    TexDesc.ArraySize = 1;
    TexDesc.Format = DXGI_FORMAT_R32_UINT;
    TexDesc.SampleDesc.Count = 1;
    TexDesc.Usage = D3D11_USAGE_DEFAULT;
    TexDesc.BindFlags = D3D11_BIND_RENDER_TARGET;

    if (FAILED(Device->CreateTexture2D(&TexDesc, nullptr, &PickColorTexture)))
        return false;

    if (FAILED(Device->CreateRenderTargetView(PickColorTexture, nullptr, &PickRTV)))
        return false;

    D3D11_TEXTURE2D_DESC DepthDesc = {};
    DepthDesc.Width = Width;
    DepthDesc.Height = Height;
    DepthDesc.MipLevels = 1;
    DepthDesc.ArraySize = 1;
    DepthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    DepthDesc.SampleDesc.Count = 1;
    DepthDesc.Usage = D3D11_USAGE_DEFAULT;
    DepthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    if (FAILED(Device->CreateTexture2D(&DepthDesc, nullptr, &PickDepthTexture)))
        return false;

    if (FAILED(Device->CreateDepthStencilView(PickDepthTexture, nullptr, &PickDSV)))
        return false;

    D3D11_TEXTURE2D_DESC ReadbackDesc = {};
    ReadbackDesc.Width = 1;
    ReadbackDesc.Height = 1;
    ReadbackDesc.MipLevels = 1;
    ReadbackDesc.ArraySize = 1;
    ReadbackDesc.Format = DXGI_FORMAT_R32_UINT;
    ReadbackDesc.SampleDesc.Count = 1;
    ReadbackDesc.Usage = D3D11_USAGE_STAGING;
    ReadbackDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

    if (FAILED(Device->CreateTexture2D(&ReadbackDesc, nullptr, &ReadbackTexture)))
        return false;

    std::wstring ShaderPath = FPaths::ShaderDir() / L"\\ShaderObjectId.hlsl";
    ID3DBlob* VSCode = nullptr;
    ID3DBlob* ErrorBlob = nullptr;
    
    if (FAILED(D3DCompileFromFile(ShaderPath.c_str(), nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &VSCode, &ErrorBlob)))
    {
        if (ErrorBlob) ErrorBlob->Release();
        return false;
    }
    Device->CreateVertexShader(VSCode->GetBufferPointer(), VSCode->GetBufferSize(), nullptr, &PickVertexShader);

    D3D11_INPUT_ELEMENT_DESC InputElements[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    Device->CreateInputLayout(InputElements, 4, VSCode->GetBufferPointer(), VSCode->GetBufferSize(), &PickInputLayout);
    VSCode->Release();

    ID3DBlob* PSCode = nullptr;
    if (FAILED(D3DCompileFromFile(ShaderPath.c_str(), nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &PSCode, &ErrorBlob)))
    {
        if (ErrorBlob) ErrorBlob->Release();
        return false;
    }
    Device->CreatePixelShader(PSCode->GetBufferPointer(), PSCode->GetBufferSize(), nullptr, &PickPixelShader);
    PSCode->Release();

    return true;
}

void FGeneralRenderer::ReleasePickResources()
{
    if (PickColorTexture) PickColorTexture->Release(); PickColorTexture = nullptr;
    if (PickRTV) PickRTV->Release(); PickRTV = nullptr;
    if (PickDepthTexture) PickDepthTexture->Release(); PickDepthTexture = nullptr;
    if (PickDSV) PickDSV->Release(); PickDSV = nullptr;
    if (ReadbackTexture) ReadbackTexture->Release(); ReadbackTexture = nullptr;
    if (PickVertexShader) PickVertexShader->Release(); PickVertexShader = nullptr;
    if (PickPixelShader) PickPixelShader->Release(); PickPixelShader = nullptr;
    if (PickInputLayout) PickInputLayout->Release(); PickInputLayout = nullptr;
}

bool FGeneralRenderer::ReadBackMousePixel(int32 MouseX, int32 MouseY, uint32& OutObjectId)
{
    ID3D11DeviceContext* DeviceContext = RHI.GetDeviceContext();
    if (DeviceContext == nullptr || PickColorTexture == nullptr || ReadbackTexture == nullptr)
        return false;

    D3D11_BOX Box = {};
    Box.left = static_cast<UINT>(MouseX);
    Box.right = static_cast<UINT>(MouseX + 1);
    Box.top = static_cast<UINT>(MouseY);
    Box.bottom = static_cast<UINT>(MouseY + 1);
    Box.front = 0;
    Box.back = 1;

    DeviceContext->CopySubresourceRegion(ReadbackTexture, 0, 0, 0, 0, PickColorTexture, 0, &Box);

    D3D11_MAPPED_SUBRESOURCE Mapped = {};
    if (FAILED(DeviceContext->Map(ReadbackTexture, 0, D3D11_MAP_READ, 0, &Mapped)))
        return false;

    OutObjectId = *reinterpret_cast<const uint32*>(Mapped.pData);
    DeviceContext->Unmap(ReadbackTexture, 0);
    return true;
}

// TODO: Re-Implement InitializeAABBResources
void FGeneralRenderer::InitializeAABBResources()
{
    ID3D11Device* Device = RHI.GetDevice();
    std::wstring ShaderDirW = FPaths::ShaderDir();
    std::wstring LineVSPath = ShaderDirW + L"\\NewShaderLine.hlsl";
    std::wstring LinePSPath = ShaderDirW + L"\\NewShaderLine.hlsl";

    auto VS = FShaderMap::Get().GetOrCreateVertexShader(Device, LineVSPath, L"VSMain");
    auto PS = FShaderMap::Get().GetOrCreatePixelShader(Device, LinePSPath, L"PSMain");

    AABBMaterial = std::make_shared<FMaterial>();
    AABBMaterial->SetOriginName("M_AABB");
    AABBMaterial->SetVertexShader(VS);
    AABBMaterial->SetPixelShader(PS);
    
    FMaterialManager::Get().Register("M_AABB", AABBMaterial);

    FRasterizerStateOption rasterizerOption;
    rasterizerOption.FillMode = D3D11_FILL_SOLID;
    rasterizerOption.CullMode = D3D11_CULL_NONE;
    AABBMaterial->SetRasterizerOption(rasterizerOption);
    AABBMaterial->SetRasterizerState(RenderStateManager->GetOrCreateRasterizerState(rasterizerOption));

    FDepthStencilStateOption depthStencilOption;
    depthStencilOption.DepthEnable = true;
    depthStencilOption.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    AABBMaterial->SetDepthStencilOption(depthStencilOption);
    AABBMaterial->SetDepthStencilState(RenderStateManager->GetOrCreateDepthStencilState(depthStencilOption));

    AABBMeshData = std::make_unique<FMeshData>();
    AABBMeshData->Topology = EMeshTopology::EMT_LineList;

    FVector4 Yellow(1.0f, 1.0f, 0.0f, 1.0f);
    AABBMeshData->Vertices = {
        { FVector(0, 0, 0), FVector::ForwardVector, Yellow }, 
        { FVector(1, 0, 0), FVector::ForwardVector, Yellow }, 
        { FVector(1, 1, 0), FVector::ForwardVector, Yellow }, 
        { FVector(0, 1, 0), FVector::ForwardVector, Yellow },
        { FVector(0, 0, 1), FVector::ForwardVector, Yellow }, 
        { FVector(1, 0, 1), FVector::ForwardVector, Yellow }, 
        { FVector(1, 1, 1), FVector::ForwardVector, Yellow }, 
        { FVector(0, 1, 1), FVector::ForwardVector, Yellow }
    };

    AABBMeshData->Indices = {
        0, 1, 1, 2, 2, 3, 3, 0,
        4, 5, 5, 6, 6, 7, 7, 4,
        0, 4, 1, 5, 2, 6, 3, 7
    };

    // AABBMeshData->CreateVertexAndIndexBuffer(Device);
}

void FGeneralRenderer::DrawAllAABBLines(ERenderLayer InRenderLayer)
{
    if (!AABBMeshData || !AABBMaterial)
        return;

    ID3D11DeviceContext* DeviceContext = RHI.GetDeviceContext();
    AABBMaterial->Bind(DeviceContext);
    RenderStateManager->BindState(AABBMaterial->GetRasterizerState());
    RenderStateManager->BindState(AABBMaterial->GetDepthStencilState());

    AABBMeshData->Bind(&RHI);
    DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    for (const auto& Cmd : CommandList)
    {
        if (Cmd.RenderLayer != InRenderLayer || !Cmd.bDrawAABB)
            continue;

        const FVector& Min = Cmd.WorldAABB.Min;
        const FVector& Max = Cmd.WorldAABB.Max;
        const FVector  Size = Max - Min;

        FMatrix AABBMatrix = FMatrix::MakeScale(Size) * FMatrix::MakeTranslation(Min);

        UpdateObjectConstantBuffer(AABBMatrix);
        DeviceContext->DrawIndexed(static_cast<UINT>(AABBMeshData->Indices.size()), 0, 0);
    }
}
