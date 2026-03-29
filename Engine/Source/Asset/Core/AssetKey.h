#pragma once

#include <filesystem>
#include <functional>
#include <type_traits>

#include "Core/CoreMinimal.h"

// =============================== Asset Key Type ==============================

enum class EAssetKeyType : uint8
{
    Unknown = 0,
    Path,
    Derived,
};

// ================================ Asset Key =================================

struct FAssetKey
{
    EAssetKeyType KeyType = EAssetKeyType::Unknown;
    FString       Value;

    FAssetKey() = default;

    FAssetKey(EAssetKeyType InKeyType, FString InValue)
        : KeyType(InKeyType), Value(std::move(InValue))
    {
    }

    bool IsValid() const { return !Value.empty(); }

    void Reset()
    {
        KeyType = EAssetKeyType::Unknown;
        Value.clear();
    }

    bool operator==(const FAssetKey& Other) const
    {
        return KeyType == Other.KeyType && Value == Other.Value;
    }

    bool operator!=(const FAssetKey& Other) const { return !(*this == Other); }
};

// ================================ Path Key ==================================

struct FPathKey : public FAssetKey
{
    FPathKey()
        : FAssetKey(EAssetKeyType::Path, FString{})
    {
    }

    explicit FPathKey(const FString& InPath)
        : FAssetKey(EAssetKeyType::Path, InPath)
    {
    }

    explicit FPathKey(FString&& InPath)
        : FAssetKey(EAssetKeyType::Path, std::move(InPath))
    {
    }

    explicit FPathKey(const FWString& InPath)
        : FAssetKey(EAssetKeyType::Path, std::filesystem::path(InPath).string())
    {
    }
};

// =============================== Derived Key ================================

struct FDerivedKey : public FAssetKey
{
    FDerivedKey()
        : FAssetKey(EAssetKeyType::Derived, FString{})
    {
    }

    explicit FDerivedKey(const FString& InValue)
        : FAssetKey(EAssetKeyType::Derived, InValue)
    {
    }

    explicit FDerivedKey(FString&& InValue)
        : FAssetKey(EAssetKeyType::Derived, std::move(InValue))
    {
    }
};

// ================================ Hash Glue =================================

namespace std
{
    template <>
    struct hash<FAssetKey>
    {
        size_t operator()(const FAssetKey& Key) const
        {
            const size_t TypeHash = std::hash<int>{}(static_cast<int>(Key.KeyType));
            const size_t ValueHash = std::hash<FString>{}(Key.Value);
            return TypeHash ^ (ValueHash + 0x9e3779b9 + (TypeHash << 6) + (TypeHash >> 2));
        }
    };

    template <>
    struct hash<FPathKey>
    {
        size_t operator()(const FPathKey& Key) const
        {
            return std::hash<FAssetKey>{}(static_cast<const FAssetKey&>(Key));
        }
    };

    template <>
    struct hash<FDerivedKey>
    {
        size_t operator()(const FDerivedKey& Key) const
        {
            return std::hash<FAssetKey>{}(static_cast<const FAssetKey&>(Key));
        }
    };
}
