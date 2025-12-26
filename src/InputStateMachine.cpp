#include "InputStateMachine.h"

#include <windows.h>

#include <sstream>

// Debug helper
static void DebugLog(const wchar_t* msg) {
    std::wstringstream ss;
    ss << "[IME][InputStateMachine] " << msg;
    OutputDebugStringW(ss.str().c_str());
    OutputDebugStringW(L"\n");
}

static void DebugLog(const wchar_t* msg, WPARAM wParam) {
    std::wstringstream ss;
    ss << "[IME][InputStateMachine] " << msg << L" wParam=0x" << std::hex << wParam;
    OutputDebugStringW(ss.str().c_str());
    OutputDebugStringW(L"\n");
}

InputStateMachine::InputStateMachine() {
    InitializeSubstitutableCharacters();
}

void InputStateMachine::InitializeSubstitutableCharacters() {
    _substitutableCharacters[L' '] = L"　";
    _substitutableCharacters[L'`'] = L"・";
    _substitutableCharacters[L'~'] = L"～";
    _substitutableCharacters[L'!'] = L"！";
    _substitutableCharacters[L'@'] = L"＠";
    _substitutableCharacters[L'#'] = L"＃";
    _substitutableCharacters[L'$'] = L"＄";
    _substitutableCharacters[L'%'] = L"％";
    _substitutableCharacters[L'^'] = L"︿";
    _substitutableCharacters[L'&'] = L"＆";
    _substitutableCharacters[L'*'] = L"＊";
    _substitutableCharacters[L'('] = L"（";
    _substitutableCharacters[L')'] = L"）";
    _substitutableCharacters[L'_'] = L"＿";
    _substitutableCharacters[L'+'] = L"＋";
    _substitutableCharacters[L'-'] = L"－";
    _substitutableCharacters[L'='] = L"＝";
    _substitutableCharacters[L'['] = L"「";
    _substitutableCharacters[L']'] = L"」";
    _substitutableCharacters[L'\\'] = L"＼";
    _substitutableCharacters[L'{'] = L"『";
    _substitutableCharacters[L'}'] = L"』";
    _substitutableCharacters[L'|'] = L"｜";
    _substitutableCharacters[L';'] = L"；";
    _substitutableCharacters[L'\''] = L"、";
    _substitutableCharacters[L':'] = L"：";
    _substitutableCharacters[L'"'] = L"＂";
    _substitutableCharacters[L','] = L"，";
    _substitutableCharacters[L'.'] = L"。";
    _substitutableCharacters[L'/'] = L"／";
    _substitutableCharacters[L'<'] = L"《";
    _substitutableCharacters[L'>'] = L"》";
    _substitutableCharacters[L'?'] = L"？";
}

bool InputStateMachine::IsToggleEnableKey(WPARAM wParam) const {
    // Shift key (VK_SHIFT, VK_LSHIFT, VK_RSHIFT)
    return wParam == VK_SHIFT || wParam == VK_LSHIFT || wParam == VK_RSHIFT;
}

bool InputStateMachine::IsStrokeKey(WPARAM wParam, wchar_t& outStroke) const {
    // Map to stroke characters using keyboard codes
    // O/Numpad9 -> POSITIVE_DIAGONAL (丿)
    if (/*wParam == 'O' || wParam == 'o' ||*/ wParam == VK_NUMPAD9) {
        outStroke = L'丿';
        return true;
    }
    // J/Numpad4 -> NEGATIVE_DIAGONAL (丶)
    if (/*wParam == 'J' || wParam == 'j' ||*/ wParam == VK_NUMPAD4) {
        outStroke = L'丶';
        return true;
    }
    // I/Numpad8 -> VERTICAL (丨)
    if (/*wParam == 'I' || wParam == 'i' ||*/ wParam == VK_NUMPAD8) {
        outStroke = L'丨';
        return true;
    }
    // U/Numpad7 -> HORIZONTAL (一)
    if (/*wParam == 'U' || wParam == 'u' ||*/ wParam == VK_NUMPAD7) {
        outStroke = L'一';
        return true;
    }
    // K/Numpad5 -> COMPOUND (フ)
    if (/*wParam == 'K' || wParam == 'k' ||*/ wParam == VK_NUMPAD5) {
        outStroke = L'フ';
        return true;
    }
    // L/Numpad6 -> WILDCARD (＊)
    if (/*wParam == 'L' || wParam == 'l' ||*/ wParam == VK_NUMPAD6) {
        outStroke = L'＊';
        return true;
    }
    return false;
}

bool InputStateMachine::IsDigitKey(WPARAM wParam, UINT& outIndex) const {
    // if (wParam >= '0' && wParam <= '9') {
    //     outIndex = static_cast<UINT>(wParam - '0');
    //     return true;
    // }
    if (wParam >= VK_NUMPAD0 && wParam <= VK_NUMPAD9) {
        outIndex = static_cast<UINT>(wParam - VK_NUMPAD0);
        return true;
    }
    return false;
}

bool InputStateMachine::IsLetterKey(WPARAM wParam) const {
    return (wParam >= 'A' && wParam <= 'Z') || (wParam >= 'a' && wParam <= 'z');
}

bool InputStateMachine::IsPageNavigationKey(WPARAM wParam, bool& outIsNext) const {
    // Next page: + (VK_ADD) or M/m
    if (wParam == VK_ADD /*|| wParam == 'M' || wParam == 'm'*/) {
        outIsNext = true;
        return true;
    }
    // Previous page: - (VK_SUBTRACT) or N/n
    if (wParam == VK_SUBTRACT /*|| wParam == 'N' || wParam == 'n'*/) {
        outIsNext = false;
        return true;
    }
    return false;
}

bool InputStateMachine::IsEscapeKey(WPARAM wParam) const {
    return (wParam == VK_ESCAPE || wParam == VK_DECIMAL);
}

bool InputStateMachine::IsBackspaceKey(WPARAM wParam) const {
    return wParam == VK_BACK;
}

bool InputStateMachine::IsEnterKey(WPARAM wParam) const {
    return wParam == VK_RETURN;
}

bool InputStateMachine::IsBlockedKey(WPARAM wParam) const {
    // Block letter keys and numpad operators
    wchar_t stroke = 0;
    bool isNext = false;
    std::wstring symbol;
    UINT digit = 0;

    return (
        IsToggleEnableKey(wParam) ||
        IsStrokeKey(wParam, stroke) ||
        IsDigitKey(wParam, digit) ||
        IsLetterKey(wParam) ||
        IsPageNavigationKey(wParam, isNext) ||
        IsEscapeKey(wParam) ||
        IsBackspaceKey(wParam) ||
        IsEnterKey(wParam) ||
        IsSubstitutableSymbol(wParam, symbol) ||
        wParam == VK_ADD || wParam == VK_SUBTRACT || wParam == VK_MULTIPLY || wParam == VK_DIVIDE || wParam == VK_DECIMAL);
}

bool InputStateMachine::IsSubstitutableSymbol(WPARAM wParam, std::wstring& outSymbol) const {
    // Block numpad operators - they should act as input keys, not trigger substitution
    if (wParam == VK_ADD || wParam == VK_SUBTRACT || wParam == VK_MULTIPLY || wParam == VK_DIVIDE ||
        wParam == VK_DECIMAL) {
        return false;
    }

    // Convert WPARAM to character
    BYTE keyboardState[256];
    GetKeyboardState(keyboardState);

    wchar_t chars[4] = {0};
    int result = ToUnicodeEx(wParam, MapVirtualKey(wParam, MAPVK_VK_TO_VSC), keyboardState, chars, 4, 0,
                             GetKeyboardLayout(0));

    if (result <= 0) {
        return false;
    }

    wchar_t asciiChar = chars[0];

    // Check if this character is substitutable
    auto it = _substitutableCharacters.find(asciiChar);
    if (it != _substitutableCharacters.end()) {
        outSymbol = it->second;
        std::wstringstream ss;
        ss << L"IsSubstitutableSymbol FOUND: '" << asciiChar << L"' -> '" << outSymbol << L"'\n";
        OutputDebugStringW(ss.str().c_str());
        return true;
    }

    return false;
}

InputAction InputStateMachine::ProcessKey(InputState currentState, WPARAM wParam) {
    wchar_t stroke = 0;
    UINT digit = 0;
    bool isNext = false;
    std::wstring symbol;

    std::wstringstream ss;
    ss << L"ProcessKey: State=" << (int)currentState << L", wParam=0x" << std::hex << wParam << L"\n";
    OutputDebugStringW(ss.str().c_str());

    // Check for modifier keys first - if non-Alt modifier is held, consume nothing
    bool otherModifier = (GetKeyState(VK_CONTROL) & 0x8000) || (GetKeyState(VK_MENU) & 0x8000) ||
                         (GetKeyState(VK_LWIN) & 0x8000) || (GetKeyState(VK_RWIN) & 0x8000);
    if (otherModifier) {
        // TODO:
        DebugLog(L"Non-Alt modifier held, NOOP_PASS_THROUGH_KEYPRESS");
        return InputAction(InputActionType::NOOP_PASS_THROUGH_KEYPRESS);
    }

    switch (currentState) {
        case InputState::DISABLED: {
            if (IsToggleEnableKey(wParam)) {
                DebugLog(L"DISABLED -> Toggle to TYPING");
                InputAction action(InputActionType::TOGGLE_ENABLE);
                action.changeNextState = true;
                action.nextState = InputState::TYPING;
                return action;
            }

            DebugLog(L"DISABLED -> Fell through, pass through keypress");
            return InputAction(InputActionType::NOOP_PASS_THROUGH_KEYPRESS);
        }

        case InputState::TYPING: {
            if (IsToggleEnableKey(wParam)) {
                DebugLog(L"TYPING -> Toggle to DISABLED");
                InputAction action(InputActionType::TOGGLE_ENABLE);
                action.changeNextState = true;
                action.nextState = InputState::DISABLED;
                return action;
            }

            if (IsStrokeKey(wParam, stroke)) {
                DebugLog(L"TYPING -> Add stroke");
                InputAction action(InputActionType::ADD_STROKE);
                action.stroke = stroke;
                return action;
            }

            if (IsSubstitutableSymbol(wParam, symbol)) {
                DebugLog(L"TYPING -> Symbol substitution");
                InputAction action(InputActionType::SUBSTITUTE_CHARACTER);
                action.character = symbol;
                return action;
            }

            if (IsBackspaceKey(wParam)) {
                DebugLog(L"TYPING -> Delete stroke");
                return InputAction(InputActionType::DELETE_STROKE);
            }

            if (IsEscapeKey(wParam)) {
                DebugLog(L"TYPING -> Clear strokes");
                return InputAction(InputActionType::CLEAR_STROKE);
            }

            if (IsEnterKey(wParam)) {
                DebugLog(L"TYPING -> Select first character");
                InputAction action(InputActionType::SELECT_CHARACTER);
                action.index = 0;
                return action;
            }

            if (IsDigitKey(wParam, digit)) {
                if (digit == 0) {
                    DebugLog(L"TYPING -> Toggle to SELECTING");
                    InputAction action(InputActionType::NOOP_CONSUME_KEYPRESS);
                    action.changeNextState = true;
                    action.nextState = InputState::SELECTING;
                    return action;
                }
            }

            if (IsBlockedKey(wParam)) {
                DebugLog(L"TYPING -> Block letter/page nav/numpad operator");
                return InputAction(InputActionType::NOOP_CONSUME_KEYPRESS);
            }

            DebugLog(L"TYPING -> Fell through, pass through keypress");
            return InputAction(InputActionType::NOOP_PASS_THROUGH_KEYPRESS);
        }

        case InputState::SELECTING: {
            if (IsToggleEnableKey(wParam)) {
                DebugLog(L"SELECTING -> Toggle to DISABLED");
                InputAction action(InputActionType::TOGGLE_ENABLE);
                action.changeNextState = true;
                action.nextState = InputState::DISABLED;
                return action;
            }

            if (IsDigitKey(wParam, digit)) {
                if (digit == 0) {
                    DebugLog(L"SELECTING -> Return to TYPING");
                    InputAction action(InputActionType::NOOP_CONSUME_KEYPRESS);
                    action.changeNextState = true;
                    action.nextState = InputState::TYPING;
                    return action;
                } else {
                    DebugLog(L"SELECTING -> Select character");
                    InputAction action(InputActionType::SELECT_CHARACTER);
                    action.index = digit - 1;  // 1-9 maps to 0-8
                    action.changeNextState = true;
                    action.nextState = InputState::TYPING;
                    return action;
                }
            }

            if (IsEnterKey(wParam)) {
                DebugLog(L"SELECTING -> Select first character");
                InputAction action(InputActionType::SELECT_CHARACTER);
                action.index = 0;
                action.changeNextState = true;
                action.nextState = InputState::TYPING;
                return action;
            }

            if (IsPageNavigationKey(wParam, isNext)) {
                DebugLog(L"SELECTING -> Page navigation", wParam);
                if (isNext) {
                    DebugLog(L"SELECTING -> Next page");
                    return InputAction(InputActionType::NEXT_SELECTION_PAGE);
                } else {
                    DebugLog(L"SELECTING -> Previous page");
                    return InputAction(InputActionType::PREVIOUS_SELECTION_PAGE);
                }
            }

            if (IsEscapeKey(wParam)) {
                DebugLog(L"SELECTING -> Clear and return to TYPING");
                InputAction action(InputActionType::CLEAR_STROKE);
                action.changeNextState = true;
                action.nextState = InputState::TYPING;
                return action;
            }

            if (IsBackspaceKey(wParam)) {
                DebugLog(L"SELECTING -> Delete and return to TYPING");
                InputAction action(InputActionType::DELETE_STROKE);
                action.changeNextState = true;
                action.nextState = InputState::TYPING;
                return action;
            }

            if (IsBlockedKey(wParam)) {
                DebugLog(L"SELECTING -> Block letter/page nav/numpad operator");
                return InputAction(InputActionType::NOOP_CONSUME_KEYPRESS);
            }

            DebugLog(L"SELECTING -> Fell through, pass through keypress");
            return InputAction(InputActionType::NOOP_PASS_THROUGH_KEYPRESS);
        }

        default:
            return InputAction(InputActionType::NOOP_PASS_THROUGH_KEYPRESS);
    }
}