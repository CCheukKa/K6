#pragma once
#include <windows.h>

#include <string>
#include <vector>

#include "TextService.h"

class CCandidateWindow {
   public:
    CCandidateWindow();
    ~CCandidateWindow();

    void SetState(CTextService::State state);
    void SetCandidates(const std::vector<std::wstring>& candidates);
    void SetPreedit(const std::wstring& preedit);
    void SetGhostPreedit(const std::wstring& ghostPreedit);
    void SetSelection(UINT index);
    void SetPage(UINT page);
    void Show();
    void Hide();
    void UpdatePosition(POINT pt);

   private:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void Paint(HDC hdc);
    void CreateWindowIfNeeded();
    RECT CalculateWindowSize();

    HWND _hwnd;
    BOOL _shown;
    std::vector<std::wstring> _candidates;
    std::wstring _preedit;
    std::wstring _ghostPreedit;
    UINT _selection;
    UINT _page;
    CTextService::State _state;

    static const UINT CANDIDATES_PER_PAGE = 9;
    static const int PADDING = 6;
    static const int LINE_HEIGHT = 26;
    static const int MIN_WIDTH = 200;
};
