// A character picks a pair of shaga pieces to link (EN build). Ported from the EN bytes (JP twin APP.dll 0x100d2110 is
// unmatched).
#include "char.h"
#include "char_dispatch.h"
#include "char_shots.h"
#include "collide_query.h"
#include "engine.h"
#include "node.h"
#include "shaga.h"
#include "visibility.h"

// The search stops once more than this many pairs are collected.
const int SHAGA_PAIR_LIMIT = 9;
// Marker of a pair that has no partner piece.
const int SHAGA_PAIR_ALONE = -2;
// A piece's second target must lie above this value to count (below it the piece is parked).
const int SHAGA_TARGET_MIN = -10000;
// Piece codes (g_shagaTargetA) are group + partner * 1000.
const int SHAGA_CODE_SCALE = 1000;
// Piece groups that never link.
const int SHAGA_GROUP_EXCLUDED_A = 0x36;
const int SHAGA_GROUP_EXCLUDED_B = 0x37;
const int SHAGA_GROUP_EXCLUDED_C = 0x3c;
const int SHAGA_GROUP_EXCLUDED_D = 0x3d;
// The link is only tried every this many frames per character slot.
const int SHAGA_LINK_PERIOD = 50;
// Behaviour states from this value up are already past the search.
const int SHAGA_LINK_MAX_STATE = 3;
// Offsets of the two link points above the character and above the piece, and the collision mask of the clear-line test.
const float SHAGA_LINK_CHARACTER_LIFT = 75.0f;
const float SHAGA_LINK_PIECE_LIFT = 25.0f;
const unsigned int SHAGA_LINK_MASK = 4;
// Added to a partner's group in the second link code.
const int SHAGA_LINK_PARTNER_BASE = 100;
// CharRecord::kind the linked character takes.
const int CHAR_KIND_SHAGA_LINKED = 0x55;
// CharState::values entries that keep the link codes, and the g_charNodes entries that keep the two pieces' nodes.
const int CHARSTATE_LINK_FIRST = 1;
const int CHARSTATE_LINK_SECOND = 2;
const int CHARNODE_LINK_FIRST = 2;
const int CHARNODE_LINK_SECOND = 3;
// CharState words that survive the reset after a link (the two link codes).
const int CHARSTATE_KEPT_WORD_A = 2;
const int CHARSTATE_KEPT_WORD_B = 3;

// One candidate: the piece to link and its partner (SHAGA_PAIR_ALONE = none).
struct ShagaPair
{
    int piece;      // piece slot
    int partner;    // partner piece slot, or SHAGA_PAIR_ALONE
};

// Once every SHAGA_LINK_PERIOD frames, if the character has no pending hit and has not started yet: collects up to ten
// pieces that are falling (their second target is below zero) with their partners, picks one at random and, when the
// line from the character to it is free of the map, stores the link codes and the two pieces' nodes in the character,
// turns it into kind 0x55 and resets its shot, state and attached nodes. Returns 0. Called by the behaviour at EN
// 0x6a0950.
extern "C" int Char_StartShagaLink(int index)
{
    int count;
    g_charTalkPending[index] = 0;
    if (g_charHitLevel[index] != 0)
        return 0;
    Enemy_StalkAndClaw(index, 0);
    if (g_frameCounter % SHAGA_LINK_PERIOD != index % SHAGA_LINK_PERIOD)
        return 0;
    CharState *state = &g_charState[index];
    if (state->state >= SHAGA_LINK_MAX_STATE)
        return 0;

    ShagaPair pairs[SHAGA_PAIR_LIMIT + 1];
    count = 0;
    for (int piece = 0; piece < SHAGA_SLOTS; piece++)
    {
        if (g_shagaNode[piece] == 0)
            continue;
        if (g_shagaTargetB[piece] <= SHAGA_TARGET_MIN || g_shagaTargetB[piece] >= 0)
            continue;
        if (g_shagaTargetA[piece] <= -1 || g_shagaTargetA[piece] == SHAGA_GROUP_EXCLUDED_A ||
            g_shagaTargetA[piece] == SHAGA_GROUP_EXCLUDED_B || g_shagaTargetA[piece] == SHAGA_GROUP_EXCLUDED_C ||
            g_shagaTargetA[piece] == SHAGA_GROUP_EXCLUDED_D)
            continue;
        int group = g_shagaTargetA[piece] % SHAGA_CODE_SCALE;
        int partnerOffset = g_shagaTargetA[piece] / SHAGA_CODE_SCALE;
        pairs[count].partner = SHAGA_PAIR_ALONE;
        if (partnerOffset != 0)
        {
            for (int other = 0; other < SHAGA_SLOTS; other++)
            {
                if (g_charNodes[other] != 0 && g_shagaTargetB[other] > SHAGA_TARGET_MIN && g_shagaTargetA[other] == group &&
                    g_shagaTargetB[other] < 0 && g_shagaTargetA[other] > -1 && other != piece)
                {
                    pairs[count].piece = other;
                    pairs[count].partner = piece;
                    count++;
                    break;
                }
            }
        }
        else
        {
            pairs[count].piece = piece;
            count++;
        }
        if (count > SHAGA_PAIR_LIMIT)
            break;
    }
    if (count == 0)
        return 0;

    int chosen = Random() % count;
    Vec4 from;
    Vec4 to;
    Vec4 hit;
    from.Copy(g_characterModels[index].matrix.GetTranslation());
    from.z += SHAGA_LINK_CHARACTER_LIFT;
    int partner = pairs[chosen].partner;
    Node *target = partner > -1 ? g_shagaNode[partner] : g_shagaNode[pairs[chosen].piece];
    to.Copy(target->worldMatrix.GetTranslation());
    to.z += SHAGA_LINK_PIECE_LIFT;
    if (g_mapRootNode == 0)
        return 0;
    if (g_mapRootNode->Collide(0, SHAGA_LINK_MASK, &from, &to, &hit) != 0)
        return 0;

    if (partner > -1)
        state->values[CHARSTATE_LINK_SECOND] =
            (g_shagaTargetA[partner] % SHAGA_CODE_SCALE + SHAGA_LINK_PARTNER_BASE) * SHAGA_CODE_SCALE + partner;
    else
        state->values[CHARSTATE_LINK_SECOND] = SHAGA_PAIR_ALONE;
    int piece = pairs[chosen].piece;
    state->values[CHARSTATE_LINK_FIRST] = (g_shagaTargetA[piece] % SHAGA_CODE_SCALE) * SHAGA_CODE_SCALE + piece;
    if (state->values[CHARSTATE_LINK_SECOND] > -1)
        g_charNodes[index][CHARNODE_LINK_SECOND] = g_shagaNode[partner];
    g_charNodes[index][CHARNODE_LINK_FIRST] = g_shagaNode[piece];
    g_charRecords[index].kind = CHAR_KIND_SHAGA_LINKED;
    for (int i = 0; i < SHOT_PARAM_COUNT; i++)
        g_shotParams[index][i] = 0;
    for (int word = 0; word < CHAR_STATE_SIZE / (int)sizeof(int); word++)
    {
        if (word != CHARSTATE_KEPT_WORD_A && word != CHARSTATE_KEPT_WORD_B)
            state->words[word] = 0;
    }
    for (int i = 0; i < CHAR_NODE_COUNT; i++)
    {
        if (i != CHARNODE_LINK_FIRST && i != CHARNODE_LINK_SECOND)
            g_charNodes[index][i] = 0;
    }
    g_shotSlots[index].position.Fill(0.0f);
    g_shotSlots[index].step.Fill(0.0f);
    g_charTalkPending[index] = 1;
    return 0;
}
