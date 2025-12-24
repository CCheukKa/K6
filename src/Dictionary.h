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

    // Initialize the dictionary
    void Initialize();

   private:
    // Map from input code to list of candidate characters
    std::map<std::wstring, std::vector<std::wstring>> _dictionary;

    void AddEntry(const std::wstring& code, const std::wstring& character);
};
