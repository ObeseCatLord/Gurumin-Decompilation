// EN-only launcher: the tab control of the main dialog (CTabCtrl2). Falcom MFC code (ClassWizard style) from the
// Steam build, in a source file of its own (CCongigDlg only calls its constructor and destructor); see NOTES.md
// "Launcher (MFC)".
#include "launcher_dialog.h"

IMPLEMENT_DYNAMIC(CTabCtrl2, CTabCtrl)

BEGIN_MESSAGE_MAP(CTabCtrl2, CTabCtrl)
    ON_WM_CTLCOLOR()
    ON_WM_ERASEBKGND()
    ON_WM_NCPAINT()
    ON_WM_DRAWITEM()
END_MESSAGE_MAP()

// Draws the tab control's controls with a transparent background.
HBRUSH CTabCtrl2::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
{
    HBRUSH hbr = CTabCtrl::OnCtlColor(pDC, pWnd, nCtlColor);
    pDC->SetBkMode(TRANSPARENT);
    return hbr;
}

// Background erase: the default processing.
BOOL CTabCtrl2::OnEraseBkgnd(CDC *pDC)
{
    return CTabCtrl::OnEraseBkgnd(pDC);
}

// The non-client area is left undrawn.
void CTabCtrl2::OnNcPaint()
{
}

// Owner-draw items: the default processing.
void CTabCtrl2::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
    CTabCtrl::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

// Tab control constructor: nothing beyond CTabCtrl.
CTabCtrl2::CTabCtrl2()
{
}

// Tab control destructor: nothing beyond CTabCtrl.
CTabCtrl2::~CTabCtrl2()
{
}
