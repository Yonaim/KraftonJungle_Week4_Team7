#pragma once

#include <cstdio>
#include "LogOutputDevice.h"
#include "Core/CoreGlobals.h"

inline const char* GetLogLevelLabel(ELogLevel Level)
{
    switch (Level)
    {
    case ELogLevel::Debug:
        return "DEBUG";
    case ELogLevel::Info:
        return "INFO";
    case ELogLevel::Warning:
        return "WARNING";
    case ELogLevel::Error:
        return "ERROR";
    case ELogLevel::Fatal:
        return "FATAL";
    default:
        return "UNKNOWN";
    }
}

/**
 * LogOutput Macro
 *
 * Category    : Category name only (e.g. Console, Renderer, Engine), no registration required
 * Level       : ELogLevel (Debug / Info / Warning / Error / Fatal)
 * Format, ... : printf-style format string
 *
 * Level colors in Console Panel
 *   Debug   : Gray
 *   Info    : White
 *   Warning : Yellow
 *   Error   : Red
 *   Fatal   : White text on red background
 *
 * Example)
 *   UE_LOG(Console, ELogLevel::Debug, "Value: %d", 42);
 *   UE_LOG(Renderer, ELogLevel::Error, "CreateBuffer failed");
 */
#define UE_LOG(Category, Level, Format, ...)                                                   \
    do                                                                                             \
    {                                                                                              \
        if (GLog)                                                                                  \
        {                                                                                          \
            char _buf[1024];                                                                       \
            snprintf(_buf, sizeof(_buf), "[%-7s] %-16s: " Format, GetLogLevelLabel(Level), \
                     #Category, ##__VA_ARGS__);                                                    \
            GLog->Log(Level, _buf);                                                            \
        }                                                                                          \
    } while (0)