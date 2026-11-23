// An enemy looking for a shaga to go to (character behaviour run from 0x100f9670). Every 50 frames (staggered by
// slot) a waiting enemy lists the shaga slots it could head for, picks one at random and, when the world collision
// leaves a clear line from the enemy to it, stores the target(s) and switches the enemy to kind CHAR_KIND_SHAGA_SEEKER.

#include "vec4.h"
#include "matrix4.h"
#include "node.h"
#include "anim_model.h"
#include "last_position.h"
#include "char_state.h"
#include "char_slot.h"
#include "char_tables.h"
#include "char_vars.h"
#include "map_vars.h"
#include "shaga.h"

extern "C" int FUN_100aa820(int index, int variant);   // the enemy's normal behaviour
extern "C" int FUN_1013f880(void);                     // the game's random number

// Kind given to an enemy that found a shaga to go to.
#define CHAR_KIND_SHAGA_SEEKER 0x55
// The search runs for slot `index` on the frames where frame % SHAGA_SEEK_PERIOD == index % SHAGA_SEEK_PERIOD.
#define SHAGA_SEEK_PERIOD 50
// At most this many candidates are listed (the search stops once more than SHAGA_CANDIDATE_LIMIT are found).
#define SHAGA_CANDIDATE_LIMIT 9
// A shaga kind of n * SHAGA_PAIR_BASE + k (n > 0) needs a partner shaga of kind k.
#define SHAGA_PAIR_BASE 1000
// A shaga's timer (DAT_1021d018) is in (SHAGA_TIMER_MIN, 0) while it can be visited.
#define SHAGA_TIMER_MIN (-10000)
// Shaga kinds never visited.
#define SHAGA_KIND_SKIP_A 0x36
#define SHAGA_KIND_SKIP_B 0x37
#define SHAGA_KIND_SKIP_C 0x3c
#define SHAGA_KIND_SKIP_D 0x3d
// The enemy only searches while its behaviour step is below this.
#define SHAGA_SEEK_MAX_STEP 3
// Marks "no partner" in a candidate and in the stored partner code.
#define SHAGA_NO_PARTNER (-2)
// Added to the partner's kind in the stored partner code.
#define SHAGA_PARTNER_CODE_BASE 100
// The line of sight runs from 75 units above the enemy to 25 units above the shaga.
#define SHAGA_EYE_HEIGHT 75.0f
#define SHAGA_TARGET_HEIGHT 25.0f
// Collision mask of the line-of-sight test.
#define SHAGA_SIGHT_MASK 4

// 0x100d2110 (Enemy_SeekShaga). Character behaviour of enemy `index`: clears its "seeking" flag (DAT_10255a88) and
// runs the normal behaviour (FUN_100aa820) unless the enemy was hit. On its frame of every SHAGA_SEEK_PERIOD, while
// its step is below SHAGA_SEEK_MAX_STEP, it lists the visitable shaga slots (model loaded, timer in
// (SHAGA_TIMER_MIN, 0), kind not skipped; a pair kind also needs a visitable partner of the paired kind), picks one
// at random and tests the line from the enemy to it. When the line is clear the enemy's state gets the target code
// in `talk` and the partner code in `range`, the shaga nodes are attached ([2] target, [3] partner), its speeds,
// state (except words 2 and 3), nodes (except [2] and [3]) and last position are cleared, its kind becomes
// CHAR_KIND_SHAGA_SEEKER and the "seeking" flag is set. Always returns 0.
extern "C" int FUN_100d2110(int index)
{
    DAT_10255a88[index] = 0;
    if (DAT_102550e0[index] != 0)
        return 0;
    FUN_100aa820(index, 0);
    if (DAT_104c0380 % SHAGA_SEEK_PERIOD != index % SHAGA_SEEK_PERIOD)
        return 0;
    if (DAT_1020b430[index].state >= SHAGA_SEEK_MAX_STEP)
        return 0;


    ShagaCandidate candidates[SHAGA_CANDIDATE_LIMIT + 1];
    int count = 0;
    for (int slot = 0;
         slot < SHAGA_SLOT_COUNT; slot++)
    {
        if (DAT_10446e00[slot] == 0)
            continue;
        if (DAT_1021d018[slot] <= SHAGA_TIMER_MIN || DAT_1021d018[slot] >= 0)
            continue;
        int kind = DAT_1042db08[slot];
        if (kind <= -1 || kind == SHAGA_KIND_SKIP_A || kind == SHAGA_KIND_SKIP_B || kind == SHAGA_KIND_SKIP_C ||
            kind == SHAGA_KIND_SKIP_D)
            continue;
        candidates[count].partner = SHAGA_NO_PARTNER;
        if (kind / SHAGA_PAIR_BASE != 0)
        {
            for (int other = 0; other < SHAGA_SLOT_COUNT; other++)
            {
                // The original tests the address of the node row, so this part of the test always passes.
                if (DAT_1021bdf8[other] != 0 && DAT_1021d018[other] > SHAGA_TIMER_MIN &&
                    DAT_1042db08[other] == kind % SHAGA_PAIR_BASE && DAT_1021d018[other] < 0 &&
                    DAT_1042db08[other] > -1 && other != slot)
                {
                    candidates[count].target = other;
                    candidates[count].partner = slot;
                    count++;
                    break;
                }
            }
        }
        else
        {
            candidates[count].target = slot;
            count++;
        }
        if (count > SHAGA_CANDIDATE_LIMIT)
            break;
    }
    if (count == 0)
        return 0;

    int pick = FUN_1013f880() % count;
    Vec4 eye;
    Vec4 goal;
    Vec4 hit;
    eye.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    eye.z += SHAGA_EYE_HEIGHT;
    int partner = candidates[pick].partner;
    if (partner > -1)
        goal.Copy(DAT_10446e00[partner]->worldMatrix.GetTranslation());
    else
        goal.Copy(DAT_10446e00[candidates[pick].target]->worldMatrix.GetTranslation());
    goal.z += SHAGA_TARGET_HEIGHT;
    if (DAT_10255f08 == 0)
        return 0;
    if (DAT_10255f08->Collide(0, SHAGA_SIGHT_MASK, &eye, &goal, &hit) != 0)
        return 0;

    if (partner > -1)
        DAT_1020b430[index].range = (DAT_1042db08[partner] % SHAGA_PAIR_BASE + SHAGA_PARTNER_CODE_BASE) * SHAGA_PAIR_BASE + partner;
    else
        DAT_1020b430[index].range = SHAGA_NO_PARTNER;
    int target = candidates[pick].target;
    DAT_1020b430[index].talk = DAT_1042db08[target] % SHAGA_PAIR_BASE * SHAGA_PAIR_BASE + target;
    if (DAT_1020b430[index].range > -1)
        DAT_1021bdf8[index][3] = DAT_10446e00[partner];
    DAT_1021bdf8[index][2] = DAT_10446e00[target];

    DAT_10daaae8[index].kind = CHAR_KIND_SHAGA_SEEKER;
    for (int speed = 0; speed < 4; speed++)
        DAT_10255160[index][speed] = 0;
    for (int word = 0; word < 0x10; word++)
    {
        if (word != 2 && word != 3)
            DAT_1020b430[index].words[word] = 0;
    }
    for (int node = 0; node < 9; node++)
    {
        if (node != 2 && node != 3)
            DAT_1021bdf8[index][node] = 0;
    }
    DAT_10802bf8[index].position.Fill(0.0f);
    DAT_10802bf8[index].step.Fill(0.0f);
    DAT_10255a88[index] = 1;
    return 0;
}
