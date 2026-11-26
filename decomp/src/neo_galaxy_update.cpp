// The Neo Galaxy boss (CharSlot::kind 0x32, slots 0x3a, 0x3c and 0x3e are its three bodies; 0x100e7280). Slot 0x3a runs
// the whole fight: it picks one of eight attacks (states 1..8: balls of light, rings, the Z slash, the final spark ...),
// summons helpers (state 0x14), is weakened at half health (second phase, CharState word WORD_ANGRY), flees and returns
// (states 0x1e..0x20) and, when beaten, plays the ending (state 0x2a: the galaxy backdrop, the explosions, the fade out
// and the score flags). Slots below 0x3a go to Enemy_DefaultStep (0x100e21a0). The slot keeps its variables in the words
// of CharState (see the WORD_ names) and starts its effects with the same matrix + EffectOptions pairs as the other
// bosses.

#include <math.h>
#include <stdlib.h>

#include "vec4.h"
#include "matrix4.h"
#include "node.h"
#include "anim_model.h"
#include "char_state.h"
#include "char_slot.h"
#include "char_tables.h"
#include "char_vars.h"
#include "game_vars.h"
#include "map_vars.h"
#include "camera_vars.h"
#include "world.h"
#include "effect_manager.h"
#include "minigame_area.h"
#include "dodge_game.h"
#include "game.h"
#include "game_main.h"
#include "hop_spit_enemy.h"
#include "partner_follow.h"
#include "route_flier.h"
#include "target_race.h"
#include "enemy_chase_hop.h"
#include "actor_scan.h"
#include "char_spawn.h"
#include "scene_reset.h"
#include "dodge_golem.h"
#include "char_hikari.h"
#include "path_walk.h"
#include "player_frame.h"
#include "script_system.h"
#include "texture_load.h"
#include "doppler_loops.h"
#include "field_update.h"
#include "minigame_hud.h"
#include "field_item_menu.h"
#include "ui_vars.h"
#include "enemy_roam_spit.h"

#include "neo_galaxy.h"

#define GALAXY_PI 3.1415f
#define GALAXY_FIRST_SLOT 0x3a                 // the main body; the two others are two and four slots on
#define GALAXY_BODY_STRIDE 2
#define GALAXY_SLOT_END 92                     // enemy slots 0x3a..0x5b are scanned when the fight ends
#define GALAXY_KIND 0x32
#define GALAXY_HELPER_KIND 0x2a                // the helpers state 0x14 summons (and clears again)
#define GALAXY_GHOST_KIND 0x48                 // cleared when the body returns (state 0x20)
#define GALAXY_PARKED_X 200000.0f              // a body is parked this far away when it is out of the scene
#define GALAXY_GONE_X -200000.0f
#define GALAXY_OFF_SCENE -20000.0f             // a body whose x is below this is not placed
#define GALAXY_FADE_FRAMES 15.0f               // the alpha ramps over 15 frames
#define GALAXY_NODE_HIDDEN 0x20                // Node flag: not drawn
#define GALAXY_NODE_FLAG_4 4
#define GALAXY_RING_RADIUS 500.0f
#define GALAXY_RING_HEIGHT 200.0f
#define GALAXY_THIRD_TURN 2.0944f              // a third of a turn, between the three bodies of a ring
#define GALAXY_FLASH_FRAMES 0x168              // WORD_FLASH while a hit is flashing
#define GALAXY_MOTION_SPEED 26.0f
#define GALAXY_MOTION_WALK 1
#define GALAXY_MOTION_IDLE 2
#define GALAXY_MOTION_ATTACK 4
#define GALAXY_SOUND_FLASH 0x23a
#define GALAXY_SCORE_FLAG_BASE 1000
#define GALAXY_STAT_SCALE 0.8f
#define GALAXY_SUMMON_HEALTH_STAT 4

// The slot's own variables (words of CharState).
#define STATE DAT_1020b430[index].state
#define STEP DAT_1020b430[index].words[1]                       // frames in the state
#define WORD_FLASH DAT_1020b430[index].words[2]                 // hit flash (0x168 while a long state holds the pose)
#define WORD_FLASH_LEVEL DAT_1020b430[index].words[3]
#define WORD_LAST_STATE DAT_1020b430[index].words[4]            // the state before the pause state 100
#define WORD_COUNT DAT_1020b430[index].words[5]                 // small counter of the state (and the attack choice)
#define WORD_FRAMES DAT_1020b430[index].words[6]                // frames since the slot started
#define WORD_DEFEATED DAT_1020b430[index].words[7]              // 1 once the fight is over (body 0x3a, 0x3c, 0x3e in words 7 of each)
#define WORD_GONE DAT_1020b430[index].words[8]                  // 1 while the body waits outside the scene
#define WORD_ATTACKS DAT_1020b430[index].words[9]               // attacks done
#define WORD_ANGRY DAT_1020b430[index].words[0xb]               // 1 in the second phase
#define WORD_STARTED DAT_1020b430[index].words[0xc]             // 1 once the slot has started
#define WORD_IDLE_FRAMES DAT_1020b430[index].words[0xd]
#define WORD_WEAK DAT_1020b430[index].words[0xe]
#define WORD_ENDED DAT_1020b430[index].words[0xf]               // 1 after the long finish (state 0x20 from step 500)
#define BODY_WORD(slot, n) DAT_1020b430[slot].words[n]

#define GALAXY_LINE_ANGRY "\202\340\202\244\223{\202\301\202\275\202\274\201I"
#define GALAXY_LINE_ENDURED "\202\346\202\255\221\317\202\246\202\352\202\275\202\310"
#define GALAXY_LINE_NAME "\203l\203I\201E\203M\203\203\203\211\203N\203V\201["
#define GALAXY_LINE_COLD_UPPER "\203R\201[\203\213\203h\203A\203b\203p\201["
#define GALAXY_LINE_FLAME_ATTACK "\203t\203\214\203C\203\200\203A\203^\203b\203N"
#define GALAXY_LINE_FINAL_SPARK "\203t\203@\203C\203i\203\213\203X\203p\201[\203N"
#define GALAXY_LINE_TAKE_THIS "\202\255\202\347\202\246\201I"
#define GALAXY_LINE_Z_SLASH "\202y\203X\203\211\203b\203V\203\205\201I\201I"
#define GALAXY_LINE_REMEMBER "\202\250\202\332\202\246\202\304\202\353\201["
#define GALAXY_LINE_DAMN "\202\255\202\273\201["
#define GALAXY_LINE_DEFEAT "\202\334\202\263\202\251\201A\225\211\202\257\202\351\202\310\202\361\202\304\201c\201c"

// States.
enum GalaxyState
{
    GALAXY_STATE_START = 0,
    GALAXY_STATE_SPAWN_HELPER = 0x14,
    GALAXY_STATE_HELPER_WAIT = 0x15,
    GALAXY_STATE_DEFEATED = 0x1e,
    GALAXY_STATE_FINISH = 0x1f,
    GALAXY_STATE_RETURN = 0x20,
    GALAXY_STATE_RING_A = 0x28,
    GALAXY_STATE_RING_B = 0x29,
    GALAXY_STATE_ENDING = 0x2a,
    GALAXY_STATE_REST = 99,
    GALAXY_STATE_CHOOSE = 100
};

#define GALAXY_ATTACK_Z_SLASH 7
#define GALAXY_ATTACK_FINAL_SPARK 8

// Sets the three-slot bodies' alpha together (the three bodies fade as one).
static void SetBodyAlpha(int index, float alpha)
{
    DAT_10db6ee8[index + 4].alpha = alpha;
    DAT_10db6ee8[index + 2].alpha = alpha;
    DAT_10db6ee8[index].alpha = alpha;
}

// Plays the "hit" sound and slows the player while the player is restricted, in the first frames of a fade.
static void SlowRestrictedBodies(int index, int firstStepLimit)
{
    if (DAT_1020b430[index].words[1] < firstStepLimit && DAT_10db6ee8[index].m_238c != 0)
    {
        DAT_10db6ee8[index].AddMotionSpeed(GALAXY_MOTION_SPEED);
        FUN_10065b10(index, GALAXY_SOUND_FLASH, 1, 0);
    }
}

// The other two bodies' restricted checks of the same fade.
static void SlowOtherBodies(int index, int firstStepLimit)
{
    if (STEP < firstStepLimit && DAT_10db6ee8[index + 2].m_238c != 0)
    {
        DAT_10db6ee8[index].AddMotionSpeed(GALAXY_MOTION_SPEED);
        FUN_10065b10(index + 2, GALAXY_SOUND_FLASH, 1, 0);
    }
    if (STEP < firstStepLimit && DAT_10db6ee8[index + 4].m_238c != 0)
    {
        DAT_10db6ee8[index].AddMotionSpeed(GALAXY_MOTION_SPEED);
        FUN_10065b10(index + 1, GALAXY_SOUND_FLASH, 1, 0);
    }
}

// Clears the health of every helper of `kind` that is placed in the scene (kind 0x2a only counts when it is placed).
static void RemoveHelpers(int kind, int onlyPlaced)
{
    for (int slot = GALAXY_FIRST_SLOT; slot < GALAXY_SLOT_END; slot++)
    {
        if (DAT_10daaae8[slot].kind != kind)
            continue;
        if (onlyPlaced && !(DAT_10db6ee8[slot].matrix.translation.x > GALAXY_OFF_SCENE))
            continue;
        DAT_10daaae8[slot].stats[STAT_HEALTH] = 0;
    }
}

// The backdrop of the ending: the map in front, the galaxy hidden.
static void ShowMapHideGalaxy(Node *galaxy)
{
    if (galaxy != 0)
        galaxy->SetFlags(GALAXY_NODE_HIDDEN);
    if (DAT_10255f08 != 0)
    {
        Node *node = DAT_10255f08->Find("@yougan1", 0);
        if (node != 0)
            node->M_1019b210(GALAXY_NODE_HIDDEN);
        node = DAT_10255f08->Find("@yougan2", 0);
        if (node != 0)
            node->M_1019b210(GALAXY_NODE_HIDDEN);
        node = DAT_10255f08->Find("@wall", 0);
        if (node != 0)
            node->M_1019b210(GALAXY_NODE_HIDDEN);
        node = DAT_10255f08->Find("@mapobj", 0);
        if (node != 0)
            node->M_1019b210(GALAXY_NODE_HIDDEN);
        node = DAT_10255f08->Find("@yogake", 0);
        if (node != 0)
            node->ClearFlags(GALAXY_NODE_HIDDEN);
        node = DAT_10255f08->Find("@ShadoMap", 0);
        if (node != 0)
            node->M_1019b210(GALAXY_NODE_HIDDEN);
    }
    if (DAT_10255f0c != 0)
    {
        Node *node = DAT_10255f0c->Find("@sky_null", 0);
        if (node != 0)
            node->ClearFlags(GALAXY_NODE_HIDDEN);
        node = DAT_10255f0c->Find("@tikei_null", 0);
        if (node != 0)
            node->ClearFlags(GALAXY_NODE_HIDDEN);
        node = DAT_10255f0c->Find("@ShadowMap", 0);
        if (node != 0)
            node->ClearFlags(GALAXY_NODE_HIDDEN);
    }
}

// The backdrop of the finale: the galaxy in front, the map hidden.
static void ShowGalaxyHideMap(Node *galaxy)
{
    if (galaxy != 0)
        galaxy->ClearFlags(GALAXY_NODE_HIDDEN);
    if (DAT_10255f08 != 0)
    {
        Node *node = DAT_10255f08->Find("@yougan1", 0);
        if (node != 0)
            node->M_1019b190(GALAXY_NODE_HIDDEN);
        node = DAT_10255f08->Find("@yougan2", 0);
        if (node != 0)
            node->M_1019b190(GALAXY_NODE_HIDDEN);
        node = DAT_10255f08->Find("@wall", 0);
        if (node != 0)
            node->M_1019b190(GALAXY_NODE_HIDDEN);
        node = DAT_10255f08->Find("@mapobj", 0);
        if (node != 0)
            node->M_1019b190(GALAXY_NODE_HIDDEN);
        node = DAT_10255f08->Find("@yogake", 0);
        if (node != 0)
            node->SetFlags(GALAXY_NODE_HIDDEN);
        node = DAT_10255f08->Find("@ShadoMap", 0);
        if (node != 0)
            node->M_1019b190(GALAXY_NODE_HIDDEN);
    }
    if (DAT_10255f0c != 0)
    {
        Node *node = DAT_10255f0c->Find("@sky_null", 0);
        if (node != 0)
            node->SetFlags(GALAXY_NODE_HIDDEN);
        node = DAT_10255f0c->Find("@tikei_null", 0);
        if (node != 0)
            node->SetFlags(GALAXY_NODE_HIDDEN);
        node = DAT_10255f0c->Find("@ShadowMap", 0);
        if (node != 0)
            node->SetFlags(GALAXY_NODE_HIDDEN);
    }
}

// Starts an effect at the origin of `matrix` (the matrices carry the placement).
static void StartAt(int index, int effect, const Matrix4 *matrix, const EffectOptions *options, Node *anchor, int frame)
{
    Vec4 origin;
    FUN_1001d8b0(index, effect, origin, -1, -1, anchor != 0 ? &anchor->worldMatrix : 0, 0, frame, matrix, options);
}

// Starts an effect of the boss at `where`, turned by `angle` about z and lifted by `lift`. The options belong to the
// main body; `param` and `scale` are what the effect reads from them, `edge` the byte at 0x0e (0 = the default).
static void FireEffect(int index, int effect, const Vec4 &where, float angle, float lift, int param, float scale, int edge,
                       Node *anchor, int frame)
{
    EffectOptions options;
    Matrix4 placement = RotationZ(angle);
    placement.SetTranslation(where);
    placement.translation.z += lift;
    options.m_0000 = 0;
    options.m_0004 = param;
    options.m_0008 = scale;
    if (edge != 0)
        options.m_000e = (char)edge;
    options.owner = GALAXY_FIRST_SLOT;
    StartAt(index, effect, &placement, &options, anchor, frame);
}

// A matrix at `position` turned by `angle` about z.
static Matrix4 Placed(const Vec4 &position, float angle)
{
    Matrix4 turn = RotationZ(angle);
    turn.SetTranslation(position);
    return turn;
}

// The ring of three bodies: three points on a circle around the player, a third of a turn apart.
static void PlaceRing(Vec4 *ring, float angle)
{
    for (int body = 0; body < 3; body++)
        ring[body].Fill(0.0f);
    ring[0].y = -GALAXY_RING_RADIUS;
    ring[1].y = -GALAXY_RING_RADIUS;
    ring[2].y = -GALAXY_RING_RADIUS;
    ring[0].RotateZ(-angle);
    ring[1].RotateZ(GALAXY_THIRD_TURN - angle);
    ring[2].RotateZ(angle - GALAXY_THIRD_TURN);
    ring[0].z = GALAXY_RING_HEIGHT;
    ring[1].z = GALAXY_RING_HEIGHT;
    ring[2].z = GALAXY_RING_HEIGHT;
}

// Moves the three bodies to the ring points.
static void MoveBodiesToRing(int index, const Vec4 *ring)
{
    for (int body = 0; body < 3; body++)
    {
        if (DAT_10db6ee8[index + body * GALAXY_BODY_STRIDE].matrix.translation.x > GALAXY_OFF_SCENE)
            DAT_10db6ee8[index + body * GALAXY_BODY_STRIDE].SetPosition(ring[body].x, ring[body].y, ring[body].z);
    }
}

// Fades the three bodies in over 15 frames, then (frame 15) shows them at full alpha and starts their idle motion.
static void FadeBodiesIn(int index, int step)
{
    if (step < 15)
        SetBodyAlpha(index, (float)step / GALAXY_FADE_FRAMES);
}

// The 0x100e7280 behaviour proper.
// Turns the body towards the player.
static void FacePlayer(AnimModel &self, const Vec4 &toPlayer)
{
    self.targetHeading = (float)FUN_10001f80(toPlayer.x, toPlayer.y) + GALAXY_PI;
}

// A quick brightening: `frames` frames of the screen fade towards white.
static void FlashWhite(int frames)
{
    FUN_1000ec70(frames, 1.0f, 1.0f, 1.0f, 1.0f);
}

// A fade to half grey for the long attack names.
static void FlashGrey()
{
    FUN_1000ec70(0xf, 0.5f, 0.5f, 0.5f, 1.0f);
}

// The matrix 0x100e7280 writes into the model's root node: a turn about x by `angle`.
static void TiltRoot(AnimModel &self, float angle)
{
    self.root->matrix = RotationX(angle);
}


// 0x100e7280 (NeoGalaxy_Update). One frame of a Neo Galaxy body in slot `index`. Slots below 0x3a take the default
// enemy step. Slot 0x3a drives the three bodies and the fight; once the scene is over (DAT_10210d30) it runs the
// fade-out and the credits-flag handling instead. Returns 0.
int NeoGalaxy_Update(int index)
{
    int motion = -1;                    // motion to start at the end of the frame (-1 = keep the current one)
    int moveMode = 0;                   // 1 = walk by `step` through the map probe, 2 = place at position + step
    Vec4 step;                          // movement of this frame, relative to the body's heading
    Vec4 playerPos;
    Vec4 ownPos;
    Vec4 toPlayer;                      // from the body to the player
    int hard = DAT_10c635f8 == 2;

    if (index < GALAXY_FIRST_SLOT)
    {
        FUN_100e21a0(index);
        return 0;
    }

    Node *&headBone = DAT_1021bdf8[index][0];
    Node *&spineBone = DAT_1021bdf8[index][1];
    Node *&rootNode = DAT_1021bdf8[index][2];
    Node *&bipBone = DAT_1021bdf8[index][3];

    if (index == GALAXY_FIRST_SLOT)
    {
        DAT_104c0110 = GALAXY_FIRST_SLOT;
        DAT_10255ac2 = 0;
        DAT_1020344c = 1.25f;
        if (DAT_10db6ee8[0].matrix.translation.z <= 300.0f)
            DAT_1101dddc = 0.0f;
        else
            DAT_1101dddc = (300.0f - DAT_10db6ee8[0].matrix.translation.z) * 0.5f;
    }
    else
        DAT_10daaae8[index].stats[STAT_BUSY] = 1;
    DAT_1021f6c0[index] = 0;

    playerPos = DAT_10db6ee8[0].matrix.GetTranslation();
    ownPos = DAT_10db6ee8[index].matrix.GetTranslation();
    toPlayer = playerPos - ownPos;

    if (WORD_ENDED != 0 && DAT_10db6ee8[0].m_238c != 0)
    {
        RemoveHelpers(GALAXY_HELPER_KIND, 1);
        WORD_ENDED = 0;
    }

    // The scene is over: fade the bodies out (DAT_10daaae8[index] 0x3a also runs the intro/outro of the galaxy backdrop).
    if (DAT_10210d30[index] != 0)
    {
        DAT_10daaae8[index].stats[STAT_BUSY] = 1;
        if (WORD_STARTED == 0)
        {
            FUN_1000ec70(0xf, 1.0f, 1.0f, 1.0f, 1.0f);
            if (index == GALAXY_FIRST_SLOT)
            {
                FUN_10011c60();
                DAT_1020c2f0[0].galaxyFade = 0;
            }
            DAT_10daaa80.RemoveOwnedBy(index);
            WORD_STARTED = 1;
        }
        if (index != GALAXY_FIRST_SLOT)
            return 0;
        if (DAT_1020c2b0 == 0x2a)
        {
            // The galaxy backdrop turns with the player's view.
            Vec4 fromCamera = playerPos - DAT_118e69e8;
            float angle = (float)atan2(fromCamera.x, fromCamera.y) + GALAXY_PI;
            Matrix4 spin = RotationZ(-angle) * RotationX(-0.01f) * RotationZ(angle);
            DAT_1021c628->matrix = DAT_1021c628->matrix * spin;
            DAT_1020c2b4 = DAT_1020c2b4 + 1;
            DAT_104c00c8 = 1;
            if (DAT_1020c2b4 < 0x1c2)
                DAT_1020c2b4 = 0x1c2;
            else if (DAT_1020c2b4 < 0x1d1)
                FUN_1000ec70(0x299 - DAT_1020c2b4, 0.0f, 0.0f, 0.0f, 1.0f);
            else if (DAT_1020c2b4 == 0x1d1)
            {
                DAT_10da90d0.Set(0.0f, 0.0f, 0.0f, 1.0f);
                DAT_1042da68 = 0;
                ShowMapHideGalaxy(DAT_1021c628);
            }
            else if (DAT_1020c2b4 < 0x1d6)
                FUN_1000ec70(0xf, 1.0f, 1.0f, 1.0f, 1.0f);
            else if (DAT_1020c2b4 == 0x1d6)
            {
                FUN_1000ec70(0xf, 1.0f, 1.0f, 1.0f, 1.0f);
                DAT_1020c2b0 = -1;
            }
        }
        int fade = DAT_1020c2f0[0].galaxyFade + 1;
        DAT_104c00c8 = 1;
        DAT_1021f9fa = 0;
        DAT_10daaae8[index].stats[STAT_BUSY] = 1;
        DAT_10daaae8[index].m_0141 = 1;
        if (fade < 0xf)
        {
            DAT_1020c2f0[0].galaxyFade = fade;
            DAT_10db6ee8[index].PlayMotion(GALAXY_MOTION_IDLE, 0, 5, 1.0f);
        }
        else
        {
            if (fade == 0xf)
            {
                DAT_10e3c78c = 0.0f;
                DAT_1020c2f0[0].galaxyFade = fade;
                DAT_10db6ee8[index].SetPosition(0.0f, 0.0f, 200.0f);
                DAT_10e3dec0 = (float)FUN_10001f80(toPlayer.x, toPlayer.y) + GALAXY_PI;
                return 0;
            }
            if (fade < 0x1e)
            {
                DAT_1020c2f0[0].galaxyFade = fade;
                DAT_1021f9fa = 0;
                DAT_104c00c8 = 1;
                DAT_10daaae8[index].stats[STAT_BUSY] = 1;
                DAT_10daaae8[index].m_0141 = 1;
                DAT_10e3c78c = (float)(fade - 0xe) / 15.0f;
                return 0;
            }
            if (fade == 0x1e)
            {
                DAT_10e3c78c = 1.0f;
                DAT_1020c2f0[0].galaxyFade = fade;
                FUN_10056260(GALAXY_FIRST_SLOT, GALAXY_LINE_DAMN, GALAXY_LINE_DEFEAT, "", "");
                return 0;
            }
            if (fade < 0x5a)
            {
                DAT_1020c2f0[0].galaxyFade = fade;
                DAT_1021f9fa = 0;
                DAT_104c00c8 = 1;
                DAT_10daaae8[index].stats[STAT_BUSY] = 1;
                DAT_10daaae8[index].m_0141 = 1;
                return 0;
            }
            if (fade == 0x5a)
            {
                DAT_1020c2f0[0].galaxyFade = fade;
                FUN_10056260(GALAXY_FIRST_SLOT, GALAXY_LINE_REMEMBER, "", "", "");
                return 0;
            }
            if (fade > 0xa4)
            {
                if (fade != 0xa5)
                {
                    DAT_1020c2f0[0].galaxyFade = fade;
                    DAT_1021f9fa = 0;
                    DAT_104c00c8 = 1;
                    DAT_10daaae8[index].stats[STAT_BUSY] = 1;
                    DAT_10daaae8[index].m_0141 = 1;
                    return 0;
                }
                // The fight is recorded as won and the credits flag is set (bit 0, or bit 1 when the pictures are skipped).
                DAT_1020c2f0[0].galaxyFade = fade;
                DAT_10db6ee8[index].SetPosition(GALAXY_GONE_X, 0.0f, 0.0f);
                DAT_10874848.MarkKilled(DAT_10daaae8[GALAXY_FIRST_SLOT].routeRecord);
                int flagIndex = DAT_10c63ffc * 2 + GALAXY_SCORE_FLAG_BASE;
                unsigned int flags = DAT_10874848.TestScriptFlag(flagIndex);
                unsigned int wanted = flags | (DAT_10c64f8c == 0 ? 1 : 2);
                if (flags != wanted)
                    DAT_10874848.SetScriptFlag(flagIndex, wanted);
                return 0;
            }
            DAT_1020c2f0[0].galaxyFade = fade;
            DAT_10db6ee8[index].PlayMotion(GALAXY_MOTION_WALK, 0, 5, 1.0f);
            DAT_10e3dec0 = (float)FUN_10001f80(toPlayer.x, toPlayer.y);
            step.x = 0.0f;
            step.y = -18.0f;
            step.RotateZ(DAT_10e3dec0);
            FUN_100241e0(GALAXY_FIRST_SLOT, step.x, step.y, step.z);
            if (DAT_1020c2f0[0].galaxyFade < 0x97)
                return 0;
        }
        if (fade >= 0x9b && DAT_10e3ddcc != 0)
        {
            DAT_10db6ee8[0].AddMotionSpeed(GALAXY_MOTION_SPEED);
            FUN_10065b10(GALAXY_FIRST_SLOT, GALAXY_SOUND_FLASH, 1, 0);
        }
        if (DAT_10e3c78c - 0.066667f < 0.0f)
            DAT_10e3c78c = 0.0f;
        else
            DAT_10e3c78c = DAT_10e3c78c - 0.066667f;
        DAT_10e3ded8 = 1;
        return 0;
    }

    if (index == GALAXY_FIRST_SLOT)
        DAT_1021f9fa = 0;
    WORD_FRAMES++;
    WORD_IDLE_FRAMES++;

    // The hit flash: it counts down while the DAT_10daaae8[index] is not stunned and rises while a stun lasts.
    if (DAT_102550e0[index] == 0)
    {
        if (WORD_FLASH > 0)
            WORD_FLASH--;
        WORD_FLASH_LEVEL = 0;
    }
    else if (WORD_FLASH_LEVEL < (int)DAT_102550e0[index])
    {
        WORD_FLASH += 0x18;
        int level = 0x14 - WORD_FLASH / 0xc;
        level = ((level < 1) - 1) & level;
        DAT_102550e0[index] = (char)level;
        WORD_FLASH_LEVEL = level & 0xff;
    }
    if (DAT_10254390[index] != 0 || DAT_102550e0[index] != 0)
        goto finish;

    // State 0: look up the bones, then the body takes its first pose.
    if (STATE == GALAXY_STATE_START)
    {
        motion = GALAXY_MOTION_IDLE;
        headBone = DAT_10db6ee8[index].root->Find("Bip01 Head", 0);
        spineBone = DAT_10db6ee8[index].root->Find("Bip01 Spine", 0);
        bipBone = DAT_10db6ee8[index].root->Find("Bip01", 0);
        STATE = GALAXY_STATE_CHOOSE;
        if (index != GALAXY_FIRST_SLOT)
        {
            STATE = GALAXY_STATE_HELPER_WAIT;
            goto finish;
        }
        DAT_1021c628 = 0;
        if (DAT_10255f0c != 0)
        {
            Node *galaxy = DAT_10255f0c->Find("@galaxy", 0);
            if (galaxy == 0)
            {
                galaxy = DAT_10255f0c->AddChild(0, 0x40);
                galaxy->LoadIt3(FUN_1013f8a0("galaxy.it3", 0), 0, 0);
                strcpy(galaxy->name, "galaxy");
                galaxy->BuildMesh(1, 0);
                galaxy->SetFlags(GALAXY_NODE_HIDDEN);
            }
            DAT_1021c628 = galaxy;
        }
        if (DAT_10255f08 != 0)
        {
            Node *fall = DAT_10255f08->Find("@fall_d", 0);
            if (fall != 0)
                fall->ClearFlags(GALAXY_NODE_FLAG_4);
        }
        goto finish;
    }

    // State 1: turn to the player and start a spinning burst at step 0x1e.
    if (STATE == 1)
    {
        motion = GALAXY_MOTION_IDLE;
        STEP++;
        if (STEP == 1)
            FacePlayer(DAT_10db6ee8[index], toPlayer);
        if (STEP == 0x1e)
        {
            FireEffect(index, 0x519, ownPos, DAT_10db6ee8[index].heading, 0.0f, DAT_10daaae8[index].stats[3], 1.0f, 0, 0, 0);
            goto finish;
        }
        if (STEP > 0x1f && STEP < 0x2f)
        {
            DAT_10db6ee8[index].targetHeading = DAT_10db6ee8[index].heading + 0.41887894f;
            DAT_10db6ee8[index].heading = DAT_10db6ee8[index].heading + 0.41887894f;
        }
        else if (STEP == 0x5a)
            STATE = GALAXY_STATE_CHOOSE;
        goto finish;
    }

    // State 2: after a pause six rings of sparks leave the body at step 0x5a.
    if (STATE == 2)
    {
        STEP++;
        if (STEP == 1)
        {
            FacePlayer(DAT_10db6ee8[index], toPlayer);
            goto finish;
        }
        if (STEP == 0x1e)
        {
            Matrix4 tilt = RotationX(1.5708f);
            Node *root = DAT_10db6ee8[index].root;
            root->matrix = tilt;
            FireEffect(index, 0x3e9, DAT_10db6ee8[index].matrix.GetTranslation(), 0.0f, 0.0f, 0, 1.0f, 0, 0, 0);
            FireEffect(index, 0x3ec, DAT_10db6ee8[index].matrix.GetTranslation(), 0.0f, 0.0f, DAT_10daaae8[index].stats[3], 1.0f, 0, 0, 0x14);
            FUN_10065b10(index, 0x2f8, 1, 0);
            FUN_10054550(0x2f8, 0, &ownPos);
            goto finish;
        }
        if (STEP == 0x5a)
        {
            for (int burst = 0; burst < 6; burst++)
            {
                Vec4 spot = DAT_10db6ee8[index].matrix.GetTranslation();
                if (burst != 0)
                {
                    spot.x = spot.x + ((float)(FUN_1013f880() % 1000) - 800.0f);
                    spot.y = spot.y + ((float)(FUN_1013f880() % 1000) - 800.0f);
                }
                spot.z = 0.0f;
                FireEffect(index, 0x160, spot, 0.0f, 0.0f, DAT_10daaae8[index].stats[3], 0.75f, 5, 0, 0);
            }
            goto finish;
        }
        if (STEP == 0x78)
            STATE = GALAXY_STATE_CHOOSE;
        goto finish;
    }

    // State 3: eight balls leave the body in a ring at step 0x1e.
    if (STATE == 3)
    {
        STEP++;
        FacePlayer(DAT_10db6ee8[index], toPlayer);
        if (STEP == 0x1e)
        {
            float ringAngle = DAT_10db6ee8[index].heading + 2.35619f;
            for (int ball = 4; ball != 0; ball--)
            {
                Matrix4 placement = RotationZ(ringAngle);
                placement.SetTranslation(DAT_10db6ee8[index].matrix.GetTranslation());
                EffectOptions options;
                options.m_0000 = 0;
                options.m_0004 = DAT_10daaae8[index].stats[3];
                options.m_0008 = 0.8f;
                options.owner = GALAXY_FIRST_SLOT;
                StartAt(index, 0x51e, &placement, &options, DAT_10db6ee8[0].headNode, 0);
                if (DAT_10c635f8 < 2)
                    ringAngle = ringAngle + 1.5708f;
                else
                {
                    ringAngle = ringAngle + 0.5236f;
                    Matrix4 second = RotationZ(ringAngle - 0.5236f - 0.5236f);
                    second.SetTranslation(DAT_10db6ee8[index].matrix.GetTranslation());
                    StartAt(index, 0x51e, &second, &options, DAT_10db6ee8[0].headNode, 0);
                    ringAngle = ringAngle + 0.5236f;
                }
            }
            FUN_10065b10(index, 0x239, 1, 0);
            STATE = GALAXY_STATE_CHOOSE;
        }
        goto finish;
    }

    // State 4: a rain of sparks falls on the player for 0x14 frames at a time until step 0x95.
    if (STATE == 4)
    {
        STEP++;
        if (STEP < 0x1e)
        {
            motion = GALAXY_MOTION_WALK;
            FacePlayer(DAT_10db6ee8[index], toPlayer);
            goto finish;
        }
        if (STEP > 0x95)
        {
            motion = GALAXY_MOTION_IDLE;
            STATE = GALAXY_STATE_CHOOSE;
            goto finish;
        }
        motion = GALAXY_MOTION_WALK;
        moveMode = 1;
        step.y = -24.0f;
        step.RotateZ(DAT_10db6ee8[index].heading);
        if (STEP % 0x14 == 0)
        {
            int effect = (FUN_1013f880() & 1) == 0 ? 0x525 : 0x523;
            FireEffect(index, effect, ownPos, DAT_10db6ee8[index].heading, 0.0f, 0, 1.1f, 0, DAT_10db6ee8[0].headNode, 0);
        }
        if (DAT_10db6ee8[index].IsClearOfMapLimited(0) == 0)
            STEP = 0x95;
        goto finish;
    }

    // State 5: a single homing orb at step 1, the state ends at step 0x3c.
    if (STATE == 5)
    {
        STEP++;
        if (STEP != 1)
        {
            if (STEP == 0x3c)
                STATE = GALAXY_STATE_CHOOSE;
            goto finish;
        }
        FacePlayer(DAT_10db6ee8[index], toPlayer);
        Vec4 where = DAT_10db6ee8[index].matrix.GetTranslation();
        where.z += 50.0f;
        FireEffect(index, 0x451, where, DAT_10db6ee8[index].targetHeading, 0.0f, 0, 0.7f, 0, 0, 0);
        FUN_10065b10(index, 0x18d, 1, 0);
        goto finish;
    }

    // State 6: a charge towards the player; at step 0x28 it may chain into another attack.
    if (STATE == 6)
    {
        motion = GALAXY_MOTION_WALK;
        moveMode = 1;
        step.y = -16.0f;
        step.RotateZ(DAT_10db6ee8[index].heading);
        STEP++;
        if (STEP == 1)
        {
            FacePlayer(DAT_10db6ee8[index], toPlayer);
            FireEffect(index, 0x655, ownPos, DAT_10db6ee8[index].targetHeading, 0.0f, 0, 1.0f, 0, spineBone, 0);
        }
        if (STEP < 0x28)
        {
            motion = GALAXY_MOTION_ATTACK;
            if (STEP > 3 && STEP < 0x23)
                DAT_1021f6c0[index] = 1;
            goto finish;
        }
        if (STEP == 0x28)
        {
            int chance = (WORD_COUNT + 1) * 0xf;
            if (FUN_1013f880() % 100 < chance || WORD_DEFEATED != 0)
            {
                WORD_COUNT = 0;
                STATE = GALAXY_STATE_CHOOSE;
                motion = GALAXY_MOTION_IDLE;
            }
            else
                WORD_COUNT++;
            goto finish;
        }
        if (STEP < 0x46)
        {
            if (STEP < 0x2d && DAT_10db6ee8[index].m_238c != 0)
            {
                DAT_10db6ee8[index].AddMotionSpeed(GALAXY_MOTION_SPEED);
                FUN_10065b10(index, GALAXY_SOUND_FLASH, 1, 0);
            }
            DAT_10db6ee8[index].alpha = (float)abs(STEP - 0x37) / GALAXY_FADE_FRAMES;
            if (STEP == 0x37)
            {
                // Reappear at a random spot near the player.
                Vec4 spot;
                spot.y = (float)(FUN_1013f880() % 0xfa) + 200.0f;
                spot.RotateZ((float)(FUN_1013f880() % 0x168));
                Vec4 target = playerPos + spot;
                if (spot.Length2D() > 1600.0f)
                {
                    spot.Fill(0.0f);
                    spot.y = (float)(FUN_1013f880() % 0x640);
                    spot.RotateZ((float)(FUN_1013f880() % 0x168));
                    target = playerPos - spot;
                }
                DAT_10db6ee8[index].matrix.translation.x = spot.x + toPlayer.x;
                DAT_10db6ee8[index].matrix.translation.y = spot.y + toPlayer.y;
                DAT_10db6ee8[index].targetHeading = (float)FUN_10001f80(-spot.x, -spot.y) + GALAXY_PI;
                DAT_10db6ee8[index].heading = DAT_10db6ee8[index].targetHeading;
            }
        }
        else if (STEP == 0x46)
        {
            DAT_10db6ee8[index].alpha = 1.0f;
            STEP = 0;
        }
        goto finish;
    }

    // State 7: the Z slash (a whirl of slashes at step 0x1d..0x32, a spin and a burst of balls).
    if (STATE == GALAXY_ATTACK_Z_SLASH)
    {
        STEP++;
        WORD_FLASH = GALAXY_FLASH_FRAMES;
        if (STEP == 1)
        {
            FacePlayer(DAT_10db6ee8[index], toPlayer);
            FUN_10056260(index, GALAXY_LINE_TAKE_THIS, GALAXY_LINE_Z_SLASH, "", "");
            motion = GALAXY_MOTION_WALK;
            goto finish;
        }
        if (STEP < 0x1e)
        {
            if (STEP == 0x1d)
                FUN_10065b10(index, 0x22b, 0, 0);
            goto finish;
        }
        if (STEP < 0x33)
        {
            if ((STEP - 1 - 0x1d) % 3 == 0)
            {
                if (STEP == 0x24)
                {
                    DAT_10db6ee8[index].heading = DAT_10db6ee8[index].targetHeading + 2.3004313f;
                    DAT_10db6ee8[index].targetHeading = DAT_10db6ee8[index].heading;
                }
                else if (STEP == 0x2d)
                {
                    DAT_10db6ee8[index].heading = DAT_10db6ee8[index].targetHeading - 2.3004313f;
                    DAT_10db6ee8[index].targetHeading = DAT_10db6ee8[index].heading;
                }
                FireEffect(index, 0x750, ownPos, DAT_10db6ee8[index].targetHeading, 5.0f, DAT_10daaae8[index].stats[3], 2.0f, 5, 0, 0);
            }
            step.y = -200.0f;
            moveMode = 2;
            step.RotateZ(DAT_10db6ee8[index].heading);
            goto finish;
        }
        if (STEP == 0x4b)
        {
            STATE = GALAXY_STATE_CHOOSE;
            motion = GALAXY_MOTION_IDLE;
        }
        goto finish;
    }

    // State 8: the final spark. It leaps (steps 0xf..0x2c), lands with a shock wave, may chain the flame attack (step
    // 0x40) and the cold upper (step 0x69), then rains whirling slashes (0x78) and ends at step 0xaa.
    if (STATE == GALAXY_ATTACK_FINAL_SPARK)
    {
        STEP++;
        if (DAT_10daaae8[index].stats[STAT_HEALTH] > 0)
            DAT_10db6ee8[index].mode = 0;
        WORD_FLASH = GALAXY_FLASH_FRAMES;
        if (STEP == 1)
        {
            FUN_10056260(index, GALAXY_LINE_FINAL_SPARK, "", "", "");
            motion = GALAXY_MOTION_IDLE;
            goto finish;
        }
        if (STEP < 0xf)
        {
            FacePlayer(DAT_10db6ee8[index], toPlayer);
            motion = GALAXY_MOTION_IDLE;
            FlashGrey();
            goto finish;
        }
        if (STEP == 0xf)
        {
            DAT_10255160[index][1] = 60.0f;
            motion = GALAXY_MOTION_IDLE;
            Vec4 where = DAT_10db6ee8[index].matrix.GetTranslation();
            where.z = 0.0f;
            FireEffect(index, 0x754, ownPos, 0.0f, 0.0f, DAT_10daaae8[index].stats[3], 1.8f, 0, 0, 2);
            goto finish;
        }
        if (STEP < 0x2d)
        {
            FacePlayer(DAT_10db6ee8[index], toPlayer);
            step.z = DAT_10255160[index][1];
            moveMode = 2;
            DAT_10255160[index][1] = DAT_10255160[index][1] - 4.0f;
            motion = GALAXY_MOTION_IDLE;
            if (STEP >= 0x1a && STEP <= 0x1e)
                DAT_10db6ee8[index].root->matrix = RotationX((float)(STEP - 0x19) * -0.62832f);
            goto finish;
        }
        if (STEP > 0x3f)
        {
            if (STEP == 0x40)
            {
                if (FUN_1013f880() % 3 == 0)
                    STEP = 1000;
                else
                {
                    FlashGrey();
                    FUN_10056260(index, GALAXY_LINE_FLAME_ATTACK, "", "", "");
                    FacePlayer(DAT_10db6ee8[index], toPlayer);
                    moveMode = 2;
                    step.z = -DAT_10db6ee8[index].matrix.translation.z;
                }
                goto finish;
            }
            if (STEP < 0x4b)
            {
                if (STEP == 0x41)
                {
                    FireEffect(index, 0x756, ownPos, 0.0f, 0.0f, DAT_10daaae8[index].stats[3], 1.9f, 5, spineBone, 0);
                    FUN_10065b10(index, 0x22d, 0, 0);
                }
                goto finish;
            }
            if (STEP > 0x68)
            {
                if (STEP == 0x69)
                {
                    if (FUN_1013f880() % 3 == 0)
                        STEP = 1000;
                    else
                    {
                        FlashGrey();
                        FacePlayer(DAT_10db6ee8[index], toPlayer);
                        FUN_10056260(index, GALAXY_LINE_COLD_UPPER, "", "", "");
                    }
                    goto finish;
                }
                if (STEP < 0xaa)
                {
                    if (STEP == 0x78)
                    {
                        // Three whirls of slashes, one frame after the other.
                        Vec4 offset;
                        offset.y = -200.0f;
                        offset.RotateZ(DAT_10db6ee8[index].heading);
                        Vec4 spot = ownPos + offset;
                        FireEffect(index, 0x71b, spot, 0.0f, 0.0f, DAT_10daaae8[index].stats[3], 2.2f, 0, 0, 0);
                        spot = DAT_10db6ee8[0].matrix.GetTranslation() + offset;
                        FireEffect(index, 0x71b, spot, 0.0f, 0.0f, DAT_10daaae8[index].stats[3], 2.2f, 0, 0, 7);
                        FUN_1003ad20(0x71b, &spot, 1.0f);
                        spot = DAT_10db6ee8[0].matrix.GetTranslation() + offset;
                        FireEffect(index, 0x71b, spot, 0.0f, 0.0f, DAT_10daaae8[index].stats[3], 2.2f, 0, 0, 0xe);
                        FUN_1003ad20(0x71b, &spot, 1.0f);
                    }
                    goto finish;
                }
                FlashWhite(0xf);
                STATE = GALAXY_STATE_CHOOSE;
                DAT_10db6ee8[index].mode = 1;
                DAT_10db6ee8[index].m_238c = 1;
                step.z = -DAT_10db6ee8[index].matrix.translation.z;
                moveMode = 2;
                motion = GALAXY_MOTION_IDLE;
                if (DAT_104bfb08[index] == 3)
                    DAT_10467c20[index] = 0x3c;
                goto finish;
            }
            // Steps 0x4b..0x68: the upper cut, tilting the body up and down while it flies.
            motion = GALAXY_MOTION_IDLE;
            step.y = -50.0f;
            if (STEP < 0x50)
            {
                DAT_10db6ee8[index].root->matrix = RotationX((float)(STEP - 0x4a) * -0.31416f);
                step.z = 20.0f;
            }
            if (STEP >= 100)
            {
                DAT_10db6ee8[index].root->matrix = RotationX((float)(0x68 - STEP) * -0.31416f);
                step.z = -20.0f;
            }
            if (STEP < 0x52)
                step.y = (1.0f / (float)(0x52 - STEP)) * step.y;
            else if (STEP > 0x62)
                step.y = (1.0f / (float)(STEP - 0x62)) * step.y;
            if (STEP % 3 == 0)
                FireEffect(index, 0x600, ownPos, 0.0f, 0.0f, 0, 1.0f, 0, 0, 0);
            moveMode = 2;
            step.RotateZ(DAT_10db6ee8[index].heading);
            goto finish;
        }
        // Steps 0x2d..0x3f: the landing, the ground shakes and the body slides back.
        if (STEP == 0x2e)
        {
            DAT_10255160[index][1] = DAT_10255160[index][1] * -0.5f;
            FUN_10055ee0();
            FUN_1016b230("");
        }
        motion = GALAXY_MOTION_IDLE;
        moveMode = 2;
        step.z = DAT_10255160[index][1];
        if (STEP > 0x2e)
        {
            step.y = -10.0f;
            step.RotateZ(DAT_10db6ee8[index].heading);
        }
        DAT_10255160[index][1] = DAT_10255160[index][1] - 4.0f;
        if (STEP <= 0x32)
            DAT_10db6ee8[index].root->matrix = RotationX((float)(0x32 - STEP) * -0.62832f);
        goto finish;
    }

    // State 0x14: summons a helper enemy (kind 0x32 in the first free DAT_10daaae8[index]) with a quarter of this one's health.
    if (STATE == GALAXY_STATE_SPAWN_HELPER)
    {
        STEP++;
        if (STEP == 1)
        {
            int helper = FUN_10050510(ownPos.x, ownPos.y, ownPos.z, GALAXY_KIND, 0);
            if (helper > 0)
            {
                if (WORD_ANGRY == 0)
                    DAT_10daaae8[helper].stats[STAT_HEALTH] = DAT_10daaae8[helper].stats[STAT_MAX_HEALTH] / 4;
                else
                    DAT_10daaae8[helper].stats[STAT_HEALTH] = DAT_10daaae8[helper].stats[STAT_MAX_HEALTH];
                DAT_10daaae8[helper].stats[GALAXY_SUMMON_HEALTH_STAT] = -0x14;
                DAT_10daaae8[helper].stats[STAT_EFFECT_PARAM] = (int)((float)DAT_10daaae8[index].stats[STAT_EFFECT_PARAM] * GALAXY_STAT_SCALE + 0.5f);
                WORD_COUNT = FUN_1013f880() & 1;
                STEP = 0;
                STATE = GALAXY_STATE_HELPER_WAIT;
                DAT_1021f940[index] = 0x1e;
                DAT_10db6ee8[index].locked = 1;
                DAT_10daaae8[index].m_0141 = 1;
                FUN_1001dc20(helper);
                Char_UpdateByKind(helper);
                DAT_1021f940[helper] = 0x1d;
                DAT_1020d540[helper] = 1;
                DAT_1020b430[helper].words[5] = WORD_COUNT == 0;
                DAT_10db6ee8[helper].locked = 1;
                DAT_10db6ee8[helper].heading = DAT_10db6ee8[index].heading;
                DAT_10db6ee8[helper].targetHeading = DAT_10db6ee8[index].targetHeading;
                DAT_10daaae8[helper].m_0141 = 1;
                DAT_10daaae8[helper].coins = 0;
                DAT_1021b4c8[helper] = (char)(helper % 5);
                DAT_102199b8[helper] = 2;
                DAT_10db6ee8[helper].alpha = 1.0f;
                DAT_10db6ee8[helper].shadowOpacity = 0xf;
                FireEffect(index, 0x5fc, ownPos, 0.0f, 0.0f, 0, 1.0f, 0, 0, 0);
                FUN_10065b10(index, 0x18c, 1, 0);
            }
        }
        if (STEP == 2)
            STATE = GALAXY_STATE_CHOOSE;
        goto finish;
    }

    // State 0x15: the summoned helper walks in and then waits for its turn.
    if (STATE == GALAXY_STATE_HELPER_WAIT)
    {
        int before = STEP;
        STEP++;
        if (STEP >= 0x3d && STEP < 0x5a)
        {
            moveMode = 1;
            step.y = -30.0f / (float)(before - 0x3b);
            step.RotateZ(DAT_10db6ee8[index].heading);
            DAT_10daaae8[index].m_0141 = 0;
            DAT_10db6ee8[index].locked = 0;
            goto finish;
        }
        if (STEP == 0x5a)
            STATE = GALAXY_STATE_CHOOSE;
        goto finish;
    }

    // State 0x1e: the body is beaten (or called away): it fades out, waits outside the scene and returns.
    if (STATE == GALAXY_STATE_DEFEATED)
    {
        STEP++;
        if (WORD_IDLE_FRAMES > 0xa8c)
            WORD_IDLE_FRAMES = 0;
        if (STEP < 5 && DAT_10db6ee8[index].m_238c != 0)
        {
            DAT_10db6ee8[index].AddMotionSpeed(GALAXY_MOTION_SPEED);
            FUN_10065b10(index, GALAXY_SOUND_FLASH, 1, 0);
        }
        if (STEP < 0xf)
            DAT_10db6ee8[index].alpha = (float)abs(STEP - 0xf) / GALAXY_FADE_FRAMES;
        else if (DAT_1020c2b0 == 0x1f)
        {
            DAT_10db6ee8[index].alpha = 0.0f;
            WORD_GONE = 1;
            DAT_10daaae8[index].stats[STAT_BUSY] = 1;
            DAT_10db6ee8[index].SetPosition(GALAXY_PARKED_X, 0.0f, 0.0f);
            if (DAT_10daaae8[index].stats[STAT_HEALTH] > 0)
                DAT_10db6ee8[index].mode = 0;
        }
        if (WORD_DEFEATED == 0 && STEP > 0xf)
        {
            WORD_GONE = 0;
            STATE = GALAXY_STATE_RETURN;
            STEP = 0;
        }
        goto finish;
    }

    // State 0x1f: the finish. Slot 0x3a gives the order to every body; all of them fade out and wait outside.
    if (STATE == GALAXY_STATE_FINISH)
    {
        STEP++;
        if (STEP == 1)
        {
            if ((WORD_ANGRY == 0 && DAT_10daaae8[index].stats[STAT_HEALTH] < DAT_10daaae8[index].stats[STAT_MAX_HEALTH] / 2) ||
                (DAT_10db6ee8[index + 2].matrix.translation.x >= GALAXY_OFF_SCENE && DAT_10210d30[index + 2] == 0 &&
                 DAT_10db6ee8[index + 4].matrix.translation.x >= GALAXY_OFF_SCENE && DAT_10210d30[index + 4] == 0))
            {
                BODY_WORD(index + 2, 7) = 1;
                BODY_WORD(index + 4, 7) = 1;
            }
            else
            {
                STATE = GALAXY_STATE_CHOOSE;
                WORD_LAST_STATE = 0;
                WORD_DEFEATED = 0;
                WORD_GONE = 0;
            }
        }
        if (STATE != GALAXY_STATE_CHOOSE)
        {
            if (STEP > 0xf)
            {
                int health = DAT_10daaae8[index].stats[STAT_HEALTH];
                if (health > 0)
                    DAT_10db6ee8[index].mode = 0;
                if (WORD_ANGRY == 0 && health < DAT_10daaae8[index].stats[STAT_MAX_HEALTH] / 2)
                {
                    bool secondOut = BODY_WORD(index + 2, 8) != 0 || DAT_10210d30[index + 2] == 2 || DAT_10210d30[index + 2] > 4 ||
                                     DAT_10db6ee8[index + 2].matrix.translation.x < GALAXY_OFF_SCENE;
                    bool thirdOut = BODY_WORD(index + 4, 8) != 0 || DAT_10210d30[index + 4] == 2 || DAT_10210d30[index + 4] > 4 ||
                                    DAT_10db6ee8[index + 4].matrix.translation.x < GALAXY_OFF_SCENE;
                    if (secondOut && thirdOut)
                        STATE = GALAXY_STATE_CHOOSE;
                }
                else
                {
                    if ((BODY_WORD(index + 2, 8) == 0 && DAT_10db6ee8[index + 2].matrix.translation.x < GALAXY_OFF_SCENE) ||
                        DAT_10210d30[index + 2] != 0 ||
                        (BODY_WORD(index + 4, 8) == 0 && DAT_10db6ee8[index + 4].matrix.translation.x < GALAXY_OFF_SCENE) ||
                        DAT_10210d30[index + 4] != 0 || STEP > 0x1c2)
                    {
                        WORD_DEFEATED = 0;
                        WORD_GONE = 0;
                        BODY_WORD(index + 2, 7) = 0;
                        BODY_WORD(index + 2, 8) = 0;
                        BODY_WORD(index + 4, 7) = 0;
                        BODY_WORD(index + 4, 8) = 0;
                        STEP = 0;
                        STATE = GALAXY_STATE_RETURN;
                    }
                    if (BODY_WORD(index + 2, 8) != 0 && BODY_WORD(index + 4, 8) != 0)
                        STATE = GALAXY_STATE_CHOOSE;
                }
            }
            if (STEP < 5 && DAT_10db6ee8[index].m_238c != 0)
            {
                DAT_10db6ee8[index].AddMotionSpeed(GALAXY_MOTION_SPEED);
                FUN_10065b10(index, GALAXY_SOUND_FLASH, 1, 0);
            }
            if (STEP < 0xf)
            {
                DAT_10db6ee8[index].alpha = (float)abs(STEP - 0xf) / GALAXY_FADE_FRAMES;
                goto finish;
            }
            if (STATE == GALAXY_STATE_FINISH)
            {
                DAT_10daaae8[index].stats[STAT_BUSY] = 1;
                DAT_10db6ee8[index].alpha = 0.0f;
                WORD_GONE = 1;
                DAT_10db6ee8[index].SetPosition(GALAXY_PARKED_X, 0.0f, 0.0f);
                goto finish;
            }
        }
        goto finish;
    }

    // State 0x20: the body comes back at a random spot and fades in.
    if (STATE == GALAXY_STATE_RETURN)
    {
        motion = GALAXY_MOTION_IDLE;
        DAT_10daaae8[index].stats[STAT_BUSY] = 0;
        STEP++;
        if (STEP == 1)
        {
            Vec4 spot;
            spot.y = (float)(FUN_1013f880() % 0x640);
            spot.RotateZ((float)FUN_1013f880());
            spot.z = GALAXY_RING_HEIGHT;
            if (DAT_10db6ee8[index].matrix.translation.x > GALAXY_OFF_SCENE)
                DAT_10db6ee8[index].SetPosition(spot.x, spot.y, spot.z);
            DAT_10db6ee8[index].mode = 1;
        }
        if (STEP < 0xf)
            DAT_10db6ee8[index].alpha = (float)STEP / GALAXY_FADE_FRAMES;
        else
        {
            STATE = GALAXY_STATE_CHOOSE;
            DAT_10db6ee8[index].alpha = 1.0f;
            RemoveHelpers(GALAXY_GHOST_KIND, 0);
        }
        goto finish;
    }

    // State 0x29: the three bodies circle the player (steps 0x10..0x10d), then leave in a burst of light.
    if (STATE == GALAXY_STATE_RING_B)
    {
        STEP++;
        WORD_FLASH = GALAXY_FLASH_FRAMES;
        WORD_IDLE_FRAMES = 0;
        if (STEP == 1)
        {
            Vec4 ring[3];
            WORD_ENDED = 0;
            for (int body = 0; body < 3; body++)
                DAT_10db6ee8[index + body * GALAXY_BODY_STRIDE].mode = 1;
            float angle = (float)(FUN_1013f880() % 0x168) * GALAXY_PI / 180.0f;
            DAT_10255160[index][0] = angle;
            PlaceRing(ring, angle);
            MoveBodiesToRing(index, ring);
            for (int body = 0; body < 3; body++)
                DAT_10daaae8[index + body * GALAXY_BODY_STRIDE].stats[STAT_BUSY] = 0;
            FlashWhite(0xf);
            RemoveHelpers(GALAXY_HELPER_KIND, 1);
            WORD_COUNT = 0;
        }
        if (STEP == 0xff)
        {
            // A burst of ten rings of light.
            for (int ring = 10; ring > 0; ring--)
            {
                float baseAngle = (float)(FUN_1013f880() % 0x168) * GALAXY_PI / 180.0f;
                float lift = (float)ring / 10.0f;
                for (int spoke = 0; spoke < 6; spoke++)
                {
                    float radial = (2.0f - lift) * lift;
                    Matrix4 placement = Scaling(radial, radial, (2.0f - radial) * radial) * RotationZ(baseAngle);
                    Vec4 origin;
                    EffectOptions options;
                    placement.translation.z = 5.0f;
                    FUN_1001d8b0(index, 0x531, origin, -1, -1, 0, 0, 0, &placement, 0);
                    baseAngle = baseAngle + 1.0471975f;
                }
            }
        }
        if (STEP < 0xf)
            SetBodyAlpha(index, (float)STEP / GALAXY_FADE_FRAMES);
        else if (STEP == 0xf)
        {
            SetBodyAlpha(index, 1.0f);
            DAT_10db6ee8[index].PlayMotion(GALAXY_MOTION_IDLE, 0, 5, 1.0f);
            DAT_10db6ee8[index + 2].PlayMotion(GALAXY_MOTION_IDLE, 0, 5, 1.0f);
            motion = GALAXY_MOTION_IDLE;
            EffectOptions options;
            if (hard)
                options.m_0004 = 0x41;
            else
                options.m_0004 = (DAT_10c635f8 != 0 ? 0x14 : 0) + 0x19;
            options.m_0000 = 0;
            options.owner = index;
            Vec4 origin;
            FUN_1001d8b0(index, 0x526, origin, -1, -1, 0, 0, 0, 0, &options);
        }
        else if (STEP < 0x10e)
        {
            Vec4 ring[3];
            float angle = (float)STEP / 25.0f * GALAXY_PI / 180.0f + DAT_10255160[index][0];
            DAT_10255160[index][0] = angle;
            PlaceRing(ring, angle);
            for (int body = 0; body < 3; body++)
            {
                DAT_10db6ee8[index + body * GALAXY_BODY_STRIDE].matrix.translation.x = ring[body].x;
                DAT_10db6ee8[index + body * GALAXY_BODY_STRIDE].matrix.translation.y = ring[body].y;
                DAT_10db6ee8[index + body * GALAXY_BODY_STRIDE].PlayMotion(GALAXY_MOTION_WALK, 0, 5, 1.0f);
                DAT_10db6ee8[index + body * GALAXY_BODY_STRIDE].targetHeading = (float)FUN_10001f80(ring[body].x, ring[body].y) + GALAXY_PI;
            }
        }
        if (STEP < 0xff || STEP > 0x10d)
        {
            if (STEP == 0x10e)
            {
                for (int body = 0; body < 3; body++)
                    DAT_10db6ee8[index + body * GALAXY_BODY_STRIDE].SetPosition(GALAXY_PARKED_X, 0.0f, 0.0f);
                for (int body = 0; body < 3; body++)
                    DAT_10db6ee8[index + body * GALAXY_BODY_STRIDE].mode = 0;
                // A shower of effects across the arena.
                int frame = FUN_1013f880() % 0x32;
                for (int burst = 0; burst < 4; burst++)
                {
                    Vec4 spot;
                    spot.x = (float)(FUN_1013f880() % 600) - 300.0f;
                    spot.y = (float)(FUN_1013f880() % 600) - 300.0f;
                    FireEffect(index, 0x160, spot, 0.0f, 0.0f, 0, 1.0f, 5, 0, frame);
                    int span = 400 - frame;
                    if (span < 1)
                        span = 1;
                    else if (span > 0x1e)
                        span = 0x1e;
                    frame = frame + FUN_1013f880() % span;
                }
            }
            else if (STEP == 500)
            {
                WORD_DEFEATED = 0;
                WORD_GONE = 0;
                BODY_WORD(index + 2, 7) = 0;
                BODY_WORD(index + 2, 8) = 0;
                BODY_WORD(index + 4, 7) = 0;
                BODY_WORD(index + 4, 8) = 0;
                STEP = 0;
                STATE = GALAXY_STATE_RETURN;
                WORD_ENDED = 1;
            }
        }
        else
        {
            SlowRestrictedBodies(index, 0x104);
            SlowOtherBodies(index, 0x104);
            SetBodyAlpha(index, (float)(0x10e - STEP) / GALAXY_FADE_FRAMES);
        }
        int summoned = FUN_10050510(10000.0f, 10000.0f, 10000.0f, 0, 1);
        if (summoned > 0)
        {
            if (WORD_COUNT == 0)
            {
                DAT_10edbd58[0].Copy(DAT_118e69e8);
                DAT_10edbd58[0].z = 0.0f;
            }
            WORD_COUNT++;
            Vec4 spot;
            spot.y = -350.0f;
            spot.z = 125.0f;
            float angle = (float)FUN_10001f80(DAT_10edbd58[0].x, DAT_10edbd58[0].y) + GALAXY_PI;
            float distance = DAT_10edbd58[0].Length2D();
            if (distance < 400.0f)
            {
                float z = DAT_10edbd58[0].z;
                DAT_10edbd58[0].z = 0.0f;
                DAT_10edbd58[0].Copy(DAT_10edbd58[0].Normalized3() * 400.0f);
                DAT_10edbd58[0].z = z;
                distance = 400.0f;
            }
            angle = distance / 1400.0f * 1.570796f + angle;
            spot.RotateZ(angle);
            spot = spot + DAT_10edbd58[0];
            if (spot.z > 1500.0f)
                spot.z = 1500.0f;
            DAT_10db6ee8[summoned].SetPosition(spot.x, spot.y, spot.z);
            FireEffect(summoned, 0x3f4, spot, 0.0f, 0.0f, 0, 1.0f, 0, 0, 0);
            DAT_10daaae8[summoned].kind = GALAXY_HELPER_KIND;
        }
        goto finish;
    }

    // State 0x2a: the defeat. The galaxy backdrop is shown, the body is carried through sparks and flashes and
    // the state hands the fight to a fresh round at step 0x27b.
    if (STATE == GALAXY_STATE_ENDING)
    {
        STEP++;
        WORD_FLASH = GALAXY_FLASH_FRAMES;
        Vec4 fromCamera = playerPos - DAT_118e69e8;
        float angle = (float)atan2(fromCamera.x, fromCamera.y) + GALAXY_PI;
        Matrix4 spin = RotationZ(-angle) * RotationX(-0.01f) * RotationZ(angle);
        rootNode->matrix = rootNode->matrix * spin;
        if (STEP == 1)
        {
            DAT_10db6ee8[index].mode = 1;
            DAT_10db6ee8[index].SetPosition(toPlayer.x, toPlayer.y, 0.0f);
            DAT_10db6ee8[index].targetHeading = (float)FUN_10001f80(toPlayer.x, toPlayer.y) + GALAXY_PI;
            DAT_10db6ee8[index].heading = DAT_10db6ee8[index].targetHeading;
            DAT_10daaae8[index].stats[STAT_BUSY] = 1;
            FUN_1000ec70(0xf, 1.0f, 1.0f, 1.0f, 1.0f);
        }
        int frame = STEP;
        if (frame < 0xf)
            DAT_10db6ee8[index].alpha = (float)frame / GALAXY_FADE_FRAMES;
        else if (frame == 0xf)
        {
            DAT_10db6ee8[index].alpha = 1.0f;
            motion = GALAXY_MOTION_IDLE;
            FUN_10056260(index, GALAXY_LINE_NAME, "", "", "");
        }
        else if (frame > 0x54)
        {
            if (frame < 100)
            {
                if (frame < 0x5a && DAT_10db6ee8[index].m_238c != 0)
                {
                    DAT_10db6ee8[index].AddMotionSpeed(GALAXY_MOTION_SPEED);
                    FUN_10065b10(index, GALAXY_SOUND_FLASH, 1, 0);
                }
                DAT_10db6ee8[index].alpha = (float)(100 - STEP) / GALAXY_FADE_FRAMES;
            }
            else if (frame == 100)
            {
                DAT_10db6ee8[index].SetPosition(GALAXY_PARKED_X, 0.0f, 0.0f);
                DAT_10db6ee8[index].mode = 0;
            }
            else if (frame > 0x1c1)
            {
                if (frame < 0x1d1)
                {
                    if (frame == 0x1c2)
                    {
                        DAT_10db6ee8[index].SetPosition(GALAXY_PARKED_X, 0.0f, 0.0f);
                        DAT_10db6ee8[index].mode = 1;
                    }
                    DAT_10db6ee8[index].alpha = (float)(STEP - 0x1c2) / GALAXY_FADE_FRAMES;
                }
                else if (frame == 0x1d1)
                    DAT_10db6ee8[index].alpha = 1.0f;
            }
        }
        frame = STEP;
        if (frame < 0x2d)
        {
            if (frame == 0x1e)
                FUN_1003ad20(0x232, &ownPos, 1.0f);
            if (STEP > 0x1d)
                FUN_1000ec70(0x2d - STEP, 0.0f, 0.0f, 0.0f, 1.0f);
        }
        else if (frame == 0x2d)
        {
            DAT_10da90d0.Set(0.0f, 0.0f, 0.0f, 1.0f);
            DAT_1042da68 = 0;
            ShowGalaxyHideMap(rootNode);
        }
        else if (frame < 0x3c)
            FlashWhite(0xf);
        else if (frame == 0x3c)
        {
            DAT_10da90d0.Set(1.0f, 1.0f, 1.0f, 1.0f);
            DAT_1042da68 = 0;
        }
        else if (frame < 0x1d6)
        {
            if (frame % 0x14 == 0 && frame < 0xf0)
            {
                // A burst of sparks at a random spot, with two rings of six around it.
                float spin = (float)(FUN_1013f880() % 0x168) * GALAXY_PI / 180.0f;
                Vec4 spot;
                spot.y = (float)(FUN_1013f880() % 900);
                spot.RotateZ((float)(FUN_1013f880() % 0x168));
                spot.z = (float)(FUN_1013f880() % 400) - 300.0f;
                spot = spot + toPlayer;
                FireEffect(index, 0x534, spot, spin, 0.0f, (hard ? 0 : 0xf) + 0x19, 1.0f, 0, 0, 0);
                for (int ring = 0; ring < 2; ring++)
                {
                    spin = spin + GALAXY_PI;
                    for (int spoke = 0; spoke < 6; spoke++)
                    {
                        Matrix4 placement;
                        Matrix4 turn = RotationZ((float)spoke * 1.0471975f + spin);
                        placement = ring == 1 ? Scaling(0.9f, 0.9f, 0.9f) * turn : turn;
                        placement.SetTranslation(spot);
                        Vec4 origin;
                        FUN_1001d8b0(index, 0x535, origin, -1, -1, 0, 0, 0, &placement, 0);
                    }
                }
            }
            frame = STEP;
            if (frame > 0x14a)
            {
                FUN_1000ec70(0xf, 1.0f, 0.5f, 0.5f, 1.0f);
                frame = STEP;
                if (frame < 0x1ae && frame % 3 == 0)
                {
                    Vec4 spot;
                    spot.y = (float)(FUN_1013f880() % ((STEP - 0x14a) * 0x11));
                    spot.RotateZ((float)(FUN_1013f880() % 0x168));
                    spot.z = (float)(FUN_1013f880() % 500 - 0xfa);
                    FireEffect(index, 0x53e, spot, 0.0f, 0.0f, 0, 1.0f, 0, 0, 0);
                }
                else if (frame % 3 == 0)
                {
                    Vec4 spot;
                    spot.y = (float)(FUN_1013f880() % 0x6a4);
                    spot.RotateZ((float)(FUN_1013f880() % 0x168));
                    spot.z = (float)(FUN_1013f880() % 500 - 0xfa);
                    FireEffect(index, 0x53f, spot, 0.0f, 0.0f, 0, 1.0f, 0, 0, 0);
                }
                frame = STEP;
            }
            if (frame == 0x14a)
            {
                FireEffect(index, 0x538, ownPos, 0.0f, 0.0f, (hard ? 0 : 0x11) + 0x53, 1.0f, 0, 0, 0);
                FUN_10054550(0x232, 0x78, 0);
            }
        }
        else if (frame < 0x1fe)
            FlashWhite(0xf);
        else if (frame < 0x20d)
            FUN_1000ec70(0x2d5 - frame, 0.0f, 0.0f, 0.0f, 1.0f);
        else if (frame == 0x20d)
        {
            DAT_10da90d0.Set(0.0f, 0.0f, 0.0f, 1.0f);
            DAT_1042da68 = 0;
            ShowMapHideGalaxy(rootNode);
        }
        else if (frame < 0x212)
            FlashWhite(0xf);
        else if (frame == 0x212)
        {
            FlashWhite(0xf);
            FUN_10056260(index, GALAXY_LINE_ENDURED, "", "", "");
        }
        else if (frame > 0x26b)
        {
            if (frame < 0x27b)
            {
                if (frame < 0x271 && DAT_10db6ee8[index].m_238c != 0)
                {
                    DAT_10db6ee8[index].AddMotionSpeed(GALAXY_MOTION_SPEED);
                    FUN_10065b10(index, GALAXY_SOUND_FLASH, 1, 0);
                }
                DAT_10db6ee8[index].alpha = (float)(0x27b - STEP) / GALAXY_FADE_FRAMES;
            }
            else if (frame == 0x27b)
            {
                DAT_10db6ee8[index].SetPosition(GALAXY_PARKED_X, 0.0f, 0.0f);
                DAT_10db6ee8[index].mode = 0;
                DAT_10daaae8[index].stats[STAT_BUSY] = 0;
                WORD_DEFEATED = 0;
                WORD_GONE = 0;
                BODY_WORD(index + 2, 7) = 0;
                BODY_WORD(index + 2, 8) = 0;
                BODY_WORD(index + 4, 7) = 0;
                BODY_WORD(index + 4, 8) = 0;
                STEP = 0;
                STATE = GALAXY_STATE_RETURN;
            }
        }
        goto finish;
    }

    // State 99: a short pause between attacks; the body fades in and out and may jump to a random spot.
    if (STATE == GALAXY_STATE_REST)
    {
        STEP++;
        if (STEP < 5 && DAT_10db6ee8[index].m_238c != 0)
        {
            DAT_10db6ee8[index].AddMotionSpeed(GALAXY_MOTION_SPEED);
            FUN_10065b10(index, GALAXY_SOUND_FLASH, 1, 0);
        }
        int frame = STEP;
        if (frame < 0x1e)
            DAT_10db6ee8[index].alpha = (float)abs(frame - 0xf) / GALAXY_FADE_FRAMES;
        else if (frame == 0x1f)
        {
            DAT_10db6ee8[index].alpha = 1.0f;
            STATE = GALAXY_STATE_CHOOSE;
        }
        if (frame == 0xf)
        {
            Vec4 spot;
            spot.y = (float)(FUN_1013f880() % 0x640);
            spot.RotateZ((float)(FUN_1013f880() % 0x168));
            DAT_10db6ee8[index].matrix.translation.x = spot.x;
            DAT_10db6ee8[index].matrix.translation.y = spot.y;
        }
        goto finish;
    }

    // State 100: choose the next state from the one that just ended.
    if (STATE == GALAXY_STATE_CHOOSE)
    {
        int last = WORD_LAST_STATE;
        WORD_COUNT = 0;
        switch (last)
        {
        default:
            STATE = GALAXY_STATE_REST;
            break;
        case GALAXY_STATE_FINISH:
            if (WORD_ANGRY == 0)
                STATE = GALAXY_STATE_RING_A + (FUN_1013f880() & 1);
            else
                STATE = GALAXY_STATE_RING_A + FUN_1013f880() % 3;
            if (WORD_ANGRY == 0 && DAT_10daaae8[index].stats[STAT_HEALTH] < DAT_10daaae8[index].stats[STAT_MAX_HEALTH] / 2)
            {
                STATE = GALAXY_STATE_ENDING;
                WORD_ANGRY = 1;
            }
            else if (WORD_WEAK != 0)
                STATE = GALAXY_STATE_RING_B;
            break;
        case GALAXY_STATE_RETURN:
        case GALAXY_STATE_REST:
            if (DAT_1020c2dc == 0)
                STATE = FUN_1013f880() % 6 + 1;
            else
            {
                int roll = FUN_1013f880();
                STATE = roll % 10 + 1;
                if (STATE > 8)
                    STATE = roll % 10 - 1;
            }
            WORD_ATTACKS++;
            if (index == GALAXY_FIRST_SLOT)
            {
                bool weak = DAT_1020c2dc == 0 && DAT_10daaae8[index].stats[STAT_HEALTH] < DAT_10daaae8[index].stats[STAT_MAX_HEALTH] / 2;
                if (FUN_1013f880() % 0x32 < DAT_1020c2d4 * 3 || weak || DAT_1020c2e4 > 0xa8c)
                {
                    DAT_1020c2d4 = 0;
                    DAT_1020c354 = 0;
                    DAT_1020c3d4 = 0;
                    DAT_1020c2cc = 1;
                    bool tooLong = DAT_1020c2e4 > 0xa8c;
                    if (tooLong)
                        DAT_1020c2e4 = 900;
                    DAT_1020c2e8 = tooLong;
                    if (weak)
                    {
                        FUN_10056260(GALAXY_FIRST_SLOT, GALAXY_LINE_ANGRY, "", "", "");
                        DAT_10254478 = 0x4b;
                    }
                }
            }
            break;
        }
        if (index == GALAXY_FIRST_SLOT)
        {
            if (DAT_1020c2c0 == 99 && (2 - hard) * 0x96 < DAT_1020c2c8)
            {
                if (DAT_10db6ee8[0x3c].matrix.translation.x < GALAXY_OFF_SCENE || DAT_10210d30[0x3c] == 2)
                    DAT_1020c2c4 = 0x3c;
                else if (DAT_10db6ee8[0x3e].matrix.translation.x < GALAXY_OFF_SCENE || DAT_10210d30[0x3e] == 2)
                    DAT_1020c2c4 = 0x3e;
                DAT_1020c2c8 = 0;
                if (DAT_1020c2c4 != 0)
                    DAT_1020c2b0 = 0x14;
            }
        }
        if (WORD_DEFEATED != 0 && WORD_LAST_STATE != GALAXY_STATE_FINISH)
        {
            if (index == GALAXY_FIRST_SLOT)
            {
                DAT_1020c2b0 = 0x1f;
                DAT_1020c2b4 = 0;
                goto finish;
            }
            STATE = GALAXY_STATE_DEFEATED;
        }
        STEP = 0;
        goto finish;
    }

    // State 0x28: the three bodies take their places on a ring around the player and take turns firing at it.
    if (STATE == GALAXY_STATE_RING_A)
    {
        STEP++;
        if (STEP == 1)
        {
            Vec4 ring[3];
            Vec4 center;
            center.z = GALAXY_RING_HEIGHT;
            for (int body = 0; body < 3; body++)
                DAT_10db6ee8[index + body * GALAXY_BODY_STRIDE].mode = 1;
            float angle = (float)(FUN_1013f880() % 0x168) * GALAXY_PI / 180.0f;
            DAT_10255160[index][0] = angle;
            PlaceRing(ring, angle);
            for (int body = 0; body < 3; body++)
            {
                Vec4 hit;
                if (FUN_100246b0(&ring[body], &center, &hit, 0, 0) != 0)
                {
                    float reach = hit.Length2D();
                    float around = (float)FUN_10001f80(ring[body].x, ring[body].y) + GALAXY_PI;
                    ring[body].x = 0.0f;
                    ring[body].y = -(reach - 100.0f);
                    ring[body].RotateZ(around);
                }
                if (DAT_10db6ee8[index + body * GALAXY_BODY_STRIDE].matrix.translation.x > GALAXY_OFF_SCENE)
                    DAT_10db6ee8[index + body * GALAXY_BODY_STRIDE].SetPosition(ring[body].x, ring[body].y, ring[body].z);
            }
            for (int body = 0; body < 3; body++)
                DAT_10daaae8[index + body * GALAXY_BODY_STRIDE].stats[STAT_BUSY] = 0;
            FlashWhite(0xf);
        }
        if (STEP < 0xf)
            SetBodyAlpha(index, (float)STEP / GALAXY_FADE_FRAMES);
        else if (STEP == 0xf)
        {
            SetBodyAlpha(index, 1.0f);
            DAT_10db6ee8[index].PlayMotion(GALAXY_MOTION_IDLE, 0, 5, 1.0f);
            DAT_10db6ee8[index + 2].PlayMotion(GALAXY_MOTION_IDLE, 0, 5, 1.0f);
            motion = GALAXY_MOTION_IDLE;
        }
        if (STEP >= 0x10 && STEP <= 0x11c)
        {
            Vec4 ring[3];
            Vec4 center;
            center.z = GALAXY_RING_HEIGHT;
            FlashGrey();
            float angle = DAT_10255160[index][0];
            PlaceRing(ring, -angle);
            for (int body = 0; body < 3; body++)
            {
                Vec4 hit;
                if (FUN_100246b0(&ring[body], &center, &hit, 0, 0) != 0)
                {
                    float reach = hit.Length2D() - 100.0f;
                    float around = (float)FUN_10001f80(ring[body].x, ring[body].y) + GALAXY_PI;
                    ring[body].x = 0.0f;
                    ring[body].y = -reach;
                    ring[body].RotateZ(around);
                }
                Vec4 offset = ring[body] - DAT_10db6ee8[index + body * GALAXY_BODY_STRIDE].matrix.GetTranslation();
                offset.z = 0.0f;
                float length = offset.Length2D();
                if (length <= 2.0f)
                    DAT_10db6ee8[index + body * GALAXY_BODY_STRIDE].PlayMotion(GALAXY_MOTION_IDLE, 0, 5, 1.0f);
                else
                {
                    offset = offset.Normalized3() * 12.0f;
                    FUN_100241e0(index + body * GALAXY_BODY_STRIDE, offset.x, offset.y, offset.z);
                    DAT_10db6ee8[index + body * GALAXY_BODY_STRIDE].PlayMotion(GALAXY_MOTION_WALK, 0, 5, 1.0f);
                }
                DAT_10db6ee8[index + body * GALAXY_BODY_STRIDE].targetHeading = (float)FUN_10001f80(ring[body].x, ring[body].y) + GALAXY_PI;
            }
            if (STEP % 0xf == 0)
            {
                int shooter = ((STEP / 0xf) % 3) * 2 + GALAXY_FIRST_SLOT;
                if (DAT_10db6ee8[shooter].matrix.translation.x > GALAXY_OFF_SCENE)
                {
                    Vec4 muzzle = DAT_10db6ee8[shooter].matrix.GetTranslation();
                    Vec4 origin;
                    EffectOptions options;
                    options.m_0000 = 0;
                    options.m_0004 = DAT_10daaae8[index].stats[3];
                    options.m_0008 = 0.9f;
                    options.owner = GALAXY_FIRST_SLOT;
                    options.m_000e = 5;
                    FUN_1001d8b0(index, 0x74f, origin, -1, -1, &DAT_10db6ee8[shooter].root->worldMatrix, 0, 0, 0, 0);
                    switch (FUN_1013f880() % (hard * 2 + 6))
                    {
                    case 0:
                    case 3:
                        FireEffect(index, 0x41a, muzzle, DAT_10db6ee8[shooter].targetHeading, 0.0f, 0, 1.0f, 0, 0, 0);
                        FireEffect(index, 0x422, muzzle, DAT_10db6ee8[shooter].targetHeading, 0.0f, 0, 1.0f, 0, 0, 0);
                        break;
                    case 1:
                    case 4:
                        FireEffect(index, 0x25, muzzle, 0.0f, 50.0f, 0, 1.0f, 0, 0, 0);
                        FireEffect(index, 0x52, muzzle, 0.0f, 50.0f, 0, 1.0f, 0, 0, 0);
                        for (int orb = 0; orb < 5; orb++)
                            FireEffect(index, 0x423, muzzle, (float)orb * 1.256637f + DAT_10db6ee8[shooter].targetHeading, 50.0f, 0, 1.0f, 0, 0, 0);
                        FUN_10065b10(shooter, 0x397, 1, 0);
                        break;
                    case 2:
                    case 7:
                    {
                        Vec4 spot;
                        spot.y = (float)(FUN_1013f880() % 0x15e);
                        spot.RotateZ((float)(FUN_1013f880() % 0x168));
                        spot = spot + toPlayer;
                        spot.z = 0.0f;
                        FUN_10054550(0x71a, 0, &spot);
                        FireEffect(index, 0x71a, spot, 0.0f, 0.0f, 0, 1.0f, 0, 0, 0);
                        break;
                    }
                    case 5:
                    case 6:
                    {
                        Vec4 spot;
                        spot.y = (float)(FUN_1013f880() % 0x15e);
                        spot.RotateZ((float)(FUN_1013f880() % 0x168));
                        spot = spot + toPlayer;
                        spot.z = 0.0f;
                        FireEffect(index, 0x72f, spot, 0.0f, 0.0f, 0, 1.0f, 0, 0, 0);
                        break;
                    }
                    }
                }
            }
        }
        if (STEP >= 0x11d && STEP <= 299)
        {
            FlashWhite(0xf);
            if (STEP < 0x122)
            {
                SlowRestrictedBodies(index, 0x122);
                SlowOtherBodies(index, 0x122);
            }
            SetBodyAlpha(index, (float)(300 - STEP) / GALAXY_FADE_FRAMES);
        }
        else if (STEP == 300)
        {
            WORD_DEFEATED = 0;
            WORD_GONE = 0;
            BODY_WORD(index + 2, 7) = 0;
            BODY_WORD(index + 2, 8) = 0;
            BODY_WORD(index + 4, 7) = 0;
            BODY_WORD(index + 4, 8) = 0;
            STEP = 0;
            STATE = GALAXY_STATE_RETURN;
        }
        goto finish;
    }

finish:
    if (STATE != GALAXY_STATE_CHOOSE)
        DAT_1020b430[index].words[4] = STATE;
    if (motion != -1)
    {
        Playback *playback = DAT_10db6ee8[index].motion;
        if ((playback->layer < 0x5a || playback->blending != 0) && (playback->nextLayer < 0x5a || DAT_104686b0[index] == 0))
            DAT_10db6ee8[index].PlayMotion(motion, 0, 5, 1.0f);
    }
    if (moveMode == 1)
        FUN_100241e0(index, step.x, step.y, step.z);
    else if (moveMode != 0)
        DAT_10db6ee8[index].SetPosition(ownPos.x + step.x, ownPos.y + step.y, ownPos.z + step.z);
    if (DAT_10db6ee8[index].matrix.translation.z < 0.0f)
        DAT_10db6ee8[index].matrix.translation.z = 0.0f;
    return 0;
}
