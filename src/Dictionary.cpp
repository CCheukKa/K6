#include "Dictionary.h"

#include <windows.h>

#include <fstream>
#include <sstream>

CDictionary::CDictionary() {
}

CDictionary::~CDictionary() {
}

std::vector<std::wstring> CDictionary::Lookup(const std::wstring& code) const {
    auto it = _dictionary.find(code);
    if (it != _dictionary.end()) {
        return it->second;
    }
    return std::vector<std::wstring>();
}

std::wstring CDictionary::GetDefaultDictionaryPath() {
    wchar_t dllPath[MAX_PATH];
    HMODULE hModule = nullptr;

    // Get handle to our DLL
    GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                      (LPCWSTR)&CDictionary::GetDefaultDictionaryPath, &hModule);

    if (hModule && GetModuleFileName(hModule, dllPath, MAX_PATH)) {
        std::wstring path(dllPath);
        size_t lastSlash = path.find_last_of(L"\\/");
        if (lastSlash != std::wstring::npos) {
            return path.substr(0, lastSlash + 1) + L"dictionary.txt";
        }
    }
    return L"dictionary.txt";
}

// Convert UTF-8 string to wide string
static std::wstring Utf8ToWide(const std::string& utf8) {
    if (utf8.empty()) return std::wstring();

    int size = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), nullptr, 0);
    if (size <= 0) return std::wstring();

    std::wstring wide(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), &wide[0], size);
    return wide;
}

bool CDictionary::LoadFromFile(const std::wstring& path) {
    _dictionary.clear();

    // Open file as UTF-8
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    int lineNum = 0;

    // Skip BOM if present
    char bom[3];
    file.read(bom, 3);
    if (!(bom[0] == '\xEF' && bom[1] == '\xBB' && bom[2] == '\xBF')) {
        file.seekg(0);  // No BOM, go back to start
    }

    while (std::getline(file, line)) {
        lineNum++;

        // Skip empty lines and comments
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }

        // Remove carriage return if present (Windows line endings)
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        // Find tab separator
        size_t tabPos = line.find('\t');
        if (tabPos == std::string::npos || tabPos == 0) {
            continue;  // Invalid line, skip
        }

        std::string code = line.substr(0, tabPos);
        std::string character = line.substr(tabPos + 1);

        if (code.empty() || character.empty()) {
            continue;
        }

        // Convert to wide strings and add
        std::wstring wcode = Utf8ToWide(code);
        std::wstring wchar = Utf8ToWide(character);

        if (!wcode.empty() && !wchar.empty()) {
            AddEntry(wcode, wchar);
        }
    }

    return !_dictionary.empty();
}

void CDictionary::AddEntry(const std::wstring& code, const std::wstring& character) {
    _dictionary[code].push_back(character);
}
