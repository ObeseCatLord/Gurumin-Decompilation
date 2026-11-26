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
END_MESSAGE_MAP()

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
