#include "Registration.h"

#include <msctf.h>
#include <shlwapi.h>
#include <strsafe.h>

#include "guid.h"

#pragma comment(lib, "shlwapi.lib")

extern HINSTANCE g_hInst;

// 港式繁中示例 IME
static const wchar_t kServiceDesc[] = L"\x4e2d\x6587\x8f38\x5165\x6cd5\x7bc4\x4f8b (\x9999\x6e2f)";
static const wchar_t kProfileDesc[] = L"\x6e2f\x5f0f\x7e41\x4e2d\x793a\x4f8b IME";

// Helper to convert GUID to string
static BOOL GuidToString(REFGUID guid, wchar_t* pszOut, DWORD cchOut) {
    return StringFromGUID2(guid, pszOut, cchOut) > 0;
}

// Register CLSID in registry so COM can find our DLL
static HRESULT RegisterCLSID() {
    wchar_t szClsid[64];
    if (!GuidToString(CLSID_ChineseIMEHKExample, szClsid, ARRAYSIZE(szClsid)))
        return E_FAIL;

    wchar_t szKey[256];
    StringCchPrintfW(szKey, ARRAYSIZE(szKey), L"CLSID\\%s", szClsid);

    HKEY hKey;
    DWORD dwDisp;
    LONG lResult = RegCreateKeyExW(HKEY_CLASSES_ROOT, szKey, 0, nullptr, REG_OPTION_NON_VOLATILE,
                                   KEY_WRITE, nullptr, &hKey, &dwDisp);
    if (lResult != ERROR_SUCCESS) return HRESULT_FROM_WIN32(lResult);

    RegSetValueExW(hKey, nullptr, 0, REG_SZ, (const BYTE*)kServiceDesc,
                   (DWORD)((wcslen(kServiceDesc) + 1) * sizeof(wchar_t)));
    RegCloseKey(hKey);

    // InprocServer32 subkey
    StringCchPrintfW(szKey, ARRAYSIZE(szKey), L"CLSID\\%s\\InprocServer32", szClsid);
    lResult = RegCreateKeyExW(HKEY_CLASSES_ROOT, szKey, 0, nullptr, REG_OPTION_NON_VOLATILE,
                              KEY_WRITE, nullptr, &hKey, &dwDisp);
    if (lResult != ERROR_SUCCESS) return HRESULT_FROM_WIN32(lResult);

    wchar_t szModule[MAX_PATH];
    GetModuleFileNameW(g_hInst, szModule, ARRAYSIZE(szModule));

    RegSetValueExW(hKey, nullptr, 0, REG_SZ, (const BYTE*)szModule,
                   (DWORD)((wcslen(szModule) + 1) * sizeof(wchar_t)));

    const wchar_t* szModel = L"Apartment";
    RegSetValueExW(hKey, L"ThreadingModel", 0, REG_SZ, (const BYTE*)szModel,
                   (DWORD)((wcslen(szModel) + 1) * sizeof(wchar_t)));

    RegCloseKey(hKey);
    return S_OK;
}

// Unregister CLSID from registry
static HRESULT UnregisterCLSID() {
    wchar_t szClsid[64];
    if (!GuidToString(CLSID_ChineseIMEHKExample, szClsid, ARRAYSIZE(szClsid)))
        return E_FAIL;

    wchar_t szKey[256];
    StringCchPrintfW(szKey, ARRAYSIZE(szKey), L"CLSID\\%s\\InprocServer32", szClsid);
    RegDeleteKeyW(HKEY_CLASSES_ROOT, szKey);

    StringCchPrintfW(szKey, ARRAYSIZE(szKey), L"CLSID\\%s", szClsid);
    RegDeleteKeyW(HKEY_CLASSES_ROOT, szKey);

    return S_OK;
}

HRESULT RegisterTextService() {
    // First register the COM server CLSID
    HRESULT hr = RegisterCLSID();
    if (FAILED(hr)) return hr;

    hr = CoInitialize(nullptr);
    bool coInit = SUCCEEDED(hr);

    ITfInputProcessorProfileMgr* profileMgr = nullptr;
    hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, nullptr, CLSCTX_INPROC_SERVER,
                          IID_ITfInputProcessorProfileMgr, (void**)&profileMgr);
    if (FAILED(hr)) {
        if (coInit) CoUninitialize();
        return hr;
    }

    // Register the text service class
    ITfCategoryMgr* catMgr = nullptr;
    hr = CoCreateInstance(CLSID_TF_CategoryMgr, nullptr, CLSCTX_INPROC_SERVER, IID_ITfCategoryMgr, (void**)&catMgr);
    if (SUCCEEDED(hr) && catMgr) {
        catMgr->RegisterCategory(CLSID_ChineseIMEHKExample, GUID_TFCAT_TIP_KEYBOARD, CLSID_ChineseIMEHKExample);
        catMgr->Release();
    }

    // Language ID: zh-HK
    LANGID langid = IME_LANGID;

    // Register a single profile
    // RegisterProfile expects: rclsid, langid, guidProfile, pchDesc, cchDesc,
    // pchIconFile, cchFile, uIconIndex, hklsubstitute, dwPreferredLayout, bEnabledByDefault, dwFlags
    hr = profileMgr->RegisterProfile(
        CLSID_ChineseIMEHKExample,
        langid,
        GUID_Profile_ChineseIMEHKExample,
        kProfileDesc,
        (ULONG)wcslen(kProfileDesc),
        nullptr,  // pchIconFile
        0,        // cchFile
        0,        // uIconIndex
        nullptr,  // hklsubstitute
        0,        // dwPreferredLayout
        TRUE,     // bEnabledByDefault
        0         // dwFlags
    );

    if (profileMgr) profileMgr->Release();
    if (coInit) CoUninitialize();
    return hr;
}

HRESULT UnregisterTextService() {
    HRESULT hr = CoInitialize(nullptr);
    bool coInit = SUCCEEDED(hr);

    ITfInputProcessorProfileMgr* profileMgr = nullptr;
    hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, nullptr, CLSCTX_INPROC_SERVER,
                          IID_ITfInputProcessorProfileMgr, (void**)&profileMgr);
    if (SUCCEEDED(hr) && profileMgr) {
        LANGID langid = IME_LANGID;
        profileMgr->UnregisterProfile(CLSID_ChineseIMEHKExample, langid, GUID_Profile_ChineseIMEHKExample, 0);
        profileMgr->Release();
    }

    ITfCategoryMgr* catMgr = nullptr;
    hr = CoCreateInstance(CLSID_TF_CategoryMgr, nullptr, CLSCTX_INPROC_SERVER, IID_ITfCategoryMgr, (void**)&catMgr);
    if (SUCCEEDED(hr) && catMgr) {
        catMgr->UnregisterCategory(CLSID_ChineseIMEHKExample, GUID_TFCAT_TIP_KEYBOARD, CLSID_ChineseIMEHKExample);
        catMgr->Release();
    }

    if (coInit) CoUninitialize();

    // Remove COM server registration
    UnregisterCLSID();

    return S_OK;
}
