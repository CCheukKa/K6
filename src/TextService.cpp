#include "TextService.h"

#include <cwctype>
#include <map>
#include <new>
#include <sstream>

#include "CandidateWindow.h"
#include "EditSession.h"

// Debug helper
static void DebugLog(const wchar_t* msg) {
    std::wstringstream ss;
    ss << "[IME][TextService] " << msg;
    OutputDebugStringW(ss.str().c_str());
    OutputDebugStringW(L"\n");
}

static void DebugLog(const wchar_t* msg, WPARAM wParam) {
    std::wstringstream ss;
    ss << "[IME][TextService] " << msg << L" wParam=0x" << std::hex << wParam;
    OutputDebugStringW(ss.str().c_str());
    OutputDebugStringW(L"\n");
}

static void DebugLogStroke(const wchar_t* msg, const std::wstring& stroke) {
    std::wstringstream ss;
    ss << "[IME][TextService] " << msg << L" stroke='" << stroke << L"' [";

    for (wchar_t c : stroke) {
        ss << L"0x" << std::hex << static_cast<int>(c) << L" ";
    }

    ss << L"]";
    OutputDebugStringW(ss.str().c_str());
    OutputDebugStringW(L"\n");
}

static void DebugLog(const wchar_t* msg, const InputAction& action) {
    std::wstringstream ss;
    ss << "[IME][TextService] " << msg << L" type=" << static_cast<int>(action.type) << L" stroke=" << action.stroke
       << L" index=" << action.index << L" char='" << action.character << L"' changeNextState=" << action.changeNextState
       << L" nextState=" << static_cast<int>(action.nextState);
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
      _state(InputState::TYPING),
      _enabled(TRUE),
      _stateMachine(std::make_unique<InputStateMachine>()) {
    OutputDebugStringW(L"CTextService constructor started\n");
    _candidateWindow = new CCandidateWindow();
    _dictionary.LoadFromFile(CDictionary::GetDefaultDictionaryPath());
    _suggestionDict.LoadFromFile(CSuggestions::GetDefaultSuggestionsPath());
    bool punctLoaded = _punctuationMap.LoadFromFile(CPunctuation::GetDefaultPunctuationPath());

    std::wstringstream ss;
    ss << L"Punctuation loaded: " << (punctLoaded ? L"SUCCESS" : L"FAILED")
       << L", entries: " << _punctuationMap.GetEntryCount() << L"\n";
    OutputDebugStringW(ss.str().c_str());
    OutputDebugStringW(L"CTextService constructor finished\n");
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
    DebugLog(L"Activate called");
    if (!ptim) return E_INVALIDARG;

    _threadMgr = ptim;
    _threadMgr->AddRef();
    _clientId = tid;

    if (SUCCEEDED(_threadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void**)&_keystrokeMgr))) {
        HRESULT hr = _keystrokeMgr->AdviseKeyEventSink(_clientId, this, TRUE);
        if (SUCCEEDED(hr)) {
            DebugLog(L"AdviseKeyEventSink SUCCESS");
        } else {
            DebugLog(L"AdviseKeyEventSink FAILED");
        }
    } else {
        DebugLog(L"Failed to get ITfKeystrokeMgr");
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
    DebugLog(L"OnSetFocus", fForeground);
    return S_OK;
}

void CTextService::HandleInputAction(ITfContext* pContext, const InputAction& action, BOOL* pfEaten) {
    switch (action.type) {
        case InputActionType::NOOP_PASS_THROUGH_KEYPRESS: {
            DebugLog(L"Action: NOOP_PASS_THROUGH_KEYPRESS");
            break;
        }

        case InputActionType::NOOP_CONSUME_KEYPRESS: {
            DebugLog(L"Action: NOOP_CONSUME_KEYPRESS");
            UpdateCandidateWindow();
            break;
        }

        case InputActionType::ADD_STROKE: {
            DebugLog(L"Action: ADD_STROKE");
            DebugLogStroke(L"ADD_STROKE details", std::wstring(1, action.stroke));
            if (_state == InputState::TYPING) {
                _ghostPreedit.clear();
                _preedit.push_back(action.stroke);
                _page = 0;
                _selectedCandidate = 0;
                UpdateQueryResults();
            }
            break;
        }

        case InputActionType::DELETE_STROKE: {
            DebugLog(L"Action: DELETE_STROKE");
            if (!_preedit.empty()) {
                _preedit.pop_back();
                _page = 0;
                _selectedCandidate = 0;
            } else {
                _ghostPreedit.clear();
                _suggestions.clear();
                *pfEaten = FALSE;
            }
            UpdateQueryResults();
            break;
        }

        case InputActionType::CLEAR_STROKE: {
            DebugLog(L"Action: CLEAR_STROKE");
            _ghostPreedit.clear();
            _preedit.clear();
            _candidates.clear();
            _suggestions.clear();
            _page = 0;
            _selectedCandidate = 0;
            UpdateCandidateWindow();
            break;
        }

        case InputActionType::TOGGLE_ENABLE: {
            DebugLog(L"Action: TOGGLE_ENABLE");
            _enabled = !_enabled;
            if (!_enabled) {
                _ghostPreedit.clear();
                _preedit.clear();
                _candidates.clear();
                _suggestions.clear();
                _page = 0;
                _selectedCandidate = 0;
            }
            UpdateCandidateWindow();
            break;
        }

        case InputActionType::SELECT_CHARACTER: {
            DebugLog(L"Action: SELECT_CHARACTER");
            const auto& list = _candidates.empty() ? _suggestions : _candidates;
            UINT idx = (_page * 9) + action.index;
            if (idx < list.size()) {
                const std::wstring chosen = list[idx];
                CommitText(pContext, chosen);
                SetGhostFromCharacter(chosen);
                _preedit.clear();
                _candidates.clear();
                _page = 0;
                _selectedCandidate = 0;
                ShowSuggestionsForCharacter(chosen);
                UpdateCandidateWindow();
            }
            break;
        }

        case InputActionType::NEXT_SELECTION_PAGE: {
            DebugLog(L"Action: NEXT_SELECTION_PAGE");
            if ((_page + 1) * 9 < (_candidates.empty() ? _suggestions.size() : _candidates.size())) {
                _page++;
            }
            UpdateCandidateWindow();
            break;
        }

        case InputActionType::PREVIOUS_SELECTION_PAGE: {
            DebugLog(L"Action: PREVIOUS_SELECTION_PAGE");
            if (_page > 0) {
                _page--;
            }
            UpdateCandidateWindow();
            break;
        }

        case InputActionType::SUBSTITUTE_CHARACTER: {
            DebugLog(L"Action: SUBSTITUTE_CHARACTER");
            CommitText(pContext, action.character);
            _ghostPreedit.clear();
            _preedit.clear();
            _candidates.clear();
            _suggestions.clear();
            _page = 0;
            _selectedCandidate = 0;
            UpdateCandidateWindow();
            break;
        }

        default:
            break;
    }
}

STDMETHODIMP CTextService::OnTestKeyDown(ITfContext*, WPARAM wParam, LPARAM, BOOL* pfEaten) {
    if (!pfEaten) return E_INVALIDARG;
    *pfEaten = FALSE;

    if (!_enabled && !_stateMachine->IsToggleEnableKey(wParam)) {
        return S_OK;
    }

    // Use state machine to determine if key should be consumed
    InputAction action = _stateMachine->ProcessKey(_state, wParam);

    // Consume the key if the state machine says to (but not on NOOP_PASS_THROUGH_KEYPRESS)
    if (action.type != InputActionType::NOOP_PASS_THROUGH_KEYPRESS) {
        *pfEaten = TRUE;
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

    DebugLog(L"======================================================");
    DebugLog(L"OnKeyDown", wParam);

    if (!_enabled && !_stateMachine->IsToggleEnableKey(wParam)) {
        DebugLog(L"OnKeyDown PASSED: IME disabled");
        return S_OK;
    }

    // Use state machine to determine action
    InputAction action = _stateMachine->ProcessKey(_state, wParam);
    if (action.changeNextState) {
        _state = action.nextState;
    }
    DebugLog(L"OnKeyDown Action", action);

    // Handle the action
    *pfEaten = (action.type != InputActionType::NOOP_PASS_THROUGH_KEYPRESS);
    HandleInputAction(pContext, action, pfEaten);

    return S_OK;
}

STDMETHODIMP CTextService::OnKeyUp(ITfContext*, WPARAM, LPARAM, BOOL* pfEaten) {
    if (!pfEaten) return E_INVALIDARG;
    *pfEaten = FALSE;
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
        ss << L"[IME][TS->CW] UpdateCandidateWindow preedit='";
        for (wchar_t c : _preedit) {
            ss << c << L"(0x" << std::hex << static_cast<int>(c) << std::dec << L") ";
        }
        ss << L"' ghost='" << _ghostPreedit << L"' cand=" << _candidates.size() << L" sugg=" << _suggestions.size()
           << L" page=" << _page;
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
    _state = _enabled ? InputState::TYPING : InputState::DISABLED;
    _candidateWindow->Hide();
}

void CTextService::ToggleEnabled() {
    _enabled = !_enabled;
    _state = _enabled ? InputState::TYPING : InputState::DISABLED;
    if (!_enabled) {
        Reset();
    }
}
