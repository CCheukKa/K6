#include "TextService.h"

#include <new>
#include <sstream>

#include "EditSession.h"

// Debug helper
static void DebugLog(const wchar_t* msg) {
    OutputDebugStringW(msg);
    OutputDebugStringW(L"\n");
}

static void DebugLog(const wchar_t* msg, WPARAM wParam) {
    std::wstringstream ss;
    ss << msg << L" wParam=0x" << std::hex << wParam;
    OutputDebugStringW(ss.str().c_str());
    OutputDebugStringW(L"\n");
}

CTextService::CTextService()
    : _refCount(1), _threadMgr(nullptr), _clientId(TF_CLIENTID_NULL), _keystrokeMgr(nullptr), _candidateWindow(nullptr), _selectedCandidate(0), _enabled(TRUE), _shiftPressed(FALSE), _otherKeyPressed(FALSE) {
    _candidateWindow = new CCandidateWindow();
    _dictionary.LoadFromFile(CDictionary::GetDefaultDictionaryPath());
}

CTextService::~CTextService() {
    delete _candidateWindow;
}

// IUnknown
STDMETHODIMP CTextService::QueryInterface(REFIID riid, void** ppvObj) {
    if (!ppvObj) return E_INVALIDARG;
    *ppvObj = nullptr;

    if (riid == IID_IUnknown || riid == IID_ITfTextInputProcessor)
        *ppvObj = static_cast<ITfTextInputProcessor*>(this);
    else if (riid == IID_ITfKeyEventSink)
        *ppvObj = static_cast<ITfKeyEventSink*>(this);

    if (*ppvObj) {
        AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG)
CTextService::AddRef() {
    return InterlockedIncrement(&_refCount);
}

STDMETHODIMP_(ULONG)
CTextService::Release() {
    ULONG c = InterlockedDecrement(&_refCount);
    if (c == 0) delete this;
    return c;
}

// ITfTextInputProcessor
STDMETHODIMP CTextService::Activate(ITfThreadMgr* ptim, TfClientId tid) {
    DebugLog(L"[IME] Activate called");
    if (!ptim) return E_INVALIDARG;

    _threadMgr = ptim;
    _threadMgr->AddRef();
    _clientId = tid;

    if (SUCCEEDED(_threadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void**)&_keystrokeMgr))) {
        HRESULT hr = _keystrokeMgr->AdviseKeyEventSink(_clientId, this, TRUE);
        if (SUCCEEDED(hr)) {
            DebugLog(L"[IME] AdviseKeyEventSink SUCCESS");
        } else {
            DebugLog(L"[IME] AdviseKeyEventSink FAILED");
        }
    } else {
        DebugLog(L"[IME] Failed to get ITfKeystrokeMgr");
    }
    return S_OK;
}

STDMETHODIMP CTextService::Deactivate() {
    if (_keystrokeMgr) {
        _keystrokeMgr->UnadviseKeyEventSink(_clientId);
        _keystrokeMgr->Release();
        _keystrokeMgr = nullptr;
    }
    if (_threadMgr) {
        _threadMgr->Release();
        _threadMgr = nullptr;
    }
    Reset();
    return S_OK;
}

// ITfKeyEventSink
STDMETHODIMP CTextService::OnSetFocus(BOOL fForeground) {
    DebugLog(L"[IME] OnSetFocus", fForeground);
    return S_OK;
}

STDMETHODIMP CTextService::OnTestKeyDown(ITfContext*, WPARAM wParam, LPARAM, BOOL* pfEaten) {
    if (!pfEaten) return E_INVALIDARG;
    *pfEaten = FALSE;

    DebugLog(L"[IME] OnTestKeyDown", wParam);

    // When IME is disabled, don't eat any keys
    if (!_enabled) {
        return S_OK;
    }

    // Letters always eaten when enabled
    if ((wParam >= 'A' && wParam <= 'Z') || (wParam >= 'a' && wParam <= 'z')) {
        *pfEaten = TRUE;
    }
    // Number keys for candidate selection
    else if (wParam >= '1' && wParam <= '9' && !_candidates.empty()) {
        *pfEaten = TRUE;
    }
    // Space/Backspace/Escape when composing
    else if ((wParam == VK_SPACE || wParam == VK_BACK || wParam == VK_ESCAPE) && !_preedit.empty()) {
        *pfEaten = TRUE;
    }
    // Arrow keys when candidates shown
    else if ((wParam == VK_UP || wParam == VK_DOWN) && !_candidates.empty()) {
        *pfEaten = TRUE;
    }
    return S_OK;
}

STDMETHODIMP CTextService::OnTestKeyUp(ITfContext*, WPARAM wParam, LPARAM, BOOL* pfEaten) {
    *pfEaten = FALSE;
    DebugLog(L"[IME] OnTestKeyUp", wParam);
    return S_OK;
}

STDMETHODIMP CTextService::OnKeyDown(ITfContext* pContext, WPARAM wParam, LPARAM, BOOL* pfEaten) {
    DebugLog(L"[IME] OnKeyDown", wParam);
    if (!pfEaten) return E_INVALIDARG;
    *pfEaten = FALSE;
    if (!pContext) return S_OK;

    // Track Shift key state
    if (wParam == VK_SHIFT || wParam == VK_LSHIFT || wParam == VK_RSHIFT) {
        DebugLog(L"[IME] Shift pressed - tracking");
        _shiftPressed = TRUE;
        _otherKeyPressed = FALSE;
        return S_OK;
    }

    // Any other key pressed while Shift is down
    if (_shiftPressed) {
        _otherKeyPressed = TRUE;
    }

    // When IME is disabled, pass through all keys
    if (!_enabled) {
        return S_OK;
    }

    // Letter keys - accumulate preedit
    if ((wParam >= 'A' && wParam <= 'Z') || (wParam >= 'a' && wParam <= 'z')) {
        *pfEaten = TRUE;
        _preedit.push_back(static_cast<wchar_t>(towlower(static_cast<wchar_t>(wParam))));
        _candidates = _dictionary.Lookup(_preedit);
        _selectedCandidate = 0;
        UpdateCandidateWindow();
    }
    // Number keys - select candidate
    else if (wParam >= '1' && wParam <= '9' && !_candidates.empty()) {
        *pfEaten = TRUE;
        UINT idx = static_cast<UINT>(wParam - '1');
        if (idx < _candidates.size()) {
            CommitText(pContext, _candidates[idx]);
            Reset();
        }
    }
    // Backspace
    else if (wParam == VK_BACK && !_preedit.empty()) {
        *pfEaten = TRUE;
        _preedit.pop_back();
        if (_preedit.empty()) {
            Reset();
        } else {
            _candidates = _dictionary.Lookup(_preedit);
            _selectedCandidate = 0;
            UpdateCandidateWindow();
        }
    }
    // Escape - cancel
    else if (wParam == VK_ESCAPE && !_preedit.empty()) {
        *pfEaten = TRUE;
        Reset();
    }
    // Space - commit selected or first candidate
    else if (wParam == VK_SPACE && !_preedit.empty()) {
        *pfEaten = TRUE;
        if (!_candidates.empty()) {
            CommitText(pContext, _candidates[_selectedCandidate]);
        } else {
            CommitText(pContext, _preedit);
        }
        Reset();
    }
    // Arrow keys - navigate candidates
    else if (wParam == VK_DOWN && !_candidates.empty()) {
        *pfEaten = TRUE;
        _selectedCandidate = (_selectedCandidate + 1) % _candidates.size();
        _candidateWindow->SetSelection(_selectedCandidate);
    } else if (wParam == VK_UP && !_candidates.empty()) {
        *pfEaten = TRUE;
        _selectedCandidate = (_selectedCandidate == 0) ? _candidates.size() - 1 : _selectedCandidate - 1;
        _candidateWindow->SetSelection(_selectedCandidate);
    }

    return S_OK;
}

STDMETHODIMP CTextService::OnKeyUp(ITfContext*, WPARAM wParam, LPARAM, BOOL* pfEaten) {
    DebugLog(L"[IME] OnKeyUp", wParam);
    *pfEaten = FALSE;

    // Toggle IME on Shift release if no other key was pressed
    if (wParam == VK_SHIFT || wParam == VK_LSHIFT || wParam == VK_RSHIFT) {
        DebugLog(L"[IME] Shift released");
        if (_shiftPressed && !_otherKeyPressed) {
            DebugLog(L"[IME] TOGGLING!");
            ToggleEnabled();
        }
        _shiftPressed = FALSE;
        _otherKeyPressed = FALSE;
    }

    return S_OK;
}

STDMETHODIMP CTextService::OnPreservedKey(ITfContext*, REFGUID rguid, BOOL* pfEaten) {
    DebugLog(L"[IME] OnPreservedKey");
    *pfEaten = FALSE;
    return S_OK;
}

void CTextService::CommitText(ITfContext* pContext, const std::wstring& text) {
    CEditSessionInsert* pSession = new (std::nothrow) CEditSessionInsert(this, pContext, text);
    if (pSession) {
        HRESULT hr;
        pContext->RequestEditSession(_clientId, pSession, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hr);
        pSession->Release();
    }
}

void CTextService::UpdateCandidateWindow() {
    _candidateWindow->SetPreedit(_preedit);
    _candidateWindow->SetCandidates(_candidates);
    _candidateWindow->SetSelection(_selectedCandidate);

    POINT pt;
    GetCaretPos(&pt);
    _candidateWindow->UpdatePosition(pt);
    _candidateWindow->Show();
}

void CTextService::Reset() {
    _preedit.clear();
    _candidates.clear();
    _selectedCandidate = 0;
    _candidateWindow->Hide();
}

void CTextService::ToggleEnabled() {
    _enabled = !_enabled;
    if (!_enabled) {
        // When disabling, clear any active composition
        Reset();
    }
}
