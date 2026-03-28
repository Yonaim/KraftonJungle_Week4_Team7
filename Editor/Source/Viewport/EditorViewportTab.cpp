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

void SEditorViewportTab::Construct(FViewportRect WindowRect)
{ 
	for (int32 i = 0; i < 4; i++)
	{
        FSceneView* NewViewport = new FSceneView();
        FEditorViewportClient* NewClient = new FEditorViewportClient();
		
		NewClient->Create();
		NewViewport->SetViewportClient(NewClient);

		SceneViews.push_back(NewViewport);
        ViewportClients.push_back(NewClient);
	}

    ViewportLayout = new FEditorViewportLayoutFourPanes();
    ViewportLayout->Initialize(WindowRect);

    auto windows = ViewportLayout->GetLeafWindows();
    SceneViews[0]->SetViewRect(windows[0]->GetViewportRect());     
    SceneViews[1]->SetViewRect(windows[1]->GetViewportRect()); 
    SceneViews[2]->SetViewRect(windows[2]->GetViewportRect());          
    SceneViews[3]->SetViewRect(windows[3]->GetViewportRect()); 
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


