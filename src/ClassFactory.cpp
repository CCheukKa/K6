#include "ClassFactory.h"

#include <new>

#include "TextService.h"
#include "guid.h"

LONG g_cRefDll = 0;

void DllAddRef() {
    InterlockedIncrement(&g_cRefDll);
}

void DllRelease() {
    InterlockedDecrement(&g_cRefDll);
}

CClassFactory::CClassFactory() : _refCount(1) {
    DllAddRef();
}

CClassFactory::~CClassFactory() {
    DllRelease();
}

STDMETHODIMP CClassFactory::QueryInterface(REFIID riid, void** ppvObj) {
    if (!ppvObj) return E_INVALIDARG;
    *ppvObj = nullptr;

    if (riid == IID_IUnknown || riid == IID_IClassFactory) {
        *ppvObj = static_cast<IClassFactory*>(this);
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG)
CClassFactory::AddRef(void) {
    return InterlockedIncrement(&_refCount);
}

STDMETHODIMP_(ULONG)
CClassFactory::Release(void) {
    ULONG c = InterlockedDecrement(&_refCount);
    if (c == 0) {
        delete this;
    }
    return c;
}

STDMETHODIMP CClassFactory::CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObj) {
    if (!ppvObj) return E_INVALIDARG;
    *ppvObj = nullptr;

    if (pUnkOuter != nullptr)
        return CLASS_E_NOAGGREGATION;

    CTextService* pTextService = new (std::nothrow) CTextService();
    if (!pTextService)
        return E_OUTOFMEMORY;

    HRESULT hr = pTextService->QueryInterface(riid, ppvObj);
    pTextService->Release();  // Release our ref; QI added one if successful
    return hr;
}

STDMETHODIMP CClassFactory::LockServer(BOOL fLock) {
    if (fLock)
        DllAddRef();
    else
        DllRelease();
    return S_OK;
}
