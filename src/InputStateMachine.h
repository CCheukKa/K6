#pragma once
#include <windows.h>

#include <map>
#include <string>
#include <vector>

// Mirrors the TypeScript State enum
enum class InputState {
    DISABLED,
    TYPING,
    SELECTING,
};

// Mirrors the TypeScript ActionType enum
enum class InputActionType {
    NOOP_PASS_THROUGH_KEYPRESS,
    NOOP_CONSUME_KEYPRESS,
    ADD_STROKE,
    DELETE_STROKE,
    CLEAR_STROKE,
    NEXT_SELECTION_PAGE,
    PREVIOUS_SELECTION_PAGE,
    SELECT_CHARACTER,
    SUBSTITUTE_CHARACTER,
    TOGGLE_ENABLE,
};

// Mirrors the TypeScript Action union type
struct InputAction {
    InputActionType type;
    wchar_t stroke = 0;
    int index = 0;
    std::wstring character;
    bool changeNextState = false;
    InputState nextState;

    InputAction() : type(InputActionType::NOOP_PASS_THROUGH_KEYPRESS) {}
    explicit InputAction(InputActionType t) : type(t) {}
};

class InputStateMachine {
   public:
    InputStateMachine();

    // Process a key event and return the action to perform
    InputAction ProcessKey(InputState currentState, WPARAM wParam);
    bool IsToggleEnableKey(WPARAM wParam) const;

   private:
    // Keybind checking helpers
    bool IsStrokeKey(WPARAM wParam, wchar_t& outStroke) const;
    bool IsDigitKey(WPARAM wParam, UINT& outIndex) const;
    bool IsLetterKey(WPARAM wParam) const;
    bool IsPageNavigationKey(WPARAM wParam, bool& outIsNext) const;
    bool IsEscapeKey(WPARAM wParam) const;
    bool IsBackspaceKey(WPARAM wParam) const;
    bool IsEnterKey(WPARAM wParam) const;
    bool IsBlockedKey(WPARAM wParam) const;
    bool IsSubstitutableSymbol(WPARAM wParam, std::wstring& outSymbol) const;

    // Substitutable character mappings
    std::map<wchar_t, std::wstring> _substitutableCharacters;

    void InitializeSubstitutableCharacters();
};
