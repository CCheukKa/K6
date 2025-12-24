#pragma once
#include <windows.h>

#include <string>
#include <vector>

class CCandidateWindow {
   public:
    CCandidateWindow();
    ~CCandidateWindow();

    void SetCandidates(const std::vector<std::wstring>& candidates);
    void SetPreedit(const std::wstring& preedit);
    void SetSelection(UINT index);
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
    UINT _selection;

    static const UINT CANDIDATES_PER_PAGE = 9;
    static const int PADDING = 8;
    static const int LINE_HEIGHT = 24;
    static const int MIN_WIDTH = 200;
};
