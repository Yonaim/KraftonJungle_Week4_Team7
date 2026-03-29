#pragma once

#include "Core/CoreMinimal.h"
#include "Window.h"

class FEditorViewportLayout
{
  public:
    virtual ~FEditorViewportLayout() = default;
    
    // 초기 트리 구성
    virtual void Initialize(FViewportRect TotalRect) = 0;

    // 창 크기 변경 시 재구성
    virtual void Resize(FViewportRect NewRect) = 0;

    // 리프 노드 반환 (최종 결과)
    virtual TArray<SWindow*> GetLeafWindows() const = 0;

protected:
    SSplitter* RootSplitter = nullptr;
};