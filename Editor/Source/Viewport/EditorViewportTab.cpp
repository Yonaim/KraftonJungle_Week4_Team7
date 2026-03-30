#include "EditorViewportTab.h"
#include "Viewport/Layout/EditorViewportLayoutFourPanes.h"

SEditorViewportTab::SEditorViewportTab() {}

SEditorViewportTab::~SEditorViewportTab() 
{
    for (auto* Viewport : Viewports)
	{
        Viewport->Release();
        delete Viewport;
	}

    for (auto* P : ControlPanels)
    {
        delete P;
    }

    delete ViewportLayout;
    ViewportLayout = nullptr;
}

void SEditorViewportTab::Construct()
{ 
    // 현재 최대 4개 Viewport만 가능
    for (int32 i = 0; i < 4; i++)
    {
        FViewport*             NewViewport = new FViewport();
        FSceneView*            NewSceneView = new FSceneView();
        FEditorViewportClient* NewViewportClient = new FEditorViewportClient();

        NewViewportClient->Create();
        NewViewport->SetSceneView(NewSceneView);
        NewViewport->SetViewportClient(NewViewportClient);

        Viewports.push_back(NewViewport);
        ViewportClients.push_back(NewViewportClient);
    }

    SetLayout(EViewportLayoutType::_1l3);
}

void SEditorViewportTab::Initialize()
{ 
    SetLayout(EViewportLayoutType::Single); 
}

void SEditorViewportTab::OnResize(FViewportRect WindowRect, bool Force)
{ 
    if (!Force && WindowRect.X == CurrentRect.X && WindowRect.Y == CurrentRect.Y &&
        WindowRect.Width == CurrentRect.Width && WindowRect.Height == CurrentRect.Height)
        return;

    CurrentRect = WindowRect;
    ViewportLayout->Resize(WindowRect);

    auto windows = ViewportLayout->GetLeafWindows();
    for (int i = 0; i < windows.size(); i++)
    {
        if (Viewports[i]->IsValid())
        {
            Viewports[i]->GetSceneView()->OnResize(windows[i]->GetViewportRect());
            Viewports[i]->GetViewportClient()->OnResize(windows[i]->GetViewportRect().Width,
                                                         windows[i]->GetViewportRect().Height);
        }
    }
}

void SEditorViewportTab::SetLayout(EViewportLayoutType NewType)
{
    delete ViewportLayout;
    ViewportLayout = nullptr;

    AdjustViewportCount(NewType);

    ViewportLayout = FEditorViewportLayoutFactory::Create(NewType);
    ViewportLayout->Initialize(CurrentRect);

    OnResize(CurrentRect, true);
    CurrentLayoutType = NewType;
}

void SEditorViewportTab::AdjustViewportCount(EViewportLayoutType NewType) 
{
    int32 Required = GetRequiredViewportCount(NewType);

    for (int32 i = 0; i < (int32)Viewports.size(); i++)
    {
        if (i < Required)
        {
            if (!Viewports[i]->IsValid())
                Viewports[i]->SetViewportClient(ViewportClients[i]);
        }
        else
        {
            Viewports[i]->RemoveViewportClient();
        }
    }
}

void SEditorViewportTab::InitializeControlPanels(FEditorContext* Context)
{
    for (int i = 0; i < Viewports.size(); i++)
    {
        FControlPanel* Panel = new FControlPanel();
        Panel->Initialize(Context, nullptr);
        Panel->SetViewportIndex(i);
        ControlPanels.push_back(Panel);
    }
}

void SEditorViewportTab::DrawControlPanels() 
{
    for (int i = 0; i < Viewports.size(); i++)
    {
        if (Viewports[i]->IsValid())
            ControlPanels[i]->Draw(); 
    }
}
