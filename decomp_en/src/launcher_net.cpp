// EN-only launcher: the network page (CNetDlg). Falcom MFC dialog code (ClassWizard style) from the Steam build;
// see NOTES.md "Launcher (MFC)".
#include "launcher_dialog.h"


// Builds the page with an empty lobby address and "no patch" cleared.
CNetDlg::CNetDlg(CWnd *pParent)
    : CDialog(IDD_NET_DIALOG, pParent)
{
    m_noPatch = 0;
    m_lobbyAddress = "";
}

// Exchanges the lobby address and the "no patch" switch.
void CNetDlg::DoDataExchange(CDataExchange *pDX)
{
    DDX_Text(pDX, IDC_NET_LOBBY_ADDRESS, m_lobbyAddress);
    DDX_Check(pDX, IDC_NET_NO_PATCH, m_noPatch);
}

BEGIN_MESSAGE_MAP(CNetDlg, CDialog)
    ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

HBRUSH g_launcherPageBrush = NULL;   // brush the network page returns for its control colours (EN 0x0090897c)

// Draws the page's static texts and check boxes with a transparent background; the brush handed back is the shared
// launcher page brush (the base class result is not used).
HBRUSH CNetDlg::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
{
    CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
    pDC->SetBkMode(TRANSPARENT);
    return g_launcherPageBrush;
}
