#pragma once

#include <cstdio>
#include "LogOutputDevice.h"
#include "Core/CoreGlobals.h"
#include <cstdarg>

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
inline void LogMessage(ELogLevel Level, const char* Category, const char* Format, ...)
{
    if (!GLog)
    {
        return;
    }

    char Prefix[256];
    snprintf(Prefix, sizeof(Prefix), "[%s] %s: ", GetLogLevelLabel(Level), Category);

    char    Message[1024];
    va_list Args;
    va_start(Args, Format);
    vsnprintf(Message, sizeof(Message), Format, Args);
    va_end(Args);

    char FinalBuffer[1280];
    snprintf(FinalBuffer, sizeof(FinalBuffer), "%s%s", Prefix, Message);

    GLog->Log(Level, FinalBuffer);
}

#define UE_LOG(Category, Level, Format, ...)                                                       \
    do                                                                                             \
    {                                                                                              \
        LogMessage(Level, #Category, Format, ##__VA_ARGS__);                                       \
    } while (0)