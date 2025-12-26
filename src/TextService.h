#pragma once
#include <msctf.h>
#include <windows.h>

#include <memory>
#include <string>
#include <vector>

#include "Dictionary.h"
#include "InputStateMachine.h"
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

   private:
    LONG _refCount;
    ITfThreadMgr* _threadMgr;
    TfClientId _clientId;
    ITfKeystrokeMgr* _keystrokeMgr;

    // State machine
    std::unique_ptr<InputStateMachine> _stateMachine;

    // Query / selection state
    std::wstring _strokeinput;               // current query strokes
    std::wstring _ghostStrokeInput;          // ghost strokes after commit
    std::vector<std::wstring> _candidates;   // character results
    std::vector<std::wstring> _suggestions;  // suggestion results
    UINT _selectedCandidate;                 // index in current page [0..8]
    UINT _page;                              // page for candidates/suggestions
    InputState _state;
    BOOL _enabled;  // overall IME enabled

    // Shift-toggle tracking
    BOOL _shiftDown = FALSE;            // whether Shift is currently held
    BOOL _shiftUsedAsModifier = FALSE;  // whether a non-Shift key was pressed while Shift held

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

    // State machine action handler
    void HandleInputAction(ITfContext* pContext, const InputAction& action, BOOL* pfEaten);
};
