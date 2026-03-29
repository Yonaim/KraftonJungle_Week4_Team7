#pragma once

#include "Object.h"

extern TArray<UObject*> GObjectArray;

template <typename TObject> 
class FObjectIterator
{
  public:
    FObjectIterator() : CurrentIndex(0) { AdvanceToNextValidObject(); }

    explicit operator bool() const
    {
        return CurrentIndex < static_cast<int32>(GUObjectArray.size());
    }

    FObjectIterator& operator++()
    {
        ++CurrentIndex;
        AdvanceToNextValidObject();
        return *this;
    }

    TObject* operator*() const { return static_cast<TObject*>(GUObjectArray[CurrentIndex]); }

    TObject* operator->() const { return static_cast<TObject*>(GUObjectArray[CurrentIndex]); }

  private:
    void AdvanceToNextValidObject()
    {
        const int32 Count = static_cast<int32>(GUObjectArray.size());
        while (CurrentIndex < Count)
        {
            UObject* Obj = GUObjectArray[CurrentIndex];
            if (Obj != nullptr && Obj->IsA(TObject::GetClass()))
            {
                return;
            }
            ++CurrentIndex;
        }
    }

  private:
    int32 CurrentIndex;
};