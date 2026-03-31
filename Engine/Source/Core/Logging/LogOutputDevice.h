#pragma once

#include "Core/EngineAPI.h"

enum class ELogLevel
{
    Debug,
    Info,
    Warning,
    Error,
    Fatal
};

class ENGINE_API ILogOutputDevice
{
  public:
    virtual ~ILogOutputDevice() = default;
    virtual void Log(ELogLevel Level, const char *Message) = 0;
};