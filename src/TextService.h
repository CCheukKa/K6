#pragma once
#include <msctf.h>
#include <windows.h>

#include <string>
#include <vector>

#include "Dictionary.h"
#include "Punctuation.h"
#include "Stroke.h"
#include "Suggestions.h"
#include "guid.h"

class CCandidateWindow;

class CTextService : public ITfTextInputProcessor, public ITfKeyEventSink {
   public:
    CTextService();
    ~CTextService();

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj) override;
    STDMETHODIMP_(ULONG)
    AddRef() override;
    STDMETHODIMP_(ULONG)
    Release() override;

    // ITfTextInputProcessor
    STDMETHODIMP Activate(ITfThreadMgr* ptim, TfClientId tid) override;
    STDMETHODIMP Deactivate() override;

    // ITfKeyEventSink
    STDMETHODIMP OnSetFocus(BOOL fForeground);
    STDMETHODIMP OnTestKillFocus(ITfContext* pContext, WPARAM wParam, LPARAM lParam, BOOL* pfEaten);
    STDMETHODIMP OnTestKeyDown(ITfContext* pContext, WPARAM wParam, LPARAM lParam, BOOL* pfEaten) override;
    STDMETHODIMP OnTestKeyUp(ITfContext* pContext, WPARAM wParam, LPARAM lParam, BOOL* pfEaten) override;
    STDMETHODIMP OnKeyDown(ITfContext* pContext, WPARAM wParam, LPARAM lParam, BOOL* pfEaten) override;
    STDMETHODIMP OnKeyUp(ITfContext* pContext, WPARAM wParam, LPARAM lParam, BOOL* pfEaten) override;
    STDMETHODIMP OnPreservedKey(ITfContext* pContext, REFGUID rguid, BOOL* pfEaten) override;

    TfClientId GetClientId() const { return _clientId; }

    enum class State {
        DISABLED,
        TYPING,
        SELECTING
    };

   private:
    LONG _refCount;
    ITfThreadMgr* _threadMgr;
    TfClientId _clientId;
    ITfKeystrokeMgr* _keystrokeMgr;

    // Query / selection state
    std::wstring _preedit;                   // current query strokes
    std::wstring _ghostPreedit;              // ghost strokes after commit
    std::vector<std::wstring> _candidates;   // character results
    std::vector<std::wstring> _suggestions;  // suggestion results
    UINT _selectedCandidate;                 // index in current page [0..8]
    UINT _page;                              // page for candidates/suggestions
    State _state;
    BOOL _enabled;          // overall IME enabled
    BOOL _shiftPressed;     // track Shift held for toggle
    BOOL _otherKeyPressed;  // any other key pressed while Shift is down

    CCandidateWindow* _candidateWindow;
    CDictionary _dictionary;
    CSuggestions _suggestionDict;
    CPunctuation _punctuationMap;

    void CommitText(ITfContext* pContext, const std::wstring& text);
    void UpdateCandidateWindow();
    void Reset();
    void ToggleEnabled();

    // Query update helpers
    void UpdateQueryResults();
    void SetGhostFromCharacter(const std::wstring& ch);
    void ShowSuggestionsForCharacter(const std::wstring& ch);

    // key helpers
    bool MapStrokeKey(WPARAM vk, wchar_t& outStroke) const;
    bool IsDigitKey(WPARAM vk, UINT& outIndex) const;  // 0..9
    bool MapChineseSymbol(WPARAM vk, std::wstring& outSymbol) const;
};
