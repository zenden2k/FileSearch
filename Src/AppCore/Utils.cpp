#include "Utils.h"

#include <windows.h>
#include <memory>

namespace FileSearch {
namespace AppCore {
namespace Utils {

std::wstring Utf8ToWideString(const std::string &str) {
    std::wstring ws;
    int n = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.size() + 1, nullptr, 0);
    if (n) {
        ws.resize(n - 1);
        if (MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.size() + 1, &ws[0], n) == 0)
            ws.clear();
    }
    return ws;
}

std::string WideStringToUtf8(const std::wstring &ws) {
    std::string str;
    int n = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), ws.size() + 1, nullptr, 0, nullptr, nullptr);
    if (n) {
        str.resize(n - 1);
        if (WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), ws.size() + 1, &str[0], n, nullptr, nullptr) == 0)
            str.clear();
    }
    return str;
}

std::wstring GetModuleFullName(HMODULE module) {
    std::unique_ptr<wchar_t[]> buf;
    DWORD  bufLen = MAX_PATH;
    DWORD  retLen;
    std::wstring res;

    while (32768 >= bufLen) {
        buf.reset(new wchar_t[bufLen]);

        if ((retLen = ::GetModuleFileName(module, buf.get(), bufLen)) == 0) {
            /* GetModuleFileName failed */
            break;
        } else if (bufLen > retLen) {
            /* Success */
            res = buf.get();
            break;
        }

        bufLen *= 2;
    }

    return res;
}

std::wstring GetAppFileName() {
    return GetModuleFullName(nullptr);
}

LPTSTR ExtractFilePath(LPCTSTR FileName, LPTSTR buf)
{
    int i, len = lstrlen(FileName);
    for (i = len; i >= 0; i--) {
        if (FileName[i] == _T('\\'))
            break;
    }
    lstrcpyn(buf, FileName, i + 2);
    return buf;
}

std::wstring GetAppFolder()
{
    std::wstring filename;
    std::wstring path;

    filename = GetModuleFullName(nullptr);
    std::unique_ptr<wchar_t[]> buffer(new wchar_t[filename.size() + 1]);
    ExtractFilePath(filename.c_str(), buffer.get());
    path = buffer.get();
    return path;
}

}
}
}