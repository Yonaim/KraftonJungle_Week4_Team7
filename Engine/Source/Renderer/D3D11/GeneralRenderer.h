#pragma once
#include "NewRenderer/Renderer.h"

// === Forward Declaraction
struct FRenderCommandQueue;
// === Forward Declaraction

class FGeneralRenderer
{
public:
    FGeneralRenderer(HWND InHwnd, int32 InWidth, int32 InHeight);
    ~FGeneralRenderer();
    
    void DEBUG_ForceInitialize(
        ID3D11Device* InDevice, ID3D11DeviceContext* InDeviceContext, 
        ID3D11RenderTargetView* InRTV, ID3D11DepthStencilView* InDSV, 
        D3D11_VIEWPORT InVP, IDXGISwapChain* InSwapChain);
    void BeginFrame();
    void EndFrame();        // 스왑체인, 콜백 등 담당
    void Release();
    bool IsOccluded();
    void OnResize(int32 NewWidth, int32 NewHeight);
    
    /** TODO: RHI 제거 후, GeneralRenderer가 직접 자원을 관리할 때 삭제 */
    void DEBUG_UpdateViewResources(ID3D11RenderTargetView* InRTV, ID3D11DepthStencilView* InDSV, const D3D11_VIEWPORT& InVP);

    void SetSceneRenderTarget(ID3D11RenderTargetView* InRenderTargetView, ID3D11DepthStencilView* InDepthStencilView, const D3D11_VIEWPORT& InViewport);
    void ClearSceneRenderTarget();
    void SetVSync(bool bEnable) { bVSyncEnabled = bEnable; }
    bool IsVSyncEnabled() const { return bVSyncEnabled; }
    
    void SubmitCommands(const FRenderCommandQueue& Queue);
    void AddCommand(const FRenderCommand& Command);
    void ClearCommandList();
    void ExecuteCommands();
    
    bool Pick(int32 MouseX, int32 MouseY, uint32& OutPickId);

    // ─── GUI 및 콜백 ───
    /** ImGui 등 외부 GUI 시스템 연동용 콜백 */
    void SetGUICallbacks(FGUICallback InInit, FGUICallback InShutdown, FGUICallback InNewFrame, FGUICallback InRender, FGUICallback InPostPresent = nullptr);
    void ClearViewportCallbacks();
    void SetGUIUpdateCallback(FGUICallback InUpdate);
    // void SetPostRenderCallback(FPostRenderCallback InCallback) { PostRenderCallback = std::move(InCallback); }
    
    static FMaterial* GetDefaultMaterial() { return DefaultMaterial.get(); }
    static FMaterial* GetDefaultSpriteMaterial() { return DefaultSpriteMaterial.get(); }
    FMaterial* GetLineMaterial() const { return AABBMaterial.get(); }
    std::unique_ptr<CRenderStateManager>& GetRenderStateManager() { return RenderStateManager; }
    ID3D11Device* GetDevice() const { return Device; }
    ID3D11DeviceContext* GetDeviceContext() const { return DeviceContext; }
    ID3D11RenderTargetView* GetRenderTargetView() const { return RenderTargetView; }
    IDXGISwapChain* GetSwapChain() const { return SwapChain; };
    HWND GetHwnd() const { return Hwnd; }

    FVector GetCameraPosition() const;
    
private:
    bool Initialize(HWND InHwnd, int32 Width, int32 Height);
    bool InitializeDefaultMaterial();
    
    void SetConstantBuffers();
    bool CreateDeviceAndSwapChain(HWND InHwnd, int32 Width, int32 Height);
    bool CreateRenderTargetAndDepthStencil(int32 Width, int32 Height);
    bool CreateConstantBuffers();
    void UpdateFrameConstantBuffer();
    void UpdateObjectConstantBuffer(const FMatrix& WorldMatrix, uint32 ObjectId = 0);
    void ClearDepthBuffer();
    
    void ExecuteRenderPass(ERenderLayer InRenderLayer);
    void DrawAllAABBLines(ERenderLayer InRenderLayer);
    
    /** AABB 전용 리소스 초기화 */
    void InitializeAABBResources();

    /** 픽킹 리소스 */
    bool CreatePickResources(int32 Width, int32 Height);
    void ReleasePickResources();
    bool ReadBackMousePixel(int32 MouseX, int32 MouseY, uint32& OutObjectId);
    
private:
    std::unique_ptr<CRenderStateManager> RenderStateManager = nullptr;
    HWND Hwnd = nullptr;
    ID3D11Device* Device = nullptr;
    ID3D11DeviceContext* DeviceContext = nullptr;
    IDXGISwapChain* SwapChain = nullptr;
    ID3D11RenderTargetView* RenderTargetView = nullptr;
    ID3D11DepthStencilView* DepthStencilView = nullptr;
    
    ID3D11Buffer* FrameConstantBuffer = nullptr;
    ID3D11Buffer* ObjectConstantBuffer = nullptr;

    // ─── 픽킹 관련 리소스 ───
    ID3D11Texture2D*        PickColorTexture = nullptr;
    ID3D11RenderTargetView* PickRTV = nullptr;
    ID3D11Texture2D*        PickDepthTexture = nullptr;
    ID3D11DepthStencilView* PickDSV = nullptr;
    ID3D11Texture2D*        ReadbackTexture = nullptr;

    ID3D11VertexShader* PickVertexShader = nullptr;
    ID3D11PixelShader*  PickPixelShader = nullptr;
    ID3D11InputLayout*  PickInputLayout = nullptr;
    // ───────────────────────
    
    FMatrix ViewMatrix;
    FMatrix ProjectionMatrix;
    D3D11_VIEWPORT Viewport = {};
    
    ID3D11RenderTargetView* SceneRenderTargetView = nullptr;
    ID3D11DepthStencilView* SceneDepthStencilView = nullptr;
    D3D11_VIEWPORT SceneViewport = {};
    
    bool bUseSceneRenderTargetOverride = false;
    bool bVSyncEnabled = false;
    bool bSwapChainOccluded = false;
    
    TArray<FRenderCommand> CommandList;
    FGUICallback GUIInit;
    FGUICallback GUIShutdown;
    FGUICallback GUINewFrame;
    FGUICallback GUIUpdate;
    FGUICallback GUIRender;
    FGUICallback GUIPostPresent;
    // FPostRenderCallback PostRenderCallback;
    
    /** 기본 공유 리소스 */
    static std::shared_ptr<FMaterial> DefaultMaterial;
    static std::shared_ptr<FMaterial> DefaultSpriteMaterial;
    std::shared_ptr<FMaterial> DefaultTextureMaterial;
    
    /** AABB 전용 리소스 */
    std::shared_ptr<FMeshData> AABBMeshData;
    std::shared_ptr<FMaterial> AABBMaterial;
    
    ID3D11SamplerState* NormalSampler = nullptr;
    
public:
    CShaderManager ShaderManager;
};
