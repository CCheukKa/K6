#pragma once

#include <windows.h>

#include <sstream>
#include <string>

class Debug {
   public:
    // Enable/disable debug logging globally
    static void SetEnabled(bool enabled);
    static bool IsEnabled();

    // Simple message logging
    static void Log(const wchar_t* component, const wchar_t* message);

    // Message with wParam
    static void Log(const wchar_t* component, const wchar_t* message, WPARAM wParam);

    // Message with stroke (shows hex codes)
    static void LogStroke(const wchar_t* component, const wchar_t* message, const std::wstring& stroke);

    // Message with InputAction (forward declaration handled in cpp)
    template <typename T>
    static void LogAction(const wchar_t* component, const wchar_t* message, const T& action);

    // Direct message (no component prefix)
    static void LogDirect(const wchar_t* message);

   private:
    static bool _enabled;
    static void OutputDebug(const std::wstring& message);
};
