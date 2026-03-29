#pragma once

#include <memory>
#include <type_traits>

#include "Asset/Core/Types.h"
#include "Asset/Core/AssetDataBase.h"
#include "Asset/Core/AssetKey.h"

// ============================== Generic Cache ===============================

template <typename TData> class TAssetCache
{
  public:
    std::shared_ptr<TData> Find(const FAssetKey& Key) const
    {
        auto It = Records.find(Key);
        return It != Records.end() ? It->second : nullptr;
    }

    template <typename TKey>
    std::shared_ptr<TData> Find(const TKey& Key) const
    {
        static_assert(std::is_base_of_v<FAssetKey, TKey>, "TKey must derive from FAssetKey.");
        return Find(static_cast<const FAssetKey&>(Key));
    }

    void Insert(const FAssetKey& Key, const std::shared_ptr<TData>& Data)
    {
        Records[Key] = Data;
    }

    template <typename TKey>
    void Insert(const TKey& Key, const std::shared_ptr<TData>& Data)
    {
        static_assert(std::is_base_of_v<FAssetKey, TKey>, "TKey must derive from FAssetKey.");
        Insert(static_cast<const FAssetKey&>(Key), Data);
    }

    void Remove(const FAssetKey& Key) { Records.erase(Key); }

    template <typename TKey>
    void Remove(const TKey& Key)
    {
        static_assert(std::is_base_of_v<FAssetKey, TKey>, "TKey must derive from FAssetKey.");
        Remove(static_cast<const FAssetKey&>(Key));
    }

    void Clear() { Records.clear(); }

  private:
    TMap<FAssetKey, std::shared_ptr<TData>> Records;
};

// =============================== Type Aliases ===============================

template <typename TIntermediate>
using TIntermediateCache = TAssetCache<TIntermediate>;

template <typename TCooked>
using TCookedCache = TAssetCache<TCooked>;
