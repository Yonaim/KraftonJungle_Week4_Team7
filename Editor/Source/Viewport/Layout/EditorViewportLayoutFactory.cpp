#include "EditorViewportLayoutFactory.h"

#include "EditorViewportLayoutSinglePane.h"
#include "EditorViewportLayoutFourPanes.h"

FEditorViewportLayout* FEditorViewportLayoutFactory::Create(EViewportLayoutType Type)
{
    switch (Type)
    {
    case EViewportLayoutType::Single:
    {
        auto* L = new FEditorViewportLayoutSinglePane();
        L->SetLayoutType(EViewportLayoutType::Single);
        return L;
    }
    case EViewportLayoutType::_2X2:
    {
        auto* L = new FEditorViewportLayoutFourPanes();
        L->SetLayoutType(EViewportLayoutType::_2X2);
        return L;
    }
    case EViewportLayoutType::_1l3:
    {
        auto* L = new FEditorViewportLayoutFourPanes();
        L->SetLayoutType(EViewportLayoutType::_1l3);
        return L;
    }
    default:
        return nullptr;
    }
}