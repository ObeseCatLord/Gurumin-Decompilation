// EN-only launcher: the title page (CTitDlg). Falcom MFC dialog code (ClassWizard style) from the Steam build;
// see NOTES.md "Launcher (MFC)".
#include "launcher_dialog.h"

IMPLEMENT_DYNAMIC(CTitDlg, CDialog)

// Builds the page; the three controls are subclassed later by DoDataExchange.
CTitDlg::CTitDlg(CWnd *pParent)
    : CDialog(IDD_TITLE_DIALOG, pParent)
{
}

// Destroys the two progress bars, the button and the dialog base.
CTitDlg::~CTitDlg()
{
}

// Subclasses the update button and the two progress bars.
void CTitDlg::DoDataExchange(CDataExchange *pDX)
{
    DDX_Control(pDX, IDC_TITLE_ONLINE_UPDATE, m_onlineUpdateCtrl);
    DDX_Control(pDX, IDC_TITLE_PROGRESS_A, m_progressACtrl);
    DDX_Control(pDX, IDC_TITLE_PROGRESS_B, m_progressBCtrl);
}

BEGIN_MESSAGE_MAP(CTitDlg, CDialog)
    ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

// Draws the page's static texts with a transparent background.
HBRUSH CTitDlg::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
{
    HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
    pDC->SetBkMode(TRANSPARENT);
    return hbr;
}
