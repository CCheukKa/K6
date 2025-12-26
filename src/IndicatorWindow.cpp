#include "IndicatorWindow.h"

#include <dwmapi.h>

#pragma comment(lib, "dwmapi.lib")

static const wchar_t* INDICATOR_WINDOW_CLASS = L"K6IMEIndicatorWindow";

#ifndef DWMWA_WINDOW_CORNER_PREFERENCE
#define DWMWA_WINDOW_CORNER_PREFERENCE 33
#endif

CIndicatorWindow::CIndicatorWindow() : _hwnd(nullptr), _shown(FALSE) {}

CIndicatorWindow::~CIndicatorWindow() {
    if (_hwnd) {
        DestroyWindow(_hwnd);
        _hwnd = nullptr;
    }
}

void CIndicatorWindow::Show() {
    CreateWindowIfNeeded();
    if (_hwnd) {
        UpdatePositionTopLeft();
        ShowWindow(_hwnd, SW_SHOWNOACTIVATE);
        InvalidateRect(_hwnd, nullptr, TRUE);  // ensure repaint with new colour
        _shown = TRUE;
    }
}

void CIndicatorWindow::Hide() {
    if (_hwnd) {
        ShowWindow(_hwnd, SW_HIDE);
    }
    _shown = FALSE;
}

void CIndicatorWindow::CreateWindowIfNeeded() {
    if (_hwnd) return;

    WNDCLASSEX wc = {sizeof(wc)};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = INDICATOR_WINDOW_CLASS;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClassEx(&wc);

    // Measure text "K6" to determine a compact width
    HDC hdc = GetDC(nullptr);
    int width = 48;   // fallback
    int height = 24;  // fallback
    if (hdc) {
        HFONT hFont = CreateFont(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                 DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                 CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
        HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
        SIZE textSize{0, 0};
        const wchar_t* label = L"K6";
        GetTextExtentPoint32(hdc, label, 2, &textSize);
        SelectObject(hdc, hOldFont);
        DeleteObject(hFont);
        ReleaseDC(nullptr, hdc);

        width = textSize.cx + 14;
        height = max(textSize.cy + 8, 20);
    }

    _hwnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW,
        INDICATOR_WINDOW_CLASS, L"", WS_POPUP,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,
        nullptr, nullptr, GetModuleHandle(nullptr), this);

    if (_hwnd) {
        // Rounded corners on Win11, fallback to region-based otherwise
        DWM_WINDOW_CORNER_PREFERENCE cornerPreference = DWMWCP_ROUND;
        HRESULT hr = DwmSetWindowAttribute(_hwnd, DWMWA_WINDOW_CORNER_PREFERENCE,
                                           &cornerPreference, sizeof(cornerPreference));
        if (FAILED(hr)) {
            RECT rect;
            GetWindowRect(_hwnd, &rect);
            int w = rect.right - rect.left;
            int h = rect.bottom - rect.top;
            HRGN rgn = CreateRoundRectRgn(0, 0, w, h, 12, 12);
            if (rgn) SetWindowRgn(_hwnd, rgn, TRUE);
        }
    }
}

void CIndicatorWindow::UpdatePositionTopLeft() {
    if (!_hwnd) return;
    HMONITOR hMonitor = MonitorFromWindow(_hwnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi{sizeof(mi)};
    int x = 6, y = 6;
    if (GetMonitorInfo(hMonitor, &mi)) {
        x = mi.rcWork.left + 6;
        y = mi.rcWork.top + 6;
    }
    RECT rc;
    GetWindowRect(_hwnd, &rc);
    SetWindowPos(_hwnd, HWND_TOPMOST, x, y, rc.right - rc.left, rc.bottom - rc.top,
                 SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

LRESULT CALLBACK CIndicatorWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    CIndicatorWindow* pThis = reinterpret_cast<CIndicatorWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    if (uMsg == WM_CREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = reinterpret_cast<CIndicatorWindow*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
        return 0;
    }

    if (pThis) {
        switch (uMsg) {
            case WM_PAINT: {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);
                pThis->Paint(hdc);
                EndPaint(hwnd, &ps);
                return 0;
            }
            case WM_ERASEBKGND:
                return 1;
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void CIndicatorWindow::Paint(HDC hdc) {
    // Colours
    const auto WINDOW_BACKGROUND_COLOUR = RGB(255, 128, 170);
    const auto WINDOW_BORDER_COLOUR = RGB(20, 20, 20);
    const auto TEXT_COLOUR = RGB(0, 0, 0);

    RECT rc;
    GetClientRect(_hwnd, &rc);

    // Background (semi-light) and border
    HBRUSH hBrush = CreateSolidBrush(WINDOW_BACKGROUND_COLOUR);
    FillRect(hdc, &rc, hBrush);
    DeleteObject(hBrush);

    HPEN hPen = CreatePen(PS_SOLID, 1, WINDOW_BORDER_COLOUR);
    HPEN oldPen = (HPEN)SelectObject(hdc, hPen);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));  // <- avoid filling
    Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(hPen);

    // Text: bold Segoe UI, dark
    HFONT hFont = CreateFont(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT oldFont = (HFONT)SelectObject(hdc, hFont);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, TEXT_COLOUR);

    RECT textRc = rc;
    textRc.left += 6;
    textRc.top += 2;
    DrawText(hdc, L"K6", -1, &textRc, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    SelectObject(hdc, oldFont);
    DeleteObject(hFont);
}
