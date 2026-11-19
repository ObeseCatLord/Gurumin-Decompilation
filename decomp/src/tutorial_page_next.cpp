// Turning the tutorial book to the next pair of pages (run from the tutorial mode 0x10065450). Each of the 0x22
// pages has a texture name (tute_lp_iti...) and a script flag that unlocks it; locked pages in the second book
// (DAT_104c03e8) show the placeholder dpartsbf.bmp instead.

#include <string.h>

// "最後のページです。" ("This is the last page."), Shift-JIS.
#define LAST_PAGE_TEXT "\x8d\xc5\x8c\xe3\x82\xcc\x83\x79\x81\x5b\x83\x57\x82\xc5\x82\xb7\x81\x42"

struct Node
{
    int SetMaterialValue(const char *material, int value);  // 0x1019e8c0 (Node_SetMaterialValue)
    int RestartColourPlugs(const char *motion, int frames); // 0x1019b300
};

// The game object (game.h Game), here with the out-of-line copy of its flag test.
struct Game
{
    int TestScriptFlag(int index);                   // 0x100022b0
};

struct FrameTimer
{
    int ResetNext();                                 // 0x1013eee0 (FrameTimer_ResetNext)
};

extern "C" Game DAT_10874848;
extern "C" FrameTimer DAT_10836c70;
extern "C" int DAT_104c03e8;                         // non-zero in the second book
extern "C" int DAT_102194c0;                         // the left page shown
extern "C" int DAT_102194c4;                         // the right page shown
extern "C" int DAT_104c03e0;
extern "C" int DAT_104c03e4;                         // 1 while the pages turn
extern "C" int DAT_1020b444;                         // frames before the "last page" message may show again
extern "C" char DAT_102028e8[0x22][0x10];            // the page textures
extern "C" char DAT_10202a48[];                      // "dpartsbf.bmp": a locked page
extern "C" int DAT_1030b870[0x22];                   // texture slots of the pages
extern "C" int DAT_1030c908[0x22];                   // script flag of each page (0 = no page)
extern "C" int DAT_104483dc;                         // page of the left texture
extern "C" int DAT_104483e0;                         // page of the right texture
extern "C" int DAT_10c62edc[0x1000];                 // script flags
extern "C" Node *DAT_104c03cc;                       // the turning sheet
extern "C" Node *DAT_104c03d0;                       // the right half
extern "C" Node *DAT_104c03d4;                       // the left half
extern "C" int DAT_102239c8;                         // frames per animation step
extern "C" int DAT_104bfd4c;
extern "C" char DAT_104c01fc;
extern "C" int DAT_104bf8c8;
extern "C" int DAT_104bf8cc;
extern "C" int DAT_104bf8d0;
extern "C" char DAT_10467ea0;
extern "C" char DAT_10219bc0[];                      // the message shown

extern "C" int FUN_1001e1a0(const char *name, const char *alphaName);  // Texture_Load
extern "C" int FUN_1000ed70(const char *text, int style, int frames);  // shows a message
extern "C" int FUN_1003ad20(int id, const float *position, float volume);  // Sound_PlayEffect

// 0x1004f4f0 (TutorialBook_NextPage). On the last page (0x0e, or 0x1c in the second book) shows LAST_PAGE_TEXT
// (at most every 60 frames). Otherwise loads the cover and the shown pages again, then looks for the next two
// pages after the right one: the first becomes the left page, the second the right (a locked page in the second
// book shows dpartsbf.bmp). When one is found the sheet turns ("pageup", textures paper2/paper) over the halves,
// the left page is remembered in flag 0x85c (0x85d in the second book) and the page sound 0x3be plays. With no page
// left, the message box is set to LAST_PAGE_TEXT instead. Returns 0.
extern "C" int FUN_1004f4f0(void)
{
    int found = 0;
    DAT_104c03e0 = 0;
    if (DAT_104c03e8 != 0 ? DAT_102194c0 > 0x1c : DAT_102194c0 > 0xe)
    {

        DAT_104c03e4 = 0;
        if (DAT_1020b444 == 0)
        {
            DAT_1020b444 = 0x3c;
            FUN_1000ed70(LAST_PAGE_TEXT, 0, 0x28);
        }
        return 0;
    }
    DAT_104c03e4 = 1;
    int cover = FUN_1001e1a0(DAT_102028e8[0], 0);
    DAT_1030b870[0] = cover;
    int newLeft = cover;
    int left = FUN_1001e1a0(DAT_102028e8[DAT_104483dc], 0);
    DAT_1030b870[DAT_104483dc] = left;
    int right = FUN_1001e1a0(DAT_102028e8[DAT_104483e0], 0);
    DAT_1030b870[DAT_104483e0] = right;
    DAT_10836c70.ResetNext();
    for (int i = DAT_102194c4 + 1; i < 0x22; i++)
    {
        if (DAT_1030c908[i] == 0)
            continue;
        if (found == 0)
        {
            if (DAT_104c03e8 == 0 || DAT_1030c908[i] == -1 ||
                (DAT_1030c908[i] > 0 && DAT_10874848.TestScriptFlag(DAT_1030c908[i]) != 0))
            {
                newLeft = FUN_1001e1a0(DAT_102028e8[i], 0);
                DAT_1030b870[i] = newLeft;
                DAT_104483dc = i;
            }
            else
            {
                newLeft = FUN_1001e1a0(DAT_10202a48, 0);
                DAT_1030b870[0x16] = newLeft;
                DAT_104483dc = 0x16;
            }
            found = 1;
            DAT_102194c0 = i;
        }
        else
        {
            if (DAT_104c03e8 != 0)
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
                    DAT_1030b870[0x16] = right;
                    DAT_104483e0 = 0x16;
                }
            }
            else
            {
                right = FUN_1001e1a0(DAT_102028e8[i], 0);
                DAT_1030b870[i] = right;
                DAT_104483e0 = i;
            }
            DAT_102194c4 = i;
            found++;
            break;
        }
    }
    if (found == 0)
    {
        DAT_104c03e4 = 0;
        DAT_104c03e0 = 0;
        DAT_104bfd4c = 0x28;
        DAT_104c01fc = 0;
        DAT_104bf8c8 = 1;
        DAT_104bf8cc = 0;
        DAT_104bf8d0 = 0;
        DAT_10467ea0 = 0;
        strcpy(DAT_10219bc0, LAST_PAGE_TEXT);
        return 0;
    }
    if (found < 2)
        DAT_102194c4 = 0;
    if (DAT_104c03e8 != 0)
        DAT_10c62edc[0x85d] = DAT_102194c0;
    else
        DAT_10c62edc[0x85c] = DAT_102194c0;
    if (DAT_104c03cc != 0)
    {
        DAT_104c03cc->SetMaterialValue("paper2", newLeft);
        DAT_104c03cc->SetMaterialValue("paper", right);
        DAT_104c03cc->RestartColourPlugs("pageup", DAT_102239c8 * 2);
    }
    if (DAT_104c03d0 != 0)
    {
        DAT_104c03d0->SetMaterialValue("paper", cover);
        DAT_104c03d0->RestartColourPlugs("stop0", DAT_102239c8 * 2);
    }
    if (DAT_104c03d4 != 0)
    {
        DAT_104c03d4->SetMaterialValue("paper2", left);
        DAT_104c03d4->RestartColourPlugs("stop0", DAT_102239c8 * 2);
    }
    DAT_10836c70.ResetNext();
    FUN_1003ad20(0x3be, 0, 1.0f);
    return 0;
}
