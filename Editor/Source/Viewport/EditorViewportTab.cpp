#include "EditorViewportTab.h"
#include "Viewport/Layout/FEditorViewportLayoutFourPanes.h"

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
    for (int32 i = 0; i < 4; i++)
    {
        FSceneView*            NewViewport = new FSceneView();
        FEditorViewportClient* NewClient = new FEditorViewportClient();

        NewClient->Create();
        NewViewport->SetViewportClient(NewClient);

        SceneViews.push_back(NewViewport);
        ViewportClients.push_back(NewClient);
    }

    ViewportLayout = new FEditorViewportLayoutFourPanes();
    ViewportLayout->Initialize({0, 0, 0, 0});
}

void SEditorViewportTab::OnResize(FViewportRect WindowRect)
{ 
    ViewportLayout->Resize(WindowRect);

    auto windows = ViewportLayout->GetLeafWindows();
    SceneViews[0]->OnResize(windows[0]->GetViewportRect());
    SceneViews[0]->GetViewportClient()->OnResize(windows[0]->GetViewportRect().Width, 
                                                 windows[0]->GetViewportRect().Height);
    
    SceneViews[1]->OnResize(windows[1]->GetViewportRect());
    SceneViews[1]->GetViewportClient()->OnResize(windows[1]->GetViewportRect().Width,
                                                 windows[1]->GetViewportRect().Height);

    SceneViews[2]->OnResize(windows[2]->GetViewportRect());
    SceneViews[2]->GetViewportClient()->OnResize(windows[2]->GetViewportRect().Width,
                                                 windows[2]->GetViewportRect().Height);

    SceneViews[3]->OnResize(windows[3]->GetViewportRect());
    SceneViews[3]->GetViewportClient()->OnResize(windows[3]->GetViewportRect().Width,
                                                 windows[3]->GetViewportRect().Height);
}

void SEditorViewportTab::CreateExtraViewportClients() 
{
    while (ViewportClients.size() < 4)
    {
        FEditorViewportClient* NewClient = new FEditorViewportClient();
        NewClient->Create();
        for (auto V : SceneViews)
        {
            if (V->GetViewportClient() == nullptr)
            {
                V->SetViewportClient(NewClient);
            }
        }
        ViewportClients.push_back(NewClient);
    }
}

void SEditorViewportTab::RemoveExtraViewportClients()
{
    while (ViewportClients.size() > 1)
    {
        SceneViews.back()->RemoveViewportClient();

        ViewportClients.back()->Release();
        delete ViewportClients.back();
        ViewportClients.pop_back();
    }
}


