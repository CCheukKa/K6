#pragma once
#include <map>
#include <string>
#include <vector>

class CDictionary {
   public:
    CDictionary();
    ~CDictionary();

    // Exact lookup for a full code
    std::vector<std::wstring> Lookup(const std::wstring& code) const;

    // Regex lookup with wildcard '＊' interpreted as '.' (anchored at start)
    std::vector<std::wstring> LookupRegex(const std::wstring& pattern) const;

    // Load dictionary from file (UTF-8 format: code<tab>character per line)
    bool LoadFromFile(const std::wstring& path);

    // Get the dictionary file path next to the DLL
    static std::wstring GetDefaultDictionaryPath();

    // Reverse lookup: collect all stroke codes for a character
    std::vector<std::wstring> GetCodesForCharacter(const std::wstring& character) const;

    // Convenience: pick any stroke sequence for the character (may be empty)
    std::wstring GetRandomStrokeForCharacter(const std::wstring& character) const;

    size_t GetEntryCount() const { return _dictionary.size(); }

   private:
    std::map<std::wstring, std::vector<std::wstring>> _dictionary;
    std::vector<std::pair<std::wstring, std::wstring>> _insertionOrder;
    mutable std::map<std::wstring, std::vector<std::wstring>> _regexCache;
    mutable std::map<std::wstring, std::vector<std::wstring>> _reverseCache;

    void AddEntry(const std::wstring& code, const std::wstring& character);
};
