#pragma once
#include <map>
#include <string>
#include <vector>

class CDictionary {
   public:
    CDictionary();
    ~CDictionary();

    // Look up candidates for a given input code
    std::vector<std::wstring> Lookup(const std::wstring& code) const;

    // Load dictionary from file (UTF-8 format: code<tab>character per line)
    bool LoadFromFile(const std::wstring& path);

    // Get the dictionary file path next to the DLL
    static std::wstring GetDefaultDictionaryPath();

    size_t GetEntryCount() const { return _dictionary.size(); }

   private:
    std::map<std::wstring, std::vector<std::wstring>> _dictionary;

    void AddEntry(const std::wstring& code, const std::wstring& character);
};
