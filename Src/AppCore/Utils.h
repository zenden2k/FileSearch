#ifndef APPCORE_UTILS_H
#define APPCORE_UTILS_H

#pragma once

#include <string>
#include <windows.h>

namespace FileSearch {
namespace AppCore {
namespace Utils {

std::wstring Utf8ToWideString(const std::string &str);
std::string WideStringToUtf8(const std::wstring &ws);
std::wstring GetModuleFullName(HMODULE module);
std::wstring GetAppFileName();
std::wstring GetAppFolder();

// templated version of my_equal so it could work with both char and wchar_t
template<typename charT>
struct my_equal {
    my_equal(const std::locale& loc) : loc_(loc) {}
    bool operator()(charT ch1, charT ch2) {
        return std::toupper(ch1, loc_) == std::toupper(ch2, loc_);
    }
private:
    const std::locale& loc_;
};

// find substring (case insensitive)
template<typename T>
int FindSubstrCaseInsensitive(const T& str1, const T& str2, const std::locale& loc = std::locale()) {
    typename T::const_iterator it = std::search(str1.begin(), str1.end(),
        str2.begin(), str2.end(), my_equal<typename T::value_type>(loc));
    if (it != str1.end()) {
        return it - str1.begin();
    }
    else {
        return -1; // not found
    }
}

}
}
}
// CString <-> std::string(utf - 8) conversion macroses
#define W2U(str) FileSearch::AppCore::Utils::WideStringToUtf8(((LPCTSTR)(str)))
#define U2W(str) CString(FileSearch::AppCore::Utils::Utf8ToWideString(str).c_str())

#endif