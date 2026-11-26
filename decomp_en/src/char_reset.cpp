// Resetting a character's state (EN build). Ported from the JP source decomp/src/char_reset.cpp (APP.dll 0x10043cc0,
// unmatched) and the EN bytes.
#include "char.h"
#include "char_shots.h"
#include "game_vars.h"

// Behaviours (g_charState behaviour) of the packed-parameter kind during which a reset first flashes the screen white.
const int CHAR_BEHAVIOUR_FLASH_A = 11;
const int CHAR_BEHAVIOUR_FLASH_B = 12;
// Frames of the white flash.
const int CHAR_RESET_FLASH_FRAMES = 15;
// Value g_charGroupTimer takes when a character starts over.
const int CHAR_RESET_GROUP_TIMER = 16;
// Divisors that pick the two-digit fields of g_charSetupCode.
const int SETUP_FIELD_RANGE = 100;
const int SETUP_FIELD_TENS_OF_THOUSANDS = 10000;
const int SETUP_FIELD_THOUSANDS_OF_THOUSANDS = 1000000;

// Starts character `index` over: a kind CHAR_KIND_PACKED character in behaviour 11 or 12 first fades the screen to white
// over CHAR_RESET_FLASH_FRAMES frames. The AI state, the shot start vectors and the node table are cleared, a few flags are
// reset, the group timer is set and the character is marked in play. Then Char_PutOnEquipment is called with the four
// fields of the character's script code and Char_SelectKind with zeros. Returns 0. Called by the enemy and NPC behaviours
// when they start over (0x6aeda0, 0x6b4610 and others).
extern "C" int Char_ResetState(int index)
{
    if (g_charRecords[index].kind == CHAR_KIND_PACKED &&
        (g_charState[index].behaviour == CHAR_BEHAVIOUR_FLASH_A || g_charState[index].behaviour == CHAR_BEHAVIOUR_FLASH_B))
        ColourFade_StartInline(CHAR_RESET_FLASH_FRAMES, 1.0f, 1.0f, 1.0f, 1.0f);
    g_charRecords[index].defeated = 0;
    for (int word = 0; word < CHAR_STATE_SIZE / sizeof(int); word++)
        g_charState[index].words[word] = 0;
    g_characterModels[index].idling = 0;
    g_characterModels[index].mode = 1;
    g_charTalkPending[index] = 1;
    g_characterModels[index].childRunning = 0;
    g_shotSlots[index].position.Fill(0);
    g_shotSlots[index].step.Fill(0);
    g_characterModels[index].m_0012 = 0;
    for (int node = 0; node < CHAR_NODE_COUNT; node++)
        g_charNodes[index][node] = 0;
    int setupCode = g_charSetupCode[index];
    g_charGroupTimer[index] = CHAR_RESET_GROUP_TIMER;
    g_charInPlay[index] = 1;
    Char_PutOnEquipment(index, setupCode / SETUP_FIELD_TENS_OF_THOUSANDS % SETUP_FIELD_RANGE,
                        setupCode / SETUP_FIELD_RANGE % SETUP_FIELD_RANGE, setupCode % SETUP_FIELD_RANGE,
                        setupCode / SETUP_FIELD_THOUSANDS_OF_THOUSANDS % SETUP_FIELD_RANGE, 1, 0);
    Char_SelectKind(index, 0, 0);
    return 0;
}
