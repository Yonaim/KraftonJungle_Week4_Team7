#pragma once

#include <cstdio>
#include "LogOutputDevice.h"
#include "Core/CoreGlobals.h"

inline const char* GetLogVerbosityLabel(ELogVerbosity Verbosity)
{
    switch (Verbosity)
    {
    case ELogVerbosity::Debug:
        return "DEBUG";
    case ELogVerbosity::Info:
        return "INFO";
    case ELogVerbosity::Warning:
        return "WARNING";
    case ELogVerbosity::Error:
        return "ERROR";
    case ELogVerbosity::Fatal:
        return "FATAL";
    default:
        return "UNKNOWN";
    }
}

/**
 * LogOutput Macro
 *
 * Category    : Category name only (e.g. Console, Renderer, Engine), no registration required
 * Verbosity   : ELogVerbosity (Debug / Info / Warning / Error / Fatal)
 * Format, ... : printf-style format string
 *
 * Verbosity colors in Console Panel
 *   Debug   : Gray
 *   Info    : White
 *   Warning : Yellow
 *   Error   : Red
 *   Fatal   : White text on red background
 *
 * Example)
 *   UE_LOG(Console, ELogVerbosity::Debug, "Value: %d", 42);
 *   UE_LOG(Renderer, ELogVerbosity::Error, "CreateBuffer failed");
 */
#define UE_LOG(Category, Verbosity, Format, ...)                                                   \
    do                                                                                             \
    {                                                                                              \
        if (GLog)                                                                                  \
        {                                                                                          \
            char _buf[1024];                                                                       \
            snprintf(_buf, sizeof(_buf), "[%-7s] %-16s: " Format, GetLogVerbosityLabel(Verbosity), \
                     #Category, ##__VA_ARGS__);                                                    \
            GLog->Log(Verbosity, _buf);                                                            \
        }                                                                                          \
    } while (0)