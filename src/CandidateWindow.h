#pragma once

#include <windows.h>

#include <string>
#include <vector>

#include "InputStateMachine.h"

class CCandidateWindow {
   public:
    CCandidateWindow();
    ~CCandidateWindow();

    void SetCandidates(const std::vector<std::wstring>& candidates);
    void SetSelection(UINT index);
    void SetPage(UINT page);
    void SetPreedit(const std::wstring& preedit);
    void SetGhostPreedit(const std::wstring& ghostPreedit);
    void SetState(InputState state);
    void Show();
    void Hide();
    void UpdatePosition(POINT pt);

   private:
    // Constants
    static constexpr int PADDING = 10;
    static constexpr int LINE_HEIGHT = 28;
    static constexpr int CANDIDATES_PER_PAGE = 9;
    static constexpr int MIN_WIDTH = 200;

    // Member variables
    HWND _hwnd;
    BOOL _shown;
    UINT _selection;
    UINT _page;
    std::vector<std::wstring> _candidates;
    std::wstring _preedit;
    std::wstring _ghostPreedit;
    InputState _state;

    // Helper methods
    void CreateWindowIfNeeded();
    RECT CalculateWindowSize();
    void Paint(HDC hdc);

    // Window procedure
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
