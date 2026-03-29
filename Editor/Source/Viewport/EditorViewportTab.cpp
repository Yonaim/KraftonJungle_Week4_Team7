#include "EditorViewportTab.h"
#include "Viewport/Layout/EditorViewportLayoutFourPanes.h"

SEditorViewportTab::SEditorViewportTab() {}

SEditorViewportTab::~SEditorViewportTab() 
{
    for (auto V : SceneViews)
	{
        delete V;
	}

	for (auto C : ViewportClients)
    {
        C->Release();
        delete C;
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

void SEditorViewportTab::OnResize(FViewportRect WindowRect)
{ 
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

    OnResize(CurrentRect);
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
