#ifndef GUI_BROWSEFOLDER_H
#define GUI_BROWSEFOLDER_H

#pragma once

namespace FileSearch {
namespace Gui {
/**
 * A simple wrapper class for the SHBrowseForFolder API on XP.
 * On Vista and later, the IFileDialog is used with the FOS_PICKFOLDERS flag.
 */
class CBrowseFolder {
public:
    enum retVal {
        CANCEL = 0,		// the user has pressed cancel
        NOPATH,			// no folder was selected
        OK				// everything went just fine
    };
public:
    CBrowseFolder(void);
    ~CBrowseFolder(void);

    void SetInfo(LPCTSTR title);

    CBrowseFolder::retVal Show(HWND parent, CString& path, const CString& sDefaultPath = CString());
    CBrowseFolder::retVal Show(HWND parent, LPTSTR path, size_t pathlen, LPCTSTR szDefaultPath = nullptr);

protected:
    TCHAR m_title[200];
    static CString m_sDefaultPath;
    TCHAR m_displayName[200];
    PCIDLIST_ABSOLUTE m_root;
    DWORD m_style;		// styles of the dialog.
};
}
}

#endif