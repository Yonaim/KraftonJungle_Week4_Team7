#include "EditorViewportTab.h"

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
}

void SEditorViewportTab::Construct() 
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

    SceneViews[0]->SetViewRect({0, 0, 960, 540});     // 좌상
    SceneViews[1]->SetViewRect({960, 0, 960, 540});   // 우상
    SceneViews[2]->SetViewRect({0, 540, 960, 540});   // 좌하
    SceneViews[3]->SetViewRect({960, 540, 960, 540}); // 우하
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


