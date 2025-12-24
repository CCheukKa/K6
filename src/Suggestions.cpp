#include "Suggestions.h"

#include <windows.h>

#include <fstream>
#include <string>

static std::wstring Utf8ToWideS(const std::string& utf8) {
    if (utf8.empty()) return std::wstring();
    int size = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), nullptr, 0);
    if (size <= 0) return std::wstring();
    std::wstring wide(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), &wide[0], size);
    return wide;
}

CSuggestions::CSuggestions() {}
CSuggestions::~CSuggestions() {}

void CSuggestions::AddEntry(const std::wstring& character, const std::wstring& suggestion) {
    _suggestions[character].push_back(suggestion);
}

std::vector<std::wstring> CSuggestions::Lookup(const std::wstring& character) const {
    auto it = _suggestions.find(character);
    if (it != _suggestions.end()) return it->second;
    return {};
}

std::wstring CSuggestions::GetDefaultSuggestionsPath() {
    wchar_t dllPath[MAX_PATH];
    HMODULE hModule = nullptr;

    GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                      (LPCWSTR)&CSuggestions::GetDefaultSuggestionsPath, &hModule);

    if (hModule && GetModuleFileName(hModule, dllPath, MAX_PATH)) {
        std::wstring path(dllPath);
        size_t lastSlash = path.find_last_of(L"\\/");
        if (lastSlash != std::wstring::npos) {
            return path.substr(0, lastSlash + 1) + L"suggestionsData.txt";
        }
    }
    return L"suggestionsData.txt";
}

bool CSuggestions::LoadFromFile(const std::wstring& path) {
    _suggestions.clear();

    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) return false;

    // Skip BOM if present
    char bom[3] = {0};
    file.read(bom, 3);
    if (!(bom[0] == '\xEF' && bom[1] == '\xBB' && bom[2] == '\xBF')) {
        file.seekg(0);
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#' || line[0] == ';') continue;
        if (!line.empty() && line.back() == '\r') line.pop_back();

        size_t tabPos = line.find('\t');
        if (tabPos == std::string::npos || tabPos == 0) continue;

        std::string ch = line.substr(0, tabPos);
        std::string sg = line.substr(tabPos + 1);
        if (ch.empty() || sg.empty()) continue;

        AddEntry(Utf8ToWideS(ch), Utf8ToWideS(sg));
    }
    return true;
}
