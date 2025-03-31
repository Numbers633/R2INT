#pragma once
#include <windows.h>
#include <stdio.h>

inline void TraceFormatted(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    char buffer[1024];
    vsnprintf_s(buffer, sizeof(buffer), fmt, args);

    va_end(args);

    OutputDebugStringA(buffer);
}

#if defined(_DEBUG)
#define TRACEF(fmt, ...) TraceFormatted(fmt, __VA_ARGS__)
#else
#define TRACEF(fmt, ...) __noop();
#endif