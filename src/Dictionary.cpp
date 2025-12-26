#include "Dictionary.h"

#include <windows.h>

#include <chrono>
#include <fstream>
#include <set>
#include <sstream>

#include "Debug.h"
#include "Stroke.h"

CDictionary::CDictionary() {
}

CDictionary::~CDictionary() {
}

std::vector<std::wstring> CDictionary::Lookup(const std::wstring& code) const {
    auto start = std::chrono::high_resolution_clock::now();

    auto it = _dictionary.find(code);
    std::vector<std::wstring> result;
    if (it != _dictionary.end()) {
        result = it->second;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    Debug::Log(L"Dictionary", (L"Lookup code: " + code +
                               L" | Results: " + std::to_wstring(result.size()) +
                               L" | Time: " + std::to_wstring(duration / 1000) + L"." + std::to_wstring(duration % 1000) + L"ms")
                                  .c_str());

    return result;
}

std::vector<std::wstring> CDictionary::LookupRegex(const std::wstring& pattern) const {
    auto start = std::chrono::high_resolution_clock::now();

    if (pattern.empty()) return {};

    auto cacheIt = _regexCache.find(pattern);
    if (cacheIt != _regexCache.end()) {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        Debug::Log(L"Dictionary", (L"LookupRegex (cached) pattern: " + pattern +
                                   L" | Results: " + std::to_wstring(cacheIt->second.size()) +
                                   L" | Time: " + std::to_wstring(duration / 1000) + L"." + std::to_wstring(duration % 1000) + L"ms")
                                      .c_str());
        return cacheIt->second;
    }

    std::vector<std::wstring> out;
    std::set<std::wstring> seen;

    // Pre-reserve capacity to reduce allocations (typical result size)
    out.reserve(50);

    // Fast wildcard matching without regex - much more efficient
    // Split pattern into segments (non-wildcard parts)
    std::vector<std::wstring> segments;
    std::wstring currentSegment;

    for (wchar_t ch : pattern) {
        if (ch == Stroke::WILDCARD[0]) {
            if (!currentSegment.empty()) {
                segments.push_back(currentSegment);
                currentSegment.clear();
            }
            segments.push_back(std::wstring(1, Stroke::WILDCARD[0]));
        } else {
            currentSegment.push_back(ch);
        }
    }
    if (!currentSegment.empty()) {
        segments.push_back(currentSegment);
    }

    // Check if pattern starts with wildcard
    bool startsWithWildcard = !segments.empty() && segments[0] == std::wstring(1, Stroke::WILDCARD[0]);

    // Iterate in insertion order instead of map order
    for (const auto& entry : _insertionOrder) {
        const std::wstring& code = entry.first;
        const std::wstring& character = entry.second;

        // Fast wildcard matching with start-aligned partial match
        // Pattern matches if all segments match sequentially from the start
        // but the code can have additional characters after the pattern
        bool matches = true;
        size_t codePos = 0;

        for (size_t i = 0; i < segments.size(); ++i) {
            const auto& segment = segments[i];

            if (segment == std::wstring(1, Stroke::WILDCARD[0])) {
                // Wildcard matches any single character
                if (codePos < code.length()) {
                    codePos++;
                } else {
                    // Wildcard but no character left in code
                    matches = false;
                    break;
                }
            } else {
                // Literal segment must match exactly at current position
                if (codePos + segment.length() > code.length() ||
                    code.substr(codePos, segment.length()) != segment) {
                    matches = false;
                    break;
                }
                codePos += segment.length();
            }
        }

        // Match if pattern matched from start (code can have additional characters after pattern)
        if (matches) {
            if (seen.insert(character).second) {
                out.push_back(character);
            }
        }
    }

    // Shrink to actual size to save memory in cache
    out.shrink_to_fit();

    _regexCache.emplace(pattern, out);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    Debug::Log(L"Dictionary", (L"LookupRegex pattern: " + pattern +
                               L" | Results: " + std::to_wstring(out.size()) +
                               L" | Entries scanned: " + std::to_wstring(_insertionOrder.size()) +
                               L" | Time: " + std::to_wstring(duration / 1000) + L"." + std::to_wstring(duration % 1000) + L"ms")
                                  .c_str());

    return out;
}

std::vector<std::wstring> CDictionary::GetCodesForCharacter(const std::wstring& character) const {
    auto start = std::chrono::high_resolution_clock::now();

    // Check reverse lookup cache first
    auto cacheIt = _reverseCache.find(character);
    if (cacheIt != _reverseCache.end()) {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        Debug::Log(L"Dictionary", (L"GetCodesForCharacter (cached): " + character +
                                   L" | Results: " + std::to_wstring(cacheIt->second.size()) +
                                   L" | Time: " + std::to_wstring(duration / 1000) + L"." + std::to_wstring(duration % 1000) + L"ms")
                                      .c_str());
        return cacheIt->second;
    }

    std::vector<std::wstring> codes;
    codes.reserve(10);  // Pre-allocate space for typical number of codes per character

    for (const auto& kv : _dictionary) {
        for (const auto& ch : kv.second) {
            if (ch == character) {
                codes.push_back(kv.first);
                break;  // Found match in this code's list, move to next code
            }
        }
    }

    // Shrink to actual size before caching
    codes.shrink_to_fit();

    // Cache the result for future lookups
    _reverseCache.emplace(character, codes);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    Debug::Log(L"Dictionary", (L"GetCodesForCharacter: " + character +
                               L" | Results: " + std::to_wstring(codes.size()) +
                               L" | Time: " + std::to_wstring(duration / 1000) + L"." + std::to_wstring(duration % 1000) + L"ms")
                                  .c_str());

    return codes;
}

// TODO: add a canonical way to select a stroke sequence
std::wstring CDictionary::GetRandomStrokeForCharacter(const std::wstring& character) const {
    auto codes = GetCodesForCharacter(character);
    if (codes.empty()) return L"";
    size_t idx = static_cast<size_t>(rand()) % codes.size();
    return codes[idx];
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
            return path.substr(0, lastSlash + 1) + L"strokeData.txt";
        }
    }
    return L"strokeData.txt";
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
    _insertionOrder.clear();
    _regexCache.clear();
    _reverseCache.clear();  // Clear reverse cache on reload

    // Open file as UTF-8
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    line.reserve(256);  // Pre-allocate typical line size
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
    _insertionOrder.push_back({code, character});
}
