#include "Core/CoreMinimal.h"
#include "GeneralRenderer.h"

#include "Core/Path.h"
#include "NewRenderer/Material.h"
#include "NewRenderer/MaterialManager.h"
#include "NewRenderer/ShaderMap.h"
#include "NewRenderer/ShaderType.h"

std::shared_ptr<FMaterial> FGeneralRenderer::DefaultMaterial;

FGeneralRenderer::FGeneralRenderer(HWND InHwnd, int32 InWidth, int32 InHeight)
{
    // RendererModule의 Device & SwapChain과 통합을 위해 잠시 주석처리
    // Initialize(InHwnd, InWidth, InHeight);
}

FGeneralRenderer::~FGeneralRenderer()
{
    Release();
}

void FGeneralRenderer::DEBUG_ForceInitialize(
    ID3D11Device* InDevice,
    ID3D11DeviceContext* InDeviceContext,
    ID3D11RenderTargetView* InRTV,
    ID3D11DepthStencilView* InDSV,
    D3D11_VIEWPORT InVP
    )
{
    Device = InDevice;
    DeviceContext = InDeviceContext;
    RenderTargetView = InRTV;
    DepthStencilView = InDSV;
    Viewport = InVP;
    
    RenderStateManager = std::make_unique<CRenderStateManager>(Device, DeviceContext);
    RenderStateManager->PrepareCommonStates();
    
    if (!CreateConstantBuffers())
        return;
    SetConstantBuffers();
    
    InitializeDefaultMaterial();
}

bool FGeneralRenderer::Initialize(HWND InHwnd, int32 Width, int32 Height)
{
    Hwnd = InHwnd;

    if (!CreateDeviceAndSwapChain(Hwnd, Width, Height))
        return false;
    if (!CreateRenderTargetAndDepthStencil(Width, Height))
        return false;

    Viewport.TopLeftX = 0.f;
    Viewport.TopLeftY = 0.f;
    Viewport.Width = static_cast<float>(Width);
    Viewport.Height = static_cast<float>(Height);
    Viewport.MinDepth = 0.f;
    Viewport.MaxDepth = 1.f;

    RenderStateManager = std::make_unique<CRenderStateManager>(Device, DeviceContext);
    RenderStateManager->PrepareCommonStates();

    if (!CreateConstantBuffers())
        return false;
    SetConstantBuffers();

    if (!InitializeDefaultMaterial()) 
        return false;

    return true;
}

bool FGeneralRenderer::InitializeDefaultMaterial()
{
    std::wstring ShaderDirW = FPaths::ShaderDir(); // 임시 Dir 경로
    // TODO: 매직 넘버 제거
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
        // 기본값 그대로 사용
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
    
    InitializeAABBResources();

    return true;
}

void FGeneralRenderer::BeginFrame()
{
    /** 프레임 시작과 끝에서의 RenderTargetView 등의 세팅은 RendererModule에 위임 (임시)
    if (GUINewFrame)
        GUINewFrame();
    if (GUIUpdate)
        GUIUpdate();
    */

    constexpr float ClearColor[4] = {0.1f, 0.1f, 0.1f, 1.0f};
    /**
    if (RenderTargetView)
        DeviceContext->ClearRenderTargetView(RenderTargetView, ClearColor);
    if (DepthStencilView)
        DeviceContext->ClearDepthStencilView(DepthStencilView,
                                             D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    */
    
    ID3D11RenderTargetView* ActiveRTV = RenderTargetView;
    ID3D11DepthStencilView* ActiveDSV = DepthStencilView;
    D3D11_VIEWPORT          ActiveVP = Viewport;

    if (bUseSceneRenderTargetOverride)
    {
        ActiveRTV = SceneRenderTargetView;
        ActiveDSV = SceneDepthStencilView;
        ActiveVP = SceneViewport;
        if (ActiveRTV && ActiveRTV != RenderTargetView)
            DeviceContext->ClearRenderTargetView(ActiveRTV, ClearColor);
        if (ActiveDSV && ActiveDSV != DepthStencilView)
            DeviceContext->ClearDepthStencilView(ActiveDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
                                                 1.0f, 0);
    }

    DeviceContext->OMSetRenderTargets(1, &ActiveRTV, ActiveDSV);
    DeviceContext->RSSetViewports(1, &ActiveVP);
    ClearCommandList();
}

void FGeneralRenderer::EndFrame()
{
    if (RenderTargetView)
    {
        DeviceContext->OMSetRenderTargets(1, &RenderTargetView, DepthStencilView);
        DeviceContext->RSSetViewports(1, &Viewport);
    }

    if (GUIRender)
        GUIRender();

    UINT    SyncInterval = bVSyncEnabled ? 1 : 0;
    HRESULT Hr = SwapChain->Present(SyncInterval, 0);
    if (Hr == DXGI_STATUS_OCCLUDED)
        bSwapChainOccluded = true;

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
    if (DepthStencilView)
        DepthStencilView->Release();
    if (RenderTargetView)
        RenderTargetView->Release();
    if (SwapChain)
        SwapChain->Release();
    if (DeviceContext)
        DeviceContext->Release();
    if (Device)
        Device->Release();
}

bool FGeneralRenderer::IsOccluded()
{
    if (bSwapChainOccluded && SwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
        return true;
    bSwapChainOccluded = false;
    return false;
}

void FGeneralRenderer::OnResize(int32 NewWidth, int32 NewHeight)
{
    if (NewWidth == 0 || NewHeight == 0)
        return;
    ClearSceneRenderTarget();
    DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
    if (RenderTargetView)
    {
        RenderTargetView->Release();
        RenderTargetView = nullptr;
    }
    if (DepthStencilView)
    {
        DepthStencilView->Release();
        DepthStencilView = nullptr;
    }
    SwapChain->ResizeBuffers(0, NewWidth, NewHeight, DXGI_FORMAT_UNKNOWN, 0);
    CreateRenderTargetAndDepthStencil(NewWidth, NewHeight);
    Viewport.Width = static_cast<float>(NewWidth);
    Viewport.Height = static_cast<float>(NewHeight);
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
        if (Cmd.MeshData)
            Cmd.MeshData->UpdateVertexAndIndexBuffer(Device);
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

    // if (PostRenderCallback) PostRenderCallback(this);
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
    // PostRenderCallback = nullptr;
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
    DeviceContext->VSSetConstantBuffers(0, 2, CBs);
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
    // PrevCommandCount = CommandList.size();
    // CommandList.reserve(PrevCommandCount);	
}

bool FGeneralRenderer::CreateDeviceAndSwapChain(HWND InHwnd, int32 Width, int32 Height)
{
    DXGI_SWAP_CHAIN_DESC SwapChainDesc = {};
    SwapChainDesc.BufferDesc.Width = Width;
    SwapChainDesc.BufferDesc.Height = Height;
    SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    SwapChainDesc.SampleDesc.Count = 1;
    SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapChainDesc.BufferCount = 2;
    SwapChainDesc.OutputWindow = InHwnd;
    SwapChainDesc.Windowed = TRUE;
    SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    UINT CreateDeviceFlags = 0;
#ifdef _DEBUG
    CreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL FeatureLevel = D3D_FEATURE_LEVEL_11_0;
    HRESULT           Hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        CreateDeviceFlags, &FeatureLevel, 1,
        D3D11_SDK_VERSION, &SwapChainDesc,
        &SwapChain, &Device, nullptr, &DeviceContext
        );

    if (FAILED(Hr))
    {
        MessageBox(nullptr, L"D3D11CreateDeviceAndSwapChain Failed.", nullptr, 0);
        return false;
    }

    return true;
}

bool FGeneralRenderer::CreateRenderTargetAndDepthStencil(int32 Width, int32 Height)
{
    ID3D11Texture2D* BackBuffer = nullptr;
    HRESULT          Hr = SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&BackBuffer);
    if (FAILED(Hr))
        return false;

    Hr = Device->CreateRenderTargetView(BackBuffer, nullptr, &RenderTargetView);
    BackBuffer->Release();
    if (FAILED(Hr))
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

    ID3D11Texture2D* DepthTex = nullptr;
    Hr = Device->CreateTexture2D(&DepthDesc, nullptr, &DepthTex);
    if (FAILED(Hr))
        return false;

    Hr = Device->CreateDepthStencilView(DepthTex, nullptr, &DepthStencilView);
    DepthTex->Release();

    return SUCCEEDED(Hr);
}

bool FGeneralRenderer::CreateConstantBuffers()
{
    D3D11_BUFFER_DESC Desc = {};
    Desc.Usage = D3D11_USAGE_DYNAMIC;
    Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    Desc.ByteWidth = sizeof(FFrameConstantBuffer);
    if (FAILED(Device->CreateBuffer(&Desc, nullptr, &FrameConstantBuffer)))
        return false;

    Desc.ByteWidth = sizeof(FObjectConstantBuffer);
    return SUCCEEDED(Device->CreateBuffer(&Desc, nullptr, &ObjectConstantBuffer));
}

void FGeneralRenderer::UpdateFrameConstantBuffer()
{
    FFrameConstantBuffer CBData;
    CBData.View = ViewMatrix;
    CBData.Projection = ProjectionMatrix;
    D3D11_MAPPED_SUBRESOURCE Mapped;
    if (SUCCEEDED(DeviceContext->Map(FrameConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Mapped)))
    {
        memcpy(Mapped.pData, &CBData, sizeof(CBData));
        DeviceContext->Unmap(FrameConstantBuffer, 0);
    }
}

void FGeneralRenderer::UpdateObjectConstantBuffer(const FMatrix& WorldMatrix)
{
    FObjectConstantBuffer CBData;
    CBData.World = WorldMatrix;
    D3D11_MAPPED_SUBRESOURCE Mapped;
    if (SUCCEEDED(DeviceContext->Map(ObjectConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Mapped)))
    {
        memcpy(Mapped.pData, &CBData, sizeof(CBData));
        DeviceContext->Unmap(ObjectConstantBuffer, 0);
    }
}

void FGeneralRenderer::ClearDepthBuffer()
{
    if (!SceneDepthStencilView)
        return;
    DeviceContext->ClearDepthStencilView(SceneDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
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

            // RenderStateManager를 통한 일괄 상태 바인딩 (캐싱 활용)
            RenderStateManager->BindState(Cmd.Material->GetRasterizerState());
            RenderStateManager->BindState(Cmd.Material->GetDepthStencilState());
            RenderStateManager->BindState(Cmd.Material->GetBlendState());

            CurrentMaterial = Cmd.Material;

            /** 특수 머티리얼 아틀라스 바인딩 보조 */
            // if (CurrentMaterial->GetOriginName() == "M_Font")
            // {
            //     DeviceContext->PSSetShaderResources(0, 1, &FontSRV);
            //     DeviceContext->PSSetSamplers(0, 1, &FontSampler);
            // }
            // else if (CurrentMaterial->GetOriginName() == "M_SubUV")
            // {
            //     DeviceContext->PSSetShaderResources(0, 1, &SubUVSRV);
            //     DeviceContext->PSSetSamplers(0, 1, &SubUVSampler);
            // }
            // else
            {
                // SRV 는 일반 Material 안에서 bind
                DeviceContext->PSSetSamplers(0, 1, &NormalSampler);
            }
        }

        if (Cmd.MeshData != CurrentMesh)
        {
            Cmd.MeshData->Bind(DeviceContext);
            CurrentMesh = Cmd.MeshData;
        }

        D3D11_PRIMITIVE_TOPOLOGY DesiredTopology = (D3D11_PRIMITIVE_TOPOLOGY)CurrentMesh->Topology;
        if (DesiredTopology != CurrentMeshTopology)
        {
            DeviceContext->IASetPrimitiveTopology(DesiredTopology);
            CurrentMeshTopology = DesiredTopology;
        }

        UpdateObjectConstantBuffer(Cmd.WorldMatrix);

        if (!Cmd.MeshData->Indices.empty())
            DeviceContext->DrawIndexed(static_cast<UINT>(Cmd.MeshData->Indices.size()), 0, 0);
        else if (!Cmd.MeshData->Vertices.empty())
            DeviceContext->Draw(static_cast<UINT>(Cmd.MeshData->Vertices.size()), 0);
    }
}

void FGeneralRenderer::InitializeAABBResources()
{
    /** 1. AABB용 Material 생성 (NewShaderLine.hlsl) */
    std::wstring ShaderDirW = FPaths::ShaderDir();
    std::wstring LineVSPath = ShaderDirW + L"\\NewShaderLine.hlsl";
    std::wstring LinePSPath = ShaderDirW + L"\\NewShaderLine.hlsl";

    auto VS = FShaderMap::Get().GetOrCreateVertexShader(Device, LineVSPath, L"VSMain");
    auto PS = FShaderMap::Get().GetOrCreatePixelShader(Device, LinePSPath, L"PSMain");

    AABBMaterial = std::make_shared<FMaterial>();
    AABBMaterial->SetOriginName("M_AABB");
    AABBMaterial->SetVertexShader(VS);
    AABBMaterial->SetPixelShader(PS);

    // 라인 렌더링용 상태 설정
    FRasterizerStateOption rasterizerOption;
    rasterizerOption.FillMode = D3D11_FILL_SOLID;
    rasterizerOption.CullMode = D3D11_CULL_NONE; // 라인이므로 컬링 불필요
    AABBMaterial->SetRasterizerOption(rasterizerOption);
    AABBMaterial->SetRasterizerState(RenderStateManager->GetOrCreateRasterizerState(rasterizerOption));

    FDepthStencilStateOption depthStencilOption;
    depthStencilOption.DepthEnable = true;
    depthStencilOption.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; // AABB는 깊이 쓰기 안함
    AABBMaterial->SetDepthStencilOption(depthStencilOption);
    AABBMaterial->SetDepthStencilState(RenderStateManager->GetOrCreateDepthStencilState(depthStencilOption));

    /** 2. AABB용 MeshData 생성 (1x1x1 Wireframe Cube) */
    AABBMeshData = std::make_unique<FMeshData>();
    AABBMeshData->Topology = EMeshTopology::EMT_LineList;

    // 정점 8개 (0~1 범위), 노란색(1,1,0,1)
    FVector4 Yellow(1.0f, 1.0f, 0.0f, 1.0f);
    AABBMeshData->Vertices = {
        { FVector(0, 0, 0), Yellow }, { FVector(1, 0, 0), Yellow }, { FVector(1, 1, 0), Yellow }, { FVector(0, 1, 0), Yellow },
        { FVector(0, 0, 1), Yellow }, { FVector(1, 0, 1), Yellow }, { FVector(1, 1, 1), Yellow }, { FVector(0, 1, 1), Yellow }
    };

    // 인덱스 24개 (12개 선)
    AABBMeshData->Indices = {
        0, 1, 1, 2, 2, 3, 3, 0, // 바닥
        4, 5, 5, 6, 6, 7, 7, 4, // 천장
        0, 4, 1, 5, 2, 6, 3, 7  // 기둥
    };

    AABBMeshData->CreateVertexAndIndexBuffer(Device);
}

void FGeneralRenderer::DrawAllAABBLines(ERenderLayer InRenderLayer)
{
    if (!AABBMeshData || !AABBMaterial)
        return;

    AABBMaterial->Bind(DeviceContext);
    RenderStateManager->BindState(AABBMaterial->GetRasterizerState());
    RenderStateManager->BindState(AABBMaterial->GetDepthStencilState());

    AABBMeshData->Bind(DeviceContext);
    DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    for (const auto& Cmd : CommandList)
    {
        if (Cmd.RenderLayer != InRenderLayer || !Cmd.bDrawAABB)
            continue;

        // AABB Min/Max를 이용해 Scale & Translation 행렬 생성
        const FVector& Min = Cmd.WorldAABB.Min;
        const FVector& Max = Cmd.WorldAABB.Max;
        const FVector  Size = Max - Min;

        // 0~1 범위 큐브를 AABB 크기와 위치에 맞게 변형
        FMatrix AABBMatrix = FMatrix::MakeScale(Size) * FMatrix::MakeTranslation(Min);

        UpdateObjectConstantBuffer(AABBMatrix);
        DeviceContext->DrawIndexed(static_cast<UINT>(AABBMeshData->Indices.size()), 0, 0);
    }
}