// Loading of the model of a character in one of its variants; for the player this also applies the costume.

#include <stdio.h>

#include "anim_model.h"
#include "char_slot.h"
#include "char_vars.h"
#include "node.h"
#include "render_vars.h"
#include "ui_vars.h"

int SeasonalEvent();                                                      // 0x1001d850
extern "C" int FUN_1000e750(void);                                        // anti-tamper guard refresh
extern "C" int FUN_1013e6f0(int id, const char *text, const char *name);  // sets the word that replaces "text" in messages
extern "C" const char *FUN_1013f8a0(const char *format, ...);             // path of a game file
extern "C" int FUN_10012220(int index, int value);                        // Model_ApplyCommands
extern "C" int FUN_100154c0(int value);                                   // Char_WearHelmet

// Costume numbers (DAT_10c655f0). The others (1-5, 9, 10) are plain numbers in the file names.
#define COSTUME_DEFAULT 0
#define COSTUME_POPON 6
#define COSTUME_SEASONAL 7
#define COSTUME_DORASURE 8
#define COSTUME_ALT_A 11
#define COSTUME_ALT_B 12

// Result of SeasonalEvent during which costume 7 may be worn.
#define SEASON_NEW_YEAR 2

// Unlock bits (DAT_104c0230) that must all be set for costume 6.
#define POPON_UNLOCK_BITS 0x17

// Player model variant (AnimModel::m_0077): <base>.it3, <base>n.it3 (1), <base>x<N>.it3 (2 and up).
#define VARIANT_NIGHT 1
#define VARIANT_EXTRA_FIRST 2

// Item numbers (CharSlot::helmet) whose face variant a costume shows.
#define HELMET_COSTUME1 0x38
#define HELMET_COSTUME5 0x28
#define HELMET_COSTUME2 0x39
#define HELMET_COSTUME4 0x25
#define HELMET_COSTUME3 0x3d

#define MODEL_PATH_BYTES 0x100
// Where the models of the other characters are put while they load.
#define FAR_AWAY_X -200000.0f
#define MOTION_IDLE 3
#define MOTION_IDLE_BLEND 5
#define NAME_WORD_PLAYER "\x83p\x83\x8a\x83\x93"
#define NAME_WORD_POPON "\x83|\x83|\x83\x93"
#define NAME_WORD_DRILL "\x83h\x83\x8a\x83\x8b"
#define NAME_WORD_DORASURE "\x83h\x83\x89\x83X\x83\x8c"

// 0x1003b6e0 (Costume_LoadPlayerModel). Loads the model file of character `index` as variant `variant`, unless that
// model is already loaded. For the player the file comes from the costume DAT_10c655f0 (<base>a.it3 while message
// words are replaced, <base>a1/a2.it3 for costumes 11 and 12, else <base><costume>.it3); a costume whose unlock bit
// in DAT_104c0230 is missing (or costume 7 outside the New Year event) is reset to 0 first. Then the replacement
// words for the player's names are set, the face variant follows from the costume and the helmet, and the model is
// loaded and put back where it stood (the player) or far away (the others). Costumes 6, 8, 11 and 12 take the
// helmet off.
extern "C" void FUN_1003b6e0(int index, int variant)
{
    if (DAT_10db6ee8[index].root == 0 || DAT_10db6ee8[index].m_0077 != variant)
    {
        Vec4 position;
        float heading;
        if (index == 0)
        {
            position.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
            heading = DAT_10db6ee8[0].heading;
        }
        DAT_11b044e4 = 0;
        char path[MODEL_PATH_BYTES];
        if (index == 0)
        {
            int costume = DAT_10c655f0;
            if (costume == COSTUME_DEFAULT && DAT_10c66cc0 != 0)
            {
                sprintf(path, "%sa.it3", DAT_10daaae8[0].m_0165, DAT_10c655f0);
            }
            else
            {
                unsigned char unlocked = DAT_104c0230;
                if ((unlocked & 1) == 0 && costume == 1)
                {
                    costume = 0;
                    DAT_10c655f0 = 0;
                    FUN_1000e750();
                }
                if ((unlocked & 2) == 0 && costume == 2)
                {
                    costume = 0;
                    DAT_10c655f0 = 0;
                    FUN_1000e750();
                }
                if ((unlocked & 4) == 0 && costume == 3)
                {
                    costume = 0;
                    DAT_10c655f0 = 0;
                    FUN_1000e750();
                }
                if ((unlocked & 8) == 0 && costume == 4)
                {
                    costume = 0;
                    DAT_10c655f0 = 0;
                    FUN_1000e750();
                }
                if ((unlocked & 0x10) == 0 && costume == 5)
                {
                    costume = 0;
                    DAT_10c655f0 = 0;
                    FUN_1000e750();
                }
                if ((unlocked & POPON_UNLOCK_BITS) != POPON_UNLOCK_BITS && costume == COSTUME_POPON)
                {
                    DAT_10c66cb0 = 0;
                    DAT_10c655f0 = 0;
                    FUN_1000e750();
                }
                int seasonal = SeasonalEvent();
                costume = DAT_10c655f0;
                if (seasonal != SEASON_NEW_YEAR && costume == COSTUME_SEASONAL)
                {
                    costume = 0;
                    DAT_10c655f0 = 0;
                    FUN_1000e750();
                }
                unsigned int unlockedNow = DAT_104c0230;
                if ((unlockedNow & 0x40) == 0 && costume == COSTUME_DORASURE)
                {
                    costume = 0;
                    DAT_10c655f0 = 0;
                    FUN_1000e750();
                }
                if ((unlockedNow & 0x80) == 0 && costume == 9)
                {
                    costume = 0;
                    DAT_10c655f0 = 0;
                    FUN_1000e750();
                }
                if ((unlockedNow & 0x20) == 0 && costume == 10)
                {
                    costume = 0;
                    DAT_10c655f0 = 0;
                    FUN_1000e750();
                }
                if ((unlockedNow & 0x100) == 0 && costume == COSTUME_ALT_A)
                {
                    costume = 0;
                    DAT_10c655f0 = 0;
                    FUN_1000e750();
                }
                if ((unlockedNow & 0x200) == 0 && costume == COSTUME_ALT_B)
                {
                    costume = 0;
                    DAT_10c655f0 = 0;
                    FUN_1000e750();
                }
                sprintf(path, costume == COSTUME_ALT_A ? "%sa1.it3" : costume == COSTUME_ALT_B ? "%sa2.it3" : "%s%d.it3",
                        DAT_10daaae8[0].m_0165, costume);
            }
            const char *secondName;
            if (DAT_10c655f0 == COSTUME_POPON)
            {
                FUN_1013e6f0(0, NAME_WORD_PLAYER, NAME_WORD_POPON);
                secondName = NAME_WORD_DORASURE;
            }
            else
            {
                FUN_1013e6f0(0, NAME_WORD_PLAYER, DAT_10daaae8[0].name);
                secondName = NAME_WORD_DRILL;
            }
            FUN_1013e6f0(1, NAME_WORD_DRILL, secondName);
            int worn = DAT_10c655f0;
            int helmet = DAT_10daaae8[0].helmet;
            DAT_104c033a = 0;
            if (worn == 1)
            {
                if (helmet == HELMET_COSTUME1)
                    DAT_104c033a = worn;
            }
            else if (worn == 5)
            {
                if (helmet == HELMET_COSTUME5)
                    DAT_104c033a = 2;
            }
            else if (worn == 2)
            {
                if (helmet == HELMET_COSTUME2)
                    DAT_104c033a = 3;
            }
            else if (worn == 4)
            {
                if (helmet == HELMET_COSTUME4)
                    DAT_104c033a = worn;
            }
            else if (worn == 3)
            {
                if (helmet == HELMET_COSTUME3)
                    DAT_104c033a = 5;
            }
            else if (worn == COSTUME_POPON)
            {
                DAT_104c033a = worn;
            }
        }
        else if (variant >= VARIANT_EXTRA_FIRST)
        {
            sprintf(path, "%sx%d.it3", DAT_10daaae8[index].m_0165, variant - VARIANT_EXTRA_FIRST);
        }
        else if (variant == VARIANT_NIGHT)
        {
            sprintf(path, "%sn.it3", DAT_10daaae8[index].m_0165);
        }
        else
        {
            sprintf(path, "%s.it3", DAT_10daaae8[index].m_0165);
        }
        DAT_10db6ee8[index].m_0077 = variant;
        AnimModel *model = &DAT_10db6ee8[index];
        model->LoadFile(FUN_1013f8a0(path, 0));
        DAT_10db6ee8[index].root->ReleaseUniqueMeshData();
        if (index == 0)
        {
            DAT_10db6ee8[0].m_0240 = 1;
            DAT_10db6ee8[0].m_2390 = 1;
        }
        else
        {
            DAT_10db6ee8[index].m_2390 = 0;
        }
        DAT_11b044e4 = 0;
        DAT_10db6ee8[index].speedScale = DAT_10daaae8[index].speedScale;
        if (index != 0)
        {
            model->Teleport(FAR_AWAY_X, 0, 0);
        }
        else
        {
            DAT_10db6ee8[0].Teleport(position.x, position.y, position.z);
            DAT_10db6ee8[0].heading = heading;
            DAT_10db6ee8[0].targetHeading = heading;
        }
        model->LoadModel(FUN_1013f8a0(DAT_10b314c8[DAT_10daaae8[index].params[PARAM_MODEL]].path, 0));
        FUN_10012220(index, -1);
        if (index != 0)
            model->PlayMotion(MOTION_IDLE, 0, MOTION_IDLE_BLEND, 1.0f);
        if (DAT_10c655f0 == COSTUME_POPON || DAT_10c655f0 == COSTUME_DORASURE || DAT_10c655f0 == COSTUME_ALT_A ||
            DAT_10c655f0 == COSTUME_ALT_B)
        {
            DAT_10daaae8[0].helmet = 0;
            FUN_100154c0(0);
        }
    }
}
