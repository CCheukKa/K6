#include "CandidateWindow.h"

#include <dwmapi.h>

#pragma comment(lib, "dwmapi.lib")

static const wchar_t* CANDIDATE_WINDOW_CLASS = L"ChineseIMECandidateWindow";

// Windows 11 rounded corner preference
#ifndef DWMWA_WINDOW_CORNER_PREFERENCE
#define DWMWA_WINDOW_CORNER_PREFERENCE 33
#endif

CCandidateWindow::CCandidateWindow()
    : _hwnd(nullptr), _shown(FALSE), _selection(0), _page(0) {
}

CCandidateWindow::~CCandidateWindow() {
    if (_hwnd) {
        DestroyWindow(_hwnd);
    }
}

void CCandidateWindow::SetCandidates(const std::vector<std::wstring>& candidates) {
    _candidates = candidates;
    _selection = 0;
    _page = 0;

    if (_hwnd) {
        RECT rc = CalculateWindowSize();
        SetWindowPos(_hwnd, HWND_TOPMOST, 0, 0, rc.right, rc.bottom, SWP_NOMOVE | SWP_NOACTIVATE);
        ApplyRoundedCorners();
        InvalidateRect(_hwnd, nullptr, TRUE);
    }
}

void CCandidateWindow::SetState(InputState state) {  // Change parameter type
    _state = state;
    if (_state == InputState::DISABLED) {
        Hide();
    } else {
        Show();
    }
}

void CCandidateWindow::SetStrokeInput(const std::wstring& strokeinput) {
    _strokeinput = strokeinput;
    if (_hwnd) {
        RECT rc = CalculateWindowSize();
        SetWindowPos(_hwnd, HWND_TOPMOST, 0, 0, rc.right, rc.bottom, SWP_NOMOVE | SWP_NOACTIVATE);
        ApplyRoundedCorners();
        InvalidateRect(_hwnd, nullptr, TRUE);
    }
}

void CCandidateWindow::SetGhostStrokeInput(const std::wstring& ghostStrokeInput) {
    _ghostStrokeInput = ghostStrokeInput;
    if (_hwnd) {
        RECT rc = CalculateWindowSize();
        SetWindowPos(_hwnd, HWND_TOPMOST, 0, 0, rc.right, rc.bottom, SWP_NOMOVE | SWP_NOACTIVATE);
        ApplyRoundedCorners();
        InvalidateRect(_hwnd, nullptr, TRUE);
    }
}

void CCandidateWindow::SetSelection(UINT index) {
    if (index < _candidates.size()) {
        _selection = index;
        if (_hwnd) {
            InvalidateRect(_hwnd, nullptr, TRUE);
        }
    }
}

void CCandidateWindow::SetPage(UINT page) {
    _page = page;
    // Clamp selection to visible range
    UINT total = static_cast<UINT>(_candidates.size());
    UINT start = _page * CANDIDATES_PER_PAGE;
    UINT count = (start < total) ? min(CANDIDATES_PER_PAGE, total - start) : 0;
    if (_selection >= count) _selection = (count == 0 ? 0 : count - 1);
    if (_hwnd) {
        RECT rc = CalculateWindowSize();
        SetWindowPos(_hwnd, HWND_TOPMOST, 0, 0, rc.right, rc.bottom, SWP_NOMOVE | SWP_NOACTIVATE);
        ApplyRoundedCorners();
        InvalidateRect(_hwnd, nullptr, TRUE);
    }
}

void CCandidateWindow::Show() {
    if (_candidates.empty() && _strokeinput.empty() && _ghostStrokeInput.empty()) {
        Hide();
        return;
    }
    CreateWindowIfNeeded();
    if (_hwnd) {
        ShowWindow(_hwnd, SW_SHOWNOACTIVATE);
        _shown = TRUE;
    }
}

void CCandidateWindow::Hide() {
    if (_hwnd) {
        ShowWindow(_hwnd, SW_HIDE);
    }
    _shown = FALSE;
}

void CCandidateWindow::UpdatePosition(POINT pt) {
    CreateWindowIfNeeded();
    if (!_hwnd) return;

    RECT rc = CalculateWindowSize();
    int width = rc.right;
    int height = rc.bottom;

    // Keep window on screen
    HMONITOR hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi = {sizeof(mi)};
    if (GetMonitorInfo(hMonitor, &mi)) {
        if (pt.x + width > mi.rcWork.right) pt.x = mi.rcWork.right - width;
        if (pt.y + height > mi.rcWork.bottom) pt.y = pt.y - height - 20;
    }

    SetWindowPos(_hwnd, HWND_TOPMOST, pt.x, pt.y, width, height, SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

LRESULT CALLBACK CCandidateWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    CCandidateWindow* pThis = reinterpret_cast<CCandidateWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    if (uMsg == WM_CREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = reinterpret_cast<CCandidateWindow*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    }

    if (pThis) {
        if (uMsg == WM_PAINT) {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            pThis->Paint(hdc);
            EndPaint(hwnd, &ps);
            return 0;
        }
        if (uMsg == WM_ERASEBKGND) return 1;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void CCandidateWindow::Paint(HDC hdc) {
    // Colours
    const auto WINDOW_BACKGROUND_COLOUR = RGB(255, 255, 255);
    const auto WINDOW_BORDER_COLOUR = RGB(128, 128, 128);
    const auto STROKE_INPUT_COLOUR = RGB(40, 40, 240);
    const auto GHOST_STROKE_INPUT_COLOUR = RGB(128, 128, 128);
    const auto CANDIDATE_TEXT_COLOUR = RGB(0, 0, 0);
    const auto CANDIDATE_NUMBER_ACTIVE_COLOUR = RGB(255, 0, 0);
    const auto CANDIDATE_NUMBER_INACTIVE_COLOUR = RGB(200, 200, 200);

    RECT rc;
    GetClientRect(_hwnd, &rc);

    // Background
    HBRUSH hBrush = CreateSolidBrush(WINDOW_BACKGROUND_COLOUR);
    FillRect(hdc, &rc, hBrush);
    DeleteObject(hBrush);

    // Border
    HPEN hPen = CreatePen(PS_SOLID, 1, WINDOW_BORDER_COLOUR);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);

    // Font (bold, slightly larger for readability)
    HFONT hFont = CreateFont(22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Microsoft JhengHei");
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
    SetBkMode(hdc, TRANSPARENT);

    int y = PADDING;

    // StrokeInput or ghost strokeinput
    if (!_strokeinput.empty() || !_ghostStrokeInput.empty()) {
        RECT strokeinputRect = {PADDING, y, rc.right - PADDING, y + LINE_HEIGHT};
        if (!_strokeinput.empty()) {
            SetTextColor(hdc, STROKE_INPUT_COLOUR);
            wchar_t buf[256];
            swprintf_s(buf, L"%s", _strokeinput.c_str());
            DrawText(hdc, buf, -1, &strokeinputRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        } else if (!_ghostStrokeInput.empty()) {
            SetTextColor(hdc, GHOST_STROKE_INPUT_COLOUR);
            wchar_t buf[256];
            swprintf_s(buf, L"%s", _ghostStrokeInput.c_str());
            DrawText(hdc, buf, -1, &strokeinputRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        }
        y += LINE_HEIGHT;
        MoveToEx(hdc, PADDING, y, nullptr);
        LineTo(hdc, rc.right - PADDING, y);
    }

    // Candidates (paged)
    UINT total = static_cast<UINT>(_candidates.size());
    UINT start = _page * CANDIDATES_PER_PAGE;
    if (start < total) {
        UINT count = min(CANDIDATES_PER_PAGE, total - start);
        for (UINT i = 0; i < count; i++) {
            RECT itemRect = {PADDING, y + (int)i * LINE_HEIGHT, rc.right - PADDING, y + (int)(i + 1) * LINE_HEIGHT};

            wchar_t numBuf[16];
            swprintf_s(numBuf, L"%d.", i + 1);

            SIZE numSize = {0, 0};
            GetTextExtentPoint32(hdc, numBuf, -1, &numSize);

            RECT numRect = itemRect;
            numRect.right = numRect.left + numSize.cx + 20;  // small padding after the number

            // Number color
            SetTextColor(hdc, (_state == InputState::SELECTING) ? CANDIDATE_NUMBER_ACTIVE_COLOUR : CANDIDATE_NUMBER_INACTIVE_COLOUR);
            DrawText(hdc, numBuf, -1, &numRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

            RECT candRect = itemRect;
            candRect.left = numRect.right;

            // Candidate color
            SetTextColor(hdc, CANDIDATE_TEXT_COLOUR);
            DrawText(hdc, _candidates[start + i].c_str(), -1, &candRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        }
    }

    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
}

void CCandidateWindow::CreateWindowIfNeeded() {
    if (_hwnd) return;

    WNDCLASSEX wc = {sizeof(wc)};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = CANDIDATE_WINDOW_CLASS;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassEx(&wc);

    RECT rc = CalculateWindowSize();
    _hwnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW,
        CANDIDATE_WINDOW_CLASS, L"", WS_POPUP | WS_BORDER,
        CW_USEDEFAULT, CW_USEDEFAULT, rc.right, rc.bottom,
        nullptr, nullptr, GetModuleHandle(nullptr), this);

    if (_hwnd) {
        ApplyRoundedCorners();
    }
}

void CCandidateWindow::ApplyRoundedCorners() {
    if (!_hwnd) return;

    // Try Windows 11 DWM rounded corners first
    DWM_WINDOW_CORNER_PREFERENCE cornerPreference = DWMWCP_ROUND;
    HRESULT hr = DwmSetWindowAttribute(_hwnd, DWMWA_WINDOW_CORNER_PREFERENCE,
                                       &cornerPreference, sizeof(cornerPreference));

    // If DWM method fails (older Windows), use region-based rounded corners
    if (FAILED(hr)) {
        RECT rect;
        GetWindowRect(_hwnd, &rect);
        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;

        // Create rounded rectangle region (8 pixel radius)
        HRGN hRgn = CreateRoundRectRgn(0, 0, width, height, 16, 16);
        if (hRgn) {
            SetWindowRgn(_hwnd, hRgn, TRUE);
            // Note: SetWindowRgn takes ownership of the region, no need to DeleteObject
        }
    }
}

RECT CCandidateWindow::CalculateWindowSize() {
    UINT total = static_cast<UINT>(_candidates.size());
    UINT start = _page * CANDIDATES_PER_PAGE;
    UINT count = (start < total) ? min(CANDIDATES_PER_PAGE, total - start) : 0;
    int height = PADDING * 2 + (int)count * LINE_HEIGHT;
    if (!_strokeinput.empty() || !_ghostStrokeInput.empty()) height += LINE_HEIGHT + 4;

    // Calculate required width based on strokeinput/ghost stroke input content
    int width = MIN_WIDTH;
    if (!_strokeinput.empty() || !_ghostStrokeInput.empty()) {
        HDC hdc = GetDC(nullptr);
        if (hdc) {
            HFONT hFont = CreateFont(22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                     DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                     CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Microsoft JhengHei");
            HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

            SIZE textSize = {0, 0};
            const std::wstring& textToMeasure = !_strokeinput.empty() ? _strokeinput : _ghostStrokeInput;
            GetTextExtentPoint32(hdc, textToMeasure.c_str(), (int)textToMeasure.length(), &textSize);

            SelectObject(hdc, hOldFont);
            DeleteObject(hFont);
            ReleaseDC(nullptr, hdc);

            // Add padding on both sides
            int requiredWidth = textSize.cx + PADDING * 2;
            if (requiredWidth > width) {
                width = requiredWidth;
            }
        }
    }

    return {0, 0, width, height};
}
