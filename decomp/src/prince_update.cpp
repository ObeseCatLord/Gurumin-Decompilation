// The prince (kind 0x43), the fighter of the dodge game that wields a swappable weapon (the models O_100..O_103) and a
// gatling gun on a node. Once the game is over (DAT_10210d30) it runs its ending scene (the camera slides along the
// prince while the screen fades, as in the other bosses); otherwise it fights: every few frames it picks a move by
// weighted odds (strafe, lunge, gatling burst, swing, charge, weapon swap, teleport back into the arena) and the moves
// steer a movement vector that is blended and applied at the end of the frame. The arena is the triangle with the corners
// (2150, -620), (-2150, -620) and (0, 3100); leaving it sends the prince back (state 15).

#include <math.h>
#include <windows.h>
#include "vec4.h"
#include "matrix4.h"
#include "node.h"
#include "anim_model.h"
#include "char_slot.h"
#include "char_state.h"
#include "effect_manager.h"
#include "texture_manager.h"
#include "prince.h"
#include "bgm_track.h"
#include "char_attach.h"
#include "dodge_golem.h"
#include "enemy_chase_hop.h"
#include "enemy_fighter.h"
#include "enemy_voice.h"
#include "game.h"
#include "game_main.h"
#include "partner_follow.h"
#include "volume_fade.h"
#include "neo_galaxy.h"
#include "texture_load.h"
#include "hikari_button.h"
#include "hop_spit_enemy.h"
#include "last_position.h"
#include "value_command.h"
#include "camera_vars.h"
#include "char_ground.h"
#include "script_system.h"
#include "char_tables.h"
#include "char_vars.h"
#include "game_vars.h"
#include "map_vars.h"
#include "render_vars.h"
#include "world.h"

#define PRINCE_PI 3.1415f
#define PRINCE_TWO_PI 6.283f
#define PRINCE_SCRIPT_FLAG_DIFFICULTY 0x1c7      // script flag holding the difficulty (0, 1 or 2)
#define PRINCE_FLAG_DEFEAT_MODEL 0x1a4           // script flag holding the model that leaves with the defeat scene
#define PRINCE_ROOM_FLAG_BASE 1000               // + 2 * room: flag word of the ending
#define PRINCE_PEOPLE_FIRST 0x39
#define PRINCE_FAR_X (-200000.0f)
#define PRINCE_ENDING_MODEL_A 88                 // the two models of the ending (char11en.it3 and char11pa.it3)
#define PRINCE_ENDING_MODEL_B 89
#define PRINCE_ENDING_START (-30)
#define PRINCE_SLOT_KIND_FREE_MARK 2             // DAT_1021f3c0 value: the slot is only a marker, the default step runs
#define PRINCE_SLOT_FLAG_SKIP 0x20               // CharSlot::m_0098 bit set while the prince updates
#define PRINCE_FLOOR_HEIGHT 40.0f
#define PRINCE_EFFECT_OWNER_BASE 200             // effect owner = slot + 200
#define PRINCE_UNSET_HEIGHT (-10000)

// States (Kind43State::state).
#define PRINCE_STATE_SET_UP 0
#define PRINCE_STATE_STALK 1                     // walks at the player until the odds say otherwise
#define PRINCE_STATE_STRAFE 2                    // slides sideways along a random heading, facing the player
#define PRINCE_STATE_PUNCH_LEAN 3                // leans back, then punches (motion 0x14, effect 0x7ca)
#define PRINCE_STATE_PUNCH 4
#define PRINCE_STATE_GATLING_SPRAY 5             // fires the gatling in random directions until its chance runs out
#define PRINCE_STATE_GATLING_AIM 6               // leans and holds the gatling at the player
#define PRINCE_STATE_SWING 7                     // weapon swing with a turn towards the player
#define PRINCE_STATE_SWING_FOLLOW 8              // swing that follows the player's heading while it winds up
#define PRINCE_STATE_HIT_RECOVER 9               // knocked back, flies, then lands (motion 0x20)
#define PRINCE_STATE_CHARGE_A 10                 // dash with a picked attack motion (0x22, 0x27..0x29)
#define PRINCE_STATE_CHARGE_B 11                 // the same with the motions 0x1d..0x1f and 0x22
#define PRINCE_STATE_LEAP 12                     // leaps at the player from afar, else a short charge
#define PRINCE_STATE_BOMBARD 13                  // drops effects from the sky (motions 0x16, 0x18)
#define PRINCE_STATE_SWAP_WEAPON 14              // puts a weapon away and takes another (O_101..O_103)
#define PRINCE_STATE_TELEPORT 15                 // fades out, reappears 1000 units in front of the player
#define PRINCE_STATE_LOCK_ON 16                  // the locked arm attack with the node @lk_null (O_108.it3)
#define PRINCE_STATE_PICK 100

// Values of Kind43State::frames that mark the later parts of a state.
#define PRINCE_FRAMES_PART_TWO 1000
#define PRINCE_FRAMES_PART_THREE 2000
#define PRINCE_FRAMES_PART_FOUR 3000

// Movement modes (PrinceFrame::moveMode).
#define PRINCE_MOVE_NONE 0
#define PRINCE_MOVE_WALK 1                       // blend the movement vector and walk by it with collisions
#define PRINCE_MOVE_PLACE 2                      // blend the vector and put the model at own position + vector

// Nodes of the prince in DAT_1021bdf8[slot][...].
#define PRINCE_NODE_GATLING 0                    // "@gatring_null"
#define PRINCE_NODE_ROOT_NULL 1                  // "@s_null"
#define PRINCE_NODE_HAND_R 2                     // "Bip01 R Hand"
#define PRINCE_NODE_HAND_L 3                     // "Bip01 L Hand"
#define PRINCE_NODE_LOCK 4                       // "@lk_null"

#define PRINCE_MOTION_NONE (-1)
#define PRINCE_MOTION_IDLE 4
#define PRINCE_MOTION_BLEND 5                    // Kind43 passes 5 as the blend length of PlayMotion

static inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

// Turns an angle into -pi..pi.
static float WrapAngle(float angle)
{
    while (angle > PRINCE_PI)
        angle -= PRINCE_TWO_PI;
    while (angle < -PRINCE_PI)
        angle += PRINCE_TWO_PI;
    return angle;
}

// The heading that faces the player.
static float FacePlayer(const PrinceFrame &f)
{
    return FUN_10001f80(f.playerPos.x - f.ownPos.x, f.playerPos.y - f.ownPos.y) + PRINCE_PI;
}

static int DifficultyIsHard()
{
    return DAT_10874848.TestScriptFlag(PRINCE_SCRIPT_FLAG_DIFFICULTY) == 1;
}

// The node `which` of the prince, or 0 when it is not attached.
static Node *PrinceNode(int index, int which)
{
    return DAT_1021bdf8[index][which];
}

// The world matrix to attach an effect to: the first of the nodes `first`, `second` and the model root that exists.
static const Matrix4 *EffectAnchor(int index, int first, int second)
{
    Node *node = PrinceNode(index, first);
    if (node == 0)
        node = PrinceNode(index, second);
    if (node == 0)
        node = DAT_10db6ee8[index].root;
    return &node->worldMatrix;
}

// Starts an effect of the prince: `owner` is the matrix it follows, `value` and `frame` are the effect's own numbers.
static void StartEffect(int index, int effect, const Vec4 &where, const Matrix4 *owner, int value, int frame,
                        const Matrix4 *pose, const EffectOptions *options)
{
    FUN_1001d8b0(index, effect, where, -1, -1, owner, value, frame, pose, options);
}

// The options of a hand or weapon effect: the effect parameter of the prince, a scale and the follow-owner flag.
static void SetUpOptions(int index, EffectOptions &options, float scale, int followOwner)
{
    options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
    options.m_0008 = scale;
    if (followOwner)
    {
        options.flags |= 2;
        options.m_000e = 5;
    }
}

// Links the two nodes with a lightning effect (a beam from the first to the second) every 5th frame of the prince's age.
static void StartLinkEffect(int index, int effect, const Vec4 &where, const Node *from, const Node *to,
                            EffectOptions &options)
{
    if (DAT_1020b430[index].kind43.age % 5 == 0)
    {
        options.owner = index + PRINCE_EFFECT_OWNER_BASE;
        FUN_1001d8b0(index, effect, where, -1, -1, &from->worldMatrix, (int)&to->worldMatrix, 0, 0, &options);
    }
}

// Says voice line `sound` with a random choice of the four prince lines (0x47..0x4a).
static void SayRandomLine(int index)
{
    FUN_100b9bb0(index, (FUN_1013f880() & 3) + 0x47, 0, 0);
}

// Turns the root node of the prince about x by `lean` (the body bends towards the player while it fires or swings).
static void LeanBody(int index, float lean)
{
    DAT_10db6ee8[index].root->matrix = RotationX(-lean);
}

// Moves `value` towards `goal` by `step` per frame; returns 1 once it has reached it.
static int ApproachValue(float &value, float goal, float step)
{
    if (goal > value)
    {
        value += step;
        if (goal < value)
        {
            value = goal;
            return 1;
        }
    }
    else if (goal < value)
    {
        value -= step;
        if (value < goal)
        {
            value = goal;
            return 1;
        }
    }
    return 0;
}

// The three corners of the arena and the test whether a position is outside the edge from `a` to `b`.
static int OutsideEdge(const Vec4 &position, float ax, float ay, float bx, float by)
{
    float cross = (bx - ax) * (position.y - ay) - (by - ay) * (position.x - ax);
    return cross > 0.0f;
}

static int CountEdgesOutsideArena(const Vec4 &position)
{
    int outside = 0;
    if (OutsideEdge(position, 2150.0f, -620.0f, -2150.0f, -620.0f))
        outside++;
    if (OutsideEdge(position, -2150.0f, -620.0f, 0.0f, 3100.0f))
        outside++;
    if (OutsideEdge(position, 0.0f, 3100.0f, 2150.0f, -620.0f))
        outside++;
    return outside;
}

// The glow rings of the weapon: effect `effect` at the end node of the weapon (or its hand, or the root) every
// tenth frame of the state, scaled by `scale`.
static void StartWeaponGlow(PrinceFrame &f, int effect, float scale)
{
    EffectOptions options;
    Matrix4 pose;
    Vec4 where;
    where.Fill(0.0f);
    pose.SetIdentity();
    SetUpOptions(f.index, options, scale, 1);
    options.owner = f.index + PRINCE_EFFECT_OWNER_BASE;
    StartEffect(f.index, effect, where, EffectAnchor(f.index, PRINCE_NODE_ROOT_NULL, PRINCE_NODE_HAND_R), 0, 0, 0,
                &options);
}

// The ending scene (DAT_10210d30 != 0 while the prince is not healing). The camera slides along the prince while the
// screen fades and the models char11en/char11pa take over; the age counts from -30 (PRINCE_ENDING_START). Returns 1.
static int RunEnding(int index, PrinceFrame &f)
{
    AnimModel &model = *f.model;
    CharSlot &slot = *f.slot;
    Kind43State &st = *f.st;
    AnimModel &endingA = DAT_10db6ee8[PRINCE_ENDING_MODEL_A];
    AnimModel &endingB = DAT_10db6ee8[PRINCE_ENDING_MODEL_B];
    int room = DAT_10c63ffc;
    int roomFlags = DAT_10874848.TestScriptFlag(room * 2 + PRINCE_ROOM_FLAG_BASE);
    DAT_10874848.SetScriptFlag(room * 2 + PRINCE_ROOM_FLAG_BASE, roomFlags | (1 << (index - PRINCE_PEOPLE_FIRST)));
    DAT_10868838[index][0].m[12] *= 0.95f;
    DAT_10868838[index][0].m[13] *= 0.95f;
    DAT_10868838[index][0].m[14] -= 10.0f;
    DAT_10868838[index][1].m[12] *= 0.95f;
    DAT_10868838[index][1].m[13] *= 0.95f;
    DAT_10868838[index][1].m[14] -= 10.0f;
    model.m_17e8 = 1;
    endingA.m_17e8 = 1;
    endingB.m_17e8 = 1;
    DAT_104c0354 = 2;
    slot.m_0141 = 1;
    slot.stats[STAT_EFFECT_PARAM] = 0;
    int timer = st.age;
    if (timer == 1)
    {
        FUN_1000ec70(0xf, 1.0f, 1.0f, 1.0f, 1.0f);
        Vec4 spot = model.matrix.GetTranslation();
        spot.z += 50.0f;
        StartEffect(index, 0x33, spot, 0, 0, 0, 0, 0);
        model.PlayMotion(0x23, 1, 5, 1.0f);
        model.mode = 0;
        endingA.mode = 0;
        endingB.mode = 0;
        endingA.m_17e8 = 1;
        endingB.m_17e8 = 1;
        DAT_10daaa80.RemoveOwnedBy(index + PRINCE_EFFECT_OWNER_BASE);
    }
    else if (timer < 1 && timer % 10 == 0)
    {
        Vec4 spot = model.matrix.GetTranslation();
        spot.z += 50.0f;
        for (int burst = 0; burst < 5; burst++)
            StartEffect(index, 0x2580, spot, 0, 0, 0, 0, 0);
    }
    if (timer == 0x10e)
    {
        FUN_1000db30(0, 0, 0);
        FUN_1000db30(0, 100, 0x1e);
    }
    else if (timer == 300)
    {
        DAT_10874848.MarkKilled(slot.routeRecord);
        FUN_1000ec70(0xf, 1.0f, 1.0f, 1.0f, 1.0f);
        model.SetPosition(PRINCE_FAR_X, 0.0f, 0.0f);
        endingA.SetPosition(PRINCE_FAR_X, 0.0f, 0.0f);
        endingB.SetPosition(PRINCE_FAR_X, 0.0f, 0.0f);
        DAT_104c00c8 = 0;
        DAT_104c0110 = 0;
        FUN_1013fb10(3, 0, 10);
        DAT_104c0354 = 0;
    }
    if (st.syncedFlag == 0)
    {
        model.holding = 0;
        model.PlayMotion(2, 0, 5, 1.0f);
        FUN_10065b10(index, 0x2d1, 1, 0);
        if (DAT_104bfe78 == 0)
            st.syncedFlag = 1;
        endingA.targetHeading = model.targetHeading;
        endingB.targetHeading = model.targetHeading;
        endingA.heading = model.heading;
        endingB.heading = model.heading;
        st.age = PRINCE_ENDING_START;
        endingA.m_0000.Copy(DAT_118e69e8);
        endingB.m_0000.Copy(DAT_118e6cb8);
        DAT_10255160[index][0] = (f.ownPos - DAT_118e6cb8).Length();
        model.root->M_101946b0(0, 0xffffffff);
        endingA.root->M_101946b0(0, 0xffffffff);
        endingB.root->M_101946b0(0, 0xffffffff);
        model.alpha = 1.0f;
        DAT_104c00c8 = 1;
        DAT_10453008[index] = -10001;
        slot.stats[STAT_EFFECT_PARAM] = 0;
        endingA.alpha = 0.0f;
        endingB.alpha = 0.0f;
    }
    timer = st.age;
    if (timer > 0x3c)
    {
        if (timer < 0xb4)
        {
            float left = (float)(0xb4 - timer);
            model.alpha = (left * left) / 14400.0f;
        }
        else
        {
            model.m_17e8 = 1;
            model.holding = 1;
            model.motionFrame = endingA.motionFrame;
            model.alpha = 0.0f;
        }
    }
    if (timer >= 0x3d && timer < 0x8c)
    {
        float lift = (float)(0x28 - (timer > 100 ? timer - 100 : 100 - timer));
        model.SetPosition(model.matrix.translation.x, model.matrix.translation.y,
                          model.matrix.translation.z + (lift * 5.0f) / 40.0f);
    }
    else if (timer >= 0x8c)
    {
        model.SetPosition(model.matrix.translation.x, model.matrix.translation.y, model.matrix.translation.z);
        model.m_17e8 = 1;
    }
    if (timer == 0x3c)
    {
        FUN_1013fb10(3, 200, 0x1e);
        FUN_100b9bb0(index, 0x4f, 1, 0);
    }
    timer = st.age;
    if (timer > 0x3c)
    {
        if (timer < 0x96)
        {
            float since = (float)(timer - 0x3c);
            endingA.scale.Set(1.0f, 0.0f, 0.0f, (since * since) / 8100.0f);
        }
        else if (timer < 0xb4)
            endingA.scale.Set(1.0f, 0.0f, 0.0f, 1.0f);
        else if (timer < 0x102)
        {
            if (timer == 0xb4)
            {
                FUN_10065b10(index, 0x2d2, 1, 0);
                if (DAT_10c64768 != 0)
                {
                    Script_ValueCommand(0x7549, 0x4b);
                    int modelId = DAT_10874848.TestScriptFlag(PRINCE_FLAG_DEFEAT_MODEL);
                    if (modelId != 0)
                    {
                        DAT_10db6ee8[modelId].SetPosition(-100000.0f - model.matrix.translation.x,
                                                          model.matrix.translation.y, model.matrix.translation.z);
                        DAT_10c6356c = 100000;
                        DAT_10c63588 = 7;
                        Script_ValueCommand(0x753f, 0x4b);
                    }
                }
            }
            float fade = (float)(0x102 - timer);
            float rise = (float)(timer - 200);
            endingA.scale.Set(1.0f, endingA.scale.y + fade * fade * 0.004213826f,
                              endingA.scale.z - rise * rise * 0.006584103f,
                              endingA.scale.w - (float)(timer - 0xb4) / 3081.0f);
            if (endingA.scale.w < 0.0f)
                endingA.scale.w = 0.0f;
        }
        else
            endingA.scale.Fill(0.0f);
    }
    timer = st.age;
    if (timer > 0x83)
    {
        if (timer < 0xb8)
        {
            float since = (float)(timer - 0x84);
            endingB.scale.z = 0.0f;
            endingB.scale.x = (float)(0xb8 - timer) * 0.0001451379f + endingB.scale.x;
            endingB.scale.y = since / 16.0f;
            endingB.scale.w = since / 208.0f + 0.25f;
            if (endingB.scale.x < 0.0f)
                endingB.scale.x = 0.0f;
        }
        else if (st.engaged < 300)     // sic: the original tests this word, not the age
        {
            if (st.engaged == 0xb8)
                endingB.scale.x = 0.3f;
            float since = (float)(timer - 0xb8);
            float left = (float)(300 - timer);
            endingB.scale.x = endingB.scale.x - since * 2.947244e-05f;
            endingB.scale.y = left * left * 0.0008686249f + endingB.scale.y;
            endingB.scale.z = since * since * 0.001357226f + endingB.scale.z;
            endingB.scale.w = (left * left * 0.5f) / 13456.0f;
            if (endingB.scale.x < 0.0f)
                endingB.scale.x = 0.0f;
            endingB.heading = endingB.heading - since * 5e-05f;
            endingB.targetHeading = endingB.heading;
        }
        else
            endingB.scale.Fill(0.0f);
    }
    endingA.m_0294 = 1;
    endingB.m_0294 = 1;
    endingA.motionFrame = model.motionFrame;
    st.age++;
    endingB.motionFrame = model.motionFrame;
    endingA.SetPosition(model.matrix.translation.x, model.matrix.translation.y, model.matrix.translation.z);
    endingB.SetPosition(model.matrix.translation.x, model.matrix.translation.y, model.matrix.translation.z);
    endingA.alpha = endingA.scale.w;
    endingB.alpha = endingB.scale.w;
    Vec4 look;
    look.Fill(0.0f);
    look.y += 950.0f;
    look.z += 200.0f;
    float spin = (float)(st.age + 0x1e);
    look.RotateZ((-(Atan2(DAT_10868818.x - DAT_10da9150.x, DAT_10868818.y - DAT_10da9150.y) + PRINCE_PI) -
                  spin * spin * 3e-05f) - 0.04363f);
    DAT_10edbd48.Copy(model.matrix.GetTranslation());
    DAT_10866d98.Copy(DAT_10edbd48 + look * 1.3f);
    if (st.age < 0xa1)
    {
        float k = (float)(0xa0 - st.age);
        k = (k + 1.0f) * k * 0.5f;
        k = k * k;
        float blend = 1.0f - k / 3.292411e+08f;
        DAT_10866d98.Copy(DAT_10da9150 + (DAT_10866d98 - DAT_10da9150) * blend);
        DAT_10edbd48.Copy(DAT_10868818 + (DAT_10edbd48 - DAT_10868818) * blend);
    }
    float distance = (DAT_10edbd48 - DAT_10866d98).Length();
    if (distance <= DAT_10255160[index][0])
        DAT_10255160[index][0] = distance;
    else
    {
        DAT_10255160[index][0] = (distance - DAT_10255160[index][0]) * 0.3f + DAT_10255160[index][0];
        Vec4 away = DAT_10edbd48 - DAT_10866d98;
        away.Copy(away.Normalized3() * DAT_10255160[index][0]);
        DAT_10866d98.Copy(DAT_10edbd48 - away);
    }
    DAT_1020344c = 1.0f;
    DAT_1101dddc = 0.0f;
    return 1;
}

// An effect at the right hand of the prince: `offset` is the position in the hand's frame (turned by the lean and the
// heading of the prince), `tilt` the lean of the body, `scale` the effect scale.
static void StartHandEffect(PrinceFrame &f, int effect, const Vec4 &offset, float tilt, float scale)
{
    Node *hand = PrinceNode(f.index, PRINCE_NODE_HAND_R);
    if (hand == 0)
        return;
    Vec4 where = offset;
    where.RotateX(-tilt);
    where.RotateZ(-f.model->heading);
    Matrix4 pose = RotationZ(f.model->heading) * RotationX(-tilt);
    pose.SetTranslation(hand->worldMatrix.GetTranslation() + where);
    EffectOptions options;
    SetUpOptions(f.index, options, scale, 0);
    Vec4 start = pose.GetTranslation();
    StartEffect(f.index, effect, start, 0, 0, 0, &pose, &options);
}

// The knock-back punch effects of the states 3 and 4: the burst at the hand, then a shot of the character.
static void StartPunchEffects(PrinceFrame &f, float tilt)
{
    Vec4 offset;
    offset.Fill(0.0f);
    offset.y = -60.0f;
    offset.z = 20.0f;
    StartHandEffect(f, 0x7ca, offset, tilt, 1.3f);
    Node *hand = PrinceNode(f.index, PRINCE_NODE_HAND_R);
    if (hand != 0)
    {
        Vec4 handPos = hand->worldMatrix.GetTranslation();
        FUN_1001a680(f.index, &handPos, 0);
    }
}

// Blends the movement vector of a punch: the stored step shrinks by `keep` every frame.
static void DampStep(PrinceFrame &f, float keep)
{
    f.moveMode = PRINCE_MOVE_PLACE;
    f.move.Copy(DAT_10802bf8[f.index].step * keep);
    DAT_10802bf8[f.index].step.Copy(f.move);
}

// Starts the rise or fall of the body: a wobble step towards the height range of the state.
static void SteerHeight(PrinceFrame &f, float low, float high, float stepSize)
{
    Kind43State &st = *f.st;
    if (f.ownPos.z > high)
    {
        f.move.z = -stepSize;
        st.heightPhase = 0x401;
        f.moveMode = PRINCE_MOVE_WALK;
    }
    if (f.ownPos.z < low)
    {
        f.move.z = stepSize;
        st.heightPhase = 0x19;
        f.moveMode = PRINCE_MOVE_WALK;
    }
}

static void StateSetUp(PrinceFrame &f)
{
    int index = f.index;
    Kind43State &st = *f.st;
    CharSlot &slot = *f.slot;
    AnimModel &endingA = DAT_10db6ee8[PRINCE_ENDING_MODEL_A];
    AnimModel &endingB = DAT_10db6ee8[PRINCE_ENDING_MODEL_B];
    if (DAT_10874848.TestScriptFlag(PRINCE_SCRIPT_FLAG_DIFFICULTY) == 1)
        slot.stats[4] -= 5;
    slot.stats[STAT_HEALTH] = slot.stats[STAT_MAX_HEALTH];
    DAT_10254390[index] = 0x2d;
    st.healing = 1;
    st.state = PRINCE_STATE_PICK;
    FUN_10012450(index, 1, "O_100.it3");
    Node *root = f.model->root;
    DAT_1021bdf8[index][PRINCE_NODE_HAND_R] = root->Find("Bip01 R Hand", 0);
    DAT_1021bdf8[index][PRINCE_NODE_HAND_L] = root->Find("Bip01 L Hand", 0);
    DAT_1021bdf8[index][PRINCE_NODE_ROOT_NULL] = root->Find("@s_null", 0);
    endingA.LoadFile(FUN_1013f8a0("char11en.it3", 0));
    endingB.LoadFile(FUN_1013f8a0("char11pa.it3", 0));
    endingA.LoadModel(FUN_1013f8a0("char11m.it3", 0));
    endingB.LoadModel(FUN_1013f8a0("char11m.it3", 0));
    endingA.mode = 0;
    endingB.mode = 0;
    endingA.scale.Fill(0.0f);
    endingB.scale.Fill(0.0f);
    endingA.SetPosition(PRINCE_FAR_X, 0.0f, 0.0f);
    endingB.SetPosition(PRINCE_FAR_X, 0.0f, 0.0f);
    DAT_10836c70.ResetFrameTimer();
}

static void StateStalk(PrinceFrame &f)
{
    Kind43State &st = *f.st;
    st.frames++;
    SteerHeight(f, 25.0f, 65.0f, 5.0f);
    if (st.wobble == 0)
        st.wobble = 1;
    f.motion = PRINCE_MOTION_IDLE;
    f.model->targetHeading = FacePlayer(f);
    if (FUN_1013f880() % st.frames < 0x97)
    {
        Vec4 away = f.playerPos - f.ownPos;
        if (away.Length() >= 500.0f)
            return;
    }
    st.state = PRINCE_STATE_PICK;
}

static void StateStrafe(PrinceFrame &f)
{
    Kind43State &st = *f.st;
    AnimModel &model = *f.model;
    if (st.frames == 0)
    {
        Vec4 away = f.ownPos - f.playerPos;
        away.z = 0.0f;
        Vec4 direction = away.Normalized3();
        direction.RotateZ(((float)(FUN_1013f880() % 2000) - 1000.0f) / 3600.0f * PRINCE_PI);
        DAT_10802bf8[f.index].step.Copy(direction * (float)(FUN_1013f880() % 1000 + 0x2ee));
        DAT_10802bf8[f.index].step.z = PRINCE_FLOOR_HEIGHT - f.ownPos.z;
        direction = DAT_10802bf8[f.index].step.Normalized3();
        DAT_10802bf8[f.index].step.Copy(direction * 80.0f);
        model.targetHeading = FacePlayer(f);
        FUN_10065b10(f.index, 0x140, 1, 0);
        st.frames++;
        return;
    }
    f.moveMode = PRINCE_MOVE_PLACE;
    f.move.Copy(DAT_10802bf8[f.index].step);
    f.move.Copy(f.move * 0.875f);
    float delta = WrapAngle(model.targetHeading - FUN_10001f80(f.move.x, f.move.y) + PRINCE_PI);
    if (delta >= -0.785375f && delta < 0.785375f)
        f.motion = 5;
    else if (delta >= 0.785375f && delta < 2.3561249f)
        f.motion = 6;
    else if (delta >= -2.3561249f && delta < -0.785375f)
        f.motion = 7;
    else
        f.motion = 8;
    float speed = f.move.Length();
    if (speed < 20.0f)
        f.motion = PRINCE_MOTION_IDLE;
    if (speed < 5.0f)
        st.state = PRINCE_STATE_PICK;
    Matrix4 pose;
    pose.SetIdentity();
    pose.SetTranslation(f.move + f.ownPos);
    Vec4 spot = pose.GetTranslation();
    StartEffect(f.index, 0x36c2, spot, 0, 0, 0, &pose, 0);
    st.frames++;
}

// The lean of the body while it punches: the tilt (DAT_10255160[slot][0]) follows the elevation of the player at 0.06
// per frame. Returns 1 once the tilt has reached it.
static int FollowElevation(PrinceFrame &f, float &elevation)
{
    float rise = f.distance3D;
    if (rise > 0.01f || rise < -0.01f)
        elevation = 2.0f * (float)atan(f.distance2D / rise);
    else
        elevation = 0.0f;
    elevation -= 0.25f;
    if (elevation <= 0.0f)
        elevation = 0.0f;
    int reached = ApproachValue(DAT_10255160[f.index][0], elevation, 0.06f);
    LeanBody(f.index, DAT_10255160[f.index][0]);
    return reached;
}

static void StatePunchLean(PrinceFrame &f)
{
    Kind43State &st = *f.st;
    AnimModel &model = *f.model;
    float elevation = 0.0f;
    int reached = 0;
    st.engaged = 1;
    model.targetHeading = FacePlayer(f);
    if (st.frames < PRINCE_FRAMES_PART_THREE)
        reached = FollowElevation(f, elevation);
    if (st.frames < PRINCE_FRAMES_PART_TWO)
    {
        f.motion = PRINCE_MOTION_IDLE;
        st.wobble = 2;
        if (f.ownPos.z < 200.0f)
        {
            f.moveMode = PRINCE_MOVE_PLACE;
            f.move.z = 25.0f;
            st.heightPhase = 0x19;
            st.frames++;
            return;
        }
        if (f.ownPos.z > 250.0f)
        {
            f.moveMode = PRINCE_MOVE_PLACE;
            f.move.z = -25.0f;
            st.heightPhase = 0x401;
            st.frames++;
            return;
        }
        if (reached)
        {
            st.frames = PRINCE_FRAMES_PART_TWO;
            f.motion = 0x14;
            st.frames++;
            return;
        }
    }
    else if (st.frames < PRINCE_FRAMES_PART_THREE)
    {
        f.motion = 0x14;
        if (model.motionFrame == 0x171)
        {
            st.counter = 1;
            f.move.Fill(0.0f);
            DAT_10802bf8[f.index].step.Fill(0.0f);
            DAT_10802bf8[f.index].step.y = -120.0f;
            DAT_10802bf8[f.index].step.RotateX(DAT_10255160[f.index][0]);
            DAT_10802bf8[f.index].step.RotateZ(PRINCE_PI - model.targetHeading);
            f.moveMode = PRINCE_MOVE_PLACE;
            StartPunchEffects(f, DAT_10255160[f.index][0]);
            FUN_10065b10(f.index, 0x145, 1, 0);
            SayRandomLine(f.index);
        }
        if (st.counter != 0)
        {
            DampStep(f, 0.8f);
            if (f.layer != 0x14)
            {
                st.frames = PRINCE_FRAMES_PART_THREE;
                st.frames++;
                return;
            }
        }
    }
    else if (st.frames < PRINCE_FRAMES_PART_FOUR)
    {
        DampStep(f, 0.8f);
        if (DAT_10255160[f.index][0] <= 0.0f)
        {
            DAT_10255160[f.index][0] = 0.0f;
            st.frames = PRINCE_FRAMES_PART_FOUR;
        }
        else
        {
            DAT_10255160[f.index][0] -= 0.07f;
            if (DAT_10255160[f.index][0] < 0.0f)
            {
                DAT_10255160[f.index][0] = 0.0f;
                st.frames = PRINCE_FRAMES_PART_FOUR;
            }
        }
        LeanBody(f.index, DAT_10255160[f.index][0]);
        st.frames++;
        return;
    }
    else if (st.frames < 4000)
    {
        DampStep(f, 0.8f);
        if (DAT_10802bf8[f.index].step.LengthSquared3() < 100.0f)
        {
            f.motion = PRINCE_MOTION_IDLE;
            st.state = PRINCE_STATE_PICK;
        }
    }
    st.frames++;
}

static void StatePunch(PrinceFrame &f)
{
    Kind43State &st = *f.st;
    AnimModel &model = *f.model;
    st.engaged = 1;
    model.targetHeading = FacePlayer(f);
    model.mode = 1;
    if (st.frames < PRINCE_FRAMES_PART_TWO)
    {
        f.motion = 2;
        st.wobble = 0;
        if (model.m_238c != 0)
        {
            st.frames = PRINCE_FRAMES_PART_TWO;
            f.motion = 0x14;
            st.frames++;
            return;
        }
    }
    else if (st.frames < PRINCE_FRAMES_PART_THREE)
    {
        f.motion = 0x14;
        if (model.motionFrame == 0x171)
        {
            st.counter = 1;
            DAT_10802bf8[f.index].step.Fill(0.0f);
            DAT_10802bf8[f.index].step.y = -80.0f;
            DAT_10802bf8[f.index].step.RotateZ(PRINCE_PI - model.targetHeading);
            f.moveMode = PRINCE_MOVE_PLACE;
            DAT_10255160[f.index][0] = 0.0f;
            StartPunchEffects(f, 0.0f);
            FUN_10065b10(f.index, 0x145, 1, 0);
            SayRandomLine(f.index);
        }
        if (st.counter != 0)
        {
            DampStep(f, 0.6f);
            if (f.layer != 0x14)
            {
                st.frames = PRINCE_FRAMES_PART_THREE;
                st.frames++;
                return;
            }
        }
    }
    else if (st.frames < PRINCE_FRAMES_PART_FOUR)
    {
        DampStep(f, 0.6f);
        if (DAT_10802bf8[f.index].step.LengthSquared3() < 100.0f)
        {
            f.motion = 2;
            st.state = PRINCE_STATE_PICK;
        }
    }
    st.frames++;
}

// The gatling burst of the states 5 and 6: a shot of the hand effect 0x36b1 and the tracer 1999 every 4th frame.
static void FireGatling(PrinceFrame &f, float tilt, float tracerSpread)
{
    Kind43State &st = *f.st;
    AnimModel &model = *f.model;
    Node *hand = PrinceNode(f.index, PRINCE_NODE_HAND_R);
    if (hand == 0)
    {
        hand = model.root->Find("Bip01 R Hand", 0);
        DAT_1021bdf8[f.index][PRINCE_NODE_HAND_R] = hand;
        if (hand == 0)
        {
            SayRandomLine(f.index);
            return;
        }
    }
    Vec4 offset;
    offset.Fill(0.0f);
    offset.y = -70.0f;
    StartHandEffect(f, 0x36b1, offset, tilt, 0.85f);
    Matrix4 pose = RotationZ(model.heading - 1.5708f + tracerSpread) * RotationX(-tilt);
    EffectOptions options;
    SetUpOptions(f.index, options, 0.85f, 0);
    Vec4 start = pose.GetTranslation();
    StartEffect(f.index, 1999, start, 0, 0, 0, &pose, &options);
    FUN_10065b10(f.index, 0x143, 1, 0);
}

static void StateGatlingAim(PrinceFrame &f)
{
    Kind43State &st = *f.st;
    AnimModel &model = *f.model;
    st.engaged = 1;
    st.wobble = 3;
    if (st.frames == 0)
    {
        Node *gatling = model.root->Find("@gatring_null", 0);
        DAT_1021bdf8[f.index][PRINCE_NODE_GATLING] = gatling;
        if (gatling != 0)
        {
            gatling->frameSource = &st.counter;
            gatling->RestartColourPlugs("r_start", 0);
        }
    }
    ApproachValue(DAT_10255160[f.index][0], 0.675f, 0.06f);
    LeanBody(f.index, DAT_10255160[f.index][0]);
    if (st.frames < PRINCE_FRAMES_PART_TWO)
    {
        model.targetHeading = FacePlayer(f);
        if (f.ownPos.z >= 300.0f)
        {
            if (f.ownPos.z <= 350.0f)
            {
                DAT_10802bf8[f.index].step.Copy(f.playerPos - f.ownPos);
                DAT_10802bf8[f.index].step.z = 0.0f;
                if ((f.playerPos - f.ownPos).LengthSquared3() < 1.0f)
                    DAT_10802bf8[f.index].step.y = f.ownPos.y <= f.playerPos.y ? 1.0f : -1.0f;
                DAT_10802bf8[f.index].step.Copy(DAT_10802bf8[f.index].step.Normalized3() * 35.0f);
                st.frames = PRINCE_FRAMES_PART_TWO;
            }
            else
            {
                f.moveMode = PRINCE_MOVE_WALK;
                f.move.z = -30.0f;
                st.heightPhase = 0x401;
            }
        }
        else
        {
            f.moveMode = PRINCE_MOVE_WALK;
            f.move.z = 30.0f;
            st.heightPhase = 0x19;
        }
    }
    else
    {
        float blend = (float)(st.frames - PRINCE_FRAMES_PART_TWO) / 30.0f;
        if (blend >= 1.0f)
            blend = 1.0f;
        f.moveMode = PRINCE_MOVE_PLACE;
        f.move.Copy(DAT_10802bf8[f.index].step * blend);
    }
    st.frames++;
    if (st.frames < 0x2e)
    {
        f.motion = PRINCE_MOTION_IDLE;
        return;
    }
    f.motion = 0xf;
    if (st.frames % 4 == 0)
    {
        FireGatling(f, DAT_10255160[f.index][0], ((float)(FUN_1013f880() % 0x1e - 0xf) * PRINCE_PI) / 180.0f);
    }
    st.counter++;
}

static void StateGatlingSpray(PrinceFrame &f)
{
    Kind43State &st = *f.st;
    AnimModel &model = *f.model;
    st.engaged = 1;
    f.motion = 0xf;
    if (st.frames == 0)
    {
        DAT_10802bf8[f.index].step.Fill(0.0f);
        DAT_10802bf8[f.index].step.y = (float)(FUN_1013f880() % 10) * -0.6f;
        DAT_10802bf8[f.index].step.RotateZ((((float)(FUN_1013f880() % 0x10e - 0x87) * PRINCE_PI) / 180.0f - model.targetHeading) +
                          PRINCE_PI);
        Node *gatling = model.root->Find("@gatring_null", 0);
        DAT_1021bdf8[f.index][PRINCE_NODE_GATLING] = gatling;
        if (gatling != 0)
        {
            gatling->frameSource = &st.frames;
            gatling->RestartColourPlugs("r_start", 0);
        }
    }
    if (st.frames < 10000)
    {
        st.frames = st.frames % 9 + 1;
        model.targetHeading = FacePlayer(f);
        if (model.motionFrame == 0x13a)
        {
            st.counter++;
            if (FUN_1013f880() % 1000 < st.counter * 0x28)
                st.frames = 10000;
            else
            {
                DAT_10802bf8[f.index].step.Fill(0.0f);
                DAT_10802bf8[f.index].step.y = -(float)(FUN_1013f880() % 10);
                DAT_10802bf8[f.index].step.RotateZ((((float)(FUN_1013f880() % 0x10e - 0x87) * PRINCE_PI) / 180.0f -
                                   model.targetHeading) + PRINCE_PI);
            }
        }
        f.moveMode = PRINCE_MOVE_WALK;
        st.wobble = 1;
        f.move.Copy(DAT_10802bf8[f.index].step);
        SteerHeight(f, 25.0f, 65.0f, 20.0f);
        if ((model.motionFrame & 3) == 0)
        {
            FireGatling(f, 0.0f, ((float)(FUN_1013f880() % 0x28 - 0x14) * PRINCE_PI) / 720.0f);
            st.frames++;
            return;
        }
    }
    else if (st.frames == 0x2723)
    {
        st.state = PRINCE_STATE_PICK;
        Node *gatling = PrinceNode(f.index, PRINCE_NODE_GATLING);
        if (gatling != 0)
        {
            gatling->RestartColourPlugs("stop", 0);
            gatling->frameSource = 0;
            st.frames++;
            return;
        }
    }
    st.frames++;
}

// Swings of the states 7 and 8: effect `effect` from the left hand with a pose turned by the heading plus `jitter`
// radians (the sparks of the weapon).
static void StartLeftHandSwing(PrinceFrame &f, int effect, float scale, float tilt, float jitter)
{
    Node *hand = PrinceNode(f.index, PRINCE_NODE_HAND_L);
    if (hand == 0)
    {
        hand = f.model->root->Find("Bip01 L Hand", 0);
        DAT_1021bdf8[f.index][PRINCE_NODE_HAND_L] = hand;
        if (hand == 0)
            return;
    }
    Matrix4 pose = RotationZ(f.model->heading + jitter) * RotationX(-tilt);
    EffectOptions options;
    SetUpOptions(f.index, options, scale, 0);
    Vec4 start = hand->worldMatrix.GetTranslation();
    StartEffect(f.index, effect, start, 0, 0, 0, &pose, &options);
}

static void StateSwing(PrinceFrame &f)
{
    Kind43State &st = *f.st;
    AnimModel &model = *f.model;
    int frames = st.frames;
    st.engaged = 1;
    model.mode = 1;
    st.wobble = 0;
    if (frames < PRINCE_FRAMES_PART_TWO)
    {
        f.motion = 2;
        model.targetHeading = FacePlayer(f);
        if (model.m_238c != 0)
        {
            st.frames = PRINCE_FRAMES_PART_TWO;
            f.motion = 0x1b;
        }
        return;
    }
    if (frames >= PRINCE_FRAMES_PART_THREE)
    {
        f.motion = 2;
        st.state = PRINCE_STATE_PICK;
        return;
    }
    f.motion = 0x1b;
    if (model.motionFrame == 0x248 || model.motionFrame == 0x256)
    {
        model.targetHeading = FacePlayer(f);
        st.counter++;
        if (FUN_1013f880() % 1000 < st.counter * 0x96)
            st.frames = PRINCE_FRAMES_PART_THREE;
    }
    if (model.motionFrame == 0x245 || model.motionFrame == 0x252)
    {
        StartLeftHandSwing(f, 0x36b4, 0.7f, 0.0f, 0.0f);
        FUN_10065b10(f.index, 0x14e, 1, 0);
        if (FUN_1013f880() % 3 == 0)
            FUN_100b9bb0(f.index, FUN_1013f880() % 3 + 0x47, 0, 1);
    }
    st.frames++;
}

static void StateSwingFollow(PrinceFrame &f)
{
    Kind43State &st = *f.st;
    AnimModel &model = *f.model;
    float elevation = 0.0f;
    st.engaged = 1;
    st.wobble = 2;
    if (st.frames >= PRINCE_FRAMES_PART_THREE || (f.distance3D <= 0.01f && f.distance3D >= -0.01f))
        elevation = 0.0f;
    else
    {
        elevation = (float)atan(f.distance2D / f.distance3D);
        if (elevation >= 1.5708f)
            elevation = 1.5708f;
    }
    ApproachValue(DAT_10255160[f.index][0], elevation, 0.06f);
    if (st.frames == 0)
    {
        st.counter = 0x3c;
        f.motion = PRINCE_MOTION_IDLE;
        model.targetHeading = FacePlayer(f);
        st.frames++;
        return;
    }
    if (st.frames < PRINCE_FRAMES_PART_TWO)
    {
        st.counter -= 4;
        f.motion = PRINCE_MOTION_IDLE;
        if (st.counter < 0x14)
            st.frames = PRINCE_FRAMES_PART_TWO;
        f.move.z = (float)st.counter;
        f.moveMode = PRINCE_MOVE_PLACE;
        LeanBody(f.index, DAT_10255160[f.index][0]);
        model.targetHeading = FacePlayer(f);
        st.frames++;
        return;
    }
    if (st.frames < PRINCE_FRAMES_PART_THREE)
    {
        f.motion = 0x1b;
        if (st.counter > 0)
        {
            st.counter -= 4;
            f.move.z = (float)st.counter;
            f.moveMode = PRINCE_MOVE_PLACE;
        }
        if (model.motionFrame == 0x248 || model.motionFrame == 0x256)
        {
            model.targetHeading = FacePlayer(f);
            st.counter++;
            if (FUN_1013f880() % 1000 < 700)
                st.frames = PRINCE_FRAMES_PART_THREE;
        }
        if (model.motionFrame == 0x245 || model.motionFrame == 0x252)
        {
            float jitter = ((float)(FUN_1013f880() % 0x14 - 10) * PRINCE_PI) / 180.0f;
            StartLeftHandSwing(f, 0x36b6, 1.1f, DAT_10255160[f.index][0], jitter);
            FUN_10065b10(f.index, 0x142, 1, 0);
            if (FUN_1013f880() % 3 == 0)
                FUN_100b9bb0(f.index, FUN_1013f880() % 3 + 0x47, 0, 1);
        }
    }
    else
    {
        if (st.frames >= PRINCE_FRAMES_PART_FOUR)
        {
            DAT_10255160[f.index][0] = 0.0f;
            st.state = PRINCE_STATE_PICK;
            st.frames++;
            return;
        }
        f.motion = PRINCE_MOTION_IDLE;
        st.wobble = 0;
        model.mode = 1;
        if (model.m_238c != 0 || f.ownPos.z < 0.0f)
            st.frames = PRINCE_FRAMES_PART_FOUR;
    }
    LeanBody(f.index, DAT_10255160[f.index][0]);
    st.frames++;
}

// The sparks that run along the line between the two hands of the prince while it is knocked back (state 9): one
// effect every 30 units, spread by a random jolt of up to 25 units.
static void StartRope(PrinceFrame &f)
{
    Node *root = PrinceNode(f.index, PRINCE_NODE_ROOT_NULL);
    Node *hand = PrinceNode(f.index, PRINCE_NODE_HAND_R);
    if (root == 0 || hand == 0)
        return;
    Vec4 from = root->worldMatrix.GetTranslation();
    Vec4 to = hand->worldMatrix.GetTranslation();
    int count = (int)(to - from).Length() + 1;
    EffectOptions options;
    SetUpOptions(f.index, options, 1.0f, 0);
    for (int at = 0, left = count; at < count; at += 0x1e, left -= 0x1e)
    {
        Vec4 spot = from * ((float)at / (float)count) + to * ((float)left / (float)count);
        spot.x += (float)(FUN_1013f880() % 0x32 - 0x19);
        spot.y += (float)(FUN_1013f880() % 0x32 - 0x19);
        spot.z += (float)(FUN_1013f880() % 0x32 - 0x19);
        Matrix4 pose;
        pose.SetIdentity();
        pose.SetTranslation(spot);
        StartEffect(f.index, 0x8599, spot, 0, 0, 0, &pose, &options);
    }
}

// The hand-and-root links of a weapon: the spark effect `spark` follows the right hand, and every 5th frame a link
// effect `link` is drawn between the two nodes.
static void StartLinkedSparks(PrinceFrame &f, int spark, int link)
{
    EffectOptions options;
    Matrix4 pose;
    Vec4 where;
    where.Fill(0.0f);
    pose.SetIdentity();
    options.flags |= 2;
    Node *hand = PrinceNode(f.index, PRINCE_NODE_HAND_R);
    Node *tip = PrinceNode(f.index, PRINCE_NODE_ROOT_NULL);
    if (hand == 0)
        return;
    StartEffect(f.index, spark, where, 0, 0, 0, &hand->worldMatrix, &options);
    if (tip != 0)
        StartLinkEffect(f.index, link, where, hand, tip, options);
}

static void StateHitRecover(PrinceFrame &f)
{
    Kind43State &st = *f.st;
    AnimModel &model = *f.model;
    int frames = st.frames;
    st.engaged = 1;
    if (frames == 0)
    {
        f.motion = 2;
        st.wobble = 0;
        model.mode = 1;
        model.targetHeading = FacePlayer(f);
        model.m_238c = 0;
        FUN_100b9bb0(f.index, 0x4b, 0, 0);
    }
    else if (frames < PRINCE_FRAMES_PART_TWO)
    {
        f.motion = 2;
        st.wobble = 0;
        model.mode = 1;
        if (model.m_238c != 0 || f.ownPos.z < 0.0f)
        {
            st.frames = PRINCE_FRAMES_PART_TWO;
            st.wobble = 1;
            FUN_10065b10(f.index, 0x147, 1, 0);
        }
        model.targetHeading = FacePlayer(f);
    }
    else if (frames < PRINCE_FRAMES_PART_THREE)
    {
        f.motion = 0x20;
        model.holding = 1;
        model.motionFrame = (frames - PRINCE_FRAMES_PART_TWO) / 3 + 0x406;
        model.targetHeading = FacePlayer(f);
        if (model.motionFrame == 0x410)
        {
            st.frames = PRINCE_FRAMES_PART_THREE;
            Vec4 toPlayer = f.playerPos - f.ownPos;
            toPlayer.z = 0.0f;
            Vec4 direction = toPlayer.Normalized3();
            DAT_10802bf8[f.index].step.Copy(direction * (f.distance2D - 100.0f) * 0.1f);
            DAT_10802bf8[f.index].step.z = 9.0f;
            f.move.Copy(DAT_10802bf8[f.index].step);
        }
    }
    else if (frames < PRINCE_FRAMES_PART_FOUR)
    {
        f.motion = 0x20;
        if (model.motionFrame < 0x415)
        {
            if ((frames & 1) == 1)
                model.motionFrame++;
            StartLinkedSparks(f, 0x7c6, 0x7cd);
        }
        else
            model.motionFrame = 0x415;
        if (st.frames == 0x7ee)
        {
            model.holding = 0;
            st.state = PRINCE_STATE_PICK;
            f.motion = 2;
        }
        if (st.frames < 0x7db)
        {
            f.moveMode = PRINCE_MOVE_PLACE;
            DAT_10802bf8[f.index].step.Copy(f.move);
        }
        else
        {
            f.move.Copy(DAT_10802bf8[f.index].step * 0.5f);
            st.wobble = 0;
        }
        if (st.frames == 0x7db)
        {
            Matrix4 pose;
            pose.SetIdentity();
            EffectOptions options;
            SetUpOptions(f.index, options, 2.0f, 1);
            options.flags |= 2;
            Vec4 where;
            where.Fill(0.0f);
            StartEffect(f.index, 0x36ba, where, EffectAnchor(f.index, PRINCE_NODE_ROOT_NULL, PRINCE_NODE_HAND_R), 0, 0, 0,
                        &options);
            FUN_10065b10(f.index, 0x146, 1, 0);
            FUN_100b9bb0(f.index, 0x49, 0, 0);
        }
    }
    if (st.frames < 0x7df && PrinceNode(f.index, PRINCE_NODE_ROOT_NULL) != 0)
    {
        StartRope(f);
        st.frames++;
        return;
    }
    st.frames++;
}

// The charge states 10 (A) and 11 (B): the prince picks one of four attack motions, then dashes with it along the
// heading it faced. Both motions end the state when the motion layer changes or `lastFrame` is passed.
static void StateCharge(PrinceFrame &f, int variant)
{
    Kind43State &st = *f.st;
    AnimModel &model = *f.model;
    int isB = variant != 0;
    static const int motionsA[4] = {0x22, 0x27, 0x28, 0x29};
    static const int motionsB[4] = {0x1d, 0x1e, 0x1f, 0x22};
    if (st.frames == 0)
    {
        model.targetHeading = FacePlayer(f);
        if (f.distance2D <= (isB ? 500.0f : 600.0f))
        {
            st.attackMotion = (isB ? motionsB : motionsA)[FUN_1013f880() & 3];
            f.motion = st.attackMotion;
            f.motionLoop = 1;
            DAT_10802bf8[f.index].step.Fill(0.0f);
            DAT_10802bf8[f.index].step.y = -30.0f;
            DAT_10802bf8[f.index].step.RotateZ(-model.targetHeading);
            FUN_10065b10(f.index, isB ? 0x14d : 0x148, 1, 0);
            SayRandomLine(f.index);
        }
        else
        {
            st.frames = -1;
            st.state = isB ? PRINCE_STATE_LEAP : PRINCE_STATE_HIT_RECOVER;
            if (FUN_1013f880() % 100 < 0x3c && st.engaged != 0)
                st.state = PRINCE_STATE_SWAP_WEAPON;
        }
    }
    else
    {
        f.motion = st.attackMotion;
        f.move.x = DAT_10802bf8[f.index].step.x;
        f.move.y = DAT_10802bf8[f.index].step.y;
        f.moveMode = PRINCE_MOVE_WALK;
        f.move.Copy(f.move * (isB ? 0.92f : 0.9f));
        if (isB)
            st.wobble = 0;
        if (!isB && st.attackMotion == 0x27)
        {
            if (f.ownPos.z < 90.0f)
            {
                f.move.z = 20.0f;
                if (f.ownPos.z + 20.0f > 95.0f)
                    f.move.z = 95.0f - f.ownPos.z;
            }
            else if (f.ownPos.z > 100.0f)
            {
                f.move.z = -20.0f;
                if (f.ownPos.z - 20.0f < 95.0f)
                    f.move.z = 95.0f - f.ownPos.z;
            }
        }
        else if (f.ownPos.z > 5.0f)
        {
            f.move.z = -20.0f;
            if (f.ownPos.z - 20.0f < 0.0f)
                f.move.z = -f.ownPos.z;
        }
        StartLinkedSparks(f, isB ? 0x7c7 : 0x7c8, isB ? 0x7c7 : 0x7c8);
        int mf = model.motionFrame;
        int glow = isB ? ((mf > 0x361 && mf < 0x366) || (mf > 0x386 && mf < 0x38c) || (mf > 0x39f && mf < 0x3aa) ||
                          (mf > 0x442 && mf < 0x448))
                       : ((mf > 0x3a0 && mf < 0x3a5) || (mf > 0x386 && mf < 0x38c) || (mf > 0x413 && mf < 0x415) ||
                          (mf > 0x442 && mf < 0x448));
        if (glow)
            StartWeaponGlow(f, isB ? 0x456 : 0x36c1, isB ? 1.4f : 1.1f);
    }
    if ((st.counter != 0 && f.layer != st.attackMotion) || st.frames > (isB ? 0x415 : 0x406))
        st.state = PRINCE_STATE_PICK;
    if (f.layer == st.attackMotion)
        st.counter = 1;
    st.frames++;
    st.engaged = 1;
}

static void StateLeap(PrinceFrame &f)
{
    Kind43State &st = *f.st;
    AnimModel &model = *f.model;
    if (st.frames == 0)
    {
        if (f.distance3D >= 500.0f)
        {
            Vec4 toPlayer = f.playerPos - f.ownPos;
            DAT_10802bf8[f.index].step.Copy(toPlayer.Normalized3() * 40.0f);
            st.counter = (int)(toPlayer.Length() / 40.0f);
            st.wobble = 1;
            model.mode = 0;
            f.motion = 9;
            f.motionLoop = 1;
            FUN_10065b10(f.index, 0x14b, 1, 0);
            FUN_100b9bb0(f.index, 0x4b, 1, 0);
        }
        else
        {
            st.frames = PRINCE_FRAMES_PART_TWO;
            st.wobble = 1;
            DAT_10802bf8[f.index].step.Fill(0.0f);
            model.mode = 0;
            f.motion = 0xb;
            f.motionLoop = 1;
        }
        model.targetHeading = FacePlayer(f);
        st.frames++;
        return;
    }
    if (st.frames < PRINCE_FRAMES_PART_TWO)
    {
        f.motion = 9;
        if (model.motionFrame > 0x9b)
        {
            st.counter--;
            f.moveMode = PRINCE_MOVE_PLACE;
            f.move.Copy(DAT_10802bf8[f.index].step);
        }
        if (f.layer == 1 || st.counter < 0)
        {
            f.motion = 0xb;
            st.frames = PRINCE_FRAMES_PART_TWO;
            model.targetHeading = FacePlayer(f);
            DAT_10802bf8[f.index].step.Fill(0.0f);
        }
        StartLinkedSparks(f, 0x7c5, 0x7c7);
        if (st.frames % 10 == 1)
            StartWeaponGlow(f, 0x3715, 1.75f);
        st.frames++;
        return;
    }
    f.motion = 0xb;
    if (model.motionFrame == 0xac)
    {
        model.holding = 1;
        FUN_100b9bb0(f.index, (FUN_1013f880() & 3) + 0x47, 0, 0);
    }
    int mf = model.motionFrame;
    if ((mf > 0xae && mf < 0xca) || (mf > 0xd5 && mf < 0xf8))
        StartLinkedSparks(f, 0x7c5, 0x7ce);
    mf = model.motionFrame;
    int arc = 0;
    if (mf == 0xaf)
    {
        model.targetHeading = FacePlayer(f);
        SayRandomLine(f.index);
    }
    else if (mf < 0xb0 || mf > 0xc5)
    {
        if (mf < 0xcd || mf > 0xd7)
        {
            if (mf > 0xd9 && mf < 0xf3)
            {
                float rise = (float)((0xf3 - mf) * (mf - 0xbf));
                f.moveMode = PRINCE_MOVE_PLACE;
                f.move.y = (rise / 26.0f) * -1.2f;
                f.move.RotateZ(-model.targetHeading);
                arc = 1;
            }
        }
        else
        {
            model.targetHeading = FacePlayer(f);
            if (model.motionFrame == 0xd2)
                SayRandomLine(f.index);
        }
    }
    else
    {
        float rise = (float)((0xc6 - mf) * (mf - 0x98));
        f.moveMode = PRINCE_MOVE_PLACE;
        f.move.y = -(rise / 23.0f);
        f.move.RotateZ(-model.targetHeading);
        arc = 1;
    }
    if (arc)
    {
        if (f.ownPos.z <= 20.0f)
        {
            if (f.ownPos.z < 20.0f)
            {
                f.move.z = 10.0f;
                if (f.ownPos.z + 10.0f > 20.0f)
                    f.move.z = 20.0f - f.ownPos.z;
            }
        }
        else
        {
            f.move.z = -10.0f;
            if (f.ownPos.z - 10.0f < 20.0f)
                f.move.z = 20.0f - f.ownPos.z;
        }
    }
    if (model.motionFrame == 0x102)
    {
        model.holding = 0;
        f.motion = PRINCE_MOTION_IDLE;
        st.state = PRINCE_STATE_PICK;
    }
    if (model.holding != 0)
        model.motionFrame++;
    mf = model.motionFrame;
    if ((mf > 0xaf && mf < 0xc5 && (mf - 0xaf) % 10 == 1) || (mf > 0xd4 && mf < 0xea && (mf - 0xd4) % 10 == 1))
        StartWeaponGlow(f, 0x36b8, 1.9f);
    if (model.motionFrame == 0xaf || model.motionFrame == 0xd5)
        FUN_10065b10(f.index, 0x14c, 1, 0);
    st.frames++;
}

// Thirty glowing hits fall from the sky over the arena (effect 0x7cc, each living 30..119 frames).
static void RainEffects(PrinceFrame &f)
{
    EffectOptions options;
    SetUpOptions(f.index, options, 1.3f, 0);
    for (int hit = 0; hit < 0x1e; hit++)
    {
        float x = (float)(FUN_1013f880() % 0x10ae) - 2135.0f;
        float limit = 3100.0f - (float)fabs(x) * 1.732051f;
        float y = (float)(FUN_1013f880() % (int)limit) - 650.0f;
        float z = f.ownPos.z + 900.0f;
        Matrix4 pose = TranslationMatrix(x, y, z) * RotationZ(-1.5708f);
        Vec4 where;
        where.Set(x, y, z, 0.0f);
        StartEffect(f.index, 0x7cc, where, 0, (int)&DAT_10db6ee8[0].matrix, FUN_1013f880() % 0x5a + 0x1e, &pose,
                    &options);
    }
}

static void StateBombard(PrinceFrame &f)
{
    Kind43State &st = *f.st;
    AnimModel &model = *f.model;
    if (st.hurt != 0)
        st.frames = 4000;
    if (st.frames == 0)
    {
        st.counter = -3;
        st.wobble = 0;
        model.mode = 1;
        model.targetHeading = FacePlayer(f);
        st.frames++;
        return;
    }
    if (st.frames >= PRINCE_FRAMES_PART_TWO)
    {
        if (st.frames >= PRINCE_FRAMES_PART_THREE)
        {
            if (st.frames >= PRINCE_FRAMES_PART_FOUR)
            {
                st.state = PRINCE_STATE_PICK;
                st.frames++;
                return;
            }
            f.motion = 0x18;
            if (model.motionFrame == 0x1c2)
            {
                st.frames = PRINCE_FRAMES_PART_FOUR + 1;
                return;
            }
            st.frames++;
            return;
        }
        f.motion = 0x16;
        model.targetHeading = FacePlayer(f);
        if (model.motionFrame == 0x1b8)
        {
            DAT_10255160[f.index][0] = (float)(FUN_1013f880() % 300 - 0x96) / 5000.0f - 0.87606f;
            Vec4 offset;
            offset.Fill(0.0f);
            offset.y = -60.0f;
            offset.z = 20.0f;
            StartHandEffect(f, 0x7ca, offset, DAT_10255160[f.index][0], 1.3f);
            StartHandEffect(f, 0x3713, offset, DAT_10255160[f.index][0], 1.3f);
            DAT_10255160[f.index][0] = 0.0f;
            FUN_10065b10(f.index, 0x145, 1, 0);
        }
        else if (model.motionFrame == 0x1bc)
        {
            st.counter++;
            if ((st.counter >= 1 && FUN_1013f880() % 1000 < st.counter * 0x32) || st.counter == 7)
            {
                st.frames = PRINCE_FRAMES_PART_THREE;
                RainEffects(f);
            }
        }
        st.frames++;
        return;
    }
    st.wobble = 0;
    model.mode = 1;
    f.motion = 2;
    model.targetHeading = FacePlayer(f);
    if (model.m_238c != 0)
    {
        st.frames = PRINCE_FRAMES_PART_TWO;
        FUN_100b9bb0(f.index, 0x4b, 0, 0);
    }
    st.frames++;
}

static void StateSwapWeapon(PrinceFrame &f)
{
    Kind43State &st = *f.st;
    int index = f.index;
    if (st.frames == 0)
    {
        if (st.engaged != 0)
        {
            DAT_1021bdf8[index][PRINCE_NODE_ROOT_NULL] = 0;
            if (st.weapon == 1)
                f.motion = 0x15;
            else if (st.weapon == 2)
            {
                DAT_1021bdf8[index][PRINCE_NODE_GATLING] = 0;
                f.motion = 0x10;
            }
            else
                f.motion = 0x25;
            st.engaged = 0;
            st.frames++;
            return;
        }
        st.state = PRINCE_STATE_PICK;
        st.frames++;
        return;
    }
    if (st.frames < PRINCE_FRAMES_PART_TWO)
    {
        int doneFrame;
        int oldWeapon = st.weapon;
        if (st.weapon == 1)
        {
            f.motion = 0x15;
            doneFrame = 0x18d;
        }
        else if (st.weapon == 2)
        {
            f.motion = 0x10;
            doneFrame = 0x14a;
        }
        else
        {
            f.motion = 0x25;
            doneFrame = 0x107;
        }
        if (f.model->motionFrame == doneFrame)
            st.weapon = -1;
        if (st.weapon == -1)
        {
            FUN_10012450(index, 0, 0);
            DAT_10daaa80.RemoveOwnedBy(index + PRINCE_EFFECT_OWNER_BASE);
            int picked;
            do
            {
                picked = FUN_1013f880() & 3;
            } while (picked == oldWeapon);
            st.weapon = picked;
            if (picked == 1)
            {
                f.motion = 0x13;
                FUN_10012450(index, 1, "O_101.it3");
            }
            else if (picked == 2)
            {
                f.motion = 0xe;
                FUN_10012450(index, 1, "O_102.it3");
            }
            else
            {
                f.motion = 0x26;
                FUN_10012450(index, 1, picked == 3 ? "O_103.it3" : "O_100.it3");
                DAT_1021bdf8[index][PRINCE_NODE_ROOT_NULL] = f.model->root->Find("@s_null", 0);
            }
            FUN_10065b10(index, 0x13f, 1, 0);
            st.frames = PRINCE_FRAMES_PART_TWO;
        }
    }
    else if (st.frames < PRINCE_FRAMES_PART_THREE)
    {
        if (st.weapon == 1)
            f.motion = 0x13;
        else
            f.motion = st.weapon == 2 ? 0xe : 0x26;
        if (f.layer == 0x11 || f.layer == 0x12 || f.layer == 0xc || f.layer == 0xd || f.layer == 2 || f.layer == 4)
        {
            st.state = PRINCE_STATE_PICK;
            st.frames++;
            return;
        }
    }
    st.frames++;
}

static void StateTeleport(PrinceFrame &f)
{
    Kind43State &st = *f.st;
    AnimModel &model = *f.model;
    int index = f.index;
    st.frames++;
    f.slot->m_0141 = 1;
    if (st.frames < 0xf)
    {
        model.alpha = (float)(0xf - st.frames) / 15.0f;
        return;
    }
    if (st.frames == 0xf)
    {
        DAT_10daaa80.RemoveOwnedBy(index + PRINCE_EFFECT_OWNER_BASE);
        model.alpha = 0.0f;
        Vec4 spot = f.playerPos;
        spot.y -= 650.0f;
        spot.z = 0.0f;
        Vec4 direction = spot.Normalized3();
        direction.RotateZ(PRINCE_PI);
        spot.Copy(direction * 1000.0f);
        spot.y += 650.0f;
        spot.z = PRINCE_FLOOR_HEIGHT;
        model.SetPosition(spot.x, spot.y, spot.z);
        DAT_10255160[index][0] = 0.0f;
        if (st.weapon == 2 && DAT_1021bdf8[index][PRINCE_NODE_GATLING] != 0)
        {
            DAT_1021bdf8[index][PRINCE_NODE_GATLING]->RestartColourPlugs("stop", 0);
            DAT_1021bdf8[index][PRINCE_NODE_GATLING]->frameSource = 0;
        }
        f.motion = PRINCE_MOTION_IDLE;
        f.motionLoop = 1;
        model.holding = 0;
        LeanBody(index, 0.0f);
        FUN_1001a760(index, 0, 0);
        float heading = FUN_10001f80(f.playerPos.x - spot.x, f.playerPos.y - spot.y) + PRINCE_PI;
        model.targetHeading = heading;
        model.heading = heading;
        DAT_10802bf8[index].step.Fill(0.0f);
        FUN_10065b10(index, 0x13d, 1, 0);
        return;
    }
    if (st.frames < 0x1e)
    {
        model.alpha = (float)(st.frames - 0xe) / 15.0f;
        return;
    }
    model.alpha = 1.0f;
    f.slot->m_0141 = 0;
    st.state = PRINCE_STATE_PICK;
}

static void StateLockOn(PrinceFrame &f)
{
    Kind43State &st = *f.st;
    AnimModel &model = *f.model;
    int index = f.index;
    if (st.frames == 0)
    {
        st.wobble = 1;
        st.counter = -5;
        model.targetHeading = FacePlayer(f);
        FUN_10065b10(index, 0x14a, 1, 0);
        st.frames++;
        return;
    }
    if (st.frames < PRINCE_FRAMES_PART_TWO)
    {
        f.motion = 0x19;
        if (model.motionFrame == 0x1e0)
        {
            FUN_1001a760(index, 1, "O_108.it3");
            DAT_1021bdf8[index][PRINCE_NODE_LOCK] = model.root->Find("@lk_null", 0);
            FUN_10065b10(index, 0x13f, 1, 0);
        }
        else if (model.motionFrame == 0x203)
        {
            DAT_10802bf8[f.index].step.Fill(0.0f);
            st.frames = PRINCE_FRAMES_PART_TWO;
            FUN_100b9bb0(index, (FUN_1013f880() & 1) + 0x47, 0, 0);
        }
        if (f.layer == 0x2a)
        {
            if (DAT_1021bdf8[index][PRINCE_NODE_LOCK] == 0)
            {
                FUN_1001a760(index, 1, "O_108.it3");
                DAT_1021bdf8[index][PRINCE_NODE_LOCK] = model.root->Find("@lk_null", 0);
            }
            DAT_10802bf8[f.index].step.Fill(0.0f);
            st.frames = PRINCE_FRAMES_PART_TWO;
            st.frames++;
            return;
        }
    }
    else if (st.frames < PRINCE_FRAMES_PART_THREE)
    {
        f.motion = 0x2a;
        if (model.motionFrame == 0x210)
        {
            st.counter++;
            if (st.counter > 0 && FUN_1013f880() % 1000 < st.counter * 2)
                st.frames = PRINCE_FRAMES_PART_THREE;
        }
        f.moveMode = PRINCE_MOVE_WALK;
        if (f.ownPos.x > f.playerPos.x)
            DAT_10802bf8[f.index].step.x -= 2.0f;
        else if (f.ownPos.x < f.playerPos.x)
            DAT_10802bf8[f.index].step.x += 2.0f;
        if (f.ownPos.y > f.playerPos.y)
            DAT_10802bf8[f.index].step.y -= 2.0f;
        else if (f.ownPos.y < f.playerPos.y)
            DAT_10802bf8[f.index].step.y += 2.0f;
        if (f.playerPos.z + 25.0f > f.ownPos.z)
        {
            f.move.z = 15.0f;
            st.heightPhase = 0x19;
        }
        else if (f.playerPos.z + 65.0f < f.ownPos.z)
        {
            f.move.z = -15.0f;
            st.heightPhase = 0x401;
        }
        if (DAT_10802bf8[f.index].step.Length2D() > 30.0f)
            DAT_10802bf8[f.index].step.Copy(DAT_10802bf8[f.index].step.Normalized3() * 30.0f);
        f.move.x = DAT_10802bf8[f.index].step.x;
        f.move.y = DAT_10802bf8[f.index].step.y;
        Node *hand = PrinceNode(index, PRINCE_NODE_HAND_L);
        Node *lock = PrinceNode(index, PRINCE_NODE_LOCK);
        if (hand != 0 && lock != 0)
            StartLinkedSparks(f, 0x7c5, 0x7c7);
        if (st.frames % 0x14 == 1)
            FUN_10065b10(index, 0x149, 1, 0);
        if (st.frames % 10 == 1)
            StartWeaponGlow(f, 0x36b8, 1.55f);
        st.frames++;
        return;
    }
    else
    {
        if (st.frames >= PRINCE_FRAMES_PART_FOUR)
        {
            st.state = PRINCE_STATE_PICK;
            f.motion = PRINCE_MOTION_IDLE;
            st.frames++;
            return;
        }
        model.motionFrame++;
        f.motion = 0x2b;
        model.holding = 1;
        DAT_10daaa80.RemoveOwnedBy(index + PRINCE_EFFECT_OWNER_BASE);
        if (model.motionFrame == 0x230)
        {
            FUN_1001a760(index, 0, 0);
            DAT_1021bdf8[index][PRINCE_NODE_LOCK] = 0;
        }
        if (model.motionFrame == 0x239)
        {
            st.frames = PRINCE_FRAMES_PART_FOUR;
            model.holding = 0;
            f.motion = PRINCE_MOTION_IDLE;
            st.frames = PRINCE_FRAMES_PART_FOUR + 1;
            return;
        }
    }
    st.frames++;
}

// The odds of the moves (weights by state number) for the weapon in hand; `healthy` = more than half its health, or
// healing; `swapped` = the last state was the weapon swap. `distant` = the player is more than 500 units away.
static void FillWeights(int *weight, int healthy, int swapped, int weapon, int distant)
{
    if (healthy)
    {
        weight[2] = 10;
        weight[1] = 10;
        if (swapped)
        {
            switch (weapon)
            {
            case 0:
                if (!distant)
                {
                    weight[12] = 0x14;
                    weight[11] = 0x50;
                }
                else
                {
                    weight[7] = 0x50;
                    weight[8] = 0x14;
                    weight[12] = 0x14;
                }
                break;
            case 1:
                weight[3] = 0x14;
                weight[4] = 0x50;
                break;
            case 2:
                weight[5] = 0x50;
                weight[6] = 0x14;
                break;
            case 3:
                weight[9] = 100;
                break;
            }
        }
        else
        {
            switch (weapon)
            {
            case 0:
                weight[1] = 0;
                if (!distant)
                {
                    weight[12] = 10;
                    weight[11] = 100;
                    weight[14] = 8;
                    weight[2] = 4;
                }
                else
                {
                    weight[7] = 0x28;
                    weight[8] = 0x14;
                    weight[14] = 0x46;
                    weight[2] = 0x1e;
                    weight[11] = 0x32;
                    weight[12] = 0x28;
                }
                break;
            case 1:
                weight[2] = 0;
                weight[3] = 0x14;
                weight[4] = 0x28;
                weight[14] = 100;
                weight[1] = 0;
                break;
            case 2:
                weight[6] = 10;
                weight[5] = 0x14;
                weight[14] = 0x46;
                weight[2] = 0x28;
                break;
            case 3:
                weight[14] = 10;
                weight[2] = 5;
                weight[10] = 100;
                weight[1] = 0;
                break;
            }
        }
    }
    else if (swapped)
    {
        weight[2] = 0x14;
        switch (weapon)
        {
        case 0:
            if (!distant)
            {
                weight[11] = 0x46;
                weight[12] = 0x1e;
                weight[16] = 0x14;
            }
            else
            {
                weight[7] = 0x28;
                weight[8] = 0x3c;
                weight[12] = 0x3c;
                weight[16] = 0x28;
            }
            break;
        case 1:
            weight[3] = 0x3c;
            weight[4] = 0x28;
            weight[13] = 0x1e;
            break;
        case 2:
            weight[5] = 0x28;
            weight[6] = 0x3c;
            break;
        case 3:
            weight[2] = 10;
            weight[9] = 100;
            weight[14] = 10;
            break;
        }
    }
    else
    {
        weight[2] = 0x1e;
        switch (weapon)
        {
        case 0:
            weight[16] = 0x1e;
            if (!distant)
            {
                weight[11] = 100;
                weight[12] = 0x14;
                weight[14] = 0xc;
                weight[2] = 6;
                weight[1] = 0;
            }
            else
            {
                weight[7] = 0x14;
                weight[8] = 0x28;
                weight[14] = 0x46;
                weight[1] = 0x1e;
                weight[12] = 0x3c;
            }
            break;
        case 1:
            weight[2] = 0;
            weight[3] = 0x32;
            weight[4] = 0x32;
            weight[13] = 0x32;
            weight[14] = 100;
            weight[1] = 0;
            break;
        case 2:
            weight[5] = 0x14;
            weight[6] = 0x28;
            weight[14] = 0x46;
            weight[2] = 0x28;
            break;
        case 3:
            weight[14] = 0xf;
            weight[2] = 10;
            weight[10] = 100;
            weight[1] = 0;
            break;
        }
    }
}

#define PRINCE_STATE_COUNT 17

// State 100: picks the next move from the weighted odds of FillWeights. A player within 500 units also pulls the
// pick towards the strafe, unless the picked move is one of the long ones.
static void StatePick(PrinceFrame &f)
{
    Kind43State &st = *f.st;
    CharSlot &slot = *f.slot;
    int weight[PRINCE_STATE_COUNT];
    for (int at = 0; at < PRINCE_STATE_COUNT; at++)
        weight[at] = 0;
    st.counter = 0;
    st.attackMotion = 0;
    int closeBy = f.distance2D < 500.0f;
    int healthy = slot.stats[STAT_MAX_HEALTH] / 2 < slot.stats[STAT_HEALTH] || st.healing != 0;
    FillWeights(weight, healthy, st.lastState == PRINCE_STATE_SWAP_WEAPON, st.weapon, f.distance3D > 500.0f);
    if (DifficultyIsHard())
        weight[2] = (unsigned int)weight[2] / 10;
    int total = 1;
    for (int at = 1; at < PRINCE_STATE_COUNT; at++)
        total += weight[at];
    int roll = FUN_1013f880() % total;
    int picked = 1;
    do
    {
        roll -= weight[picked];
        if (roll < 2)
        {
            st.state = picked;
            break;
        }
        picked++;
    } while (picked < PRINCE_STATE_COUNT);
    if (closeBy &&
        (((picked != 0xb && picked != 0xc && picked != 0x10 && picked != 9 && picked != 10) ||
          (FUN_1013f880() % 10000 < 0x65 && FUN_1013f880() % 10000 < 0x65)) &&
         (!DifficultyIsHard() || FUN_1013f880() % 100 < 10)))
    {
        picked = PRINCE_STATE_STRAFE;
        st.state = PRINCE_STATE_STRAFE;
    }
    st.frames = 0;
    st.lastState = picked;
}

// What the game does with the movement vector at the end of the frame.
static void ApplyMovement(PrinceFrame &f)
{
    Kind43State &st = *f.st;
    AnimModel &model = *f.model;
    if (f.moveMode == PRINCE_MOVE_WALK)
    {
        if (st.wobble != 0)
            f.move.z += (float)(st.heightPhase % 1000 - 0x19) / 12.5f;
        f.move.Copy(DAT_10802bf8[f.index].position * 0.9f + f.move * 0.1f);
        FUN_100241e0(f.index, f.move.x, f.move.y, f.move.z);
        DAT_10802bf8[f.index].position.Copy(f.move);
    }
    else if (f.moveMode == PRINCE_MOVE_PLACE)
    {
        f.move.Copy(DAT_10802bf8[f.index].position * 0.65f + f.move * 0.35f);
        DAT_10802bf8[f.index].position.Copy(f.move);
        f.move.Copy(f.move + f.ownPos);
        model.SetPosition(f.move.x, f.move.y, f.move.z);
    }
    else if (st.wobble != 0)
    {
        float bob = (float)(st.heightPhase % 1000 - 0x19) / 12.5f;
        DAT_10802bf8[f.index].position.Copy(DAT_10802bf8[f.index].position * 0.9f);
        DAT_10802bf8[f.index].position.z = bob * 0.1f + DAT_10802bf8[f.index].position.z;
        FUN_100241e0(f.index, DAT_10802bf8[f.index].position.x, DAT_10802bf8[f.index].position.y, DAT_10802bf8[f.index].position.z);
    }
}

// 0x100edea0 (Prince_Update). One frame of the prince in slot `index`. Once the game is over (DAT_10210d30) and the
// prince is not healing it runs the ending scene; otherwise it takes hits (the hit level DAT_102550e0 sends it to the
// strafe or voice lines), keeps itself inside the arena triangle and runs the state it is in (PRINCE_STATE_*). Each
// state sets the motion it wants and the movement vector; ApplyMovement and the motion request run at the end of
// the frame. Returns 1 in the ending scene, else 0.
int Prince_Update(int index)
{
    DAT_1020d438[index] = 1;
    if (DAT_1021f3c0[index] == PRINCE_SLOT_KIND_FREE_MARK)
    {
        FUN_100e21a0(index);
        return 0;
    }
    DAT_10daaae8[index].m_0098 |= PRINCE_SLOT_FLAG_SKIP;
    if (DAT_104bfdf0 != 0)
        return 0;
    if (DAT_104bfe1c != 0)
        return 0;
    if (DAT_104c0135 != 0)
        return 0;
    PrinceFrame f;
    f.index = index;
    f.model = &DAT_10db6ee8[index];
    f.slot = &DAT_10daaae8[index];
    f.st = &DAT_1020b430[index].kind43;
    f.moveMode = PRINCE_MOVE_NONE;
    f.motion = PRINCE_MOTION_NONE;
    f.motionLoop = 0;
    f.motionBlend = PRINCE_MOTION_BLEND;
    f.layer = DAT_10db6ee8[index].motion->layer;
    f.nextLayer = DAT_10db6ee8[index].motion->nextLayer;
    if (DAT_1020b430[index].kind43.syncedFlag == 0 || DAT_10315350[index] < DAT_1020b430[index].kind43.syncedFlag)
        DAT_1020b430[index].kind43.syncedFlag = DAT_10315350[index];
    else if (DAT_1020b430[index].kind43.syncedFlag < DAT_10315350[index])
        DAT_10315350[index] = (unsigned char)DAT_1020b430[index].kind43.syncedFlag;
    DAT_104c0110 = index;
    DAT_1021f6c0[index] = 0;
    f.playerPos.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    f.ownPos.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    if (DAT_104c0380 % 0x1e == index % 0x1e)
    {
        DAT_10db6ee8[index].m_0012 = 2;
        if (DAT_102022b8 != 0)
            FUN_1001c090(index, 0);
    }
    Vec4 offset = f.ownPos - f.playerPos;
    f.distance2D = offset.Length2D();
    offset = f.ownPos - f.playerPos;
    f.distance3D = offset.Length();

    if (DAT_10210d30[index] != 0)
    {
        DAT_10daaae8[index].stats[STAT_BUSY] = 1;
        if (DAT_1020b430[index].kind43.healing == 0)
            return RunEnding(index, f);
    }
    if (DAT_10daaae8[index].stats[STAT_BUSY] != 0)
        DAT_10daaae8[index].stats[STAT_BUSY] = 0;
    int level = DAT_102550e0[index];
    DAT_10210d30[index] = 0;
    DAT_1021f9c0[index] = 0;
    DAT_10daaae8[index].m_0141 = 0;
    if (level != 0)
    {
        DAT_1020b430[index].kind43.hurt = 1;
        if (level > 7)
            DAT_102550e0[index] = 7;
        int bothHard = DAT_10c635f8 == 2 && DAT_10c66cc0 == 2;
        int skipAdjust = 0;
        if (DAT_10db6ee8[index].holding == 0 || ((!bothHard || DAT_102550e0[index] < 3) && DAT_102550e0[index] < 8))
        {
            if (((bothHard && DAT_102550e0[index] > 2) || DAT_102550e0[index] > 5) &&
                (DAT_1020b430[index].kind43.state == PRINCE_STATE_CHARGE_A || DAT_1020b430[index].kind43.state == PRINCE_STATE_CHARGE_B ||
                 DAT_1020b430[index].kind43.state == PRINCE_STATE_SWAP_WEAPON))
            {
                if (DifficultyIsHard() && FUN_1013f880() % 100 > 4)
                    skipAdjust = 1;
                if (!skipAdjust)
                {
                    DAT_102550e0[index] = 5;
                    if (bothHard)
                        DAT_102550e0[index] = 2;
                    DAT_1020b430[index].kind43.state = PRINCE_STATE_STRAFE;
                    DAT_1020b430[index].kind43.frames = 0;
                }
            }
        }
        else
        {
            DAT_102550e0[index] = 7;
            if (bothHard)
                DAT_102550e0[index] = 2;
        }
        if (DAT_10db6ee8[index].holding == 0 && DAT_1020b430[index].kind43.state != PRINCE_STATE_LOCK_ON &&
            (!DifficultyIsHard() || FUN_1013f880() % 100 < 5))
        {
            DAT_1020b430[index].kind43.state = PRINCE_STATE_STRAFE;
            DAT_1020b430[index].kind43.frames = 0;
        }
        if (DAT_102550e0[index] != 0)
        {
            switch (FUN_1013f880() % 9)
            {
            case 0:
            case 2:
            case 4:
            case 6:
                FUN_100b9bb0(index, 0x4c, 0, 0);
                return 0;
            case 1:
            case 3:
            case 5:
            case 7:
                FUN_100b9bb0(index, 0x4d, 0, 0);
                return 0;
            default:
                FUN_100b9bb0(index, FUN_1013f880() % 3 + 0x4c, 0, 0);
                return 0;
            }
        }
    }
    int wobble = DAT_1020b430[index].kind43.wobble;
    DAT_1020b430[index].kind43.age++;
    DAT_10daaae8[index].m_0141 = 0;
    DAT_10255a88[index] = 0;
    DAT_10210d30[index] = 0;
    if (wobble == 0)
        DAT_10db6ee8[index].mode = 1;
    if (DAT_10db6ee8[index].mode == 0)
    {
        DAT_10db6ee8[index].m_238c = 0;
        DAT_1020b430[index].kind43.lastHeight = PRINCE_UNSET_HEIGHT;
    }
    else
    {
        int height = (int)f.ownPos.z;
        if (height == DAT_1020b430[index].kind43.lastHeight)
            DAT_10db6ee8[index].m_238c = 1;
        DAT_1020b430[index].kind43.lastHeight = height;
    }
    int phase = DAT_1020b430[index].kind43.heightPhase;
    DAT_10db6ee8[index].mode = 0;
    DAT_1101dddc = 100.0f;
    DAT_1020344c = 1.0f;
    if (phase / 1000 == 0)
    {
        DAT_1020b430[index].kind43.heightPhase = phase + 1;
        if ((phase + 1) % 1000 == 0x32)
            DAT_1020b430[index].kind43.heightPhase = phase + 0x3e9;
    }
    else
    {
        DAT_1020b430[index].kind43.heightPhase = phase - 1;
        if ((phase - 1) % 1000 == 0)
            DAT_1020b430[index].kind43.heightPhase = phase - 0x3e9;
    }
    if (DAT_1020b430[index].kind43.state != PRINCE_STATE_TELEPORT && CountEdgesOutsideArena(f.ownPos) != 0)
    {
        DAT_1020b430[index].kind43.state = PRINCE_STATE_TELEPORT;
        DAT_1020b430[index].kind43.frames = 0;
        DAT_10daaae8[index].m_0141 = 1;
        DAT_10db6ee8[index].holding = 1;
        FUN_10065b10(index, 0x13e, 1, 0);
    }

    switch (DAT_1020b430[index].kind43.state)
    {
    case PRINCE_STATE_SET_UP:
        StateSetUp(f);
        break;
    case PRINCE_STATE_STALK:
        StateStalk(f);
        break;
    case PRINCE_STATE_STRAFE:
        StateStrafe(f);
        break;
    case PRINCE_STATE_PUNCH_LEAN:
        StatePunchLean(f);
        break;
    case PRINCE_STATE_PUNCH:
        StatePunch(f);
        break;
    case PRINCE_STATE_GATLING_SPRAY:
        StateGatlingSpray(f);
        break;
    case PRINCE_STATE_GATLING_AIM:
        StateGatlingAim(f);
        break;
    case PRINCE_STATE_SWING:
        StateSwing(f);
        break;
    case PRINCE_STATE_SWING_FOLLOW:
        StateSwingFollow(f);
        break;
    case PRINCE_STATE_HIT_RECOVER:
        StateHitRecover(f);
        break;
    case PRINCE_STATE_CHARGE_A:
        StateCharge(f, 0);
        break;
    case PRINCE_STATE_CHARGE_B:
        StateCharge(f, 1);
        break;
    case PRINCE_STATE_LEAP:
        StateLeap(f);
        break;
    case PRINCE_STATE_BOMBARD:
        StateBombard(f);
        break;
    case PRINCE_STATE_SWAP_WEAPON:
        StateSwapWeapon(f);
        break;
    case PRINCE_STATE_TELEPORT:
        StateTeleport(f);
        break;
    case PRINCE_STATE_LOCK_ON:
        StateLockOn(f);
        break;
    case PRINCE_STATE_PICK:
        StatePick(f);
        break;
    default:
        DAT_1020b430[index].kind43.state = PRINCE_STATE_PICK;
        break;
    }

    if (DAT_1020b430[index].kind43.healing != 0)
    {
        DAT_10daaae8[index].stats[STAT_HEALTH] += 0x1e;
        if (DAT_10daaae8[index].stats[STAT_MAX_HEALTH] <= DAT_10daaae8[index].stats[STAT_HEALTH])
        {
            DAT_10daaae8[index].stats[STAT_HEALTH] = DAT_10daaae8[index].stats[STAT_MAX_HEALTH];
            DAT_1020b430[index].kind43.healing = 0;
        }
    }
    if (f.ownPos.z < 0.0f)
    {
        DAT_1020b430[index].kind43.wobble = 0;
        DAT_10db6ee8[index].matrix.translation.z = 0.0f;
    }
    ApplyMovement(f);
    if (f.motion >= 0)
    {
        int motion = f.motion;
        if (f.motion == 2 || f.motion == 4 || f.layer == 2 || f.layer == 4 || f.nextLayer == 2 || f.nextLayer == 4)
        {
            if (DAT_1020b430[index].kind43.weapon == 1)
                motion = 0x11;
            else if (DAT_1020b430[index].kind43.weapon == 2)
                motion = 0xc;
        }
        DAT_10db6ee8[index].PlayMotion(motion, f.motionLoop, f.motionBlend, 1.0f);
        if (DAT_10db6ee8[index].motion->blending != 0 && DAT_10db6ee8[index].motion->m_0004 > 0.0f)
            Sleep(0);
    }
    DAT_1020b430[index].kind43.hurt = 0;
    if (PrinceNode(index, PRINCE_NODE_ROOT_NULL) != 0)
        DAT_10868838[index][0] = PrinceNode(index, PRINCE_NODE_ROOT_NULL)->worldMatrix;
    return 0;
}
