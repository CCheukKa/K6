#pragma once
#include <msctf.h>

#include <string>

class CTextService;

class CEditSessionInsert : public ITfEditSession {
   public:
    CEditSessionInsert(CTextService* pTextService, ITfContext* pContext, const std::wstring& text);
    ~CEditSessionInsert();

    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj) override;
    STDMETHODIMP_(ULONG)
    AddRef() override;
    STDMETHODIMP_(ULONG)
    Release() override;
    STDMETHODIMP DoEditSession(TfEditCookie ec) override;

   private:
    LONG _refCount;
    CTextService* _pTextService;
    ITfContext* _pContext;
    std::wstring _text;
};
