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
#define PRINCE_ARENA_HALF_WIDTH 2150.0f          // the arena triangle: base corners at +-this, y = PRINCE_ARENA_BACK_Y
#define PRINCE_ARENA_BACK_Y -623.0f
#define PRINCE_ARENA_APEX_Y 3112.0f              // third corner at x = 0
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
#define PRINCE_MIN_SWING_LEAN 0.17453f           // smallest lean of a swing spark
#define PRINCE_LEAP_SPEED 40.0f                  // units per frame of the leap
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

// The heading that faces the player.
static __forceinline float FacePlayer(const Vec4 &playerPos, const Vec4 &ownPos)
{
    return FUN_10001f80(playerPos.x - ownPos.x, playerPos.y - ownPos.y) + PRINCE_PI;
}

static __forceinline int DifficultyIsHard()
{
    return DAT_10874848.TestScriptFlag(PRINCE_SCRIPT_FLAG_DIFFICULTY) == 1;
}

// The node `which` of the prince, or 0 when it is not attached.
static __forceinline Node *PrinceNode(int index, int which)
{
    return DAT_1021bdf8[index][which];
}

// The world matrix to attach an effect to: the first of the nodes `first`, `second` and the model root that exists.
static __forceinline const Matrix4 *EffectAnchor(int index, int first, int second)
{
    Node *node = PrinceNode(index, first);
    if (node == 0)
        node = PrinceNode(index, second);
    if (node == 0)
        node = DAT_10db6ee8[index].root;
    return &node->worldMatrix;
}

// Starts an effect of the prince: `owner` is the matrix it follows, `value` and `frame` are the effect's own numbers.
static __forceinline void StartEffect(int index, int effect, const Vec4 &where, const Matrix4 *owner, int value, int frame,
                        const Matrix4 *pose, const EffectOptions *options)
{
    FUN_1001d8b0(index, effect, where, -1, -1, owner, value, frame, pose, options);
}

// The options of a hand or weapon effect: the effect parameter of the prince, a scale and the follow-owner flag.
static __forceinline void SetUpOptions(int index, EffectOptions &options, float scale, int followOwner)
{
    options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
    options.m_0008 = scale;
    if (followOwner)
    {
        options.flags |= 2;
        options.m_000e = 5;
    }
}

// Says voice line `sound` with a random choice of the four prince lines (0x47..0x4a).
static __forceinline void SayRandomLine(int index)
{
    FUN_100b9bb0(index, (FUN_1013f880() & 3) + 0x47, 0, 0);
}

// Turns the root node of the prince about x by `lean` (the body bends towards the player while it fires or swings).
static __forceinline void LeanBody(int index, float lean)
{
    DAT_10db6ee8[index].root->matrix = RotationX(-lean);
}

// Moves `value` towards `goal` by `step` per frame; returns 1 once it has reached it.
static __forceinline int ApproachValue(float &value, float goal, float step)
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

// Brings an angle into -PRINCE_PI..PRINCE_PI.
static __forceinline void WrapAngle(float &angle)
{
    while (angle > PRINCE_PI)
        angle -= PRINCE_TWO_PI;
    while (angle < -PRINCE_PI)
        angle += PRINCE_TWO_PI;
}

// One edge of the arena triangle: 1 when the position and the arena centre lie on different sides of the edge that
// starts at `corner` and runs along `edge` (the angles from the edge to the position and to the centre have opposite
// signs). `delta` is the scratch vector.
static __forceinline int SeparatedByEdge(const Vec4 &position, const Vec4 &corner, const Vec4 &edge, Vec4 &delta)
{
    float edgeAngle = FUN_10001f80(edge.x, edge.y);
    delta.Copy(position - corner);
    float positionSide = FUN_10001f80(delta.x, delta.y) - edgeAngle;
    delta.Copy(corner * -1.0f);
    float centreSide = FUN_10001f80(delta.x, delta.y) - edgeAngle;
    WrapAngle(positionSide);
    WrapAngle(centreSide);
    return (positionSide < 0.0f && centreSide >= 0.0f) || (positionSide >= 0.0f && centreSide < 0.0f);
}

// The glow rings of the weapon: effect `effect` at the end node of the weapon (or its hand, or the root) every
// tenth frame of the state, scaled by `scale`.
static __forceinline void StartWeaponGlow(int index, int effect, float scale)
{
    EffectOptions options;
    Matrix4 pose;
    Vec4 where;
    where.Fill(0.0f);
    pose.SetIdentity();
    SetUpOptions(index, options, scale, 1);
    StartEffect(index, effect, where, EffectAnchor(index, PRINCE_NODE_ROOT_NULL, PRINCE_NODE_HAND_R), 0, 0, 0,
                &options);
}

// The burst of the knock-back punch at the right hand: effect 0x7ca at the end of the offset (60 behind and 20 above
// the hand), turned by `yaw` (and by the lean of the body when `tilted`), on an upright pose. Then a shot of the
// character at the same place. Only for a prince that has its hand node.
static __forceinline void StartPunchEffects(int index, int tilted, float tilt, float yaw)
{
    Node *hand = PrinceNode(index, PRINCE_NODE_HAND_R);
    if (hand != 0)
    {
        Matrix4 pose;
        EffectOptions options;
        Vec4 where;
        where.y = -60.0f;
        where.z = 20.0f;
        if (tilted)
            where.RotateX(-tilt);
        where.RotateZ(yaw);
        pose.SetIdentity();
        pose.SetTranslation(where + hand->worldMatrix.GetTranslation());
        where.Fill(0.0f);
        SetUpOptions(index, options, 1.3f, 0);
        StartEffect(index, 0x7ca, where, 0, 0, 0, &pose, &options);
        Vec4 handPos = hand->worldMatrix.GetTranslation();
        FUN_1001a680(index, &handPos, 0);
    }
}

// A shot of the hand: effect `effect` at the right hand on a pose turned by the heading and the lean of the body.
static __forceinline void StartTurnedHandEffect(int index, int effect, float tilt, float scale)
{
    Node *hand = PrinceNode(index, PRINCE_NODE_HAND_R);
    if (hand == 0)
        return;
    Matrix4 pose = RotationZ(DAT_10db6ee8[index].heading) * RotationX(-tilt);
    EffectOptions options;
    Vec4 where;
    pose.SetTranslation(where + hand->worldMatrix.GetTranslation());
    SetUpOptions(index, options, scale, 0);
    StartEffect(index, effect, where, 0, 0, 0, &pose, &options);
}

// Blends the movement vector of a punch: the stored step shrinks by `keep` every frame.
static __forceinline void DampStep(int index, int &moveMode, Vec4 &move, float keep)
{
    moveMode = PRINCE_MOVE_PLACE;
    move.Copy(DAT_10802bf8[index].step * keep);
    DAT_10802bf8[index].step.Copy(move);
}

// Starts the rise or fall of the body: a wobble step towards the height range of the state.
static __forceinline void SteerHeight(int index, const Vec4 &ownPos, Vec4 &move, int &moveMode, float low, float high, float stepSize)
{
    Kind43State &st = DAT_1020b430[index].kind43;
    if (ownPos.z > high)
    {
        move.z = -stepSize;
        st.heightPhase = 0x401;
        moveMode = PRINCE_MOVE_WALK;
    }
    if (ownPos.z < low)
    {
        move.z = stepSize;
        st.heightPhase = 0x19;
        moveMode = PRINCE_MOVE_WALK;
    }
}

// The gatling burst of the states 5 and 6: a shot of the hand effect 0x36b1 and the tracer 1999 every 4th frame,
// both at 70 in front of the hand. `tilted` (state 6) leans them by `tilt` and rolls the tracer about y.
static __forceinline void FireGatling(int index, int tilted, float tilt)
{
    AnimModel &model = DAT_10db6ee8[index];
    Node *hand = PrinceNode(index, PRINCE_NODE_HAND_R);
    Matrix4 pose;
    Vec4 where;
    EffectOptions options;
    SetUpOptions(index, options, 0.85f, 0);
    pose.SetIdentity();
    if (hand == 0)
    {
        hand = model.root->Find("Bip01 R Hand", 0);
        DAT_1021bdf8[index][PRINCE_NODE_HAND_R] = hand;
        if (hand == 0)
        {
            SayRandomLine(index);
            return;
        }
    }
    where.y = -70.0f;
    if (tilted)
        where.RotateX(-tilt);
    where.RotateZ(-model.heading);
    where.Copy(where + hand->worldMatrix.GetTranslation());
    if (tilted)
        pose = RotationZ(model.heading) * RotationX(-tilt);
    else
        pose = RotationZ(model.heading);
    pose.SetTranslation(where);
    where.Fill(0.0f);
    StartEffect(index, 0x36b1, where, 0, 0, 0, &pose, &options);
    where.y = -70.0f;
    if (tilted)
        where.RotateX(-tilt);
    where.RotateZ(-model.heading);
    where.Copy(where + hand->worldMatrix.GetTranslation());
    if (tilted)
    {
        float spread = ((float)(FUN_1013f880() % 0x1e - 0xf) * PRINCE_PI) / 180.0f;
        float roll = ((float)(FUN_1013f880() % 0x1e - 0xf) * PRINCE_PI) / 180.0f + tilt;
        pose = RotationZ(model.heading - 1.5708f + spread) * RotationY(roll);
    }
    else
    {
        pose = RotationZ(model.heading - 1.5708f + ((float)(FUN_1013f880() % 0x28 - 0x14) * PRINCE_PI) / 720.0f);
    }
    pose.SetTranslation(where);
    where.Fill(0.0f);
    StartEffect(index, 1999, where, 0, 0, 0, &pose, &options);
    FUN_10065b10(index, 0x143, 1, 0);
}

// Swings of the states 7 and 8: effect `effect` from the left hand on a pose turned about z by `turn` and about x
// by `rise` (the sparks of the weapon).
static __forceinline void StartLeftHandSwing(int index, int effect, float scale, float turn, float rise)
{
    Node *hand = PrinceNode(index, PRINCE_NODE_HAND_L);
    if (hand == 0)
    {
        hand = DAT_10db6ee8[index].root->Find("Bip01 L Hand", 0);
        DAT_1021bdf8[index][PRINCE_NODE_HAND_L] = hand;
        if (hand == 0)
            return;
    }
    Matrix4 pose;
    EffectOptions options;
    Vec4 where;
    pose = RotationZ(turn) * RotationX(rise);
    pose.SetTranslation(hand->worldMatrix.GetTranslation());
    SetUpOptions(index, options, scale, 0);
    StartEffect(index, effect, where, 0, 0, 0, &pose, &options);
}

// The hand-and-root links of a weapon: the spark effect `spark` follows the right hand, and every 5th frame a link
// effect `link` is drawn between the two nodes.
static __forceinline void StartLinkedSparks(int index, int spark, int link)
{
    EffectOptions options;
    Matrix4 pose;
    Vec4 where;
    where.Fill(0.0f);
    pose.SetIdentity();
    options.flags |= 2;
    Node *hand = PrinceNode(index, PRINCE_NODE_HAND_R);
    Node *tip = PrinceNode(index, PRINCE_NODE_ROOT_NULL);
    if (hand == 0)
        return;
    StartEffect(index, spark, where, 0, 0, 0, &hand->worldMatrix, &options);
    // Every 5th frame of the prince's age a lightning beam links the two nodes.
    if (tip != 0 && DAT_1020b430[index].kind43.age % 5 == 0)
    {
        options.owner = index + PRINCE_EFFECT_OWNER_BASE;
        FUN_1001d8b0(index, link, where, -1, -1, &hand->worldMatrix, (int)&tip->worldMatrix, 0, 0, &options);
    }
}

#define PRINCE_STATE_COUNT 17

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
    int moveMode = PRINCE_MOVE_NONE;
    int wantMotion = PRINCE_MOTION_NONE;
    int motionLoop = 0;
    int motionBlend = PRINCE_MOTION_BLEND;
    int layer = DAT_10db6ee8[index].motion->layer;
    int nextLayer = DAT_10db6ee8[index].motion->nextLayer;
    if (DAT_1020b430[index].kind43.syncedFlag == 0 || DAT_10315350[index] < DAT_1020b430[index].kind43.syncedFlag)
        DAT_1020b430[index].kind43.syncedFlag = DAT_10315350[index];
    else if (DAT_1020b430[index].kind43.syncedFlag < DAT_10315350[index])
        DAT_10315350[index] = (unsigned char)DAT_1020b430[index].kind43.syncedFlag;
    DAT_104c0110 = index;
    DAT_1021f6c0[index] = 0;
    Vec4 playerPos;
    Vec4 ownPos;
    Vec4 move;
    playerPos.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    ownPos.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    if (DAT_104c0380 % 0x1e == index % 0x1e)
    {
        DAT_10db6ee8[index].m_0012 = 2;
        if (DAT_102022b8 != 0)
            FUN_1001c090(index, 0);
    }
    Vec4 offset = ownPos - playerPos;
    float distance2D = offset.Length2D();
    offset = ownPos - playerPos;
    float distance3D = offset.Length();

    if (DAT_10210d30[index] != 0)
    {
        DAT_10daaae8[index].stats[STAT_BUSY] = 1;
        if (DAT_1020b430[index].kind43.healing == 0)
            // The ending scene (DAT_10210d30 != 0 while the prince is not healing). The camera slides along the prince while the
            // screen fades and the models char11en/char11pa take over; the age counts from -30 (PRINCE_ENDING_START). Returns 1.
            {
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
                DAT_10db6ee8[index].m_17e8 = 1;
                endingA.m_17e8 = 1;
                endingB.m_17e8 = 1;
                DAT_104c0354 = 2;
                DAT_10daaae8[index].m_0141 = 1;
                DAT_10daaae8[index].stats[STAT_EFFECT_PARAM] = 0;
                int timer = DAT_1020b430[index].kind43.age;
                if (timer == 1)
                {
                    FUN_1000ec70(0xf, 1.0f, 1.0f, 1.0f, 1.0f);
                    Vec4 spot = DAT_10db6ee8[index].matrix.GetTranslation();
                    spot.z += 50.0f;
                    StartEffect(index, 0x33, spot, 0, 0, 0, 0, 0);
                    DAT_10db6ee8[index].PlayMotion(0x23, 1, 5, 1.0f);
                    DAT_10db6ee8[index].mode = 0;
                    endingA.mode = 0;
                    endingB.mode = 0;
                    endingA.m_17e8 = 1;
                    endingB.m_17e8 = 1;
                    DAT_10daaa80.RemoveOwnedBy(index + PRINCE_EFFECT_OWNER_BASE);
                }
                else if (timer < 1 && timer % 10 == 0)
                {
                    Vec4 spot = DAT_10db6ee8[index].matrix.GetTranslation();
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
                    DAT_10874848.MarkKilled(DAT_10daaae8[index].routeRecord);
                    FUN_1000ec70(0xf, 1.0f, 1.0f, 1.0f, 1.0f);
                    DAT_10db6ee8[index].SetPosition(PRINCE_FAR_X, 0.0f, 0.0f);
                    endingA.SetPosition(PRINCE_FAR_X, 0.0f, 0.0f);
                    endingB.SetPosition(PRINCE_FAR_X, 0.0f, 0.0f);
                    DAT_104c00c8 = 0;
                    DAT_104c0110 = 0;
                    FUN_1013fb10(3, 0, 10);
                    DAT_104c0354 = 0;
                }
                if (DAT_1020b430[index].kind43.syncedFlag == 0)
                {
                    DAT_10db6ee8[index].holding = 0;
                    DAT_10db6ee8[index].PlayMotion(2, 0, 5, 1.0f);
                    FUN_10065b10(index, 0x2d1, 1, 0);
                    if (DAT_104bfe78 == 0)
                        DAT_1020b430[index].kind43.syncedFlag = 1;
                    endingA.targetHeading = DAT_10db6ee8[index].targetHeading;
                    endingB.targetHeading = DAT_10db6ee8[index].targetHeading;
                    endingA.heading = DAT_10db6ee8[index].heading;
                    endingB.heading = DAT_10db6ee8[index].heading;
                    DAT_1020b430[index].kind43.age = PRINCE_ENDING_START;
                    endingA.m_0000.Copy(DAT_118e69e8);
                    endingB.m_0000.Copy(DAT_118e6cb8);
                    DAT_10255160[index][0] = (ownPos - DAT_118e6cb8).Length();
                    DAT_10db6ee8[index].root->M_101946b0(0, 0xffffffff);
                    endingA.root->M_101946b0(0, 0xffffffff);
                    endingB.root->M_101946b0(0, 0xffffffff);
                    DAT_10db6ee8[index].alpha = 1.0f;
                    DAT_104c00c8 = 1;
                    DAT_10453008[index] = -10001;
                    DAT_10daaae8[index].stats[STAT_EFFECT_PARAM] = 0;
                    endingA.alpha = 0.0f;
                    endingB.alpha = 0.0f;
                }
                timer = DAT_1020b430[index].kind43.age;
                if (timer > 0x3c)
                {
                    if (timer < 0xb4)
                    {
                        float left = (float)(0xb4 - timer);
                        DAT_10db6ee8[index].alpha = (left * left) / 14400.0f;
                    }
                    else
                    {
                        DAT_10db6ee8[index].m_17e8 = 1;
                        DAT_10db6ee8[index].holding = 1;
                        DAT_10db6ee8[index].motionFrame = endingA.motionFrame;
                        DAT_10db6ee8[index].alpha = 0.0f;
                    }
                }
                if (timer >= 0x3d && timer < 0x8c)
                {
                    float lift = (float)(0x28 - (timer > 100 ? timer - 100 : 100 - timer));
                    DAT_10db6ee8[index].SetPosition(DAT_10db6ee8[index].matrix.translation.x, DAT_10db6ee8[index].matrix.translation.y,
                                      DAT_10db6ee8[index].matrix.translation.z + (lift * 5.0f) / 40.0f);
                }
                else if (timer >= 0x8c)
                {
                    DAT_10db6ee8[index].SetPosition(DAT_10db6ee8[index].matrix.translation.x, DAT_10db6ee8[index].matrix.translation.y, DAT_10db6ee8[index].matrix.translation.z);
                    DAT_10db6ee8[index].m_17e8 = 1;
                }
                if (timer == 0x3c)
                {
                    FUN_1013fb10(3, 200, 0x1e);
                    FUN_100b9bb0(index, 0x4f, 1, 0);
                }
                timer = DAT_1020b430[index].kind43.age;
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
                                    DAT_10db6ee8[modelId].SetPosition(-100000.0f - DAT_10db6ee8[index].matrix.translation.x,
                                                                      DAT_10db6ee8[index].matrix.translation.y, DAT_10db6ee8[index].matrix.translation.z);
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
                timer = DAT_1020b430[index].kind43.age;
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
                    else if (DAT_1020b430[index].kind43.engaged < 300)     // sic: the original tests this word, not the age
                    {
                        if (DAT_1020b430[index].kind43.engaged == 0xb8)
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
                endingA.motionFrame = DAT_10db6ee8[index].motionFrame;
                DAT_1020b430[index].kind43.age++;
                endingB.motionFrame = DAT_10db6ee8[index].motionFrame;
                endingA.SetPosition(DAT_10db6ee8[index].matrix.translation.x, DAT_10db6ee8[index].matrix.translation.y, DAT_10db6ee8[index].matrix.translation.z);
                endingB.SetPosition(DAT_10db6ee8[index].matrix.translation.x, DAT_10db6ee8[index].matrix.translation.y, DAT_10db6ee8[index].matrix.translation.z);
                endingA.alpha = endingA.scale.w;
                endingB.alpha = endingB.scale.w;
                Vec4 look;
                look.Fill(0.0f);
                look.y += 950.0f;
                look.z += 200.0f;
                float spin = (float)(DAT_1020b430[index].kind43.age + 0x1e);
                look.RotateZ((-(Atan2(DAT_10868818.x - DAT_10da9150.x, DAT_10868818.y - DAT_10da9150.y) + PRINCE_PI) -
                              spin * spin * 3e-05f) - 0.04363f);
                DAT_10edbd48.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
                DAT_10866d98.Copy(DAT_10edbd48 + look * 1.3f);
                if (DAT_1020b430[index].kind43.age < 0xa1)
                {
                    float k = (float)(0xa0 - DAT_1020b430[index].kind43.age);
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
        int height = (int)ownPos.z;
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
    // A prince outside any edge of the arena triangle is sent back by the teleport state.
    if (DAT_1020b430[index].kind43.state != PRINCE_STATE_TELEPORT)
    {
        Vec4 cornerA;
        Vec4 cornerB;
        Vec4 cornerC;
        Vec4 delta;
        Vec4 edgeAB;
        Vec4 edgeBC;
        Vec4 edgeCA;
        int separated = 0;
        cornerA.Set(PRINCE_ARENA_HALF_WIDTH, PRINCE_ARENA_BACK_Y, 0.0f, 0.0f);
        cornerB.Set(-PRINCE_ARENA_HALF_WIDTH, PRINCE_ARENA_BACK_Y, 0.0f, 0.0f);
        cornerC.Set(0.0f, PRINCE_ARENA_APEX_Y, 0.0f, 0.0f);
        edgeAB.Copy(cornerB - cornerA);
        edgeBC.Copy(cornerC - cornerB);
        edgeCA.Copy(cornerA - cornerC);
        if (SeparatedByEdge(ownPos, cornerA, edgeAB, delta))
            separated = 1;
        if (SeparatedByEdge(ownPos, cornerB, edgeBC, delta))
            separated++;
        if (SeparatedByEdge(ownPos, cornerC, edgeCA, delta))
            separated++;
        if (separated != 0)
        {
            DAT_1020b430[index].kind43.state = PRINCE_STATE_TELEPORT;
            DAT_1020b430[index].kind43.frames = 0;
            DAT_10daaae8[index].m_0141 = 1;
            DAT_10db6ee8[index].holding = 1;
            FUN_10065b10(index, 0x13e, 1, 0);
        }
    }

    switch (DAT_1020b430[index].kind43.state)
    {
    case PRINCE_STATE_SET_UP:
        {
            AnimModel &endingA = DAT_10db6ee8[PRINCE_ENDING_MODEL_A];
            AnimModel &endingB = DAT_10db6ee8[PRINCE_ENDING_MODEL_B];
            if (DAT_10874848.TestScriptFlag(PRINCE_SCRIPT_FLAG_DIFFICULTY) == 1)
                DAT_10daaae8[index].stats[4] -= 5;
            DAT_10daaae8[index].stats[STAT_HEALTH] = DAT_10daaae8[index].stats[STAT_MAX_HEALTH];
            DAT_10254390[index] = 0x2d;
            DAT_1020b430[index].kind43.healing = 1;
            DAT_1020b430[index].kind43.state = PRINCE_STATE_PICK;
            FUN_10012450(index, 1, "O_100.it3");
            Node *root = DAT_10db6ee8[index].root;
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
        break;
    case PRINCE_STATE_STALK:
        {
            DAT_1020b430[index].kind43.frames++;
            SteerHeight(index, ownPos, move, moveMode, 25.0f, 65.0f, 5.0f);
            if (DAT_1020b430[index].kind43.wobble == 0)
                DAT_1020b430[index].kind43.wobble = 1;
            wantMotion = PRINCE_MOTION_IDLE;
            DAT_10db6ee8[index].targetHeading = FacePlayer(playerPos, ownPos);
            if (FUN_1013f880() % DAT_1020b430[index].kind43.frames < 0x97)
            {
                Vec4 away = playerPos - ownPos;
                if (away.Length() >= 500.0f)
                    break;
            }
            DAT_1020b430[index].kind43.state = PRINCE_STATE_PICK;
        }
        break;
    case PRINCE_STATE_STRAFE:
        {
            if (DAT_1020b430[index].kind43.frames == 0)
            {
                Vec4 away = ownPos - playerPos;
                away.z = 0.0f;
                Vec4 direction = away.Normalized3();
                direction.RotateZ(((float)(FUN_1013f880() % 2000) - 1000.0f) / 3600.0f * PRINCE_PI);
                DAT_10802bf8[index].step.Copy(direction * (float)(FUN_1013f880() % 1000 + 0x2ee));
                DAT_10802bf8[index].step.z = PRINCE_FLOOR_HEIGHT - ownPos.z;
                direction = DAT_10802bf8[index].step.Normalized3();
                DAT_10802bf8[index].step.Copy(direction * 80.0f);
                DAT_10db6ee8[index].targetHeading = FacePlayer(playerPos, ownPos);
                FUN_10065b10(index, 0x140, 1, 0);
                DAT_1020b430[index].kind43.frames++;
                break;
            }
            moveMode = PRINCE_MOVE_PLACE;
            move.Copy(DAT_10802bf8[index].step);
            move.Copy(move * 0.875f);
            // The angle between the heading and the movement, turned into -pi..pi.
            float delta = DAT_10db6ee8[index].targetHeading - FUN_10001f80(move.x, move.y) + PRINCE_PI;
            while (delta > PRINCE_PI)
                delta -= PRINCE_TWO_PI;
            while (delta < -PRINCE_PI)
                delta += PRINCE_TWO_PI;
            if (delta >= -0.785375f && delta < 0.785375f)
                wantMotion = 5;
            else if (delta >= 0.785375f && delta < 2.3561249f)
                wantMotion = 6;
            else if (delta >= -2.3561249f && delta < -0.785375f)
                wantMotion = 7;
            else
                wantMotion = 8;
            float speed = move.Length();
            if (speed < 20.0f)
                wantMotion = PRINCE_MOTION_IDLE;
            if (speed < 5.0f)
                DAT_1020b430[index].kind43.state = PRINCE_STATE_PICK;
            Matrix4 pose;
            pose.SetIdentity();
            pose.SetTranslation(move + ownPos);
            Vec4 spot = pose.GetTranslation();
            StartEffect(index, 0x36c2, spot, 0, 0, 0, &pose, 0);
            DAT_1020b430[index].kind43.frames++;
        }
        break;
    case PRINCE_STATE_PUNCH_LEAN:
        {
            float elevation = 0.0f;
            int reached = 0;
            DAT_1020b430[index].kind43.engaged = 1;
            DAT_10db6ee8[index].targetHeading = FacePlayer(playerPos, ownPos);
            // The lean of the body while it punches: the tilt (DAT_10255160[slot][0]) follows the
            // elevation of the player at 0.06 per frame. Sets `reached` once the tilt has reached it.
            if (DAT_1020b430[index].kind43.frames < PRINCE_FRAMES_PART_THREE)
            {
                float rise = distance3D;
                if (rise > 0.01f || rise < -0.01f)
                    elevation = 2.0f * (float)atan(distance2D / rise);
                else
                    elevation = 0.0f;
                elevation -= 0.25f;
                if (elevation <= 0.0f)
                    elevation = 0.0f;
                reached = ApproachValue(DAT_10255160[index][0], elevation, 0.06f);
                LeanBody(index, DAT_10255160[index][0]);
            }
            if (DAT_1020b430[index].kind43.frames < PRINCE_FRAMES_PART_TWO)
            {
                wantMotion = PRINCE_MOTION_IDLE;
                DAT_1020b430[index].kind43.wobble = 2;
                if (ownPos.z < 200.0f)
                {
                    moveMode = PRINCE_MOVE_PLACE;
                    move.z = 25.0f;
                    DAT_1020b430[index].kind43.heightPhase = 0x19;
                    DAT_1020b430[index].kind43.frames++;
                    break;
                }
                if (ownPos.z > 250.0f)
                {
                    moveMode = PRINCE_MOVE_PLACE;
                    move.z = -25.0f;
                    DAT_1020b430[index].kind43.heightPhase = 0x401;
                    DAT_1020b430[index].kind43.frames++;
                    break;
                }
                if (reached)
                {
                    DAT_1020b430[index].kind43.frames = PRINCE_FRAMES_PART_TWO;
                    wantMotion = 0x14;
                    DAT_1020b430[index].kind43.frames++;
                    break;
                }
            }
            else if (DAT_1020b430[index].kind43.frames < PRINCE_FRAMES_PART_THREE)
            {
                wantMotion = 0x14;
                if (DAT_10db6ee8[index].motionFrame == 0x171)
                {
                    DAT_1020b430[index].kind43.counter = 1;
                    move.Fill(0.0f);
                    DAT_10802bf8[index].step.Fill(0.0f);
                    DAT_10802bf8[index].step.y = -120.0f;
                    DAT_10802bf8[index].step.RotateX(DAT_10255160[index][0]);
                    DAT_10802bf8[index].step.RotateZ(PRINCE_PI - DAT_10db6ee8[index].targetHeading);
                    moveMode = PRINCE_MOVE_PLACE;
                    StartPunchEffects(index, 1, DAT_10255160[index][0], -DAT_10db6ee8[index].heading);
                    FUN_10065b10(index, 0x145, 1, 0);
                    SayRandomLine(index);
                }
                if (DAT_1020b430[index].kind43.counter != 0)
                {
                    DampStep(index, moveMode, move, 0.8f);
                    if (layer != 0x14)
                    {
                        DAT_1020b430[index].kind43.frames = PRINCE_FRAMES_PART_THREE;
                        DAT_1020b430[index].kind43.frames++;
                        break;
                    }
                }
            }
            else if (DAT_1020b430[index].kind43.frames < PRINCE_FRAMES_PART_FOUR)
            {
                DampStep(index, moveMode, move, 0.8f);
                if (DAT_10255160[index][0] <= 0.0f)
                {
                    DAT_10255160[index][0] = 0.0f;
                    DAT_1020b430[index].kind43.frames = PRINCE_FRAMES_PART_FOUR;
                }
                else
                {
                    DAT_10255160[index][0] -= 0.07f;
                    if (DAT_10255160[index][0] < 0.0f)
                    {
                        DAT_10255160[index][0] = 0.0f;
                        DAT_1020b430[index].kind43.frames = PRINCE_FRAMES_PART_FOUR;
                    }
                }
                LeanBody(index, DAT_10255160[index][0]);
                DAT_1020b430[index].kind43.frames++;
                break;
            }
            else if (DAT_1020b430[index].kind43.frames < 4000)
            {
                DampStep(index, moveMode, move, 0.8f);
                if (DAT_10802bf8[index].step.LengthSquared3() < 100.0f)
                {
                    wantMotion = PRINCE_MOTION_IDLE;
                    DAT_1020b430[index].kind43.state = PRINCE_STATE_PICK;
                }
            }
            DAT_1020b430[index].kind43.frames++;
        }
        break;
    case PRINCE_STATE_PUNCH:
        {
            DAT_1020b430[index].kind43.engaged = 1;
            DAT_10db6ee8[index].targetHeading = FacePlayer(playerPos, ownPos);
            DAT_10db6ee8[index].mode = 1;
            if (DAT_1020b430[index].kind43.frames < PRINCE_FRAMES_PART_TWO)
            {
                wantMotion = 2;
                DAT_1020b430[index].kind43.wobble = 0;
                if (DAT_10db6ee8[index].m_238c != 0)
                {
                    DAT_1020b430[index].kind43.frames = PRINCE_FRAMES_PART_TWO;
                    wantMotion = 0x14;
                    DAT_1020b430[index].kind43.frames++;
                    break;
                }
            }
            else if (DAT_1020b430[index].kind43.frames < PRINCE_FRAMES_PART_THREE)
            {
                wantMotion = 0x14;
                if (DAT_10db6ee8[index].motionFrame == 0x171)
                {
                    DAT_1020b430[index].kind43.counter = 1;
                    DAT_10802bf8[index].step.Fill(0.0f);
                    DAT_10802bf8[index].step.y = -80.0f;
                    DAT_10802bf8[index].step.RotateZ(PRINCE_PI - DAT_10db6ee8[index].targetHeading);
                    moveMode = PRINCE_MOVE_PLACE;
                    DAT_10255160[index][0] = 0.0f;
                    StartPunchEffects(index, 0, 0.0f, -DAT_10db6ee8[index].heading);
                    FUN_10065b10(index, 0x145, 1, 0);
                    SayRandomLine(index);
                }
                if (DAT_1020b430[index].kind43.counter != 0)
                {
                    DampStep(index, moveMode, move, 0.6f);
                    if (layer != 0x14)
                    {
                        DAT_1020b430[index].kind43.frames = PRINCE_FRAMES_PART_THREE;
                        DAT_1020b430[index].kind43.frames++;
                        break;
                    }
                }
            }
            else if (DAT_1020b430[index].kind43.frames < PRINCE_FRAMES_PART_FOUR)
            {
                DampStep(index, moveMode, move, 0.6f);
                if (DAT_10802bf8[index].step.LengthSquared3() < 100.0f)
                {
                    wantMotion = 2;
                    DAT_1020b430[index].kind43.state = PRINCE_STATE_PICK;
                }
            }
            DAT_1020b430[index].kind43.frames++;
        }
        break;
    case PRINCE_STATE_GATLING_SPRAY:
        {
            DAT_1020b430[index].kind43.engaged = 1;
            wantMotion = 0xf;
            if (DAT_1020b430[index].kind43.frames == 0)
            {
                DAT_10802bf8[index].step.Fill(0.0f);
                DAT_10802bf8[index].step.y = (float)(FUN_1013f880() % 10) * -0.6f;
                DAT_10802bf8[index].step.RotateZ((((float)(FUN_1013f880() % 0x10e - 0x87) * PRINCE_PI) / 180.0f - DAT_10db6ee8[index].targetHeading) +
                                  PRINCE_PI);
                Node *gatling = DAT_10db6ee8[index].root->Find("@gatring_null", 0);
                DAT_1021bdf8[index][PRINCE_NODE_GATLING] = gatling;
                if (gatling != 0)
                {
                    gatling->frameSource = &DAT_1020b430[index].kind43.frames;
                    gatling->RestartColourPlugs("r_start", 0);
                }
            }
            if (DAT_1020b430[index].kind43.frames < 10000)
            {
                DAT_1020b430[index].kind43.frames = DAT_1020b430[index].kind43.frames % 9 + 1;
                DAT_10db6ee8[index].targetHeading = FacePlayer(playerPos, ownPos);
                if (DAT_10db6ee8[index].motionFrame == 0x13a)
                {
                    DAT_1020b430[index].kind43.counter++;
                    if (FUN_1013f880() % 1000 < DAT_1020b430[index].kind43.counter * 0x28)
                        DAT_1020b430[index].kind43.frames = 10000;
                    else
                    {
                        DAT_10802bf8[index].step.Fill(0.0f);
                        DAT_10802bf8[index].step.y = -(float)(FUN_1013f880() % 10);
                        DAT_10802bf8[index].step.RotateZ((((float)(FUN_1013f880() % 0x10e - 0x87) * PRINCE_PI) / 180.0f -
                                           DAT_10db6ee8[index].targetHeading) + PRINCE_PI);
                    }
                }
                moveMode = PRINCE_MOVE_WALK;
                DAT_1020b430[index].kind43.wobble = 1;
                move.Copy(DAT_10802bf8[index].step);
                SteerHeight(index, ownPos, move, moveMode, 25.0f, 65.0f, 20.0f);
                if ((DAT_10db6ee8[index].motionFrame & 3) == 0)
                {
                    FireGatling(index, 0, 0.0f);
                    DAT_1020b430[index].kind43.frames++;
                    break;
                }
            }
            else if (DAT_1020b430[index].kind43.frames == 0x2723)
            {
                DAT_1020b430[index].kind43.state = PRINCE_STATE_PICK;
                Node *gatling = PrinceNode(index, PRINCE_NODE_GATLING);
                if (gatling != 0)
                {
                    gatling->RestartColourPlugs("stop", 0);
                    gatling->frameSource = 0;
                    DAT_1020b430[index].kind43.frames++;
                    break;
                }
            }
            DAT_1020b430[index].kind43.frames++;
        }
        break;
    case PRINCE_STATE_GATLING_AIM:
        {
            DAT_1020b430[index].kind43.engaged = 1;
            DAT_1020b430[index].kind43.wobble = 3;
            if (DAT_1020b430[index].kind43.frames == 0)
            {
                Node *gatling = DAT_10db6ee8[index].root->Find("@gatring_null", 0);
                DAT_1021bdf8[index][PRINCE_NODE_GATLING] = gatling;
                if (gatling != 0)
                {
                    gatling->frameSource = &DAT_1020b430[index].kind43.counter;
                    gatling->RestartColourPlugs("r_start", 0);
                }
            }
            ApproachValue(DAT_10255160[index][0], 0.675f, 0.06f);
            LeanBody(index, DAT_10255160[index][0]);
            if (DAT_1020b430[index].kind43.frames < PRINCE_FRAMES_PART_TWO)
            {
                DAT_10db6ee8[index].targetHeading = FacePlayer(playerPos, ownPos);
                if (ownPos.z >= 300.0f)
                {
                    if (ownPos.z <= 350.0f)
                    {
                        DAT_10802bf8[index].step.Copy(playerPos - ownPos);
                        DAT_10802bf8[index].step.z = 0.0f;
                        if ((playerPos - ownPos).LengthSquared3() < 1.0f)
                            DAT_10802bf8[index].step.y = ownPos.y <= playerPos.y ? 1.0f : -1.0f;
                        DAT_10802bf8[index].step.Copy(DAT_10802bf8[index].step.Normalized3() * 35.0f);
                        DAT_1020b430[index].kind43.frames = PRINCE_FRAMES_PART_TWO;
                    }
                    else
                    {
                        moveMode = PRINCE_MOVE_WALK;
                        move.z = -30.0f;
                        DAT_1020b430[index].kind43.heightPhase = 0x401;
                    }
                }
                else
                {
                    moveMode = PRINCE_MOVE_WALK;
                    move.z = 30.0f;
                    DAT_1020b430[index].kind43.heightPhase = 0x19;
                }
            }
            else
            {
                float blend = (float)(DAT_1020b430[index].kind43.frames - PRINCE_FRAMES_PART_TWO) / 30.0f;
                if (blend >= 1.0f)
                    blend = 1.0f;
                moveMode = PRINCE_MOVE_PLACE;
                move.Copy(DAT_10802bf8[index].step * blend);
            }
            DAT_1020b430[index].kind43.frames++;
            if (DAT_1020b430[index].kind43.frames < 0x2e)
            {
                wantMotion = PRINCE_MOTION_IDLE;
                break;
            }
            wantMotion = 0xf;
            if (DAT_1020b430[index].kind43.frames % 4 == 0)
            {
                FireGatling(index, 1, DAT_10255160[index][0]);
            }
            DAT_1020b430[index].kind43.counter++;
        }
        break;
    case PRINCE_STATE_SWING:
        {
            int frames = DAT_1020b430[index].kind43.frames;
            DAT_1020b430[index].kind43.engaged = 1;
            DAT_10db6ee8[index].mode = 1;
            DAT_1020b430[index].kind43.wobble = 0;
            if (frames < PRINCE_FRAMES_PART_TWO)
            {
                wantMotion = 2;
                DAT_10db6ee8[index].targetHeading = FacePlayer(playerPos, ownPos);
                if (DAT_10db6ee8[index].m_238c != 0)
                {
                    DAT_1020b430[index].kind43.frames = PRINCE_FRAMES_PART_TWO;
                    wantMotion = 0x1b;
                }
                break;
            }
            if (frames >= PRINCE_FRAMES_PART_THREE)
            {
                wantMotion = 2;
                DAT_1020b430[index].kind43.state = PRINCE_STATE_PICK;
                break;
            }
            wantMotion = 0x1b;
            if (DAT_10db6ee8[index].motionFrame == 0x248 || DAT_10db6ee8[index].motionFrame == 0x256)
            {
                DAT_10db6ee8[index].targetHeading = FacePlayer(playerPos, ownPos);
                DAT_1020b430[index].kind43.counter++;
                if (FUN_1013f880() % 1000 < DAT_1020b430[index].kind43.counter * 0x96)
                    DAT_1020b430[index].kind43.frames = PRINCE_FRAMES_PART_THREE;
            }
            if (DAT_10db6ee8[index].motionFrame == 0x245 || DAT_10db6ee8[index].motionFrame == 0x252)
            {
                StartLeftHandSwing(index, 0x36b4, 0.7f, DAT_10db6ee8[index].heading, 1.5708f);
                FUN_10065b10(index, 0x14e, 1, 0);
                if (FUN_1013f880() % 3 == 0)
                    FUN_100b9bb0(index, FUN_1013f880() % 3 + 0x47, 0, 1);
            }
            DAT_1020b430[index].kind43.frames++;
        }
        break;
    case PRINCE_STATE_SWING_FOLLOW:
        {
            float elevation = 0.0f;
            DAT_1020b430[index].kind43.engaged = 1;
            DAT_1020b430[index].kind43.wobble = 2;
            if (DAT_1020b430[index].kind43.frames >= PRINCE_FRAMES_PART_THREE || (distance3D <= 0.01f && distance3D >= -0.01f))
                elevation = 0.0f;
            else
            {
                elevation = (float)atan(distance2D / distance3D);
                if (elevation >= 1.5708f)
                    elevation = 1.5708f;
            }
            ApproachValue(DAT_10255160[index][0], elevation, 0.06f);
            if (DAT_1020b430[index].kind43.frames == 0)
            {
                DAT_1020b430[index].kind43.counter = 0x3c;
                wantMotion = PRINCE_MOTION_IDLE;
                DAT_10db6ee8[index].targetHeading = FacePlayer(playerPos, ownPos);
                DAT_1020b430[index].kind43.frames++;
                break;
            }
            if (DAT_1020b430[index].kind43.frames < PRINCE_FRAMES_PART_TWO)
            {
                DAT_1020b430[index].kind43.counter -= 4;
                wantMotion = PRINCE_MOTION_IDLE;
                if (DAT_1020b430[index].kind43.counter < 0x14)
                    DAT_1020b430[index].kind43.frames = PRINCE_FRAMES_PART_TWO;
                move.z = (float)DAT_1020b430[index].kind43.counter;
                moveMode = PRINCE_MOVE_PLACE;
                LeanBody(index, DAT_10255160[index][0]);
                DAT_10db6ee8[index].targetHeading = FacePlayer(playerPos, ownPos);
                DAT_1020b430[index].kind43.frames++;
                break;
            }
            if (DAT_1020b430[index].kind43.frames < PRINCE_FRAMES_PART_THREE)
            {
                wantMotion = 0x1b;
                if (DAT_1020b430[index].kind43.counter > 0)
                {
                    DAT_1020b430[index].kind43.counter -= 4;
                    move.z = (float)DAT_1020b430[index].kind43.counter;
                    moveMode = PRINCE_MOVE_PLACE;
                }
                if (DAT_10db6ee8[index].motionFrame == 0x248 || DAT_10db6ee8[index].motionFrame == 0x256)
                {
                    DAT_10db6ee8[index].targetHeading = FacePlayer(playerPos, ownPos);
                    DAT_1020b430[index].kind43.counter++;
                    if (FUN_1013f880() % 1000 < 700)
                        DAT_1020b430[index].kind43.frames = PRINCE_FRAMES_PART_THREE;
                }
                if (DAT_10db6ee8[index].motionFrame == 0x245 || DAT_10db6ee8[index].motionFrame == 0x252)
                {
                    float lean = ((float)(FUN_1013f880() % 0x14 - 10) * PRINCE_PI) / 180.0f + DAT_10255160[index][0];
                    if (lean <= PRINCE_MIN_SWING_LEAN)
                        lean = PRINCE_MIN_SWING_LEAN;
                    else
                        lean = ((float)(FUN_1013f880() % 0x14 - 10) * PRINCE_PI) / 180.0f + DAT_10255160[index][0];
                    float turn = ((float)(FUN_1013f880() % 0x14 - 10) * PRINCE_PI) / 180.0f + DAT_10db6ee8[index].heading;
                    StartLeftHandSwing(index, 0x36b6, 1.1f, turn, -lean);
                    FUN_10065b10(index, 0x142, 1, 0);
                    if (FUN_1013f880() % 3 == 0)
                        FUN_100b9bb0(index, FUN_1013f880() % 3 + 0x47, 0, 1);
                }
            }
            else
            {
                if (DAT_1020b430[index].kind43.frames >= PRINCE_FRAMES_PART_FOUR)
                {
                    DAT_10255160[index][0] = 0.0f;
                    DAT_1020b430[index].kind43.state = PRINCE_STATE_PICK;
                    DAT_1020b430[index].kind43.frames++;
                    break;
                }
                wantMotion = PRINCE_MOTION_IDLE;
                DAT_1020b430[index].kind43.wobble = 0;
                DAT_10db6ee8[index].mode = 1;
                if (DAT_10db6ee8[index].m_238c != 0 || ownPos.z < 0.0f)
                    DAT_1020b430[index].kind43.frames = PRINCE_FRAMES_PART_FOUR;
            }
            LeanBody(index, DAT_10255160[index][0]);
            DAT_1020b430[index].kind43.frames++;
        }
        break;
    case PRINCE_STATE_HIT_RECOVER:
        {
            int frames = DAT_1020b430[index].kind43.frames;
            DAT_1020b430[index].kind43.engaged = 1;
            if (frames == 0)
            {
                wantMotion = 2;
                DAT_1020b430[index].kind43.wobble = 0;
                DAT_10db6ee8[index].mode = 1;
                DAT_10db6ee8[index].targetHeading = FacePlayer(playerPos, ownPos);
                DAT_10db6ee8[index].m_238c = 0;
                FUN_100b9bb0(index, 0x4b, 0, 0);
            }
            else if (frames < PRINCE_FRAMES_PART_TWO)
            {
                wantMotion = 2;
                DAT_1020b430[index].kind43.wobble = 0;
                DAT_10db6ee8[index].mode = 1;
                if (DAT_10db6ee8[index].m_238c != 0 || ownPos.z < 0.0f)
                {
                    DAT_1020b430[index].kind43.frames = PRINCE_FRAMES_PART_TWO;
                    DAT_1020b430[index].kind43.wobble = 1;
                    FUN_10065b10(index, 0x147, 1, 0);
                }
                DAT_10db6ee8[index].targetHeading = FacePlayer(playerPos, ownPos);
            }
            else if (frames < PRINCE_FRAMES_PART_THREE)
            {
                wantMotion = 0x20;
                DAT_10db6ee8[index].holding = 1;
                DAT_10db6ee8[index].motionFrame = (frames - PRINCE_FRAMES_PART_TWO) / 3 + 0x406;
                DAT_10db6ee8[index].targetHeading = FacePlayer(playerPos, ownPos);
                if (DAT_10db6ee8[index].motionFrame == 0x410)
                {
                    DAT_1020b430[index].kind43.frames = PRINCE_FRAMES_PART_THREE;
                    Vec4 toPlayer = playerPos - ownPos;
                    toPlayer.z = 0.0f;
                    Vec4 direction = toPlayer.Normalized3();
                    DAT_10802bf8[index].step.Copy(direction * (distance2D - 100.0f) * 0.1f);
                    DAT_10802bf8[index].step.z = 9.0f;
                    move.Copy(DAT_10802bf8[index].step);
                }
            }
            else if (frames < PRINCE_FRAMES_PART_FOUR)
            {
                wantMotion = 0x20;
                if (DAT_10db6ee8[index].motionFrame < 0x415)
                {
                    if ((frames & 1) == 1)
                        DAT_10db6ee8[index].motionFrame++;
                    StartLinkedSparks(index, 0x7c6, 0x7cd);
                }
                else
                    DAT_10db6ee8[index].motionFrame = 0x415;
                if (DAT_1020b430[index].kind43.frames == 0x7ee)
                {
                    DAT_10db6ee8[index].holding = 0;
                    DAT_1020b430[index].kind43.state = PRINCE_STATE_PICK;
                    wantMotion = 2;
                }
                if (DAT_1020b430[index].kind43.frames < 0x7db)
                {
                    moveMode = PRINCE_MOVE_PLACE;
                    DAT_10802bf8[index].step.Copy(move);
                }
                else
                {
                    move.Copy(DAT_10802bf8[index].step * 0.5f);
                    DAT_1020b430[index].kind43.wobble = 0;
                }
                if (DAT_1020b430[index].kind43.frames == 0x7db)
                {
                    Matrix4 pose;
                    pose.SetIdentity();
                    EffectOptions options;
                    SetUpOptions(index, options, 2.0f, 1);
                    options.flags |= 2;
                    Vec4 where;
                    where.Fill(0.0f);
                    StartEffect(index, 0x36ba, where, EffectAnchor(index, PRINCE_NODE_ROOT_NULL, PRINCE_NODE_HAND_R), 0, 0, 0,
                                &options);
                    FUN_10065b10(index, 0x146, 1, 0);
                    FUN_100b9bb0(index, 0x49, 0, 0);
                }
            }
            if (DAT_1020b430[index].kind43.frames < 0x7df && PrinceNode(index, PRINCE_NODE_ROOT_NULL) != 0)
            {
                // The sparks that run along the line between the two hands of the prince while it is knocked back (state 9): one
                // effect every 30 units, spread by a random jolt of up to 25 units.
                {
                    Node *root = PrinceNode(index, PRINCE_NODE_ROOT_NULL);
                    Node *hand = PrinceNode(index, PRINCE_NODE_HAND_R);
                    if (root == 0 || hand == 0)
                        break;
                    Vec4 from = root->worldMatrix.GetTranslation();
                    Vec4 to = hand->worldMatrix.GetTranslation();
                    int count = (int)(to - from).Length() + 1;
                    EffectOptions options;
                    SetUpOptions(index, options, 1.0f, 0);
                    for (int at = 0, left = count; at < count; at += 0x1e, left -= 0x1e)
                    {
                        Vec4 spot = from * ((float)at / (float)count) + to * ((float)left / (float)count);
                        spot.x += (float)(FUN_1013f880() % 0x32 - 0x19);
                        spot.y += (float)(FUN_1013f880() % 0x32 - 0x19);
                        spot.z += (float)(FUN_1013f880() % 0x32 - 0x19);
                        Matrix4 pose;
                        pose.SetIdentity();
                        pose.SetTranslation(spot);
                        StartEffect(index, 0x8599, spot, 0, 0, 0, &pose, &options);
                    }
                }
                DAT_1020b430[index].kind43.frames++;
                break;
            }
            DAT_1020b430[index].kind43.frames++;
        }
        break;
    case PRINCE_STATE_CHARGE_A:
        // The charge state A: the prince picks one of four attack motions, then dashes with it along the heading it faced.
        // The state ends when the motion layer changes or the frame count passes 0x406.
        {
            if (DAT_1020b430[index].kind43.frames == 0)
            {
                DAT_10db6ee8[index].targetHeading = FacePlayer(playerPos, ownPos);
                if (distance2D <= 600.0f)
                {
                    switch (FUN_1013f880() % 4)
                    {
                    case 0:
                        DAT_1020b430[index].kind43.attackMotion = 0x22;
                        break;
                    case 1:
                        DAT_1020b430[index].kind43.attackMotion = 0x27;
                        break;
                    case 2:
                        DAT_1020b430[index].kind43.attackMotion = 0x28;
                        break;
                    case 3:
                        DAT_1020b430[index].kind43.attackMotion = 0x29;
                        break;
                    }
                    wantMotion = DAT_1020b430[index].kind43.attackMotion;
                    motionLoop = 1;
                    DAT_10802bf8[index].step.Fill(0.0f);
                    DAT_10802bf8[index].step.y = -30.0f;
                    DAT_10802bf8[index].step.RotateZ(-DAT_10db6ee8[index].targetHeading);
                    FUN_10065b10(index, 0x148, 1, 0);
                    FUN_100b9bb0(index, FUN_1013f880() % 4 + 0x47, 0, 0);
                }
                else
                {
                    DAT_1020b430[index].kind43.frames = -1;
                    DAT_1020b430[index].kind43.state = PRINCE_STATE_HIT_RECOVER;
                    if (FUN_1013f880() % 100 < 0x3c && DAT_1020b430[index].kind43.engaged != 0)
                        DAT_1020b430[index].kind43.state = PRINCE_STATE_SWAP_WEAPON;
                }
            }
            else
            {
                wantMotion = DAT_1020b430[index].kind43.attackMotion;
                move.x = DAT_10802bf8[index].step.x;
                move.y = DAT_10802bf8[index].step.y;
                moveMode = PRINCE_MOVE_WALK;
                move.Copy(move * 0.9f);
                if (DAT_1020b430[index].kind43.attackMotion == 0x27)
                {
                    if (ownPos.z < 90.0f)
                    {
                        move.z = 20.0f;
                        if (ownPos.z + 20.0f > 95.0f)
                            move.z = 95.0f - ownPos.z;
                    }
                    else if (ownPos.z > 100.0f)
                    {
                        move.z = -20.0f;
                        if (ownPos.z - 20.0f < 95.0f)
                            move.z = 95.0f - ownPos.z;
                    }
                }
                else if (ownPos.z > 5.0f)
                {
                    move.z = -20.0f;
                    if (ownPos.z - 20.0f < 0.0f)
                        move.z = -ownPos.z;
                }
                Vec4 where;
                Matrix4 pose;
                EffectOptions options;
                pose.SetIdentity();
                options.flags |= 2;
                where.Fill(0.0f);
                Node *hand = PrinceNode(index, PRINCE_NODE_HAND_R);
                Node *tip = PrinceNode(index, PRINCE_NODE_ROOT_NULL);
                // Every 5th frame of the prince's age a lightning beam links the hand and the tip of the weapon.
                if (hand != 0 && tip != 0)
                {
                    where.Fill(0.0f);
                    if (DAT_1020b430[index].kind43.age % 5 == 0)
                    {
                        options.owner = index + PRINCE_EFFECT_OWNER_BASE;
                        FUN_1001d8b0(index, 0x7c8, where, -1, -1, &hand->worldMatrix, (int)&tip->worldMatrix, 0, 0, &options);
                    }
                }
                int mf = DAT_10db6ee8[index].motionFrame;
                if ((mf > 0x3a0 && mf < 0x3a5) || (mf > 0x386 && mf < 0x38c) || (mf > 0x413 && mf < 0x415) || (mf > 0x442 && mf < 0x448))
                {
                    // The glow sits at the middle of the weapon (the tip and its parent), else at the hand or the prince.
                    if (tip != 0)
                        where.Copy((tip->worldMatrix.GetTranslation() + tip->parent->worldMatrix.GetTranslation()) * 0.5f);
                    else if (hand != 0)
                        where.Copy(hand->worldMatrix.GetTranslation());
                    else
                        where.Copy(ownPos);
                    pose = TranslationMatrix(where.x, where.y, where.z);
                    where.Fill(0.0f);
                    SetUpOptions(index, options, 1.1f, 1);
                    StartEffect(index, 0x36c1, where, 0, 0, 0, &pose, &options);
                }
            }
            if ((DAT_1020b430[index].kind43.counter != 0 && layer != DAT_1020b430[index].kind43.attackMotion) || DAT_1020b430[index].kind43.frames > 0x406)
                DAT_1020b430[index].kind43.state = PRINCE_STATE_PICK;
            if (layer == DAT_1020b430[index].kind43.attackMotion)
                DAT_1020b430[index].kind43.counter = 1;
            DAT_1020b430[index].kind43.frames++;
            DAT_1020b430[index].kind43.engaged = 1;
        }
        break;
    case PRINCE_STATE_CHARGE_B:
        // The charge state B: the prince picks one of four attack motions, then dashes with it along the heading it faced.
        // The state ends when the motion layer changes or the frame count passes 0x415.
        {
            if (DAT_1020b430[index].kind43.frames == 0)
            {
                DAT_10db6ee8[index].targetHeading = FacePlayer(playerPos, ownPos);
                if (distance2D <= 500.0f)
                {
                    switch (FUN_1013f880() % 4)
                    {
                    case 0:
                        DAT_1020b430[index].kind43.attackMotion = 0x1d;
                        break;
                    case 1:
                        DAT_1020b430[index].kind43.attackMotion = 0x1e;
                        break;
                    case 2:
                        DAT_1020b430[index].kind43.attackMotion = 0x1f;
                        break;
                    case 3:
                        DAT_1020b430[index].kind43.attackMotion = 0x22;
                        break;
                    }
                    wantMotion = DAT_1020b430[index].kind43.attackMotion;
                    motionLoop = 1;
                    DAT_10802bf8[index].step.Fill(0.0f);
                    DAT_10802bf8[index].step.y = -30.0f;
                    DAT_10802bf8[index].step.RotateZ(-DAT_10db6ee8[index].targetHeading);
                    FUN_10065b10(index, 0x14d, 1, 0);
                    FUN_100b9bb0(index, FUN_1013f880() % 4 + 0x47, 0, 0);
                }
                else
                {
                    DAT_1020b430[index].kind43.frames = -1;
                    DAT_1020b430[index].kind43.state = PRINCE_STATE_LEAP;
                    if (FUN_1013f880() % 100 < 0x3c && DAT_1020b430[index].kind43.engaged != 0)
                        DAT_1020b430[index].kind43.state = PRINCE_STATE_SWAP_WEAPON;
                }
            }
            else
            {
                wantMotion = DAT_1020b430[index].kind43.attackMotion;
                move.x = DAT_10802bf8[index].step.x;
                move.y = DAT_10802bf8[index].step.y;
                moveMode = PRINCE_MOVE_WALK;
                move.Copy(move * 0.92f);
                DAT_1020b430[index].kind43.wobble = 0;
                if (ownPos.z > 5.0f)
                {
                    move.z = -20.0f;
                    if (ownPos.z - 20.0f < 0.0f)
                        move.z = -ownPos.z;
                }
                Vec4 where;
                Matrix4 pose;
                EffectOptions options;
                pose.SetIdentity();
                options.flags |= 2;
                where.Fill(0.0f);
                Node *hand = PrinceNode(index, PRINCE_NODE_HAND_R);
                Node *tip = PrinceNode(index, PRINCE_NODE_ROOT_NULL);
                // Every 5th frame of the prince's age a lightning beam links the hand and the tip of the weapon.
                if (hand != 0 && tip != 0)
                {
                    where.Fill(0.0f);
                    if (DAT_1020b430[index].kind43.age % 5 == 0)
                    {
                        options.owner = index + PRINCE_EFFECT_OWNER_BASE;
                        FUN_1001d8b0(index, 0x7c7, where, -1, -1, &hand->worldMatrix, (int)&tip->worldMatrix, 0, 0, &options);
                    }
                }
                int mf = DAT_10db6ee8[index].motionFrame;
                if ((mf > 0x361 && mf < 0x366) || (mf > 0x386 && mf < 0x38c) || (mf > 0x39f && mf < 0x3aa) || (mf > 0x442 && mf < 0x448))
                {
                    // The glow sits at the middle of the weapon (the tip and its parent), else at the hand or the prince.
                    if (tip != 0)
                        where.Copy((tip->worldMatrix.GetTranslation() + tip->parent->worldMatrix.GetTranslation()) * 0.5f);
                    else if (hand != 0)
                        where.Copy(hand->worldMatrix.GetTranslation());
                    else
                        where.Copy(ownPos);
                    pose = TranslationMatrix(where.x, where.y, where.z);
                    where.Fill(0.0f);
                    SetUpOptions(index, options, 1.4f, 1);
                    StartEffect(index, 0x456, where, 0, 0, 0, &pose, &options);
                }
            }
            if ((DAT_1020b430[index].kind43.counter != 0 && layer != DAT_1020b430[index].kind43.attackMotion) || DAT_1020b430[index].kind43.frames > 0x415)
                DAT_1020b430[index].kind43.state = PRINCE_STATE_PICK;
            if (layer == DAT_1020b430[index].kind43.attackMotion)
                DAT_1020b430[index].kind43.counter = 1;
            DAT_1020b430[index].kind43.frames++;
            DAT_1020b430[index].kind43.engaged = 1;
        }
        break;
    case PRINCE_STATE_LEAP:
        {
            if (DAT_1020b430[index].kind43.frames == 0)
            {
                if (distance3D >= 500.0f)
                {
                    DAT_10802bf8[index].step.Copy(playerPos - ownPos);
                    DAT_10802bf8[index].step.Normalized3();
                    DAT_10802bf8[index].step.Copy(DAT_10802bf8[index].step * PRINCE_LEAP_SPEED);
                    DAT_1020b430[index].kind43.counter = (int)(distance3D / DAT_10802bf8[index].step.Length());
                    DAT_1020b430[index].kind43.wobble = 1;
                    DAT_10db6ee8[index].mode = 0;
                    wantMotion = 9;
                    motionLoop = 1;
                    FUN_10065b10(index, 0x14b, 1, 0);
                    FUN_100b9bb0(index, 0x4b, 1, 0);
                }
                else
                {
                    DAT_1020b430[index].kind43.frames = PRINCE_FRAMES_PART_TWO;
                    DAT_1020b430[index].kind43.wobble = 1;
                    DAT_10802bf8[index].step.Fill(0.0f);
                    DAT_10db6ee8[index].mode = 0;
                    wantMotion = 0xb;
                    motionLoop = 1;
                }
                DAT_10db6ee8[index].targetHeading = FacePlayer(playerPos, ownPos);
                DAT_1020b430[index].kind43.frames++;
                break;
            }
            if (DAT_1020b430[index].kind43.frames < PRINCE_FRAMES_PART_TWO)
            {
                wantMotion = 9;
                if (DAT_10db6ee8[index].motionFrame > 0x9b)
                {
                    DAT_1020b430[index].kind43.counter--;
                    moveMode = PRINCE_MOVE_PLACE;
                    move.Copy(DAT_10802bf8[index].step);
                }
                if (layer == 2 || DAT_1020b430[index].kind43.counter < 0)
                {
                    wantMotion = 0xb;
                    DAT_1020b430[index].kind43.frames = PRINCE_FRAMES_PART_TWO;
                    DAT_10db6ee8[index].targetHeading = FacePlayer(playerPos, ownPos);
                    DAT_10802bf8[index].step.Fill(0.0f);
                }
                StartLinkedSparks(index, 0x7c5, 0x7c7);
                if (DAT_1020b430[index].kind43.frames % 10 == 1)
                    StartWeaponGlow(index, 0x3715, 1.75f);
                DAT_1020b430[index].kind43.frames++;
                break;
            }
            wantMotion = 0xb;
            if (DAT_10db6ee8[index].motionFrame == 0xac)
            {
                DAT_10db6ee8[index].holding = 1;
                FUN_100b9bb0(index, (FUN_1013f880() & 3) + 0x47, 0, 0);
            }
            int mf = DAT_10db6ee8[index].motionFrame;
            if ((mf > 0xae && mf < 0xca) || (mf > 0xd5 && mf < 0xf8))
                StartLinkedSparks(index, 0x7c5, 0x7ce);
            mf = DAT_10db6ee8[index].motionFrame;
            int arc = 0;
            if (mf == 0xaf)
            {
                DAT_10db6ee8[index].targetHeading = FacePlayer(playerPos, ownPos);
                SayRandomLine(index);
            }
            else if (mf < 0xb0 || mf > 0xc5)
            {
                if (mf < 0xcd || mf > 0xd7)
                {
                    if (mf > 0xd9 && mf < 0xf3)
                    {
                        float rise = (float)((0xf3 - mf) * (mf - 0xbf));
                        moveMode = PRINCE_MOVE_PLACE;
                        move.y = (rise / 26.0f) * -1.2f;
                        move.RotateZ(-DAT_10db6ee8[index].targetHeading);
                        arc = 1;
                    }
                }
                else
                {
                    DAT_10db6ee8[index].targetHeading = FacePlayer(playerPos, ownPos);
                    if (DAT_10db6ee8[index].motionFrame == 0xd2)
                        SayRandomLine(index);
                }
            }
            else
            {
                float rise = (float)((0xc6 - mf) * (mf - 0x98));
                moveMode = PRINCE_MOVE_PLACE;
                move.y = -(rise / 23.0f);
                move.RotateZ(-DAT_10db6ee8[index].targetHeading);
                arc = 1;
            }
            if (arc)
            {
                if (ownPos.z <= 20.0f)
                {
                    if (ownPos.z < 20.0f)
                    {
                        move.z = 10.0f;
                        if (ownPos.z + 10.0f > 20.0f)
                            move.z = 20.0f - ownPos.z;
                    }
                }
                else
                {
                    move.z = -10.0f;
                    if (ownPos.z - 10.0f < 20.0f)
                        move.z = 20.0f - ownPos.z;
                }
            }
            if (DAT_10db6ee8[index].motionFrame == 0x102)
            {
                DAT_10db6ee8[index].holding = 0;
                wantMotion = PRINCE_MOTION_IDLE;
                DAT_1020b430[index].kind43.state = PRINCE_STATE_PICK;
            }
            if (DAT_10db6ee8[index].holding != 0)
                DAT_10db6ee8[index].motionFrame++;
            mf = DAT_10db6ee8[index].motionFrame;
            if ((mf > 0xaf && mf < 0xc5 && (mf - 0xaf) % 10 == 1) || (mf > 0xd4 && mf < 0xea && (mf - 0xd4) % 10 == 1))
                StartWeaponGlow(index, 0x36b8, 1.9f);
            if (DAT_10db6ee8[index].motionFrame == 0xaf || DAT_10db6ee8[index].motionFrame == 0xd5)
                FUN_10065b10(index, 0x14c, 1, 0);
            DAT_1020b430[index].kind43.frames++;
        }
        break;
    case PRINCE_STATE_BOMBARD:
        {
            if (DAT_1020b430[index].kind43.hurt != 0)
                DAT_1020b430[index].kind43.frames = 4000;
            if (DAT_1020b430[index].kind43.frames == 0)
            {
                DAT_1020b430[index].kind43.counter = -3;
                DAT_1020b430[index].kind43.wobble = 0;
                DAT_10db6ee8[index].mode = 1;
                DAT_10db6ee8[index].targetHeading = FacePlayer(playerPos, ownPos);
                DAT_1020b430[index].kind43.frames++;
                break;
            }
            if (DAT_1020b430[index].kind43.frames >= PRINCE_FRAMES_PART_TWO)
            {
                if (DAT_1020b430[index].kind43.frames >= PRINCE_FRAMES_PART_THREE)
                {
                    if (DAT_1020b430[index].kind43.frames >= PRINCE_FRAMES_PART_FOUR)
                    {
                        DAT_1020b430[index].kind43.state = PRINCE_STATE_PICK;
                        DAT_1020b430[index].kind43.frames++;
                        break;
                    }
                    wantMotion = 0x18;
                    if (DAT_10db6ee8[index].motionFrame == 0x1c2)
                    {
                        DAT_1020b430[index].kind43.frames = PRINCE_FRAMES_PART_FOUR + 1;
                        break;
                    }
                    DAT_1020b430[index].kind43.frames++;
                    break;
                }
                wantMotion = 0x16;
                DAT_10db6ee8[index].targetHeading = FacePlayer(playerPos, ownPos);
                if (DAT_10db6ee8[index].motionFrame == 0x1b8)
                {
                    DAT_10255160[index][0] = (float)(FUN_1013f880() % 300 - 0x96) / 5000.0f - 0.87606f;
                    StartPunchEffects(index, 1, DAT_10255160[index][0],
                                      (float)(FUN_1013f880() % 300 - 0x96) / 5000.0f - DAT_10db6ee8[index].targetHeading);
                    StartTurnedHandEffect(index, 0x3713, DAT_10255160[index][0], 1.3f);
                    DAT_10255160[index][0] = 0.0f;
                    FUN_10065b10(index, 0x145, 1, 0);
                }
                else if (DAT_10db6ee8[index].motionFrame == 0x1bc)
                {
                    DAT_1020b430[index].kind43.counter++;
                    if ((DAT_1020b430[index].kind43.counter >= 1 && FUN_1013f880() % 1000 < DAT_1020b430[index].kind43.counter * 0x32) || DAT_1020b430[index].kind43.counter == 7)
                    {
                        DAT_1020b430[index].kind43.frames = PRINCE_FRAMES_PART_THREE;
                        // Thirty glowing hits fall from the sky over the arena (effect 0x7cc, each living 30..119 frames).
                        {
                            EffectOptions options;
                            SetUpOptions(index, options, 1.3f, 0);
                            for (int hit = 0; hit < 0x1e; hit++)
                            {
                                float x = (float)(FUN_1013f880() % 0x10ae) - 2135.0f;
                                float limit = 3100.0f - (float)fabs(x) * 1.732051f;
                                float y = (float)(FUN_1013f880() % (int)limit) - 650.0f;
                                float z = ownPos.z + 900.0f;
                                Matrix4 pose = TranslationMatrix(x, y, z) * RotationZ(-1.5708f);
                                Vec4 where;
                                where.Set(x, y, z, 0.0f);
                                StartEffect(index, 0x7cc, where, 0, (int)&DAT_10db6ee8[0].matrix, FUN_1013f880() % 0x5a + 0x1e, &pose,
                                            &options);
                            }
                        }
                    }
                }
                DAT_1020b430[index].kind43.frames++;
                break;
            }
            DAT_1020b430[index].kind43.wobble = 0;
            DAT_10db6ee8[index].mode = 1;
            wantMotion = 2;
            DAT_10db6ee8[index].targetHeading = FacePlayer(playerPos, ownPos);
            if (DAT_10db6ee8[index].m_238c != 0)
            {
                DAT_1020b430[index].kind43.frames = PRINCE_FRAMES_PART_TWO;
                FUN_100b9bb0(index, 0x4b, 0, 0);
            }
            DAT_1020b430[index].kind43.frames++;
        }
        break;
    case PRINCE_STATE_SWAP_WEAPON:
        {
            if (DAT_1020b430[index].kind43.frames == 0)
            {
                if (DAT_1020b430[index].kind43.engaged != 0)
                {
                    DAT_1021bdf8[index][PRINCE_NODE_ROOT_NULL] = 0;
                    if (DAT_1020b430[index].kind43.weapon == 1)
                        wantMotion = 0x15;
                    else if (DAT_1020b430[index].kind43.weapon == 2)
                    {
                        DAT_1021bdf8[index][PRINCE_NODE_GATLING] = 0;
                        wantMotion = 0x10;
                    }
                    else
                        wantMotion = 0x25;
                    DAT_1020b430[index].kind43.engaged = 0;
                    DAT_1020b430[index].kind43.frames++;
                    break;
                }
                DAT_1020b430[index].kind43.state = PRINCE_STATE_PICK;
                DAT_1020b430[index].kind43.frames++;
                break;
            }
            if (DAT_1020b430[index].kind43.frames < PRINCE_FRAMES_PART_TWO)
            {
                int doneFrame;
                int oldWeapon = DAT_1020b430[index].kind43.weapon;
                if (DAT_1020b430[index].kind43.weapon == 1)
                {
                    wantMotion = 0x15;
                    doneFrame = 0x18d;
                }
                else if (DAT_1020b430[index].kind43.weapon == 2)
                {
                    wantMotion = 0x10;
                    doneFrame = 0x14a;
                }
                else
                {
                    wantMotion = 0x25;
                    doneFrame = 0x107;
                }
                if (DAT_10db6ee8[index].motionFrame == doneFrame)
                    DAT_1020b430[index].kind43.weapon = -1;
                if (DAT_1020b430[index].kind43.weapon == -1)
                {
                    FUN_10012450(index, 0, 0);
                    DAT_10daaa80.RemoveOwnedBy(index + PRINCE_EFFECT_OWNER_BASE);
                    int picked;
                    do
                    {
                        picked = FUN_1013f880() & 3;
                    } while (picked == oldWeapon);
                    DAT_1020b430[index].kind43.weapon = picked;
                    if (picked == 1)
                    {
                        wantMotion = 0x13;
                        FUN_10012450(index, 1, "O_101.it3");
                    }
                    else if (picked == 2)
                    {
                        wantMotion = 0xe;
                        FUN_10012450(index, 1, "O_102.it3");
                    }
                    else
                    {
                        wantMotion = 0x26;
                        FUN_10012450(index, 1, picked == 3 ? "O_103.it3" : "O_100.it3");
                        DAT_1021bdf8[index][PRINCE_NODE_ROOT_NULL] = DAT_10db6ee8[index].root->Find("@s_null", 0);
                    }
                    FUN_10065b10(index, 0x13f, 1, 0);
                    DAT_1020b430[index].kind43.frames = PRINCE_FRAMES_PART_TWO;
                }
            }
            else if (DAT_1020b430[index].kind43.frames < PRINCE_FRAMES_PART_THREE)
            {
                if (DAT_1020b430[index].kind43.weapon == 1)
                    wantMotion = 0x13;
                else
                    wantMotion = DAT_1020b430[index].kind43.weapon == 2 ? 0xe : 0x26;
                if (layer == 0x11 || layer == 0x12 || layer == 0xc || layer == 0xd || layer == 2 || layer == 4)
                {
                    DAT_1020b430[index].kind43.state = PRINCE_STATE_PICK;
                    DAT_1020b430[index].kind43.frames++;
                    break;
                }
            }
            DAT_1020b430[index].kind43.frames++;
        }
        break;
    case PRINCE_STATE_TELEPORT:
        {
            DAT_1020b430[index].kind43.frames++;
            DAT_10daaae8[index].m_0141 = 1;
            if (DAT_1020b430[index].kind43.frames < 0xf)
            {
                DAT_10db6ee8[index].alpha = (float)(0xf - DAT_1020b430[index].kind43.frames) / 15.0f;
                break;
            }
            if (DAT_1020b430[index].kind43.frames == 0xf)
            {
                DAT_10daaa80.RemoveOwnedBy(index + PRINCE_EFFECT_OWNER_BASE);
                DAT_10db6ee8[index].alpha = 0.0f;
                Vec4 spot = playerPos;
                spot.y -= 650.0f;
                spot.z = 0.0f;
                Vec4 direction = spot.Normalized3();
                direction.RotateZ(PRINCE_PI);
                spot.Copy(direction * 1000.0f);
                spot.y += 650.0f;
                spot.z = PRINCE_FLOOR_HEIGHT;
                DAT_10db6ee8[index].SetPosition(spot.x, spot.y, spot.z);
                DAT_10255160[index][0] = 0.0f;
                if (DAT_1020b430[index].kind43.weapon == 2 && DAT_1021bdf8[index][PRINCE_NODE_GATLING] != 0)
                {
                    DAT_1021bdf8[index][PRINCE_NODE_GATLING]->RestartColourPlugs("stop", 0);
                    DAT_1021bdf8[index][PRINCE_NODE_GATLING]->frameSource = 0;
                }
                wantMotion = PRINCE_MOTION_IDLE;
                motionLoop = 1;
                DAT_10db6ee8[index].holding = 0;
                LeanBody(index, 0.0f);
                FUN_1001a760(index, 0, 0);
                float heading = FUN_10001f80(playerPos.x - spot.x, playerPos.y - spot.y) + PRINCE_PI;
                DAT_10db6ee8[index].targetHeading = heading;
                DAT_10db6ee8[index].heading = heading;
                DAT_10802bf8[index].step.Fill(0.0f);
                FUN_10065b10(index, 0x13d, 1, 0);
                break;
            }
            if (DAT_1020b430[index].kind43.frames < 0x1e)
            {
                DAT_10db6ee8[index].alpha = (float)(DAT_1020b430[index].kind43.frames - 0xe) / 15.0f;
                break;
            }
            DAT_10db6ee8[index].alpha = 1.0f;
            DAT_10daaae8[index].m_0141 = 0;
            DAT_1020b430[index].kind43.state = PRINCE_STATE_PICK;
        }
        break;
    case PRINCE_STATE_LOCK_ON:
        {
            if (DAT_1020b430[index].kind43.frames == 0)
            {
                DAT_1020b430[index].kind43.wobble = 1;
                DAT_1020b430[index].kind43.counter = -5;
                DAT_10db6ee8[index].targetHeading = FacePlayer(playerPos, ownPos);
                FUN_10065b10(index, 0x14a, 1, 0);
                DAT_1020b430[index].kind43.frames++;
                break;
            }
            if (DAT_1020b430[index].kind43.frames < PRINCE_FRAMES_PART_TWO)
            {
                wantMotion = 0x19;
                if (DAT_10db6ee8[index].motionFrame == 0x1e0)
                {
                    FUN_1001a760(index, 1, "O_108.it3");
                    DAT_1021bdf8[index][PRINCE_NODE_LOCK] = DAT_10db6ee8[index].root->Find("@lk_null", 0);
                    FUN_10065b10(index, 0x13f, 1, 0);
                }
                else if (DAT_10db6ee8[index].motionFrame == 0x203)
                {
                    DAT_10802bf8[index].step.Fill(0.0f);
                    DAT_1020b430[index].kind43.frames = PRINCE_FRAMES_PART_TWO;
                    FUN_100b9bb0(index, (FUN_1013f880() & 1) + 0x47, 0, 0);
                }
                if (layer == 0x2a)
                {
                    if (DAT_1021bdf8[index][PRINCE_NODE_LOCK] == 0)
                    {
                        FUN_1001a760(index, 1, "O_108.it3");
                        DAT_1021bdf8[index][PRINCE_NODE_LOCK] = DAT_10db6ee8[index].root->Find("@lk_null", 0);
                    }
                    DAT_10802bf8[index].step.Fill(0.0f);
                    DAT_1020b430[index].kind43.frames = PRINCE_FRAMES_PART_TWO;
                    DAT_1020b430[index].kind43.frames++;
                    break;
                }
            }
            else if (DAT_1020b430[index].kind43.frames < PRINCE_FRAMES_PART_THREE)
            {
                wantMotion = 0x2a;
                if (DAT_10db6ee8[index].motionFrame == 0x210)
                {
                    DAT_1020b430[index].kind43.counter++;
                    if (DAT_1020b430[index].kind43.counter > 0 && FUN_1013f880() % 1000 < DAT_1020b430[index].kind43.counter * 2)
                        DAT_1020b430[index].kind43.frames = PRINCE_FRAMES_PART_THREE;
                }
                moveMode = PRINCE_MOVE_WALK;
                if (ownPos.x > playerPos.x)
                    DAT_10802bf8[index].step.x -= 2.0f;
                else if (ownPos.x < playerPos.x)
                    DAT_10802bf8[index].step.x += 2.0f;
                if (ownPos.y > playerPos.y)
                    DAT_10802bf8[index].step.y -= 2.0f;
                else if (ownPos.y < playerPos.y)
                    DAT_10802bf8[index].step.y += 2.0f;
                if (playerPos.z + 25.0f > ownPos.z)
                {
                    move.z = 15.0f;
                    DAT_1020b430[index].kind43.heightPhase = 0x19;
                }
                else if (playerPos.z + 65.0f < ownPos.z)
                {
                    move.z = -15.0f;
                    DAT_1020b430[index].kind43.heightPhase = 0x401;
                }
                if (DAT_10802bf8[index].step.Length2D() > 30.0f)
                    DAT_10802bf8[index].step.Copy(DAT_10802bf8[index].step.Normalized3() * 30.0f);
                move.x = DAT_10802bf8[index].step.x;
                move.y = DAT_10802bf8[index].step.y;
                // The sparks of both hands: the right hand (to the root node) and the left hand (to the lock node, turned
                // half a turn about x).
                Vec4 where;
                Matrix4 pose;
                EffectOptions options;
                pose.SetIdentity();
                options.flags |= 2;
                where.Fill(0.0f);
                Node *handR = PrinceNode(index, PRINCE_NODE_HAND_R);
                Node *tip = PrinceNode(index, PRINCE_NODE_ROOT_NULL);
                if (handR != 0)
                {
                    StartEffect(index, 0x7c5, where, 0, 0, 0, &handR->worldMatrix, &options);
                    if (tip != 0)
                    {
                        where.Fill(0.0f);
                        if (DAT_1020b430[index].kind43.age % 5 == 0)
                        {
                            options.owner = index + PRINCE_EFFECT_OWNER_BASE;
                            FUN_1001d8b0(index, 0x7c7, where, -1, -1, &handR->worldMatrix, (int)&tip->worldMatrix, 0, 0, &options);
                        }
                    }
                }
                options.owner = 0;
                Node *handL = PrinceNode(index, PRINCE_NODE_HAND_L);
                Node *lock = PrinceNode(index, PRINCE_NODE_LOCK);
                if (handL != 0)
                {
                    pose = RotationX(PRINCE_PI) * handL->worldMatrix;
                    StartEffect(index, 0x7c5, where, 0, 0, 0, &pose, &options);
                    if (lock != 0)
                    {
                        where.Fill(0.0f);
                        if (DAT_1020b430[index].kind43.age % 5 == 0)
                        {
                            options.owner = index + PRINCE_EFFECT_OWNER_BASE;
                            FUN_1001d8b0(index, 0x7c7, where, -1, -1, &handL->worldMatrix, (int)&lock->worldMatrix, 0, 0, &options);
                        }
                    }
                }
                if (DAT_1020b430[index].kind43.frames % 0x14 == 1)
                    FUN_10065b10(index, 0x149, 1, 0);
                if (DAT_1020b430[index].kind43.frames % 10 == 1)
                {
                    // Two glow rings, one at each arm: the weapon tip (else the right hand) and the lock node (else the left hand).
                    EffectOptions glow;
                    Vec4 glowWhere;
                    glow.flags |= 2;
                    glow.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                    glow.owner = index + PRINCE_EFFECT_OWNER_BASE;
                    glow.m_0008 = 1.55f;
                    glow.m_000e = 5;
                    glowWhere.Fill(0.0f);
                    StartEffect(index, 0x36b8, glowWhere, EffectAnchor(index, PRINCE_NODE_ROOT_NULL, PRINCE_NODE_HAND_R), 0, 0, 0, &glow);
                    glowWhere.Fill(0.0f);
                    StartEffect(index, 0x36b8, glowWhere, EffectAnchor(index, PRINCE_NODE_LOCK, PRINCE_NODE_HAND_L), 0, 0, 0, &glow);
                }
                DAT_1020b430[index].kind43.frames++;
                break;
            }
            else
            {
                if (DAT_1020b430[index].kind43.frames >= PRINCE_FRAMES_PART_FOUR)
                {
                    DAT_1020b430[index].kind43.state = PRINCE_STATE_PICK;
                    wantMotion = PRINCE_MOTION_IDLE;
                    DAT_1020b430[index].kind43.frames++;
                    break;
                }
                DAT_10db6ee8[index].motionFrame++;
                wantMotion = 0x2b;
                DAT_10db6ee8[index].holding = 1;
                DAT_10daaa80.RemoveOwnedBy(index + PRINCE_EFFECT_OWNER_BASE);
                if (DAT_10db6ee8[index].motionFrame == 0x230)
                {
                    FUN_1001a760(index, 0, 0);
                    DAT_1021bdf8[index][PRINCE_NODE_LOCK] = 0;
                }
                if (DAT_10db6ee8[index].motionFrame == 0x239)
                {
                    DAT_1020b430[index].kind43.frames = PRINCE_FRAMES_PART_FOUR;
                    DAT_10db6ee8[index].holding = 0;
                    wantMotion = PRINCE_MOTION_IDLE;
                    DAT_1020b430[index].kind43.frames = PRINCE_FRAMES_PART_FOUR + 1;
                    break;
                }
            }
            DAT_1020b430[index].kind43.frames++;
        }
        break;
    case PRINCE_STATE_PICK:
        // State 100: picks the next move from the weighted odds of FillWeights. A player within 500 units also pulls the
        // pick towards the strafe, unless the picked move is one of the long ones.
        {
            int weight[PRINCE_STATE_COUNT];
            for (int at = 0; at < PRINCE_STATE_COUNT; at++)
                weight[at] = 0;
            DAT_1020b430[index].kind43.counter = 0;
            DAT_1020b430[index].kind43.attackMotion = 0;
            int closeBy = distance2D < 500.0f;
            int healthy = DAT_10daaae8[index].stats[STAT_MAX_HEALTH] / 2 < DAT_10daaae8[index].stats[STAT_HEALTH] || DAT_1020b430[index].kind43.healing != 0;
            // The odds of the moves (weights by state number) for the weapon in hand; `healthy` = more than half its health, or
            // healing; `swapped` = the last state was the weapon swap. `distant` = the player is more than 500 units away.
            {
                if (healthy)
                {
                    weight[2] = 10;
                    weight[1] = 10;
                    if ((DAT_1020b430[index].kind43.lastState == PRINCE_STATE_SWAP_WEAPON))
                    {
                        switch (DAT_1020b430[index].kind43.weapon)
                        {
                        case 0:
                            if (!(distance3D > 500.0f))
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
                        switch (DAT_1020b430[index].kind43.weapon)
                        {
                        case 0:
                            weight[1] = 0;
                            if (!(distance3D > 500.0f))
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
                else if ((DAT_1020b430[index].kind43.lastState == PRINCE_STATE_SWAP_WEAPON))
                {
                    weight[2] = 0x14;
                    switch (DAT_1020b430[index].kind43.weapon)
                    {
                    case 0:
                        if (!(distance3D > 500.0f))
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
                    switch (DAT_1020b430[index].kind43.weapon)
                    {
                    case 0:
                        weight[16] = 0x1e;
                        if (!(distance3D > 500.0f))
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
                    DAT_1020b430[index].kind43.state = picked;
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
                DAT_1020b430[index].kind43.state = PRINCE_STATE_STRAFE;
            }
            DAT_1020b430[index].kind43.frames = 0;
            DAT_1020b430[index].kind43.lastState = picked;
        }
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
    if (ownPos.z < 0.0f)
    {
        DAT_1020b430[index].kind43.wobble = 0;
        DAT_10db6ee8[index].matrix.translation.z = 0.0f;
    }
    // What the game does with the movement vector at the end of the frame.
    {
        if (moveMode == PRINCE_MOVE_WALK)
        {
            if (DAT_1020b430[index].kind43.wobble != 0)
                move.z += (float)(DAT_1020b430[index].kind43.heightPhase % 1000 - 0x19) / 12.5f;
            move.Copy(DAT_10802bf8[index].position * 0.9f + move * 0.1f);
            FUN_100241e0(index, move.x, move.y, move.z);
            DAT_10802bf8[index].position.Copy(move);
        }
        else if (moveMode == PRINCE_MOVE_PLACE)
        {
            move.Copy(DAT_10802bf8[index].position * 0.65f + move * 0.35f);
            DAT_10802bf8[index].position.Copy(move);
            move.Copy(move + ownPos);
            DAT_10db6ee8[index].SetPosition(move.x, move.y, move.z);
        }
        else if (DAT_1020b430[index].kind43.wobble != 0)
        {
            float bob = (float)(DAT_1020b430[index].kind43.heightPhase % 1000 - 0x19) / 12.5f;
            DAT_10802bf8[index].position.Copy(DAT_10802bf8[index].position * 0.9f);
            DAT_10802bf8[index].position.z = bob * 0.1f + DAT_10802bf8[index].position.z;
            FUN_100241e0(index, DAT_10802bf8[index].position.x, DAT_10802bf8[index].position.y, DAT_10802bf8[index].position.z);
        }
    }
    if (wantMotion >= 0)
    {
        int motion = wantMotion;
        if (wantMotion == 2 || wantMotion == 4 || layer == 2 || layer == 4 || nextLayer == 2 || nextLayer == 4)
        {
            if (DAT_1020b430[index].kind43.weapon == 1)
                motion = 0x11;
            else if (DAT_1020b430[index].kind43.weapon == 2)
                motion = 0xc;
        }
        DAT_10db6ee8[index].PlayMotion(motion, motionLoop, motionBlend, 1.0f);
        if (DAT_10db6ee8[index].motion->blending != 0 && DAT_10db6ee8[index].motion->m_0004 > 0.0f)
            Sleep(0);
    }
    DAT_1020b430[index].kind43.hurt = 0;
    if (PrinceNode(index, PRINCE_NODE_ROOT_NULL) != 0)
        DAT_10868838[index][0] = PrinceNode(index, PRINCE_NODE_ROOT_NULL)->worldMatrix;
    return 0;
}
