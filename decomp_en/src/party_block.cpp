// Party members blocking each other's moves (EN build). Ported from the matched JP source decomp/src/party_block.cpp.
#include <math.h>
#include "char.h"
#include "party_block.h"

// Angle between the move's heading and the direction to the other member below which the move is heading at it.
const float PARTY_HEADING_TOLERANCE = 0.25f;

// Fraction of the range within which another member always blocks the move.
const float PARTY_CLOSE_FRACTION = 0.4f;

// Party_BlocksMove: returns 1 when one of the PARTY_SIZE party characters other than `self` would be within `range` of
// character `self` after it moves by `step` (on the ground plane) and either the move brings `self` closer to it while
// heading at it (within PARTY_HEADING_TOLERANCE radians) or it would be closer than PARTY_CLOSE_FRACTION of `range`; else
// 0. Called by the party members' movement code.
extern "C" int Party_BlocksMove(int self, const Vec4 &step, int range)
{
    Vec4 ahead = g_characterModels[self].matrix.GetTranslation() + step;
    Vec4 other;
    for (int i = 0; i < PARTY_SIZE; i++)
    {
        if (g_partyCharacters[i] == self)
            continue;
        other.Copy(g_characterModels[g_partyCharacters[i]].matrix.GetTranslation());
        float distance = (other - ahead).Length2D();
        float gain = distance - (other - g_characterModels[self].matrix.GetTranslation()).Length2D();
        float heading = atan2f(step.x, step.y);
        float toOther = atan2f(other.x - ahead.x, other.y - ahead.y);
        if (distance < (float)range)
        {
            if (gain > 0.0f && PARTY_HEADING_TOLERANCE > fabsf(heading - toOther))
                return 1;
            if (distance < (float)range * PARTY_CLOSE_FRACTION)
                return 1;
        }
    }
    return 0;
}
