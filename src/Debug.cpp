#include "Debug.h"

#include "InputStateMachine.h"

bool Debug::_enabled = false;

void Debug::SetEnabled(bool enabled) {
    _enabled = enabled;
}

bool Debug::IsEnabled() {
    return _enabled;
}

void Debug::OutputDebug(const std::wstring& message) {
    if (!_enabled) return;
    OutputDebugStringW(message.c_str());
}

void Debug::Log(const wchar_t* component, const wchar_t* message) {
    if (!_enabled) return;

    std::wstringstream ss;
    ss << L"[IME][" << component << L"] " << message << L"\n";
    OutputDebug(ss.str());
}

void Debug::Log(const wchar_t* component, const wchar_t* message, WPARAM wParam) {
    if (!_enabled) return;

    std::wstringstream ss;
    ss << L"[IME][" << component << L"] " << message << L" wParam=0x" << std::hex << wParam << L"\n";
    OutputDebug(ss.str());
}

void Debug::LogStroke(const wchar_t* component, const wchar_t* message, const std::wstring& stroke) {
    if (!_enabled) return;

    std::wstringstream ss;
    ss << L"[IME][" << component << L"] " << message << L" stroke='" << stroke << L"' [";

    for (wchar_t c : stroke) {
        ss << L"0x" << std::hex << static_cast<int>(c) << L" ";
    }

    ss << L"]\n";
    OutputDebug(ss.str());
}

template <>
void Debug::LogAction(const wchar_t* component, const wchar_t* message, const InputAction& action) {
    if (!_enabled) return;

    std::wstringstream ss;
    ss << L"[IME][" << component << L"] " << message << L" type=" << static_cast<int>(action.type)
       << L" stroke=" << action.stroke << L" index=" << action.index << L" char='" << action.character
       << L"' changeNextState=" << action.changeNextState << L" nextState=" << static_cast<int>(action.nextState)
       << L"\n";
    OutputDebug(ss.str());
}

void Debug::LogDirect(const wchar_t* message) {
    if (!_enabled) return;
    OutputDebug(message);
}
