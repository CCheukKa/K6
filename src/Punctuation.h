#pragma once
#include <map>
#include <string>

class CPunctuation {
   public:
    CPunctuation();
    ~CPunctuation();

    // Simple lookup: ASCII character → Chinese character
    bool Lookup(wchar_t asciiChar, std::wstring& outSymbol) const;

    // Load from file (UTF-8 format: ASCII_char<tab>Chinese_char per line)
    bool LoadFromFile(const std::wstring& path);

    // Get the punctuation file path next to the DLL
    static std::wstring GetDefaultPunctuationPath();

    size_t GetEntryCount() const { return _substitutionMap.size(); }

   private:
    // Simple map: ASCII character → Chinese character
    std::map<wchar_t, std::wstring> _substitutionMap;
};
