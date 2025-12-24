#pragma once
#include <unknwn.h>
#include <windows.h>

class CClassFactory : public IClassFactory {
   public:
    CClassFactory();
    ~CClassFactory();

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj) override;
    STDMETHODIMP_(ULONG)
    AddRef(void) override;
    STDMETHODIMP_(ULONG)
    Release(void) override;

    // IClassFactory
    STDMETHODIMP CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObj) override;
    STDMETHODIMP LockServer(BOOL fLock) override;

   private:
    LONG _refCount;
};

// Global lock count for DllCanUnloadNow
extern LONG g_cRefDll;
void DllAddRef();
void DllRelease();
