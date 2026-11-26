// EN-only launcher: the "installing, please wait" notice (CInstDlg). Falcom MFC dialog code (ClassWizard style) from
// the Steam build; see NOTES.md "Launcher (MFC)".
#include "launcher_dialog.h"

IMPLEMENT_DYNAMIC(CInstDlg, CDialog)

// Builds the notice dialog. Created by the launcher while it installs an update.
CInstDlg::CInstDlg(CWnd *pParent)
    : CDialog(IDD_INSTALL_DIALOG, pParent)
{
}

// Destroys the dialog base (nothing of its own to release). The linker drops this standalone copy; the scalar deleting
// destructor (EN 0x006eb630) carries the body.
CInstDlg::~CInstDlg()
{
}

// Nothing is exchanged: the dialog only shows a static text.
void CInstDlg::DoDataExchange(CDataExchange *pDX)
{
}

BEGIN_MESSAGE_MAP(CInstDlg, CDialog)
END_MESSAGE_MAP()
