#pragma once
#include <initguid.h>

// {B1A3F37A-7D2E-4B13-9A5F-88A0A0AB8A01}
DEFINE_GUID(CLSID_K6,
            0xb1a3f37a, 0x7d2e, 0x4b13, 0x9a, 0x5f, 0x88, 0xa0, 0xa0, 0xab, 0x8a, 0x01);

// Profiles and category GUIDs we need
// GUID_TFCAT_TIP_KEYBOARD is already defined in uuid.lib, just declare it
EXTERN_C const GUID GUID_TFCAT_TIP_KEYBOARD;

// Language: zh-Hant-HK (Traditional Chinese - Hong Kong)
// Use LANGID for zh-HK
#define IME_LANGID MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_HONGKONG)

// Unique GUID for the input profile
// {8A74C6D2-9A9A-4C2A-9D7E-0F6A4D4A5F11}
DEFINE_GUID(GUID_Profile_K6,
            0x8a74c6d2, 0x9a9a, 0x4c2a, 0x9d, 0x7e, 0x0f, 0x6a, 0x4d, 0x4a, 0x5f, 0x11);
