#include <windowsx.h>
#include "BrowseFolder.h"
#include <strsafe.h>
#include "AppCore/Library.h"

namespace FileSearch {
namespace Gui {
CString CBrowseFolder::m_sDefaultPath;

Library hLib(L"shell32.dll");

CBrowseFolder::CBrowseFolder(void)
    : m_style(0),
    m_root(nullptr)
{
    SecureZeroMemory(m_displayName, sizeof(m_displayName));
    SecureZeroMemory(m_title, sizeof(m_title));
}

CBrowseFolder::~CBrowseFolder(void)
{
}

//show the dialog
CBrowseFolder::retVal CBrowseFolder::Show(HWND parent, LPTSTR path, size_t pathlen, LPCTSTR szDefaultPath /* = NULL */)
{
    CString temp;
    temp = path;
    CString sDefault;
    if (szDefaultPath)
        sDefault = szDefaultPath;
    CBrowseFolder::retVal ret = Show(parent, temp, sDefault);
    _tcscpy_s(path, pathlen, temp);
    return ret;
}
CBrowseFolder::retVal CBrowseFolder::Show(HWND parent, CString& path, const CString& sDefaultPath /* = CString() */)
{
    retVal ret = OK;		//assume OK
    m_sDefaultPath = sDefaultPath;
    if (m_sDefaultPath.IsEmpty() && !path.IsEmpty()) {
        while (!PathFileExists(path) && !path.IsEmpty()) {
            CString p = path.Left(path.ReverseFind(L'\\'));
            if ((p.GetLength() == 2) && (p[1] == L':')) {
                p += L"\\";
                if (p.Compare(path) == 0)
                    p.Empty();
            }
            if (p.GetLength() < 2)
                p.Empty();
            path = p;
        }
        // if the result path already contains a path, use that as the default path
        m_sDefaultPath = path;
    }

    HRESULT hr;

    // Create a new common open file dialog
    IFileOpenDialog* pfd = NULL;
    hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
    if (SUCCEEDED(hr)) {
        // Set the dialog as a folder picker
        DWORD dwOptions;
        if (SUCCEEDED(hr = pfd->GetOptions(&dwOptions))) {
            hr = pfd->SetOptions(dwOptions | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST);
        }

        // Set a title
        if (SUCCEEDED(hr)) {
            TCHAR * nl = _tcschr(m_title, '\n');
            if (nl)
                *nl = 0;
            pfd->SetTitle(m_title);
        }

        // set the default folder
        if (SUCCEEDED(hr)) {
            typedef HRESULT(WINAPI *SHCIFPN)(PCWSTR pszPath, IBindCtx * pbc, REFIID riid, void ** ppv);

            SHCIFPN pSHCIFPN = hLib.GetProcAddress<SHCIFPN>("SHCreateItemFromParsingName");
            if (pSHCIFPN) {
                IShellItem *psiDefault = 0;
                hr = pSHCIFPN(m_sDefaultPath, NULL, IID_PPV_ARGS(&psiDefault));
                if (SUCCEEDED(hr)) {
                    hr = pfd->SetFolder(psiDefault);
                    psiDefault->Release();
                }
            }

        }

        // Show the open file dialog
        if (SUCCEEDED(hr) && SUCCEEDED(hr = pfd->Show(parent))) {
            // Get the selection from the user
            IShellItem* psiResult = NULL;
            hr = pfd->GetResult(&psiResult);
            if (SUCCEEDED(hr)) {
                PWSTR pszPath = NULL;
                hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszPath);
                if (SUCCEEDED(hr)) {
                    path = pszPath;
                    CoTaskMemFree(pszPath);
                }
                psiResult->Release();
            } else
                ret = CANCEL;
        } else
            ret = CANCEL;

        pfd->Release();
    } else {
        BROWSEINFO browseInfo = {};
        browseInfo.hwndOwner = parent;
        browseInfo.pidlRoot = m_root;
        browseInfo.pszDisplayName = m_displayName;
        browseInfo.lpszTitle = m_title;
        browseInfo.ulFlags = m_style;
        browseInfo.lParam = reinterpret_cast<LPARAM>(this);

        PCIDLIST_ABSOLUTE itemIDList = SHBrowseForFolder(&browseInfo);

        //is the dialog canceled?
        if (!itemIDList)
            ret = CANCEL;

        if (ret != CANCEL) {
            if (!SHGetPathFromIDList(itemIDList, path.GetBuffer(MAX_PATH)))		// MAX_PATH ok. Explorer can't handle paths longer than MAX_PATH.
                ret = NOPATH;

            path.ReleaseBuffer();

            CoTaskMemFree((LPVOID)itemIDList);
        }
    }

    return ret;
}

void CBrowseFolder::SetInfo(LPCTSTR title)
{
    ASSERT(title);

    if (title)
        _tcscpy_s(m_title, title);
}

}
}