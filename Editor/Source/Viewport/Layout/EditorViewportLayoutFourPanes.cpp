#include "EditorViewportLayoutFourPanes.h"
#include "EditorViewportLayoutSinglePane.h"

FEditorViewportLayoutFourPanes::~FEditorViewportLayoutFourPanes()
{
    delete WindowA;
    delete WindowB;
    delete WindowC;
    delete WindowD;

    delete Splitter1;
    delete Splitter2;
}

void FEditorViewportLayoutFourPanes::Initialize(FViewportRect TotalRect) 
{
    WindowA = new SWindow();
    WindowB = new SWindow();
    WindowC = new SWindow();
    WindowD = new SWindow();

    // Root 설정
    SSplitterH* RootSH = new SSplitterH(0.5f);
    RootSH->SetViewportRect(TotalRect);
    RootSplitter = RootSH;

    // Build Tree
    if (Type == EViewportLayoutType::_2X2)
    {
        Build_2X2Tree();
    }
    else if (Type == EViewportLayoutType::_1l3)
    {
        Build_1l3Tree();
    }
    else if (Type == EViewportLayoutType::_3l1)
    {
        Build_3l1Tree();
    }
    else if (Type == EViewportLayoutType::_1_3)
    {
        Build_1_3Tree();
    }
    else if (Type == EViewportLayoutType::_3_1)
    {
        Build_3_1Tree();
    }
}

void FEditorViewportLayoutFourPanes::Resize(FViewportRect NewRect) 
{
    RootSplitter->SetViewportRect(NewRect);
    RootSplitter->LayoutChildren();

    Splitter1->LayoutChildren();
    Splitter2->LayoutChildren();
}

TArray<SWindow*> FEditorViewportLayoutFourPanes::GetLeafWindows() const
{
    return {WindowA, WindowB, WindowC, WindowD};
}

void FEditorViewportLayoutFourPanes::Build_2X2Tree()
{
    //  SSplitterH (root)
    //  ├─ SideLT: SSplitterV
    //  │   ├─ SideLT: WindowA  (좌상)
    //  │   └─ SideRB: WindowB  (좌하)
    //  └─ SideRB: SSplitterV
    //      ├─ SideLT: WindowC  (우상)
    //      └─ SideRB: WindowD  (우하)

    if (RootSplitter == nullptr)
        return;

    Splitter1 = new SSplitterV(0.5f);
    Splitter2 = new SSplitterV(0.5f);

    RootSplitter->SetSideLT(Splitter1);
    RootSplitter->SetSideRB(Splitter2);
    RootSplitter->LayoutChildren();

    Splitter1->SetSideLT(WindowA);
    Splitter1->SetSideRB(WindowB);
    Splitter1->LayoutChildren();

    Splitter2->SetSideLT(WindowC);
    Splitter2->SetSideRB(WindowD);
    Splitter2->LayoutChildren();
}

void FEditorViewportLayoutFourPanes::Build_1l3Tree() 
{
    // SSplitterH (root)
    // ├─ SideLT: WindowA          (좌)
    // └─ SideRB: SSplitterV
    //     ├─ SideLT: WindowB      (우상)
    //     └─ SideRB: SSplitterV
    //         ├─ SideLT: WindowC  (우중)
    //         └─ SideRB: WindowD  (우하)

    if (RootSplitter == nullptr)
        return;

    Splitter1 = new SSplitterV(0.33f);
    Splitter2 = new SSplitterV(0.5f);

    RootSplitter->SetSideLT(WindowA);
    RootSplitter->SetSideRB(Splitter1);
    RootSplitter->LayoutChildren();

    Splitter1->SetSideLT(WindowB);
    Splitter1->SetSideRB(Splitter2);
    Splitter1->LayoutChildren();

    Splitter2->SetSideLT(WindowC);
    Splitter2->SetSideRB(WindowD);
    Splitter2->LayoutChildren();
}

void FEditorViewportLayoutFourPanes::Build_3l1Tree() {}

void FEditorViewportLayoutFourPanes::Build_1_3Tree() {}

void FEditorViewportLayoutFourPanes::Build_3_1Tree() {}


