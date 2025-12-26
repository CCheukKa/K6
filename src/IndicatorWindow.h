#pragma once

#include <windows.h>

class CIndicatorWindow {
   public:
    CIndicatorWindow();
    ~CIndicatorWindow();

    void Show();
    void Hide();

   private:
    HWND _hwnd;
    BOOL _shown;

    void CreateWindowIfNeeded();
    void UpdatePositionTopLeft();
    void Paint(HDC hdc);

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
