#include "EditorViewportTab.h"
#include "Viewport/Layout/EditorViewportLayoutFourPanes.h"

SEditorViewportTab::SEditorViewportTab() {}

SEditorViewportTab::~SEditorViewportTab() 
{
    for (auto* V : SceneViews)
	{
        delete V;
	}

	for (auto* C : ViewportClients)
    {
        C->Release();
        delete C;
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
        FSceneView*            NewViewport = new FSceneView();
        FEditorViewportClient* NewClient = new FEditorViewportClient();

        NewClient->Create();
        NewViewport->SetViewportClient(NewClient);

        SceneViews.push_back(NewViewport);
        ViewportClients.push_back(NewClient);
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
        if (SceneViews[i]->IsValid())
        {
            SceneViews[i]->OnResize(windows[i]->GetViewportRect());
            SceneViews[i]->GetViewportClient()->OnResize(windows[i]->GetViewportRect().Width,
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

    for (int32 i = 0; i < (int32)SceneViews.size(); i++)
    {
        if (i < Required)
        {
            if (!SceneViews[i]->IsValid())
                SceneViews[i]->SetViewportClient(ViewportClients[i]);
        }
        else
        {
            SceneViews[i]->RemoveViewportClient();
        }
    }
}

void SEditorViewportTab::InitializeControlPanels(FEditorContext* Context)
{
    for (int i = 0; i < SceneViews.size(); i++)
    {
        FControlPanel* Panel = new FControlPanel();
        Panel->Initialize(Context, nullptr);
        Panel->SetViewportIndex(i);
        ControlPanels.push_back(Panel);
    }
}

void SEditorViewportTab::DrawControlPanels() 
{
    for (int i = 0; i < SceneViews.size(); i++)
    {
        if (SceneViews[i]->IsValid())
            ControlPanels[i]->Draw(); 
    }
}
