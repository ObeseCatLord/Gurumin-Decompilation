// EN-only launcher: the Othello ("Osero") page (COseDlg). Falcom MFC dialog code (ClassWizard style) from the Steam
// build; see NOTES.md "Launcher (MFC)".
#include "launcher_dialog.h"

int g_oseBoard[OSE_BOARD_WORDS];   // the packed board (EN 0x01614d60)
int g_oseComputerTurnPending;               // computer owes a move (EN 0x01614d70)
int g_oseChosenCell;                        // cell chosen by the move search, -1 = none (EN 0x01614d74)
int g_oseChosenValue;                       // value of the chosen move (EN 0x01615298)

// Message box text when the computer finds no move ("masu") and when the move it found is illegal ("okashii").
static const char OSE_NO_MOVE_TEXT[] = "Ï·";
static const char OSE_ILLEGAL_MOVE_TEXT[] = "¨©µ¢";

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
    ON_WM_TIMER()
    ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

// Reset button: empties the board, cancels any pending computer move and repaints.
void COseDlg::OnReset()
{
    unsigned char cells[OSE_CELL_COUNT];

    g_oseComputerTurnPending = 0;
    memset(cells, 0, sizeof(cells));
    int cell = 0;
    for (int word = 0; word < OSE_BOARD_WORDS; word++) {
        g_oseBoard[word] = 0;
        for (int slot = 0; slot < 16; slot += 4) {
            g_oseBoard[word] |= cells[cell] << (slot * 2);
            g_oseBoard[word] |= cells[cell + 1] << (slot * 2 + 2);
            g_oseBoard[word] |= cells[cell + 2] << (slot * 2 + 4);
            g_oseBoard[word] |= cells[cell + 3] << (slot * 2 + 6);
            cell += 4;
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

// Game timer: when a move is owed, unpacks the board, lets the computer search a move and plays it, or tells the
// player that no move was found.
void COseDlg::OnTimer(UINT_PTR nIDEvent)
{
    unsigned char cells[OSE_CELL_COUNT];

    if (g_oseComputerTurnPending) {
        UpdateData(TRUE);
        g_oseComputerTurnPending = 0;
        g_oseChosenCell = -1;
        int cell = 0;
        for (int word = 0; word < OSE_BOARD_WORDS; word++) {
            for (int shift = 0; shift < 32; shift += 2) {
                cells[cell++] = (g_oseBoard[word] >> shift) & 3;
            }
        }
        Ose_SearchMove(cells, 0, m_playerColor + 1, m_playerColor + 1);
        if (g_oseChosenCell != -1) {
            if (!Ose_PlayMove(g_oseBoard, g_oseChosenCell, g_oseChosenValue, m_playerColor + 1)) {
                MessageBox(OSE_ILLEGAL_MOVE_TEXT, "", MB_OK);
            }
            Invalidate(TRUE);
        } else {
            MessageBox(OSE_NO_MOVE_TEXT, "", MB_OK);
        }
    }
    CDialog::OnTimer(nIDEvent);
}

// Click during the setup phase: puts a white piece on the clicked cell and repaints.
void COseDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
    if (m_started == 0) {
        unsigned char cells[OSE_CELL_COUNT];

        int cell = 0;
        for (int word = 0; word < OSE_BOARD_WORDS; word++) {
            for (int shift = 0; shift < 32; shift += 2) {
                cells[cell++] = (g_oseBoard[word] >> shift) & 3;
            }
        }
        int column = point.x / OSE_CELL_PIXELS;
        if (column >= 0 && column > OSE_BOARD_SIZE - 1) {
            column = OSE_BOARD_SIZE - 1;
        } else {
            column = column < 0 ? 0 : column;
        }
        int row = point.y / OSE_CELL_PIXELS;
        if (row >= 0 && row > OSE_BOARD_SIZE - 1) {
            row = OSE_BOARD_SIZE - 1;
        } else {
            row = row < 0 ? 0 : row;
        }
        cells[column * OSE_BOARD_SIZE + row] = OSE_CELL_WHITE;
        cell = 0;
        for (int word = 0; word < OSE_BOARD_WORDS; word++) {
            g_oseBoard[word] = 0;
            for (int slot = 0; slot < 16; slot += 4) {
                g_oseBoard[word] |= cells[cell] << (slot * 2);
                g_oseBoard[word] |= cells[cell + 1] << (slot * 2 + 2);
                g_oseBoard[word] |= cells[cell + 2] << (slot * 2 + 4);
                g_oseBoard[word] |= cells[cell + 3] << (slot * 2 + 6);
                cell += 4;
            }
        }
        Invalidate(TRUE);
    }
    CDialog::OnLButtonDown(nFlags, point);
}
