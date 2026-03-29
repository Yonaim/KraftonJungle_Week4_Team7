#include "EditorViewportLayoutSinglePane.h"

FEditorViewportLayoutSinglePane::~FEditorViewportLayoutSinglePane()
{
    delete WindowA;
    WindowA = nullptr;
}

void FEditorViewportLayoutSinglePane::Initialize(FViewportRect TotalRect) 
{
    WindowA = new SWindow();
    RootSplitter = nullptr;

    Type == EViewportLayoutType::Single;
    WindowA->SetViewportRect(TotalRect);
}

void FEditorViewportLayoutSinglePane::Resize(FViewportRect NewRect) 
{
    if (WindowA != nullptr)
        WindowA->SetViewportRect(NewRect);
}

TArray<SWindow*> FEditorViewportLayoutSinglePane::GetLeafWindows() const
{ 
    return { WindowA };
}
