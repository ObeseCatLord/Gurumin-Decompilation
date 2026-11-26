// EN-only launcher: the Othello ("Osero") page (COseDlg). Falcom MFC dialog code (ClassWizard style) from the Steam
// build; see NOTES.md "Launcher (MFC)".
#include "launcher_dialog.h"

unsigned int g_oseBoard[OSE_BOARD_WORDS];   // the packed board (EN 0x01614d60)
int g_oseComputerTurnPending;               // computer owes a move (EN 0x01614d70)

// Builds the page: no colour chosen yet and the game not started.
COseDlg::COseDlg(CWnd *pParent)
    : CDialog(IDD_OSE_DIALOG, pParent)
{
    m_playerColor = -1;
    m_started = 0;
}

// Exchanges the colour radio group and the "started" check box.
void COseDlg::DoDataExchange(CDataExchange *pDX)
{
    DDX_Radio(pDX, IDC_OSE_PLAYER_BLACK, m_playerColor);
    DDX_Check(pDX, IDC_OSE_STARTED, m_started);
}

BEGIN_MESSAGE_MAP(COseDlg, CDialog)
    ON_BN_CLICKED(IDC_OSE_RESET, &COseDlg::OnReset)
    ON_BN_CLICKED(IDC_OSE_PASS, &COseDlg::OnPass)
END_MESSAGE_MAP()

// Reset button: empties the board, cancels any pending computer move and repaints.
void COseDlg::OnReset()
{
    unsigned char cells[OSE_CELL_COUNT];

    int cell = 0;

    g_oseComputerTurnPending = 0;
    memset(cells, 0, sizeof(cells));
    for (int word = 0; word < OSE_BOARD_WORDS; word++) {
        g_oseBoard[word] = 0;
        for (int shift = 0; shift < 32; shift += 2) {
            g_oseBoard[word] |= cells[cell++] << shift;
        }
    }
    Invalidate(TRUE);
}

// Pass button: lets the computer move.
void COseDlg::OnPass()
{
    g_oseComputerTurnPending = 1;
}

// Starts the page as black-to-move, shows the values and starts the 10 ms game timer.
BOOL COseDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    g_oseComputerTurnPending = 0;
    m_playerColor = 1;
    UpdateData(FALSE);
    SetTimer(10, 10, NULL);
    return TRUE;
}
