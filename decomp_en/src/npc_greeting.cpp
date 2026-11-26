// The voice of the character the player stands next to (EN build). Ported from the JP source (APP.dll 0x100ba040, open
// there) and the EN bytes.
#include <math.h>

#include "anim_model.h"
#include "char.h"
#include "char_inline.h"
#include "char_move.h"
#include "char_script.h"
#include "head_look.h"
#include "map_records.h"
#include "npc_greet.h"
#include "vec4.h"

// Character slots that can be greeted (the NPC slots) and their kind.
const int GREETING_FIRST_SLOT = 0x5c;
const int GREETING_SLOT_END = 0x80;
const int GREETING_KIND = 0x140;
// How near the greeted character must stay (x/y distance and height) to keep its voice, and how near a new one must come.
const float GREETING_KEEP_RANGE = 125.0f;
const float GREETING_KEEP_HEIGHT = 15.0f;
const float GREETING_NEW_HEIGHT = 25.0f;
// Frames after the player walked away before the greeting is forgotten.
const int GREETING_FORGET_FRAMES = 0x1e;
// The voice line of a character is a signed four-digit field of its route point; the voice sounds start here.
const int GREETING_VOICE_MODULUS = 10000;
const int GREETING_VOICE_SIGN_LIMIT = 5000;
const int GREETING_SOUND_FIRST = 0x240;
// AnimModel::mode of a model over a pit.
const int GREETING_MODE_PIT = 2;

// The voice of the greeted character: counts down the frames since the player walked away, then, while the player
// stands on the ground, keeps the greeted character while it is within 125 units in x/y and 15 in height; otherwise the
// first kind-0x140 character within 125 units and 25 in height becomes the greeted one and its voice line (from its
// route point, modulo 10000) plays on the player. Walking away from all of them clears the greeting after 30 frames, and
// a player over a pit clears it at once. Returns 0. Called by the frame update at EN 0x5b49d0.
extern "C" int Npc_PlayGreeting(void)
{
    if (g_greetingForgetTimer > 0 && --g_greetingForgetTimer == 0)
        g_greetingVoice = 0;
    if (g_characterModels[PLAYER_SLOT].idleMotionEnabled == IDLE_MOTION_ACTIVE)
    {
        float distance;
        Vec4 player;
        Vec4 other;
        player.Copy(g_characterModels[PLAYER_SLOT].matrix.GetTranslation());
        if (g_scriptSpeaker != 0)
        {
            other.Copy(g_characterModels[g_scriptSpeaker].matrix.GetTranslation());
            float height = (float)fabs(other.z - player.z);
            distance = (other - player).Length2D();
            if (height < GREETING_KEEP_HEIGHT && distance < GREETING_KEEP_RANGE)
            {
                g_greetingForgetTimer = 0;
                goto checkPit;
            }
        }
        for (int slot = GREETING_FIRST_SLOT; slot < GREETING_SLOT_END; slot++)
        {
            if (g_charRecords[slot].kind != GREETING_KIND)
                continue;
            other.Copy(g_characterModels[slot].matrix.GetTranslation());
            float height = (float)fabs(other.z - player.z);
            distance = (other - player).Length2D();
            if (height < GREETING_NEW_HEIGHT && distance < GREETING_KEEP_RANGE)
            {
                if (g_scriptSpeaker != slot)
                {
                    g_scriptSpeaker = slot;
                    g_greetingVoice = g_mapRecords[g_characterModels[slot].switchNode].points[g_charState[slot].values[1]].y %
                                      GREETING_VOICE_MODULUS;
                    if (g_greetingVoice > GREETING_VOICE_SIGN_LIMIT)
                        g_greetingVoice -= GREETING_VOICE_MODULUS;
                    Character_PlaySoundInline(PLAYER_SLOT, g_greetingVoice + GREETING_SOUND_FIRST, 1, 0);
                }
                g_greetingForgetTimer = 0;
                goto checkPit;
            }
        }
        if (g_scriptSpeaker != 0)
            g_greetingForgetTimer = GREETING_FORGET_FRAMES;
        g_scriptSpeaker = 0;
        g_scriptPlayback = 0;
    }
checkPit:
    if (g_characterModels[PLAYER_SLOT].mode == GREETING_MODE_PIT)
    {
        g_scriptSpeaker = 0;
        g_greetingVoice = 0;
        g_scriptPlayback = 0;
    }
    return 0;
}
