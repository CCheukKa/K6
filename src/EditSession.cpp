#include "EditSession.h"

#include "TextService.h"

CEditSessionInsert::CEditSessionInsert(CTextService* pTextService, ITfContext* pContext, const std::wstring& text)
    : _refCount(1), _pTextService(pTextService), _pContext(pContext), _text(text) {
    if (_pContext) _pContext->AddRef();
}

CEditSessionInsert::~CEditSessionInsert() {
    if (_pContext) _pContext->Release();
}

STDMETHODIMP CEditSessionInsert::QueryInterface(REFIID riid, void** ppvObj) {
    if (!ppvObj) return E_INVALIDARG;
    *ppvObj = nullptr;

    if (riid == IID_IUnknown || riid == IID_ITfEditSession) {
        *ppvObj = static_cast<ITfEditSession*>(this);
        AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG)
CEditSessionInsert::AddRef() {
    return InterlockedIncrement(&_refCount);
}

STDMETHODIMP_(ULONG)
CEditSessionInsert::Release() {
    ULONG c = InterlockedDecrement(&_refCount);
    if (c == 0) delete this;
    return c;
}

STDMETHODIMP CEditSessionInsert::DoEditSession(TfEditCookie ec) {
    if (!_pContext) return E_FAIL;

    ITfInsertAtSelection* pInsert = nullptr;
    HRESULT hr = _pContext->QueryInterface(IID_ITfInsertAtSelection, (void**)&pInsert);
    if (FAILED(hr) || !pInsert) return hr;

    ITfRange* pRange = nullptr;
    hr = pInsert->InsertTextAtSelection(ec, 0, _text.c_str(), (LONG)_text.size(), &pRange);

    if (pRange) {
        pRange->Collapse(ec, TF_ANCHOR_END);
        TF_SELECTION sel = {};
        sel.range = pRange;
        sel.style.ase = TF_AE_END;
        sel.style.fInterimChar = FALSE;
        _pContext->SetSelection(ec, 1, &sel);
        pRange->Release();
    }
    pInsert->Release();

    return hr;
}
