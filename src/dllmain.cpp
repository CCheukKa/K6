#include <windows.h>

#include <new>

#include "ClassFactory.h"
#include "Registration.h"
#include "guid.h"

HINSTANCE g_hInst = nullptr;

BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD ul_reason_for_call,
                      LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            g_hInst = hModule;
            DisableThreadLibraryCalls(hModule);
            break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

// COM class factory entry point
extern "C" HRESULT __stdcall DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppvObj) {
    if (!ppvObj) return E_INVALIDARG;
    *ppvObj = nullptr;

    if (rclsid != CLSID_K6)
        return CLASS_E_CLASSNOTAVAILABLE;

    CClassFactory* pFactory = new (std::nothrow) CClassFactory();
    if (!pFactory)
        return E_OUTOFMEMORY;

    HRESULT hr = pFactory->QueryInterface(riid, ppvObj);
    pFactory->Release();
    return hr;
}

// COM unload check
extern "C" HRESULT __stdcall DllCanUnloadNow(void) {
    return (g_cRefDll == 0) ? S_OK : S_FALSE;
}

// COM self-registration for TSF TIP
extern "C" HRESULT __stdcall DllRegisterServer(void) {
    return RegisterTextService();
}

extern "C" HRESULT __stdcall DllUnregisterServer(void) {
    return UnregisterTextService();
}
