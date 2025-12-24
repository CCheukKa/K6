#pragma once
#include <msctf.h>
#include <windows.h>

#include <string>
#include <vector>

#include "CandidateWindow.h"
#include "Dictionary.h"
#include "guid.h"

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
    STDMETHODIMP OnSetFocus(BOOL fForeground) override;
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

    std::wstring _preedit;
    std::vector<std::wstring> _candidates;
    UINT _selectedCandidate;

    CCandidateWindow* _candidateWindow;
    CDictionary _dictionary;

    void CommitText(ITfContext* pContext, const std::wstring& text);
    void UpdateCandidateWindow();
    void Reset();
};
