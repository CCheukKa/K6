#pragma once
#include <map>
#include <string>
#include <vector>

class CSuggestions {
   public:
    CSuggestions();
    ~CSuggestions();

    std::vector<std::wstring> Lookup(const std::wstring& character) const;
    bool LoadFromFile(const std::wstring& path);
    static std::wstring GetDefaultSuggestionsPath();
    size_t GetEntryCount() const { return _suggestions.size(); }

   private:
    std::map<std::wstring, std::vector<std::wstring>> _suggestions;
    void AddEntry(const std::wstring& character, const std::wstring& suggestion);
};
