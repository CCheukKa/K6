#include "CandidateWindow.h"

static const wchar_t* CANDIDATE_WINDOW_CLASS = L"ChineseIMECandidateWindow";

CCandidateWindow::CCandidateWindow()
    : _hwnd(nullptr), _shown(FALSE), _selection(0) {
}

CCandidateWindow::~CCandidateWindow() {
    if (_hwnd) {
        DestroyWindow(_hwnd);
    }
}

void CCandidateWindow::SetCandidates(const std::vector<std::wstring>& candidates) {
    _candidates = candidates;
    _selection = 0;

    if (_hwnd) {
        RECT rc = CalculateWindowSize();
        SetWindowPos(_hwnd, HWND_TOPMOST, 0, 0, rc.right, rc.bottom, SWP_NOMOVE | SWP_NOACTIVATE);
        InvalidateRect(_hwnd, nullptr, TRUE);
    }
}

void CCandidateWindow::SetPreedit(const std::wstring& preedit) {
    _preedit = preedit;
    if (_hwnd) {
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

void CCandidateWindow::Show() {
    if (_candidates.empty() && _preedit.empty()) {
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
    RECT rc;
    GetClientRect(_hwnd, &rc);

    // Background
    HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255));
    FillRect(hdc, &rc, hBrush);
    DeleteObject(hBrush);

    // Border
    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(128, 128, 128));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);

    // Font
    HFONT hFont = CreateFont(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Microsoft JhengHei");
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
    SetBkMode(hdc, TRANSPARENT);

    int y = PADDING;

    // Preedit
    if (!_preedit.empty()) {
        RECT preeditRect = {PADDING, y, rc.right - PADDING, y + LINE_HEIGHT};
        SetTextColor(hdc, RGB(0, 0, 255));
        wchar_t buf[256];
        swprintf_s(buf, L"[%s]", _preedit.c_str());
        DrawText(hdc, buf, -1, &preeditRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        y += LINE_HEIGHT;
        MoveToEx(hdc, PADDING, y, nullptr);
        LineTo(hdc, rc.right - PADDING, y);
        y += 4;
    }

    // Candidates
    UINT count = min(CANDIDATES_PER_PAGE, static_cast<UINT>(_candidates.size()));
    for (UINT i = 0; i < count; i++) {
        RECT itemRect = {PADDING, y + i * LINE_HEIGHT, rc.right - PADDING, y + (i + 1) * LINE_HEIGHT};

        if (i == _selection) {
            HBRUSH hSelBrush = CreateSolidBrush(RGB(0, 120, 215));
            FillRect(hdc, &itemRect, hSelBrush);
            DeleteObject(hSelBrush);
            SetTextColor(hdc, RGB(255, 255, 255));
        } else {
            SetTextColor(hdc, RGB(0, 0, 0));
        }

        wchar_t buf[256];
        swprintf_s(buf, L"%d. %s", i + 1, _candidates[i].c_str());
        DrawText(hdc, buf, -1, &itemRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
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
}

RECT CCandidateWindow::CalculateWindowSize() {
    UINT count = min(CANDIDATES_PER_PAGE, static_cast<UINT>(_candidates.size()));
    int height = PADDING * 2 + count * LINE_HEIGHT;
    if (!_preedit.empty()) height += LINE_HEIGHT + 4;
    return {0, 0, MIN_WIDTH, height};
}
