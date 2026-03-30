#pragma once

enum class EViewportLayoutType
{
    // Single
    Single,

    //// Two
    //_1l1,
    //_1_1,

    //// Three
    //_1l2,
    //_2l1,
    //_1_2,
    //_2_1,

    // Four
    _2X2,
    _1l3,
    _3l1,
    _1_3,
    _3_1
};

inline int32 GetRequiredViewportCount(EViewportLayoutType Type)
{
    switch (Type)
    {
    case EViewportLayoutType::Single:
        return 1;
    case EViewportLayoutType::_2X2:
    case EViewportLayoutType::_1l3:
        return 4;
    default:
        return 1;
    }
}
