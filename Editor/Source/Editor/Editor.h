#pragma once
#pragma once

#include "Core/CoreMinimal.h"

#include "Chrome/EditorChrome.h"
#include "Content/EditorContentIndex.h"
#include "EditorContext.h"
#include "EditorSettings.h"
#include "Menu/EditorMenuRegistry.h"

#include "ApplicationCore/Input/InputRouter.h"
#include "ApplicationCore/Input/InputSystem.h"
#include "Viewport/Global/EditorGlobalController.h"
#include "Input/EditorGlobalContext.h"
#include "Engine/Scene.h"
#include "Engine/World.h"
#include "Logging/EditorLogBuffer.h"
#include "Viewport/EditorViewportTab.h"
#include "Renderer/EditorRenderData.h"
#include "Renderer/SceneRenderData.h"
#include "Renderer/SceneView.h"

#include <filesystem>
#include <memory>

class FPanelManager;
struct FPanelDescriptor;
class AActor;
class UObject;
class FD3D11RHI;
class UAssetManager;
//struct FTextureResource;

enum class EDeferredSceneActionType
{
    None,
    NewScene,
    ClearScene,
    OpenScene,
    CloseEditor
};

struct FDeferredSceneAction
{
    EDeferredSceneActionType Type = EDeferredSceneActionType::None;
    std::filesystem::path    ScenePath;

    void Reset()
    {
        Type = EDeferredSceneActionType::None;
        ScenePath.clear();
    }
};

struct FSceneDocumentState
{
    std::filesystem::path CurrentScenePath;
    bool                  bDirty = false;

    FDeferredSceneAction DeferredAction;
};

class FEditor
{
  public:
    void Create();
    void Release();

    void Initialize();
    void Tick(float DeltaTime, Engine::ApplicationCore::FInputSystem* InputSystem);
    void SetChromeHost(IEditorChromeHost* InChromeHost);
    void SetRuntimeServices(FD3D11RHI* InRHI, UAssetManager* InAssetManager);

    void OnWindowResized(float Width, float Height);
    void OnViewportResized();
    void SetMainLoopFPS(float FPS)
    {
        CurFPS = FPS;
        EditorContext.CurrentFPS = FPS;
    }

    void     CreateNewScene();
    void     ClearScene();
    bool     RequestCloseEditor();
    bool     SaveCurrentSceneToDisk();
    bool     SaveSceneAsPath(const std::filesystem::path& FilePath);
    bool     OpenSceneFromPath(const std::filesystem::path& FilePath);
    void     RequestSaveSceneAs();
    void     RequestOpenSceneDialog();
    void     RequestAboutPopUp();
    bool     CanDeleteSelectedActors() const;
    bool     DeleteSelectedActors();
    void     RefreshContentIndex();
    void     SetSelectedObject(UObject* InSelectedObject);
    void     AddActorToScene(AActor* InActor, bool bSelectActor = true);
    void     MarkSceneDirty();
    UObject* GetSelectedObject() const { return EditorContext.SelectedObject; }
    std::filesystem::path GetCurrentScenePath() const { return SceneDocument.CurrentScenePath; }
    std::filesystem::path GetDefaultSceneDirectory() const;

    const TArray<FEditorRenderData>& GetEditorRenderData() const { return EditorRenderDatas; }
    const TArray<FSceneRenderData>&  GetSceneRenderData() const { return SceneRenderDatas; }
    SEditorViewportTab&              GetViewportTab() { return ViewportTab; }

    void DrawPanel();

  private:
    void BuildRenderData();
    void BuildSceneView();
    // 커스텀 타이틀바 아래 전체 영역을 도킹 가능한 루트 dockspace로 사용합니다.
    void DrawRootDockSpace();
    void DrawAboutPopup();
    void EnsureAboutImageLoaded();
    void RequestAboutPopup();
    void RegisterDefaultCommands();
    void RegisterDefaultMenus();
    void RegisterWindowPanelCommand(const FPanelDescriptor& Descriptor);
    void LoadEditorSettings();
    void SaveEditorSettings() const;
    bool SaveScene();
    void SaveSceneAs();
    void RequestOpenScene();
    void MarkSceneClean();
    void PerformNewScene();
    void PerformClearScene();
    bool SaveSceneToPath(const std::filesystem::path& FilePath, bool bUpdateCurrentPath);
    bool LoadSceneFromPath(const std::filesystem::path& FilePath);
    void ReplaceCurrentScene(std::unique_ptr<FScene> NewScene);
    bool ConfirmProceedWithDirtyScene(const FDeferredSceneAction& Action);
    void ExecuteDeferredSceneAction(FDeferredSceneAction Action);
    std::filesystem::path GetSceneDirectory() const;
    void ResolveActorAssetReferences(AActor* Actor);
    void ResolveSceneAssetReferences(FScene* Scene);


  private:
    uint32_t           RootDockSpaceId = 0;
    SEditorViewportTab ViewportTab; 

    Engine::ApplicationCore::FInputRouter GlobalInputRouter;
    FEditorGlobalController GlobalInputController;
    FEditorGlobalContext GlobalInputContext{&GlobalInputController};

    FEditorContext        EditorContext;
    FEditorSettings       PersistentSettings;
    FEditorContentIndex   ContentIndex;
    FPanelManager*        PanelManager = nullptr;
    FEditorChrome         EditorChrome;
    FEditorMenuRegistry   MenuRegistry;
    IEditorChromeHost*    ChromeHost = nullptr;

    TArray<FEditorRenderData> EditorRenderDatas;
    TArray<FSceneRenderData>  SceneRenderDatas;

    FWorld*             CurWorld = nullptr;
    FSceneDocumentState SceneDocument;

    /* Logging */
    FEditorLogBuffer LogBuffer;

    float WindowWidth = 0.0f;
    float WindowHeight = 0.0f;
    float CurFPS = 0.0f;
    bool  bRequestOpenAboutPopup = false;
    bool  bAboutPopupOpen = false;
    bool  bAttemptedAboutImageLoad = false;
    FTextureResource* AboutImageResource = nullptr;
};
