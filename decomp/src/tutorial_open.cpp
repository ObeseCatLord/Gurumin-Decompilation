// Opening the tutorial book (called from the main menu and from the book's own mode).

#include "texture_load.h"
#include "vec4.h"
#include "matrix4.h"
#include "node.h"
#include "engine_memory.h"
#include "texture_manager.h"
#include "game.h"
#include "game_vars.h"
#include "render_vars.h"
#include "ui_vars.h"

extern "C" int FUN_100235e0(int mode);
extern "C" int FUN_1001e1a0(const char *name, const char *alphaName);  // Texture_Load
extern "C" int FUN_1003ad20(int id, const float *position, float volume);  // Sound_PlayEffect
extern "C" const char *FUN_1013f8a0(const char *name, int folder);

#define BOOK_PAGE_COUNT 0x22
#define LOCKED_PAGE_SLOT 0x16
#define BOOK_SHEET_FLAG 0x20
#define BOOK_OPEN_SOUND 0x3bd
#define BOOK_CAMERA_Y -300.0f
#define BOOK_CAMERA_Z 70.710678f

// 0x1004ed60 (TutorialBook_Open). Opens tutorial book `book` (0 = first, non-zero = second): loads the book model
// "tutebook.it3" once (node DAT_104c03c8, tag "tsm_tutebook"), finds its parts (@test the turning sheet, @lpage and
// @rpage the halves, @cover) and stops their colour plugs. Loads the cover texture and, from the page last shown
// (DAT_10c6504c / DAT_10c65050), the first two existing pages: the left and the right (a locked page in the second
// book shows dpartsbf.bmp). Sets the sheet, half and cover materials, the @center / @bcenter flags, the overlay camera,
// and plays the book sound 0x3bd. Returns 0.
extern "C" int FUN_1004ed60(int book)
{
    DAT_1020b444 = 0;
    DAT_104c03dc = 1;
    DAT_104c03ec = 0.0f;
    if (DAT_104c03c8 == 0)
    {
        DAT_104c03c8 = (Node *)DAT_11b0b938(0x18a0, "tsm_tutebook");
        DAT_104c03c8->Init();
        DAT_104c03c8->LoadIt3(FUN_1013f8a0("tutebook.it3", 0), 0, 0);
    }
    DAT_104c03c8->Rebuild();
    DAT_104c03c8->BuildMesh(1, 0);
    DAT_104c03c8->M_101944d0();
    DAT_104c03e8 = book;
    DAT_104c03e4 = 6;
    DAT_104c03e0 = 0;
    DAT_102194c0 = 0;
    DAT_102194c4 = 0;
    DAT_104483dc = 0;
    DAT_104483e0 = 0;
    Node *part = DAT_104c03c8->Find("@test", 0);
    if (part != 0)
    {
        DAT_104c03cc = part;
        part->RestartColourPlugs("stop0", DAT_102239c8 * 2);
    }
    part = DAT_104c03c8->Find("@lpage", 0);
    if (part != 0)
    {
        DAT_104c03d4 = part;
        part->RestartColourPlugs("stop2", DAT_102239c8 * 2);
    }
    part = DAT_104c03c8->Find("@rpage", 0);
    if (part != 0)
    {
        DAT_104c03d0 = part;
        part->RestartColourPlugs("stop0", DAT_102239c8 * 2);
    }
    part = DAT_104c03c8->Find("@cover", 0);
    if (part != 0)
    {
        DAT_104c03d8 = part;
        part->RestartColourPlugs("open", DAT_102239c8 * 2);
    }
    FUN_100235e0(book);
    int cover = LoadTexture(DAT_102028e8[0], 0);
    DAT_1030b870[0] = cover;
    int left = cover;
    int right = cover;
    int found = 0;
    DAT_102194c0 = book != 0 ? DAT_10c65050 : DAT_10c6504c;
    for (int i = DAT_102194c0; i < BOOK_PAGE_COUNT; i++)
    {
        if (DAT_1030c908[i] == 0)
            continue;
        if (found == 0)
        {
            if (book == 0 || DAT_1030c908[i] == -1 ||
                (DAT_1030c908[i] > 0 && DAT_10874848.TestScriptFlag(DAT_1030c908[i]) != 0))
            {
                left = FUN_1001e1a0(DAT_102028e8[i], 0);
                DAT_1030b870[i] = left;
                DAT_104483dc = i;
            }
            else
            {
                left = FUN_1001e1a0(DAT_10202a48, 0);
                DAT_1030b870[LOCKED_PAGE_SLOT] = left;
                DAT_104483dc = LOCKED_PAGE_SLOT;
            }
            found = 1;
            DAT_102194c0 = i;
        }
        else
        {
            if (book != 0)
            {
                if (DAT_1030c908[i] == -1)
                {
                    right = FUN_1001e1a0(DAT_102028e8[i], 0);
                    DAT_1030b870[i] = right;
                    DAT_104483e0 = i;
                }
                else if (DAT_1030c908[i] > 0 && DAT_10874848.TestScriptFlag(DAT_1030c908[i]) != 0)
                {
                    right = FUN_1001e1a0(DAT_102028e8[i], 0);
                    DAT_1030b870[i] = right;
                    DAT_104483e0 = i;
                }
                else
                {
                    right = FUN_1001e1a0(DAT_10202a48, 0);
                    DAT_1030b870[LOCKED_PAGE_SLOT] = right;
                    DAT_104483e0 = LOCKED_PAGE_SLOT;
                }
            }
            else
            {
                right = FUN_1001e1a0(DAT_102028e8[i], 0);
                DAT_1030b870[i] = right;
                DAT_104483e0 = i;
            }
            found++;
            DAT_102194c4 = i;
            break;
        }
    }
    if (found < 2)
        DAT_102194c4 = DAT_102194c0;
    if (book == 0)
        DAT_10c6504c = DAT_102194c0;
    else
        DAT_10c65050 = DAT_102194c0;
    Node *sheet = DAT_104c03cc;
    if (sheet != 0)
    {
        sheet->SetMaterialValue("haji", cover);
        sheet->SetMaterialValue("haji2", cover);
        sheet->SetMaterialValue("paper", cover);
        sheet->SetMaterialValue("paper2", left);
    }
    Node *rightHalf = DAT_104c03d0;
    if (rightHalf != 0)
    {
        rightHalf->SetMaterialValue("haji", cover);
        rightHalf->SetMaterialValue("paper", right);
    }
    Node *leftHalf = DAT_104c03d4;
    if (leftHalf != 0)
    {
        leftHalf->SetMaterialValue("haji2", cover);
        leftHalf->SetMaterialValue("paper2", cover);
    }
    Node *center = DAT_104c03c8->Find("@center", 0);
    if (center != 0)
    {
        if (book == 0)
            center->ClearFlags(BOOK_SHEET_FLAG);
        else
            center->SetFlags(BOOK_SHEET_FLAG);
    }
    Node *bottomCenter = DAT_104c03c8->Find("@bcenter", 0);
    if (book == 0)
        bottomCenter->SetFlags(BOOK_SHEET_FLAG);
    else
        bottomCenter->ClearFlags(BOOK_SHEET_FLAG);
    DAT_10c66edc.x = 0.0f;
    DAT_10c66edc.y = BOOK_CAMERA_Y;
    DAT_10c66edc.z = BOOK_CAMERA_Z;
    DAT_10edbdf8.x = 0.0f;
    DAT_10edbdf8.y = 0.0f;
    DAT_10edbdf8.z = BOOK_CAMERA_Z;
    DAT_104c03c8->matrix.SetIdentity();
    DAT_10836c70.ResetFrameTimer();
    DAT_104c03e0 = 0;
    FUN_1003ad20(BOOK_OPEN_SOUND, 0, 1.0f);
    return 0;
}
