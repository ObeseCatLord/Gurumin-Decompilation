// EN-only launcher: the keyboard key assignment page (CKbDlg). Falcom MFC dialog code (ClassWizard style) from the
// Steam build; see NOTES.md "Launcher (MFC)".
#include "launcher_dialog.h"

IMPLEMENT_DYNAMIC(CKbDlg, CDialog)

// Builds the page; every assignment starts on row 0 (not assigned).
CKbDlg::CKbDlg(CWnd *pParent)
    : CDialog(IDD_KEYBOARD_DIALOG, pParent)
{
    m_dash = 0;
    m_cameraEnemy = 0;
    m_cameraLeft = 0;
    m_cameraRight = 0;
    m_cameraForward = 0;
    m_menu = 0;
    m_jump = 0;
    m_attack = 0;
    m_moveForward = 0;
    m_moveRight = 0;
    m_moveLeft = 0;
    m_moveBackward = 0;
    m_swapAttackMove = 0;
    m_wheelCamera = 0;
    m_dashDoubleClick = 0;
}

// Destroys the dialog base (nothing of its own to release). The linker drops this standalone copy; the scalar deleting
// destructor (EN 0x006eb770) carries the body.
CKbDlg::~CKbDlg()
{
}

// Exchanges the twelve key lists (row index) and the three mouse check boxes.
void CKbDlg::DoDataExchange(CDataExchange *pDX)
{
    DDX_CBIndex(pDX, IDC_KB_DASH, m_dash);
    DDX_CBIndex(pDX, IDC_KB_CAMERA_ENEMY, m_cameraEnemy);
    DDX_CBIndex(pDX, IDC_KB_CAMERA_LEFT, m_cameraLeft);
    DDX_CBIndex(pDX, IDC_KB_CAMERA_RIGHT, m_cameraRight);
    DDX_CBIndex(pDX, IDC_KB_CAMERA_FORWARD, m_cameraForward);
    DDX_CBIndex(pDX, IDC_KB_MENU, m_menu);
    DDX_CBIndex(pDX, IDC_KB_JUMP, m_jump);
    DDX_CBIndex(pDX, IDC_KB_ATTACK, m_attack);
    DDX_CBIndex(pDX, IDC_KB_MOVE_FORWARD, m_moveForward);
    DDX_CBIndex(pDX, IDC_KB_MOVE_RIGHT, m_moveRight);
    DDX_CBIndex(pDX, IDC_KB_MOVE_LEFT, m_moveLeft);
    DDX_CBIndex(pDX, IDC_KB_MOVE_BACKWARD, m_moveBackward);
    DDX_Check(pDX, IDC_KB_SWAP_ATTACK_MOVE, m_swapAttackMove);
    DDX_Check(pDX, IDC_KB_WHEEL_CAMERA, m_wheelCamera);
    DDX_Check(pDX, IDC_KB_DASH_DOUBLE_CLICK, m_dashDoubleClick);
}

BEGIN_MESSAGE_MAP(CKbDlg, CDialog)
    ON_CBN_SELCHANGE(IDC_KB_MOVE_FORWARD, &CKbDlg::OnSelchangeMoveForward)
    ON_CBN_SELCHANGE(IDC_KB_MOVE_LEFT, &CKbDlg::OnSelchangeMoveLeft)
    ON_CBN_SELCHANGE(IDC_KB_MOVE_RIGHT, &CKbDlg::OnSelchangeMoveRight)
    ON_CBN_SELCHANGE(IDC_KB_MOVE_BACKWARD, &CKbDlg::OnSelchangeMoveBackward)
    ON_CBN_SELCHANGE(IDC_KB_MENU, &CKbDlg::OnSelchangeMenu)
    ON_CBN_SELCHANGE(IDC_KB_DASH, &CKbDlg::OnSelchangeDash)
    ON_CBN_SELCHANGE(IDC_KB_JUMP, &CKbDlg::OnSelchangeJump)
    ON_CBN_SELCHANGE(IDC_KB_ATTACK, &CKbDlg::OnSelchangeAttack)
    ON_CBN_SELCHANGE(IDC_KB_CAMERA_FORWARD, &CKbDlg::OnSelchangeCameraForward)
    ON_CBN_SELCHANGE(IDC_KB_CAMERA_LEFT, &CKbDlg::OnSelchangeCameraLeft)
    ON_CBN_SELCHANGE(IDC_KB_CAMERA_RIGHT, &CKbDlg::OnSelchangeCameraRight)
    ON_CBN_SELCHANGE(IDC_KB_CAMERA_ENEMY, &CKbDlg::OnSelchangeCameraEnemy)
END_MESSAGE_MAP()

// Resets the other key list that already shows `row` to row 0 (not assigned) and refreshes the page's values, so no
// key is bound to two actions. Called by the twelve OnSelchange handlers below.
void CKbDlg::ClearDuplicateKey(int row, int sourceId)
{
    int keyLists[12] = {
        IDC_KB_DASH, IDC_KB_MOVE_LEFT, IDC_KB_MOVE_FORWARD, IDC_KB_ATTACK, IDC_KB_CAMERA_FORWARD,
        IDC_KB_MOVE_RIGHT, IDC_KB_MOVE_BACKWARD, IDC_KB_JUMP, IDC_KB_CAMERA_ENEMY, IDC_KB_CAMERA_LEFT,
        IDC_KB_MENU, IDC_KB_CAMERA_RIGHT
    };

    for (unsigned int i = 0; i < 12; i++) {
        if (keyLists[i] == sourceId) {
            continue;
        }
        CComboBox *list = (CComboBox *)GetDlgItem(keyLists[i]);
        if (list->GetCurSel() == row) {
            list->SetCurSel(0);
            UpdateData(TRUE);
            return;
        }
    }
}

// Warns when any key list is still on row 0. If the player accepts, the default input setup is loaded instead and the
// start goes ahead; if the player cancels, the start is refused. Called by the launcher's start button.
bool CKbDlg::IsStartRefused(CWnd *owner)
{
    int keyLists[12] = {
        IDC_KB_DASH, IDC_KB_MOVE_LEFT, IDC_KB_MOVE_FORWARD, IDC_KB_ATTACK, IDC_KB_CAMERA_FORWARD,
        IDC_KB_MOVE_RIGHT, IDC_KB_MOVE_BACKWARD, IDC_KB_JUMP, IDC_KB_CAMERA_ENEMY, IDC_KB_CAMERA_LEFT,
        IDC_KB_MENU, IDC_KB_CAMERA_RIGHT
    };
    bool refused = false;   // true once an unassigned list was found and the player declined the defaults

    for (unsigned int i = 0; i < 12; i++) {
        CComboBox *list = (CComboBox *)GetDlgItem(keyLists[i]);
        if (list->GetCurSel() == 0) {
            refused = true;
            if (owner->MessageBox("Input device key assignment is not complete.\nStart game with default input setup?",
                                  "Warning", MB_OKCANCEL | MB_ICONEXCLAMATION) == IDOK) {
                Launcher_LoadConfig(2);
                return false;
            }
            return refused;
        }
    }
    return refused;
}

// Each handler: the selection of one key list changed, so clear any other list that shows the same key, then restore
// the selection of the list the player touched (UpdateData in ClearDuplicateKey reloads every value).
void CKbDlg::OnSelchangeCameraEnemy()
{
    CComboBox *list = (CComboBox *)GetDlgItem(IDC_KB_CAMERA_ENEMY);
    int row = list->GetCurSel();
    ClearDuplicateKey(row, IDC_KB_CAMERA_ENEMY);
    list->SetCurSel(row);
}

void CKbDlg::OnSelchangeCameraLeft()
{
    CComboBox *list = (CComboBox *)GetDlgItem(IDC_KB_CAMERA_LEFT);
    int row = list->GetCurSel();
    ClearDuplicateKey(row, IDC_KB_CAMERA_LEFT);
    list->SetCurSel(row);
}

void CKbDlg::OnSelchangeMenu()
{
    CComboBox *list = (CComboBox *)GetDlgItem(IDC_KB_MENU);
    int row = list->GetCurSel();
    ClearDuplicateKey(row, IDC_KB_MENU);
    list->SetCurSel(row);
}

void CKbDlg::OnSelchangeCameraRight()
{
    CComboBox *list = (CComboBox *)GetDlgItem(IDC_KB_CAMERA_RIGHT);
    int row = list->GetCurSel();
    ClearDuplicateKey(row, IDC_KB_CAMERA_RIGHT);
    list->SetCurSel(row);
}

void CKbDlg::OnSelchangeDash()
{
    CComboBox *list = (CComboBox *)GetDlgItem(IDC_KB_DASH);
    int row = list->GetCurSel();
    ClearDuplicateKey(row, IDC_KB_DASH);
    list->SetCurSel(row);
}

void CKbDlg::OnSelchangeMoveLeft()
{
    CComboBox *list = (CComboBox *)GetDlgItem(IDC_KB_MOVE_LEFT);
    int row = list->GetCurSel();
    ClearDuplicateKey(row, IDC_KB_MOVE_LEFT);
    list->SetCurSel(row);
}

void CKbDlg::OnSelchangeMoveForward()
{
    CComboBox *list = (CComboBox *)GetDlgItem(IDC_KB_MOVE_FORWARD);
    int row = list->GetCurSel();
    ClearDuplicateKey(row, IDC_KB_MOVE_FORWARD);
    list->SetCurSel(row);
}

void CKbDlg::OnSelchangeAttack()
{
    CComboBox *list = (CComboBox *)GetDlgItem(IDC_KB_ATTACK);
    int row = list->GetCurSel();
    ClearDuplicateKey(row, IDC_KB_ATTACK);
    list->SetCurSel(row);
}

void CKbDlg::OnSelchangeCameraForward()
{
    CComboBox *list = (CComboBox *)GetDlgItem(IDC_KB_CAMERA_FORWARD);
    int row = list->GetCurSel();
    ClearDuplicateKey(row, IDC_KB_CAMERA_FORWARD);
    list->SetCurSel(row);
}

void CKbDlg::OnSelchangeMoveRight()
{
    CComboBox *list = (CComboBox *)GetDlgItem(IDC_KB_MOVE_RIGHT);
    int row = list->GetCurSel();
    ClearDuplicateKey(row, IDC_KB_MOVE_RIGHT);
    list->SetCurSel(row);
}

void CKbDlg::OnSelchangeMoveBackward()
{
    CComboBox *list = (CComboBox *)GetDlgItem(IDC_KB_MOVE_BACKWARD);
    int row = list->GetCurSel();
    ClearDuplicateKey(row, IDC_KB_MOVE_BACKWARD);
    list->SetCurSel(row);
}

void CKbDlg::OnSelchangeJump()
{
    CComboBox *list = (CComboBox *)GetDlgItem(IDC_KB_JUMP);
    int row = list->GetCurSel();
    ClearDuplicateKey(row, IDC_KB_JUMP);
    list->SetCurSel(row);
}

// Removes the last two rows (8 and 7) from every key list: the shared key table has two entries the launcher does
// not offer. Called by MFC when the page is created.
BOOL CKbDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    ((CComboBox *)GetDlgItem(IDC_KB_MOVE_FORWARD))->DeleteString(8);
    ((CComboBox *)GetDlgItem(IDC_KB_MOVE_LEFT))->DeleteString(8);
    ((CComboBox *)GetDlgItem(IDC_KB_MOVE_RIGHT))->DeleteString(8);
    ((CComboBox *)GetDlgItem(IDC_KB_MOVE_BACKWARD))->DeleteString(8);
    ((CComboBox *)GetDlgItem(IDC_KB_MENU))->DeleteString(8);
    ((CComboBox *)GetDlgItem(IDC_KB_DASH))->DeleteString(8);
    ((CComboBox *)GetDlgItem(IDC_KB_JUMP))->DeleteString(8);
    ((CComboBox *)GetDlgItem(IDC_KB_ATTACK))->DeleteString(8);
    ((CComboBox *)GetDlgItem(IDC_KB_CAMERA_FORWARD))->DeleteString(8);
    ((CComboBox *)GetDlgItem(IDC_KB_CAMERA_LEFT))->DeleteString(8);
    ((CComboBox *)GetDlgItem(IDC_KB_CAMERA_RIGHT))->DeleteString(8);
    ((CComboBox *)GetDlgItem(IDC_KB_CAMERA_ENEMY))->DeleteString(8);
    ((CComboBox *)GetDlgItem(IDC_KB_MOVE_FORWARD))->DeleteString(7);
    ((CComboBox *)GetDlgItem(IDC_KB_MOVE_LEFT))->DeleteString(7);
    ((CComboBox *)GetDlgItem(IDC_KB_MOVE_RIGHT))->DeleteString(7);
    ((CComboBox *)GetDlgItem(IDC_KB_MOVE_BACKWARD))->DeleteString(7);
    ((CComboBox *)GetDlgItem(IDC_KB_MENU))->DeleteString(7);
    ((CComboBox *)GetDlgItem(IDC_KB_DASH))->DeleteString(7);
    ((CComboBox *)GetDlgItem(IDC_KB_JUMP))->DeleteString(7);
    ((CComboBox *)GetDlgItem(IDC_KB_ATTACK))->DeleteString(7);
    ((CComboBox *)GetDlgItem(IDC_KB_CAMERA_FORWARD))->DeleteString(7);
    ((CComboBox *)GetDlgItem(IDC_KB_CAMERA_LEFT))->DeleteString(7);
    ((CComboBox *)GetDlgItem(IDC_KB_CAMERA_RIGHT))->DeleteString(7);
    ((CComboBox *)GetDlgItem(IDC_KB_CAMERA_ENEMY))->DeleteString(7);
    return TRUE;
}
