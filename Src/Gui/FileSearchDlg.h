
// FileSearchDlg.h : header file
//

#pragma once
#include "afxwin.h"

#include <atomic>


namespace FileSearch {
namespace Gui{

// CFileSearchDlg dialog
class CFileSearchDlg : public CDialogEx {
    // Construction
public:
    CFileSearchDlg(CWnd* pParent = NULL);	// standard constructor

    // Dialog Data
    enum { IDD = IDD_FILESEARCH_DIALOG };
    enum { IDM_POPUPMENU_ITEM_FIRST = 5001, IDM_POPUPMENU_ITEM_LAST = 5100 };
protected:
    virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


    // Implementation
protected:
    HICON m_hIcon;

    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnPopupMenuItemClicked(UINT nID);
    DECLARE_MESSAGE_MAP()
public:
    CButton m_SearchButton;
    CButton m_ChooseButton;
    CButton m_StopButton;
    CButton m_CloseButton;
    CButton m_ExampleQueriesButton;
    CEdit m_DirectoryEdit;
    CEdit m_QueryEdit;
    CListBox m_ResultsListBox;
    CProgressCtrl m_ProgressBar;
    std::atomic<bool> m_StopFlag;
    void backgroundTaskStarted();
    void backgroundTaskFinished();
    afx_msg void OnBnClickedSearchButton();
    afx_msg void OnBnClickedChooseButton();
    afx_msg void OnBnClickedStopButton();
    afx_msg void OnBnClickedExampleQueriesButton();
};

}
}