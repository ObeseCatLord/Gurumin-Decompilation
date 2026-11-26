// EN-only launcher: the gamepad assignment page (CPadDlg). Falcom MFC dialog code (ClassWizard style) from the Steam
// build; see NOTES.md "Launcher (MFC)".
#include "launcher_dialog.h"
#include "small_b11.h"   // Input_GetPadIndex

// Builds the page. The wizard-style constructor zeroes the exchanged check, slider and text values in the member
// initialiser list (in declaration order, between the control objects); the button lists start on -1 (none).
CPadDlg::CPadDlg(CWnd *pParent)
    : CDialog(IDD_PAD_DIALOG, pParent)
    , m_check408(0)
    , m_check40a(0)
    , m_check41b(0)
    , m_check41c(0)
    , m_check41d(0)
    , m_text421("")
    , m_check41e(0)
    , m_pos423(0)
    , m_check424(0)
    , m_check425(0)
    , m_check426(0)
    , m_check427(0)
    , m_check428(0)
    , m_check3eb(0)
    , m_pos429(0)
    , m_check42a(0)
{
    m_list3fe = -1;
    m_list407 = -1;
    m_list3ff = -1;
    m_list400 = -1;
    m_list401 = -1;
    m_list402 = -1;
    m_list403 = -1;
    m_list404 = -1;
    m_list405 = -1;
    m_list406 = -1;
    m_00d0 = -1;
    m_00e8 = -1;
    m_00ec = -1;
    m_00f0 = -1;
    m_00f4 = -1;
    m_00f8 = -1;
    m_00fc = -1;
    m_0100 = -1;
    m_0104 = -1;
    m_0108 = -1;
    m_list3ee = -1;
}

// Exchanges every value with its control and subclasses the controls the page drives directly.
void CPadDlg::DoDataExchange(CDataExchange *pDX)
{
    DDX_CBIndex(pDX, IDC_PAD_3FE, m_list3fe);
    DDX_CBIndex(pDX, IDC_PAD_407, m_list407);
    DDX_CBIndex(pDX, IDC_PAD_40F, m_list40f);
    DDX_CBIndex(pDX, IDC_PAD_410, m_list410);
    DDX_CBIndex(pDX, IDC_PAD_412, m_list412);
    DDX_CBIndex(pDX, IDC_PAD_415, m_list415);
    DDX_CBIndex(pDX, IDC_PAD_418, m_list418);
    DDX_CBIndex(pDX, IDC_PAD_3FF, m_list3ff);
    DDX_CBIndex(pDX, IDC_PAD_400, m_list400);
    DDX_CBIndex(pDX, IDC_PAD_401, m_list401);
    DDX_CBIndex(pDX, IDC_PAD_402, m_list402);
    DDX_CBIndex(pDX, IDC_PAD_403, m_list403);
    DDX_CBIndex(pDX, IDC_PAD_404, m_list404);
    DDX_CBIndex(pDX, IDC_PAD_405, m_list405);
    DDX_CBIndex(pDX, IDC_PAD_406, m_list406);
    DDX_CBIndex(pDX, IDC_PAD_3EE, m_list3ee);
    DDX_Control(pDX, IDC_PAD_41A, m_btn41a);
    DDX_Control(pDX, IDC_PAD_417, m_btn417);
    DDX_Control(pDX, IDC_PAD_414, m_btn414);
    DDX_Control(pDX, IDC_PAD_411, m_btn411);
    DDX_Control(pDX, IDC_PAD_3FB, m_btn3fb);
    DDX_Control(pDX, IDC_PAD_3F9, m_btn3f9);
    DDX_Control(pDX, IDC_PAD_3ED, m_btn3ed);
    DDX_Control(pDX, IDC_PAD_3FD, m_btn3fd);
    DDX_Control(pDX, IDC_PAD_40B, m_btn40b);
    DDX_Control(pDX, IDC_PAD_40C, m_btn40c);
    DDX_Control(pDX, IDC_PAD_3EC, m_btn3ec);
    DDX_Control(pDX, IDC_PAD_409, m_btn409);
    DDX_Control(pDX, IDC_PAD_408, m_btn408);
    DDX_Control(pDX, IDC_PAD_40A, m_btn40a);
    DDX_Control(pDX, IDC_PAD_40D, m_btn40d);
    DDX_Check(pDX, IDC_PAD_41A, m_check41a);
    DDX_Check(pDX, IDC_PAD_417, m_check417);
    DDX_Check(pDX, IDC_PAD_414, m_check414);
    DDX_Check(pDX, IDC_PAD_411, m_check411);
    DDX_Check(pDX, IDC_PAD_3FB, m_check3fb);
    DDX_Check(pDX, IDC_PAD_3F9, m_check3f9);
    DDX_Check(pDX, IDC_PAD_3ED, m_check3ed);
    DDX_Check(pDX, IDC_PAD_3FD, m_check3fd);
    DDX_Check(pDX, IDC_PAD_40B, m_check40b);
    DDX_Check(pDX, IDC_PAD_40C, m_check40c);
    DDX_Check(pDX, IDC_PAD_3EC, m_check3ec);
    DDX_Check(pDX, IDC_PAD_409, m_check409);
    DDX_Check(pDX, IDC_PAD_408, m_check408);
    DDX_Check(pDX, IDC_PAD_40A, m_check40a);
    DDX_Check(pDX, IDC_PAD_40D, m_check40d);
    DDX_Check(pDX, IDC_PAD_41B, m_check41b);
    DDX_Check(pDX, IDC_PAD_41C, m_check41c);
    DDX_Check(pDX, IDC_PAD_41D, m_check41d);
    DDX_Text(pDX, IDC_PAD_421, m_text421);
    DDX_Control(pDX, IDC_PAD_3EE, m_combo3ee);
    DDX_Check(pDX, IDC_PAD_41E, m_check41e);
    DDX_Slider(pDX, IDC_PAD_423, m_pos423);
    DDX_Control(pDX, IDC_PAD_423, m_slider423);
    DDX_Check(pDX, IDC_PAD_424, m_check424);
    DDX_Check(pDX, IDC_PAD_425, m_check425);
    DDX_Control(pDX, IDC_PAD_424, m_btn424);
    DDX_Check(pDX, IDC_PAD_426, m_check426);
    DDX_Control(pDX, IDC_PAD_426, m_btn426);
    DDX_Check(pDX, IDC_PAD_427, m_check427);
    DDX_Control(pDX, IDC_PAD_427, m_btn427);
    DDX_Check(pDX, IDC_PAD_428, m_check428);
    DDX_Control(pDX, IDC_PAD_428, m_btn428);
    DDX_Check(pDX, IDC_PAD_3EB, m_check3eb);
    DDX_Control(pDX, IDC_PAD_429, m_slider429);
    DDX_Slider(pDX, IDC_PAD_429, m_pos429);
    DDX_Control(pDX, IDC_PAD_42A, m_btn42a);
    DDX_Check(pDX, IDC_PAD_42A, m_check42a);
}

BEGIN_MESSAGE_MAP(CPadDlg, CDialog)
    ON_WM_CTLCOLOR()
    ON_WM_PAINT()
    ON_WM_DESTROY()
    ON_WM_TIMER()
END_MESSAGE_MAP()

// Draws the page's controls with a transparent background.
HBRUSH CPadDlg::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
{
    HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
    pDC->SetBkMode(TRANSPARENT);
    return hbr;
}

// Paint: only creates (and releases) the paint context.
void CPadDlg::OnPaint()
{
    CPaintDC dc(this);
}

// Window destruction: stops the 50 ms pad polling timer (id 0x32) after the base class cleans up.
void CPadDlg::OnDestroy()
{
    CDialog::OnDestroy();
    KillTimer(0x32);
}

// Warns when any pad list is still on row 0. If the player accepts, the default pad setup is loaded instead and the
// start goes ahead; if the player cancels, the start is refused. Called by the launcher's start button.
bool CPadDlg::IsStartRefused(CWnd *owner)
{
    int padLists[12] = {
        IDC_PAD_LIST_DASH, IDC_PAD_LIST_MOVE_LEFT, IDC_PAD_LIST_MOVE_FORWARD, IDC_PAD_LIST_ATTACK,
        IDC_PAD_LIST_CAMERA_FORWARD, IDC_PAD_LIST_MOVE_RIGHT, IDC_PAD_LIST_MOVE_BACKWARD, IDC_PAD_LIST_JUMP,
        IDC_PAD_LIST_CAMERA_ENEMY, IDC_PAD_LIST_CAMERA_LEFT, IDC_PAD_LIST_MENU, IDC_PAD_LIST_CAMERA_RIGHT
    };
    bool refused = false;   // true once an unassigned list was found and the player declined the defaults

    for (unsigned int i = 0; i < 12; i++) {
        CComboBox *list = (CComboBox *)GetDlgItem(padLists[i]);
        if (list->GetCurSel() == 0) {
            refused = true;
            if (owner->MessageBox("Input device key assignment is not complete.\nStart game with default input setup?",
                                  "Warning", MB_OKCANCEL | MB_ICONEXCLAMATION) == IDOK) {
                Launcher_LoadConfig(1);
                return false;
            }
            return refused;
        }
    }
    return refused;
}

// Swallows Enter, Esc and F1 key presses so they cannot close the launcher or open the help by accident.
BOOL CPadDlg::PreTranslateMessage(MSG *pMsg)
{
    if (pMsg->message == WM_KEYDOWN) {
        if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_F1) {
            return TRUE;
        }
    }
    return CDialog::PreTranslateMessage(pMsg);
}

int g_padWaitingAction;      // 1-based action slot waiting for a button press (EN 0x016152a0)
HWND g_padOwnerWindow;       // top-level window handed to the pad scan (EN 0x016152a4)
int g_padCaptureStep;        // countdown that steps by 2 on every timer tick (EN 0x016152b4)
unsigned char g_padFlash;    // repaint ticks still owed after a capture (EN 0x016152b8)

const int PAD_SLOT_COUNT = 15;           // action slots on the page (twelve visible lists and three hidden ones)
const int PAD_CAPTURE_STEP_START = 16;   // value the capture countdown restarts at
const int PAD_CAPTURE_STEP = 2;          // amount the capture countdown drops per timer tick
const unsigned char PAD_FLASH_TICKS = 2; // repaint ticks owed after a capture

// Selects, in each of the fifteen lists, the row that shows the stored button number, then runs UpdateData.
// Stored numbers 1-2 sit two rows lower, 3-4 two rows higher, 5-8 sixteen rows lower and 21-24 sixteen rows higher
// (the list orders the stick directions differently from the scan codes).
void CPadDlg::ShowAssignments(BOOL saveAndValidate)
{
    int listIds[PAD_SLOT_COUNT] = {
        0x3ef, 0x3f0, 0x3f1, 0x3f2, 0x3f3, 0x3f4, 0x3f5, 0x3f6, 0x3f7, 0x3f8, 0x3fa, 0x3fc, 0x413, 0x416, 0x419
    };
    int stored[PAD_SLOT_COUNT] = {
        m_00d0, m_00ec, m_00f0, m_00f4, m_00f8, m_00fc, m_0100, m_0104,
        m_0108, m_00e8, m_00e4, m_00e0, m_00dc, m_00d8, m_00d4
    };

    for (int i = 0; i < PAD_SLOT_COUNT; i++) {
        int row = stored[i];
        if (row != 0) {
            if (row <= 2) {
                row += 2;
            } else if (row <= 4) {
                row -= 2;
            } else if (row <= 8) {
                row += 0x10;
            } else if (row > 0x14) {
                if (row <= 0x18) {
                    row -= 0x10;
                }
            }
        }
        ((CComboBox *)GetDlgItem(listIds[i]))->SetCurSel(row);
    }
    UpdateData(saveAndValidate);
}

// Stores `button` for action slot `action` (1-based). If another slot already holds the same button, that slot is
// cleared and the lists are refreshed; otherwise they are refreshed only when `refresh` is set.
void CPadDlg::AssignPadButton(int action, int button, bool refresh)
{
    int *slots[PAD_SLOT_COUNT] = {
        &m_00d0, &m_00ec, &m_00f0, &m_00f4, &m_00f8, &m_00fc, &m_0100, &m_0104,
        &m_0108, &m_00e8, &m_00e4, &m_00e0, &m_00dc, &m_00d8, &m_00d4
    };
    int chosen = action - 1;

    *slots[chosen] = button;
    for (unsigned int i = 0; i < PAD_SLOT_COUNT; i++) {
        if (i != chosen && *slots[i] == button) {
            *slots[i] = 0;
            ShowAssignments(TRUE);
            return;
        }
    }
    if (refresh) {
        ShowAssignments(TRUE);
    }
}

// 50 ms timer: while an action waits for a button, scans the pad and assigns the pressed button; keeps the page
// repainting for a couple of ticks after a capture and steps the capture countdown.
void CPadDlg::OnTimer(UINT_PTR nIDEvent)
{
    if (m_hWnd) {
        if (g_padWaitingAction != 0) {
            int button = Pad_FindPressedButton(g_padOwnerWindow, m_check425 != 0);
            if (button != 0) {
                AssignPadButton(g_padWaitingAction, button, true);
                ResetSetButtons();
                g_padWaitingAction = 0;
                m_0110 = Input_GetPadIndex();
            }
        }
        if (g_padWaitingAction != 0) {
            {
                CPaintDC dc(this);
            }
            g_padFlash = PAD_FLASH_TICKS;
        } else if (g_padFlash != 0) {
            {
                CPaintDC dc(this);
            }
            if (g_padFlash != 0) {
                g_padFlash--;
            }
        }
        int step = g_padCaptureStep;
        if (step <= 0) {
            step = PAD_CAPTURE_STEP_START;
        }
        g_padCaptureStep = step - PAD_CAPTURE_STEP;
        CDialog::OnTimer(nIDEvent);
    }
}
