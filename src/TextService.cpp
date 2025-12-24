#include "TextService.h"

#include <cwctype>
#include <new>
#include <sstream>

#include "CandidateWindow.h"
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
    : _refCount(1),
      _threadMgr(nullptr),
      _clientId(TF_CLIENTID_NULL),
      _keystrokeMgr(nullptr),
      _candidateWindow(nullptr),
      _selectedCandidate(0),
      _page(0),
      _state(State::TYPING),
      _enabled(TRUE),
      _shiftPressed(FALSE),
      _otherKeyPressed(FALSE) {
    _candidateWindow = new CCandidateWindow();
    _dictionary.LoadFromFile(CDictionary::GetDefaultDictionaryPath());
    _suggestionDict.LoadFromFile(CSuggestions::GetDefaultSuggestionsPath());
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

bool CTextService::MapStrokeKey(WPARAM wParam, wchar_t& outStroke) const {
    // Explicitly block numpad operators FIRST - they have ASCII values that match letters
    // VK_ADD (0x6b) = 'k', VK_MULTIPLY (0x6a) = 'j', VK_DIVIDE (0x6f) = 'o', VK_SUBTRACT (0x6d) = 'm'
    if (wParam == VK_ADD || wParam == VK_SUBTRACT || wParam == VK_MULTIPLY || wParam == VK_DIVIDE) {
        return false;
    }

    if (wParam == 'O' || wParam == 'o' || wParam == VK_NUMPAD9) {
        outStroke = Stroke::POSITIVE_DIAGONAL[0];
        return true;
    }
    if (wParam == 'J' || wParam == 'j' || wParam == VK_NUMPAD4) {
        outStroke = Stroke::NEGATIVE_DIAGONAL[0];
        return true;
    }
    if (wParam == 'I' || wParam == 'i' || wParam == VK_NUMPAD8) {
        outStroke = Stroke::VERTICAL[0];
        return true;
    }
    if (wParam == 'U' || wParam == 'u' || wParam == VK_NUMPAD7) {
        outStroke = Stroke::HORIZONTAL[0];
        return true;
    }
    if (wParam == 'K' || wParam == 'k' || wParam == VK_NUMPAD5) {
        outStroke = Stroke::COMPOUND[0];
        return true;
    }
    if (wParam == 'L' || wParam == 'l' || wParam == VK_NUMPAD6) {
        outStroke = Stroke::WILDCARD[0];
        return true;
    }
    return false;
}

bool CTextService::IsDigitKey(WPARAM wParam, UINT& outIndex) const {
    if (wParam >= '0' && wParam <= '9') {
        outIndex = static_cast<UINT>(wParam - '0');
        return true;
    }
    if (wParam >= VK_NUMPAD0 && wParam <= VK_NUMPAD9) {
        outIndex = static_cast<UINT>(wParam - VK_NUMPAD0);
        return true;
    }
    return false;
}

STDMETHODIMP CTextService::OnTestKeyDown(ITfContext*, WPARAM wParam, LPARAM, BOOL* pfEaten) {
    if (!pfEaten) return E_INVALIDARG;
    *pfEaten = FALSE;

    DebugLog(L"[IME] OnTestKeyDown", wParam);

    if (_state == State::DISABLED) return S_OK;

    UINT digit = 0;
    wchar_t stroke = 0;
    if (MapStrokeKey(wParam, stroke)) {
        *pfEaten = TRUE;
        return S_OK;
    }
    if (IsDigitKey(wParam, digit)) {
        if (_state == State::SELECTING && digit >= 1 && digit <= 9) {
            *pfEaten = TRUE;
            return S_OK;
        }
        if (digit == 0) {
            *pfEaten = TRUE;
            return S_OK;
        }
    }
    if (wParam == VK_BACK || wParam == VK_ESCAPE || wParam == VK_DECIMAL) {
        *pfEaten = TRUE;
        return S_OK;
    }

    return S_OK;
}

STDMETHODIMP CTextService::OnTestKeyUp(ITfContext*, WPARAM, LPARAM, BOOL* pfEaten) {
    *pfEaten = FALSE;
    return S_OK;
}

void CTextService::UpdateQueryResults() {
    if (_preedit.empty()) {
        _candidates.clear();
        // keep suggestions (ghost mode)
    } else {
        _candidates = _dictionary.LookupRegex(_preedit);
        _suggestions.clear();
    }

    // Reset selection/page if overflow
    const auto& list = _candidates.empty() ? _suggestions : _candidates;
    if (_page * 9 >= list.size()) {
        _page = 0;
        _selectedCandidate = 0;
    }

    UpdateCandidateWindow();
}

void CTextService::SetGhostFromCharacter(const std::wstring& ch) {
    if (ch.empty()) {
        _ghostPreedit.clear();
        return;
    }
    std::wstring key(1, ch.back());
    _ghostPreedit = _dictionary.GetRandomStrokeForCharacter(key);
}

void CTextService::ShowSuggestionsForCharacter(const std::wstring& ch) {
    if (ch.empty()) {
        _suggestions.clear();
        return;
    }
    std::wstring key(1, ch.back());
    _suggestions = _suggestionDict.Lookup(key);
}

STDMETHODIMP CTextService::OnKeyDown(ITfContext* pContext, WPARAM wParam, LPARAM, BOOL* pfEaten) {
    if (!pfEaten) return E_INVALIDARG;
    *pfEaten = FALSE;
    if (!pContext) return S_OK;

    DebugLog(L"[IME] OnKeyDown", wParam);

    std::wstringstream debugSS;
    debugSS << L"[IME] OnKeyDown wParam=0x" << std::hex << wParam << L" (" << std::dec << wParam << L")";
    OutputDebugStringW(debugSS.str().c_str());
    OutputDebugStringW(L"\n");

    // Keep legacy shift tracking variables but don't toggle on them
    if (wParam == VK_SHIFT || wParam == VK_LSHIFT || wParam == VK_RSHIFT) {
        _shiftPressed = TRUE;
        _otherKeyPressed = FALSE;
        return S_OK;
    }
    if (_shiftPressed) _otherKeyPressed = TRUE;

    if (_state == State::DISABLED || !_enabled) {
        return S_OK;
    }

    wchar_t stroke = 0;
    UINT digit = 0;

    switch (_state) {
        case State::TYPING: {
            auto clearAll = [this]() {
                _ghostPreedit.clear();
                _preedit.clear();
                _candidates.clear();
                _suggestions.clear();
                _page = 0;
                _selectedCandidate = 0;
                UpdateCandidateWindow();
            };

            if (MapStrokeKey(wParam, stroke)) {
                OutputDebugStringW(L"[IME] MapStrokeKey matched\n");
                *pfEaten = TRUE;
                _ghostPreedit.clear();
                _preedit.push_back(stroke);
                _page = 0;
                _selectedCandidate = 0;
                UpdateQueryResults();
                return S_OK;
            }
            OutputDebugStringW(L"[IME] MapStrokeKey did NOT match\n");

            if (wParam == VK_BACK) {
                OutputDebugStringW(L"[IME] VK_BACK matched\n");
                *pfEaten = TRUE;
                if (!_preedit.empty()) {
                    _preedit.pop_back();
                    _page = 0;
                    _selectedCandidate = 0;
                } else {
                    _ghostPreedit.clear();
                    _suggestions.clear();
                }
                UpdateQueryResults();
                return S_OK;
            }
            if (wParam == VK_ESCAPE) {
                OutputDebugStringW(L"[IME] VK_ESCAPE matched\n");
                *pfEaten = TRUE;
                clearAll();
                return S_OK;
            }
            if (wParam == VK_DECIMAL) {
                OutputDebugStringW(L"[IME] VK_DECIMAL (keypad .) matched, treating as escape\n");
                *pfEaten = TRUE;
                clearAll();
                return S_OK;
            }
            if (wParam == VK_RETURN) {
                OutputDebugStringW(L"[IME] VK_RETURN matched\n");
                const auto& list = _candidates.empty() ? _suggestions : _candidates;
                if (!list.empty()) {
                    *pfEaten = TRUE;
                    const std::wstring chosen = list[_page * 9 + 0];
                    CommitText(pContext, chosen);
                    SetGhostFromCharacter(chosen);
                    _preedit.clear();
                    _candidates.clear();
                    _page = 0;
                    _selectedCandidate = 0;
                    ShowSuggestionsForCharacter(chosen);
                    UpdateCandidateWindow();
                }
                return S_OK;
            }
            if (IsDigitKey(wParam, digit)) {
                debugSS.str(L"");
                debugSS << L"[IME] IsDigitKey matched, digit=" << digit;
                OutputDebugStringW(debugSS.str().c_str());
                OutputDebugStringW(L"\n");
                *pfEaten = TRUE;
                if (digit == 0) {
                    // Only enter SELECTING mode if there are candidates
                    if (!_candidates.empty() || !_suggestions.empty()) {
                        _state = State::SELECTING;
                        UpdateCandidateWindow();
                    }
                }
                return S_OK;
            }
            OutputDebugStringW(L"[IME] IsDigitKey did NOT match\n");

            // Block numpad operators
            if (wParam == VK_MULTIPLY || wParam == VK_DIVIDE || wParam == VK_ADD || wParam == VK_SUBTRACT) {
                debugSS.str(L"");
                debugSS << L"[IME] BLOCKING VK_MULTIPLY/VK_DIVIDE (wParam=0x" << std::hex << wParam << std::dec << L")";
                OutputDebugStringW(debugSS.str().c_str());
                OutputDebugStringW(L"\n");
                *pfEaten = TRUE;
                return S_OK;
            }

            debugSS.str(L"");
            debugSS << L"[IME] No handler matched for wParam=0x" << std::hex << wParam << std::dec;
            OutputDebugStringW(debugSS.str().c_str());
            OutputDebugStringW(L"\n");
            break;
        }
        case State::SELECTING: {
            if (IsDigitKey(wParam, digit)) {
                debugSS.str(L"");
                debugSS << L"[IME] SELECTING: IsDigitKey matched, digit=" << digit;
                OutputDebugStringW(debugSS.str().c_str());
                OutputDebugStringW(L"\n");
                *pfEaten = TRUE;
                if (digit == 0) {
                    _state = State::TYPING;
                    UpdateCandidateWindow();
                    return S_OK;
                }
                if (digit >= 1 && digit <= 9) {
                    const auto& list = _candidates.empty() ? _suggestions : _candidates;
                    UINT idx = (_page * 9) + (digit - 1);
                    if (idx < list.size()) {
                        const std::wstring chosen = list[idx];
                        CommitText(pContext, chosen);
                        SetGhostFromCharacter(chosen);
                        _preedit.clear();
                        _candidates.clear();
                        _page = 0;
                        _selectedCandidate = 0;
                        _state = State::TYPING;
                        ShowSuggestionsForCharacter(chosen);
                        UpdateCandidateWindow();
                    }
                }
                return S_OK;
            }
            if (wParam == VK_ADD || wParam == 'm' || wParam == 'M') {
                debugSS.str(L"");
                debugSS << L"[IME] SELECTING: VK_ADD matched (wParam=0x" << std::hex << wParam << std::dec << L")";
                OutputDebugStringW(debugSS.str().c_str());
                OutputDebugStringW(L"\n");
                *pfEaten = TRUE;
                _page += 1;
                UpdateCandidateWindow();
                return S_OK;
            }
            if (wParam == VK_SUBTRACT || wParam == 'n' || wParam == 'N') {
                debugSS.str(L"");
                debugSS << L"[IME] SELECTING: VK_SUBTRACT matched (wParam=0x" << std::hex << wParam << std::dec << L")";
                OutputDebugStringW(debugSS.str().c_str());
                OutputDebugStringW(L"\n");
                *pfEaten = TRUE;
                if (_page > 0) _page -= 1;
                UpdateCandidateWindow();
                return S_OK;
            }
            // Block numpad operators
            if (wParam == VK_MULTIPLY || wParam == VK_DIVIDE) {
                debugSS.str(L"");
                debugSS << L"[IME] SELECTING: BLOCKING VK_MULTIPLY/VK_DIVIDE (wParam=0x" << std::hex << wParam << std::dec << L")";
                OutputDebugStringW(debugSS.str().c_str());
                OutputDebugStringW(L"\n");
                *pfEaten = TRUE;
                return S_OK;
            }
            if (wParam == VK_ESCAPE) {
                OutputDebugStringW(L"[IME] SELECTING: VK_ESCAPE matched\n");
                *pfEaten = TRUE;
                _state = State::TYPING;
                _ghostPreedit.clear();
                _preedit.clear();
                _candidates.clear();
                _suggestions.clear();
                _page = 0;
                _selectedCandidate = 0;
                UpdateCandidateWindow();
                return S_OK;
            }
            if (wParam == VK_DECIMAL) {
                OutputDebugStringW(L"[IME] SELECTING: VK_DECIMAL (keypad .) matched, treating as escape\n");
                *pfEaten = TRUE;
                _state = State::TYPING;
                _ghostPreedit.clear();
                _preedit.clear();
                _candidates.clear();
                _suggestions.clear();
                _page = 0;
                _selectedCandidate = 0;
                UpdateCandidateWindow();
                return S_OK;
            }
            break;
        }
        default:
            break;
    }

    return S_OK;
}

STDMETHODIMP CTextService::OnKeyUp(ITfContext*, WPARAM wParam, LPARAM, BOOL* pfEaten) {
    if (!pfEaten) return E_INVALIDARG;
    *pfEaten = FALSE;
    if (wParam == VK_SHIFT || wParam == VK_LSHIFT || wParam == VK_RSHIFT) {
        if (_shiftPressed && !_otherKeyPressed) {
            // Toggle IME on lone Shift press
            ToggleEnabled();
            *pfEaten = TRUE;
        }
        _shiftPressed = FALSE;
        _otherKeyPressed = FALSE;
    }
    return S_OK;
}

STDMETHODIMP CTextService::OnPreservedKey(ITfContext*, REFGUID, BOOL* pfEaten) {
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
    {
        std::wstringstream ss;
        ss << L"[IME] UpdateCandidateWindow preedit='" << _preedit << L"' ghost='" << _ghostPreedit
           << L"' cand=" << _candidates.size() << L" sugg=" << _suggestions.size() << L" page=" << _page;
        OutputDebugStringW(ss.str().c_str());
        OutputDebugStringW(L"\n");
    }

    _candidateWindow->SetPreedit(_preedit);
    _candidateWindow->SetGhostPreedit(_ghostPreedit);
    _candidateWindow->SetCandidates(_candidates.empty() ? _suggestions : _candidates);
    _candidateWindow->SetSelection(_selectedCandidate);
    _candidateWindow->SetPage(_page);
    _candidateWindow->SetState(_state);

    POINT pt;
    GetCaretPos(&pt);
    _candidateWindow->UpdatePosition(pt);
    _candidateWindow->Show();
}

void CTextService::Reset() {
    _preedit.clear();
    _ghostPreedit.clear();
    _candidates.clear();
    _suggestions.clear();
    _selectedCandidate = 0;
    _page = 0;
    _state = _enabled ? State::TYPING : State::DISABLED;
    _candidateWindow->Hide();
}

void CTextService::ToggleEnabled() {
    _enabled = !_enabled;
    _state = _enabled ? State::TYPING : State::DISABLED;
    if (!_enabled) {
        Reset();
    }
}
