// EN-only launcher: the main dialog (CCongigDlg, sic), its tab control and the About box. Falcom MFC dialog code
// (ClassWizard style) from the Steam build; see NOTES.md "Launcher (MFC)".
#include "launcher_dialog.h"

// About box constructor (inlined into CCongigDlg::OnSysCommand): loads dialog template 100.
CAboutDlg::CAboutDlg()
    : CDialog(IDD)
{
}

// The About box exchanges nothing.
void CAboutDlg::DoDataExchange(CDataExchange *pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CCongigDlg, CDialog)

// Builds the main dialog and loads its icon (resource 128) from the module's resources.
CCongigDlg::CCongigDlg(CWnd *pParent)
    : CDialog(IDD_LAUNCHER_DIALOG, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(128);
}

BEGIN_MESSAGE_MAP(CCongigDlg, CDialog)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_WM_CTLCOLOR()
    ON_WM_CREATE()
    ON_BN_CLICKED(IDC_LAUNCHER_HELP, &CCongigDlg::OnHelp)
    ON_NOTIFY(TCN_SELCHANGE, IDC_LAUNCHER_TAB, &CCongigDlg::OnTcnSelchange)
    ON_BN_CLICKED(IDC_LAUNCHER_START, &CCongigDlg::OnStartGame)
    ON_WM_CLOSE()
    ON_BN_CLICKED(IDOK, &CCongigDlg::OnCloseButton)
    ON_BN_CLICKED(IDCANCEL, &CCongigDlg::OnCancelButton)
    ON_BN_CLICKED(IDC_LAUNCHER_RESET, &CCongigDlg::OnResetDefaults)
END_MESSAGE_MAP()

bool g_launcherStartGame;   // true when the launcher closes to start the game (EN 0x016126b0)
int g_launcherClosing;      // set to 1 by every handler that closes the launcher (EN 0x016126a8)
int g_launcherCancelled;    // set to 1 when the launcher is cancelled (EN 0x016126ac)

// The tab control is the only control the main dialog exchanges.
void CCongigDlg::DoDataExchange(CDataExchange *pDX)
{
    DDX_Control(pDX, IDC_LAUNCHER_TAB, m_tab);
}

// Swallows Enter, Esc and F1 key presses so they cannot close the launcher or open the help by accident.
BOOL CCongigDlg::PreTranslateMessage(MSG *pMsg)
{
    if (pMsg->message == WM_KEYDOWN) {
        if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_F1) {
            return TRUE;
        }
    }
    return CDialog::PreTranslateMessage(pMsg);
}

// Tab changed: shows the page of the newly selected tab.
void CCongigDlg::OnTcnSelchange(NMHDR *pNMHDR, LRESULT *pResult)
{
    ShowPage(m_tab.GetCurSel());
    *pResult = 0;
}

// Shows the page that belongs to tab `page` inside the tab control's display area, scaled from dialog units to
// pixels, and hides every other page.
void CCongigDlg::ShowPage(int page)
{
    CWnd *shown;

    g_padPage.ShowWindow(SW_HIDE);
    g_kbPage.ShowWindow(SW_HIDE);
    g_videoPage.ShowWindow(SW_HIDE);
    g_netPage.ShowWindow(SW_HIDE);
    g_titlePage.ShowWindow(SW_HIDE);

    CRect unit(100, 100, 100, 100);   // 100 dialog units in each direction, converted to pixels below
    MapDialogRect(&unit);
    float scaleX = (float)unit.left / 175.0f;
    float scaleY = (float)unit.top / 150.0f;
    RECT area = { -3, 0, 872, 419 };  // display area of the 175 x 150 dialog-unit design size
    m_tab.SendMessage(TCM_ADJUSTRECT, FALSE, (LPARAM)&area);
    area.right = (int)((float)area.right * scaleX);
    area.bottom = (int)((float)area.bottom * scaleY);

    if (page == 1) {
        g_videoPage.ShowWindow(SW_SHOW);
        shown = &g_videoPage;
    } else if (page == 2) {
        g_padPage.ShowWindow(SW_SHOW);
        shown = &g_padPage;
    } else if (page == 3) {
        g_kbPage.ShowWindow(SW_SHOW);
        shown = &g_kbPage;
    } else if (page == 0) {
        g_titlePage.ShowWindow(SW_SHOW);
        shown = &g_titlePage;
    } else {
        return;
    }
    shown->MoveWindow(area.left, area.top, area.right - area.left, area.bottom - area.top, TRUE);
}

// "Reset To Default": after confirmation reloads the default settings.
void CCongigDlg::OnResetDefaults()
{
    if (MessageBox("Are you sure you want to restore default settings?", "Confirm", MB_YESNO) == IDYES) {
        Launcher_LoadConfig(0);
    }
}

// "Start Game": refuses to leave while the pad or keyboard assignment is incomplete (switching to that tab), else
// closes the launcher with the start flag set.
void CCongigDlg::OnStartGame()
{
    if (g_padPage.IsStartRefused(this)) {
        m_tab.SetCurSel(2);
        ShowPage(2);
        return;
    }
    if (g_kbPage.IsStartRefused(this)) {
        m_tab.SetCurSel(3);
        ShowPage(3);
        return;
    }
    g_launcherStartGame = true;
    g_launcherClosing = 1;
    Launcher_SaveSettings();
    OnOK();
}

// "Close" button: same checks as Start Game, but closes without starting the game.
void CCongigDlg::OnCloseButton()
{
    if (g_padPage.IsStartRefused(this)) {
        m_tab.SetCurSel(2);
        ShowPage(2);
        return;
    }
    if (g_kbPage.IsStartRefused(this)) {
        m_tab.SetCurSel(3);
        ShowPage(3);
        return;
    }
    g_launcherStartGame = false;
    g_launcherClosing = 1;
    Launcher_SaveSettings();
    OnOK();
}

// "Cancel" button: closes without starting the game and flags the launcher as cancelled.
void CCongigDlg::OnCancelButton()
{
    g_launcherStartGame = false;
    g_launcherClosing = 1;
    g_launcherCancelled = 1;
    Launcher_SaveSettings();
    OnCancel();
}

// Window close box: closes without starting the game.
void CCongigDlg::OnClose()
{
    g_launcherStartGame = false;
    g_launcherClosing = 1;
    Launcher_SaveSettings();
    CDialog::OnClose();
}

// Help button: opens the PDF manual in the associated viewer.
void CCongigDlg::OnHelp()
{
    ShellExecute(NULL, "open", "manual.pdf", NULL, NULL, SW_SHOWNORMAL);
}

// Window creation: sets the window class name "Falcom" and lets the base class finish; -1 means failure.
int CCongigDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    lpCreateStruct->lpszClass = "Falcom";
    if (CDialog::OnCreate(lpCreateStruct) == -1) {
        return -1;
    }
    return 0;
}

// Paints the dialog; while minimised, draws the dialog icon centred in the client area instead.
void CCongigDlg::OnPaint()
{
    if (IsIconic()) {
        CPaintDC dc(this);
        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;
        dc.DrawIcon(x, y, m_hIcon);
    } else {
        CDialog::OnPaint();
    }
}

// Cursor shown while the minimised window is dragged: the dialog icon.
HCURSOR CCongigDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

// System-menu commands: the About entry opens the About box, everything else goes to the base class.
void CCongigDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX) {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    } else {
        CDialog::OnSysCommand(nID, lParam);
    }
}

// Makes static texts and check boxes draw with a transparent background over the dialog bitmap.
HBRUSH CCongigDlg::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
{
    HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
    pDC->SetBkMode(TRANSPARENT);
    return hbr;
}
