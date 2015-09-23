
// FileSearchDlg.cpp : implementation file
//

#include "FileSearch.h"
#include "FileSearchDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include "AppCore/Utils.h"

#include "AppCore/ParserTypes.h"
#include "BrowseFolder.h"
#include "AppCore/Parser.h"
#include "AppCore/SearchEngine.h"
#include <future>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

namespace FileSearch {
namespace Gui{

const TCHAR * const SampleQueries[] = {
    _T("(name == \"config.cfg\" and size < 100) or ( attribute is hidden or attribute not directory)"),
    _T("name contains \".txt\""),
    _T("attribute is directory and name contains \"(\""),
    _T("strlen(name)<15")
};

namespace {
struct TaskDispatcherMessageStruct {
    TaskDispatcherTask callback;
    //bool async;
    //Object* sender;
};
}

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx {
public:
    CAboutDlg();

    // Dialog Data
    enum { IDD = IDD_ABOUTBOX };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Implementation
protected:
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CFileSearchDlg dialog



CFileSearchDlg::CFileSearchDlg(CWnd* pParent /*=NULL*/)
    : CDialogEx(CFileSearchDlg::IDD, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CFileSearchDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_CHOOSEBUTTON, m_ChooseButton);
    DDX_Control(pDX, IDC_SEARCHBUTTON, m_SearchButton);
    DDX_Control(pDX, IDC_STOPBUTTON, m_StopButton);
    DDX_Control(pDX, IDC_EXAMPLEQUERIESBUTTON, m_ExampleQueriesButton);
    DDX_Control(pDX, IDC_STARTDIRECTORYEDIT, m_DirectoryEdit);
    DDX_Control(pDX, IDC_QUERYEDIT, m_QueryEdit);
    DDX_Control(pDX, IDC_RESULTSLIST, m_ResultsListBox);
    DDX_Control(pDX, IDC_PROGRESSBAR, m_ProgressBar);

    DDX_Control(pDX, IDCANCEL, m_CloseButton);
}

BEGIN_MESSAGE_MAP(CFileSearchDlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_MESSAGE(WM_TASKDISPATCHERMSG, &CFileSearchDlg::OnTaskDispatcherMsg)
    ON_BN_CLICKED(IDC_SEARCHBUTTON, &CFileSearchDlg::OnBnClickedSearchButton)
    ON_BN_CLICKED(IDC_CHOOSEBUTTON, &CFileSearchDlg::OnBnClickedChooseButton)
    ON_BN_CLICKED(IDC_STOPBUTTON, &CFileSearchDlg::OnBnClickedStopButton)
    ON_BN_CLICKED(IDC_EXAMPLEQUERIESBUTTON, &CFileSearchDlg::OnBnClickedExampleQueriesButton)
    ON_COMMAND_RANGE(IDM_POPUPMENU_ITEM_FIRST, IDM_POPUPMENU_ITEM_LAST, &CFileSearchDlg::OnPopupMenuItemClicked)
END_MESSAGE_MAP()


// CFileSearchDlg message handlers

BOOL CFileSearchDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // Add "About..." menu item to system menu.

    // IDM_ABOUTBOX must be in the system command range.
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != NULL) {
        BOOL bNameValid;
        CString strAboutMenu;
        bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
        ASSERT(bNameValid);
        if (!strAboutMenu.IsEmpty()) {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);			// Set big icon
    SetIcon(m_hIcon, FALSE);		// Set small icon

    m_DirectoryEdit.SetWindowText(FileSearch::AppCore::Utils::GetAppFolder().c_str());
    m_QueryEdit.SetWindowText(_T("size > 3 and name ==\"test\""));
    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CFileSearchDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX) {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    } else {
        CDialogEx::OnSysCommand(nID, lParam);
    }
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CFileSearchDlg::OnPaint()
{
    if (IsIconic()) {
        CPaintDC dc(this); // device context for painting

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // Draw the icon
        dc.DrawIcon(x, y, m_hIcon);
    } else {
        CDialogEx::OnPaint();
    }
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CFileSearchDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

void CFileSearchDlg::OnPopupMenuItemClicked(UINT id) {
    int queryIndex = id - IDM_POPUPMENU_ITEM_FIRST;
    m_QueryEdit.SetWindowText(SampleQueries[queryIndex]);
}

void CFileSearchDlg::backgroundTaskStarted() {
    m_ProgressBar.SetMarquee(TRUE, 40);
    m_SearchButton.ShowWindow(SW_HIDE);
    m_StopButton.ShowWindow(SW_SHOW);
    m_ProgressBar.ShowWindow(SW_SHOW);
    m_CloseButton.EnableWindow(FALSE);
}

void CFileSearchDlg::backgroundTaskFinished() {
    m_ProgressBar.SetMarquee(FALSE, 0);
    m_ProgressBar.ShowWindow(SW_HIDE);
    m_SearchButton.ShowWindow(SW_SHOW);
    m_StopButton.ShowWindow(SW_HIDE);
    m_CloseButton.EnableWindow(TRUE);
    m_StopFlag = false;
}

void CFileSearchDlg::OnBnClickedSearchButton()
{
    //Check input
    m_ResultsListBox.ResetContent();
    CString startDir;
    m_DirectoryEdit.GetWindowText(startDir);
    if (startDir.IsEmpty()) {
        MessageBox(_T("Пожалуйста, выберите начальную папку."), _T("Ошибка"), MB_ICONERROR);
        return;
    }
    std::string startDirU8 = W2U(startDir);
    namespace fs = boost::filesystem;
    if (!fs::is_directory(startDirU8) || !fs::exists(startDirU8)) {
        MessageBox(_T("Начальная папка не найдена."), _T("Ошибка"), MB_ICONERROR);
        return;
    }
    CString query;
    m_QueryEdit.GetWindowText(query);
    if (query.IsEmpty()) {
        MessageBox(_T("Пожалуйста, введите условие поиска."), _T("Ошибка"), MB_ICONERROR);
        return;
    }
    std::string queryU8 = W2U(query);
    using namespace FileSearch::AppCore;
    Parser parser;
    std::shared_ptr<Expression> expr;
    try {
        expr = parser.compile(queryU8);
    } catch (ParserException& ex) {
        std::string errorMessage = boost::str(boost::format("Failed to compile expression.\r\n\r\nError at pos %d: %s ") % ex.pos() % ex.what());
        MessageBox(U2W(errorMessage), _T("Ошибка"), MB_ICONERROR);
        return;
    }

    backgroundTaskStarted();
    std::future<int> futureFilesCount = std::async(std::launch::async, [this, startDirU8, expr]() -> int {
        SearchEngine se;
        int res = 0;
        try {
            res = se.searchFiles(startDirU8, expr, m_StopFlag, [&](const FoundFile& f) {
                runInGuiThread([this, &f]() {
                    m_ResultsListBox.AddString(U2W(f.path()));
                });
                
            });
        } catch (std::exception& ex) {
            runInGuiThread([this, &ex]() {
                MessageBox(U2W(ex.what()), _T("Ошибка"), MB_ICONERROR);
            });
        }
        backgroundTaskFinished();
        return res;
    });
}


void CFileSearchDlg::OnBnClickedChooseButton()
{
    CBrowseFolder dlg;
    CString path;
    m_DirectoryEdit.GetWindowText(path);
    if (dlg.Show(m_hWnd, path, path) == CBrowseFolder::retVal::OK) {
        m_DirectoryEdit.SetWindowText(path);
    }
}

void CFileSearchDlg::OnBnClickedStopButton() {
    m_StopFlag = true;
}

void CFileSearchDlg::OnBnClickedExampleQueriesButton() {
    RECT rc;
    m_ExampleQueriesButton.GetWindowRect(&rc);
    POINT menuOrigin = { rc.left, rc.bottom };

    CMenu popupMenu;
    popupMenu.CreatePopupMenu();
    int i = IDM_POPUPMENU_ITEM_FIRST;

    for (int j = 0; j < ARRAYSIZE(SampleQueries); j++) {
        popupMenu.AppendMenu(MF_STRING, i++, SampleQueries[j]);
    }

    TPMPARAMS excludeArea;
    ZeroMemory(&excludeArea, sizeof(excludeArea));
    excludeArea.cbSize = sizeof(excludeArea);
    excludeArea.rcExclude = rc;
    popupMenu.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_LEFTBUTTON, menuOrigin.x, menuOrigin.y, this, &excludeArea);
}

LRESULT CFileSearchDlg::OnTaskDispatcherMsg(WPARAM wParam, LPARAM ) {
    TaskDispatcherMessageStruct* msg = reinterpret_cast<TaskDispatcherMessageStruct*>(wParam);
    msg->callback();
    return 0;
}

void CFileSearchDlg::runInGuiThread(TaskDispatcherTask&& task) {
    TaskDispatcherMessageStruct msg;
    msg.callback = std::move(task);
    SendMessage(WM_TASKDISPATCHERMSG, reinterpret_cast<WPARAM>(&msg), 0);

}
}

}