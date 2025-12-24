#include "Punctuation.h"

#include <windows.h>

#include <fstream>
#include <sstream>

CPunctuation::CPunctuation() {
}

CPunctuation::~CPunctuation() {
}

bool CPunctuation::Lookup(wchar_t asciiChar, std::wstring& outSymbol) const {
    auto it = _substitutionMap.find(asciiChar);
    if (it != _substitutionMap.end()) {
        outSymbol = it->second;
        return true;
    }
    return false;
}

bool CPunctuation::LoadFromFile(const std::wstring& path) {
    // Open file in binary mode and handle UTF-8 decoding manually
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        OutputDebugStringW(L"[IME] Failed to open punctuation file\n");
        return false;
    }

    _substitutionMap.clear();

    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // Convert UTF-8 line to wide string for processing
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, &line[0], (int)line.size(), NULL, 0);
        std::wstring wline(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, &line[0], (int)line.size(), &wline[0], size_needed);

        // Parse format: ASCII_char<tab>Chinese_char
        std::wstringstream ss(wline);
        std::wstring asciiStr, chineseStr;

        if (!std::getline(ss, asciiStr, L'\t')) continue;
        if (!std::getline(ss, chineseStr, L'\t')) continue;

        if (asciiStr.empty() || chineseStr.empty()) {
            continue;
        }

        wchar_t asciiChar = asciiStr[0];
        _substitutionMap[asciiChar] = chineseStr;

        std::wstringstream debugSS;
        debugSS << L"[IME] Loaded punctuation: '" << asciiChar << L"' -> '" << chineseStr << L"'\n";
        OutputDebugStringW(debugSS.str().c_str());
    }

    file.close();

    std::wstringstream ss;
    ss << L"[IME] Punctuation map loaded: " << _substitutionMap.size() << L" entries\n";
    OutputDebugStringW(ss.str().c_str());

    return true;
}

std::wstring CPunctuation::GetDefaultPunctuationPath() {
    wchar_t dllPath[MAX_PATH];
    HMODULE hModule = nullptr;

    // Get handle to our DLL
    GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                      (LPCWSTR)&CPunctuation::GetDefaultPunctuationPath, &hModule);

    if (hModule && GetModuleFileName(hModule, dllPath, MAX_PATH)) {
        std::wstring path(dllPath);
        size_t lastSlash = path.find_last_of(L"\\/");
        if (lastSlash != std::wstring::npos) {
            path = path.substr(0, lastSlash + 1) + L"punctuationData.txt";
        }

        std::wstringstream ss;
        ss << L"[IME] Punctuation file path: " << path << L"\n";
        OutputDebugStringW(ss.str().c_str());

        return path;
    }

    return L"punctuationData.txt";
}
