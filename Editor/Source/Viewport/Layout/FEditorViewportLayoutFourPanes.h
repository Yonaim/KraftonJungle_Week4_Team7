#pragma once

#include "EditorViewportLayout.h"

enum class EFourPanesType
{
    _2X2,
    _1l3,
    _3l1,
    _1_3,
    _3_1
};

class FEditorViewportLayoutFourPanes : public FEditorViewportLayout
{
public:
    FEditorViewportLayoutFourPanes() = default;
    ~FEditorViewportLayoutFourPanes() override;

public:
    void             SetLayoutType(EFourPanesType NewType) { Type = NewType; }
    void             Initialize(FViewportRect TotalRect) override;
    void             Resize(FViewportRect NewRect) override;
    TArray<SWindow*> GetLeafWindows() const override;

private:
    void Build_2X2Tree();
    void Build_1l3Tree();
    void Build_3l1Tree();
    void Build_1_3Tree();
    void Build_3_1Tree();

private:
    EFourPanesType Type = EFourPanesType::_1l3;

    SWindow* WindowA = nullptr; 
    SWindow* WindowB = nullptr;
    SWindow* WindowC = nullptr;
    SWindow* WindowD = nullptr;

    SSplitter* Splitter1 = nullptr;
    SSplitter* Splitter2 = nullptr;
};