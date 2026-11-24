// Balloons reacting to a blast (EN build). Ported from the matched JP source decomp/src/balloon_burst.cpp.
#include <math.h>
#include <windows.h>
#include "char.h"
#include "balloon.h"
#include "node.h"
#include "sound_effects.h"

// Character slots the balloons use: the first slot and the slot after the last.
const int BALLOON_FIRST_SLOT = 0x5c;
const int BALLOON_END_SLOT = 0x7c;

// Kind letter of a balloon in g_charKind.
const char CHAR_KIND_BALLOON = 'B';

// Scale from the blast direction in radians to the thousandths the balloon stores.
const float BLAST_ANGLE_SCALE = 1000.0f;

// Reach of a blast: a balloon farther away than this feels nothing.
const float BLAST_REACH = 600.0f;

// Share of the remaining reach that becomes the push strength.
const float BLAST_PUSH_SHARE = 0.25f;

// Push strengths are rounded down to a multiple of BLAST_PUSH_UNIT and raised by BLAST_PUSH_BASE.
const int BLAST_PUSH_UNIT = 13;
const int BLAST_PUSH_BASE = 3;

// Push strength above which the balloon bursts.
const int BLAST_BURST_STRENGTH = 50;

// Height the burst balloon's model is moved to (out of sight), and the sound it plays.
const float BALLOON_BURST_Z = -10000.0f;
const int BALLOON_BURST_SOUND = 0x2e5;

// Balloon_Blast: lets every balloon that has not burst react to a blast at `blast`: it gets the direction of the blast and
// a push of BLAST_PUSH_SHARE of (BLAST_REACH - distance), rounded down to a multiple of BLAST_PUSH_UNIT plus
// BLAST_PUSH_BASE. When that share is over BLAST_BURST_STRENGTH the balloon bursts: its model is moved far away and the
// burst sound plays. Returns 0.
extern "C" int Balloon_Blast(Vec4 blast)
{
    for (int slot = BALLOON_FIRST_SLOT; slot < BALLOON_END_SLOT; slot++)
    {
        if (g_charKind[slot] == CHAR_KIND_BALLOON && g_characterModels[slot].burst == 0)
        {
            Vec4 position;
            position.Copy(g_characterModels[slot].matrix.GetTranslation());
            float angle = atan2f(position.x - blast.x, position.y - blast.y);
            g_characterModels[slot].blastAngle = (int)(angle * BLAST_ANGLE_SCALE);
            int strength = (int)(max(BLAST_REACH - (position - blast).Length2D(), 0.0f) * BLAST_PUSH_SHARE);
            g_characterModels[slot].blastPush = strength / BLAST_PUSH_UNIT * BLAST_PUSH_UNIT + BLAST_PUSH_BASE;
            if (strength > BLAST_BURST_STRENGTH)
            {
                g_characterModels[slot].burst = 1;
                g_characterModels[slot].root->children[0]->matrix = TranslationMatrix(0, 0, BALLOON_BURST_Z);
                Sound_PlayEffect(BALLOON_BURST_SOUND, &position, 1.0f);
            }
        }
    }
    return 0;
}
