// Advancing a running effect by one frame (EffectManager::Step, called for every record of a list by Update in
// char_effects.cpp). An effect moves its record, runs the events of its definition and starts its child effects.

#include "vec4.h"

#include "matrix4.h"

#include "key_track.h"

#include "map_vars.h"

#include "effect_manager.h"

#include "game_load_data.h"

#include "matrix_transform.h"

extern "C" int FUN_1013f880(void);   // the game's random number

// A record whose effect is over (id) stays this many frames after its children have gone, then is unlinked.
#define EFFECT_ENDED (-2)
#define EFFECT_ENDED_LINGER_FRAMES 4
// How many points a light keeps before it stops taking new ones.
#define EFFECT_LIGHT_FILL_LIMIT 0x80
// The effect kinds that drive a light (6, 7, 0x16), the ones that jitter a ribbon (8, 9) and the ones that ask the
// host to test a hit box (0xf and 0x10; any other kind tests a ball).
#define EFFECT_KIND_RIBBON_8 8
#define EFFECT_KIND_RIBBON_9 9
#define EFFECT_KIND_LINE_BOX 0xf
#define EFFECT_KIND_SWEEP_BOX 0x10
// EffectDef::spawnKind: 0, 1 and 3 take the parent's matrix at the start (1 in full, 3 its position only) and then
// let go of the parent; 2 and 4 stay attached to it.
#define EFFECT_SPAWN_PARENT_MATRIX 1
#define EFFECT_SPAWN_ATTACHED 2
#define EFFECT_SPAWN_PARENT_POSITION 3
#define EFFECT_SPAWN_ATTACHED_POSITION 4
// Random angles are drawn from a circle of this many degrees (1/1000 radian for the spread of the start position).
#define EFFECT_RANDOM_DEGREES 360
#define EFFECT_SPREAD_STEP 1000.0f
#define EFFECT_SPREAD_RESOLUTION 100.0f
#define EFFECT_SPREAD_SCALE 0.01f
#define EFFECT_PI 3.1415f
#define EFFECT_HALF_TURN_DEGREES 180.0f
// The jitter of a ribbon point is drawn from -50..49 and scaled by 10 / 100.
#define EFFECT_JITTER_RANGE 100
#define EFFECT_JITTER_HALF 50
#define EFFECT_JITTER_SCALE 10.0f
#define EFFECT_JITTER_DIVISOR 100.0f
// A steering weight is stored in per cent.
#define EFFECT_PERCENT 100.0f
// The slack above the bottom of the visible area before an effect counts as in front of it.
#define EFFECT_AREA_HEIGHT_MARGIN 8.0f
// Defaults of the hit box when its track has no key, and how a track value widens a light.
#define EFFECT_BOX_LENGTH_DEFAULT 100.0f
#define EFFECT_BOX_RADIUS_DEFAULT 30.0f
#define EFFECT_LIGHT_WIDTH_DEFAULT 10.0f
#define EFFECT_LIGHT_WIDTH_SCALE 10.0f
// Screen effect commands asked of the host (EffectScreenHook).
#define EFFECT_SCREEN_FLASH 0.0f
#define EFFECT_SCREEN_FADE 1.0f
#define EFFECT_SCREEN_TINT 2.0f
#define EFFECT_SCREEN_SHAKE 4.0f
// A tint trigger packs three colour parts in one number: 1,000,000s, 100s and 1s of 1/10.
#define EFFECT_TINT_BIG_STEP 10000
#define EFFECT_TINT_STEP 100
#define EFFECT_TINT_UNIT 0.1f

// A random angle in radians, uniform over `degrees` degrees centred on 0.
static __forceinline float RandomTurn(float degrees)
{
    return ((float)(FUN_1013f880() % EFFECT_RANDOM_DEGREES) * degrees / (float)EFFECT_RANDOM_DEGREES -
            degrees * 0.5f) * EFFECT_PI / EFFECT_HALF_TURN_DEGREES;
}

// A random offset of one coordinate of a ribbon point, in -5..4.9.
static __forceinline float RandomJitter()
{
    return (float)(FUN_1013f880() % EFFECT_JITTER_RANGE - EFFECT_JITTER_HALF) * EFFECT_JITTER_SCALE /
           EFFECT_JITTER_DIVISOR;
}

// Gives the record a light of its own when it has none: the first free entry of DAT_118e99f8 (entry 0 is never
// used). Returns 0 when all are taken.
static __forceinline int AttachLight(EffectRecord &rec, int definition)
{
    if (rec.light == 0)
    {
        int light;
        for (light = 1; light < EFFECT_LIGHT_COUNT; light++)
        {
            if (DAT_118e99f8[light].active == 0)
                break;
        }
        if (light == EFFECT_LIGHT_COUNT)
            return 0;
        rec.light = (short)light;
        DAT_118e99f8[light].type = definition;
        DAT_118e99f8[light].frame = 0;
        DAT_118e99f8[light].fade = 1.0f;
        DAT_118e99f8[light].lastFrame = -1;
    }
    return 1;
}

// Turns the step of an effect towards the direction `toTarget` by the fraction `turn` (0 = keep the step, 1 = point
// straight at the target). The target is given in the world; the step is kept in the space of the effect's matrix,
// so the direction is brought into it first. The step keeps its length.
static __forceinline void SteerStep(EffectRecord &rec, const Vec4 &toTarget, float turn)
{
    Vec4 aim;
    float length = rec.step.Length();
    Matrix4 inverse = InverseMatrix(rec.worldMatrix);
    aim.Copy(toTarget);
    aim.Normalized3();
    Vec4 heading = rec.step;
    heading.Normalized3();
    aim.Copy(TransformNormal(aim, inverse) * turn + heading * (1.0f - turn));
    aim.Normalized3();
    rec.step.Copy(aim * length);
}

// Starts the `child.count` copies of a child effect of record `record` (each knows its number in
// options.m_0018). Returns how many of them stay attached to it.
static int StartChildEffects(EffectManager *manager, int record, const EffectChild &child, int updateLastFrame)
{
    EffectRecord &rec = DAT_1115cfe8[record];
    Vec4 position;
    position.Fill(0.0f);
    int attached = 0;
    for (int i = 0; i < child.count; i++)
    {
        int number = rec.options.m_0018;
        rec.options.m_0018 = i;
        attached += manager->Start(child.effect, position, record, 0, rec.matrixOwner, rec.m_0188, child.frame, 0,
                                   &rec.options, rec.pause);
        rec.options.m_0018 = number;
        if (updateLastFrame && child.frame > rec.m_0018)
            rec.m_0018 = child.frame;
    }
    return attached;
}

// 0x10184440 (EffectManager::Step). Advances record `record` of effect list `list` by one frame:
// - a record still waiting for its start delay only counts it down (returns 1); a finished one (id -2) waits for
//   its children and is then unlinked from the list;
// - the first update places the effect: start position with a random spread, a random turn of its direction and
//   the matrix it follows (owner or parent, by the definition's spawnKind);
// - every update moves it along `step` (re-aimed every few frames, steered towards the owner, the target or a
//   direction of the host), spins its matrix, keeps the position of the two parts and feeds the ribbons and
//   lights of the definition's kind;
// - then it runs the definition's hit test and events, starts the children due now and ends the effect when an
//   event says so or its time is up (the children of "at the end" start then).
// Returns 1 only while the record waits for its start delay.
int EffectManager::Step(int record, int list)
{
    EffectRecord &rec = DAT_1115cfe8[record];
    if (rec.frame > 0)
    {
        rec.frame--;
        return 1;
    }
    rec.m_001c++;
    int definition = rec.id;
    if (definition == EFFECT_ENDED)
    {
        if (rec.m_018c > 0)
            return 0;
        if (rec.m_001c > rec.m_0018 + EFFECT_ENDED_LINGER_FRAMES)
        {
            rec.id = EFFECT_NONE;
            if (rec.prev == EFFECT_NONE)
                DAT_1101d820[list] = rec.next;
            else
                DAT_1115cfe8[rec.prev].next = rec.next;
            if (rec.next == EFFECT_NONE)
                DAT_11018604[list] = rec.prev;
            else
                DAT_1115cfe8[rec.next].prev = rec.prev;
            if (rec.parent != EFFECT_NONE)
                DAT_1115cfe8[rec.parent].m_018c--;
        }
        rec.m_001c++;
        return 0;
    }
    EffectDef &def = DAT_1101dfdc[definition];
    EffectDef &head = DAT_1101dfdc[definition - 1];   // the first fields of a definition sit in the slot before it
    if (def.m_0008 != 0)
        return 0;
    rec.part = (char)(rec.m_001c & 1);
    int part = rec.part;
    rec.pad_018f[0] = 0;
    rec.pad_018f[1] = 0;

    if (rec.m_0010 == -1)
    {
        if (rec.frame < 0)
            rec.m_0010 = -rec.frame;
        else
            rec.m_0010 = 0;
        rec.frame = 0;
        rec.position.Copy(rec.position + head.m_0264[0]);
        rec.origin.Copy(rec.position);
        if (head.m_03d4 != 0.0f)
        {
            Vec4 spread;
            spread.Set(1.0f, 0.0f, 0.0f, 0.0f);
            spread.RotateZ((float)FUN_1013f880() / EFFECT_SPREAD_STEP);
            spread.Copy(spread * ((float)(FUN_1013f880() % (int)(head.m_03d4 * EFFECT_SPREAD_RESOLUTION)) *
                                  EFFECT_SPREAD_SCALE));
            if (def.m_0068 != 0)
            {
                int range = (int)head.m_03d4;
                spread.z = (float)(FUN_1013f880() % range - range / 2);
            }
            rec.position.Copy(rec.position + spread);
        }
        rec.startMatrix = TranslationMatrix(rec.position.x, rec.position.y, rec.position.z);
        rec.acceleration.Fill(0.0f);
        rec.localVelocity.Fill(0.0f);
        rec.spin.Fill(0.0f);
        Matrix4 turn;
        turn.SetIdentity();
        if ((int)head.m_0264[2].x != 0)
            turn = RotationX(RandomTurn(head.m_0264[2].x));
        if ((int)head.m_0264[2].y != 0)
            turn = turn * RotationY(RandomTurn(head.m_0264[2].y));
        if ((int)head.m_0264[2].z != 0)
            turn = turn * RotationZ(RandomTurn(head.m_0264[2].z));
        rec.velocity.x = RandomTurn(head.m_0264[7].x);
        rec.velocity.y = RandomTurn(head.m_0264[7].y);
        rec.velocity.z = RandomTurn(head.m_0264[7].z);
        rec.spin.x = RandomTurn(head.m_0264[5].x);
        rec.spin.y = RandomTurn(head.m_0264[5].y);
        rec.spin.z = RandomTurn(head.m_0264[5].z);
        rec.step.Copy(Project(head.m_0264[1], turn));
        rec.direction.Copy(rec.step);
        if (def.spawnKind == EFFECT_SPAWN_PARENT_MATRIX && rec.parent != EFFECT_NONE)
        {
            rec.matrix = DAT_1115cfe8[rec.parent].worldMatrix * rec.matrix;
            rec.startMatrix = rec.matrix;
        }
        if (def.spawnKind == EFFECT_SPAWN_PARENT_POSITION && rec.parent != EFFECT_NONE)
        {
            Matrix4 anchor;
            anchor = TranslationMatrix(DAT_1115cfe8[rec.parent].worldMatrix._41,
                                       DAT_1115cfe8[rec.parent].worldMatrix._42,
                                       DAT_1115cfe8[rec.parent].worldMatrix._43);
            rec.matrix = anchor * rec.matrix;
            rec.startMatrix = rec.matrix;
        }
        if (def.spawnKind == 0 || def.spawnKind == EFFECT_SPAWN_PARENT_MATRIX ||
            def.spawnKind == EFFECT_SPAWN_PARENT_POSITION)
            rec.parent = EFFECT_NONE;
    }
    rec.position.Copy(rec.position + rec.step);
    int interval = def.m_0058[0];
    if (interval > 0 && rec.m_0010 % interval == 0)
    {
        Matrix4 wobble;
        if (def.m_0058[1] != 0)
            wobble = RotationX(((float)(FUN_1013f880() % EFFECT_RANDOM_DEGREES * def.m_0058[1] / EFFECT_RANDOM_DEGREES) -
                                (float)def.m_0058[0] * 0.5f) * EFFECT_PI / EFFECT_HALF_TURN_DEGREES);
        if (def.m_0058[2] != 0)
            wobble = wobble * RotationY(((float)(FUN_1013f880() % EFFECT_RANDOM_DEGREES * def.m_0058[2] /
                                                 EFFECT_RANDOM_DEGREES) -
                                         (float)def.m_0058[1] * 0.5f) * EFFECT_PI / EFFECT_HALF_TURN_DEGREES);
        if (def.m_0058[3] != 0)
            wobble = wobble * RotationZ(((float)(FUN_1013f880() % EFFECT_RANDOM_DEGREES * def.m_0058[3] /
                                                 EFFECT_RANDOM_DEGREES) -
                                         (float)def.m_0058[2] * 0.5f) * EFFECT_PI / EFFECT_HALF_TURN_DEGREES);
        rec.step.Copy(Project(rec.direction, wobble));
    }
    if (def.spawnKind == EFFECT_SPAWN_ATTACHED)
    {
        if (rec.parent != EFFECT_NONE)
        {
            rec.startMatrix = DAT_1115cfe8[rec.parent].worldMatrix;
            rec.worldMatrix = DAT_1115cfe8[rec.parent].worldMatrix * rec.matrix;
        }
        else
            rec.worldMatrix = rec.matrix;
    }
    else if (def.spawnKind == EFFECT_SPAWN_ATTACHED_POSITION)
    {
        if (rec.parent != EFFECT_NONE)
        {
            Matrix4 anchor;
            anchor = TranslationMatrix(DAT_1115cfe8[rec.parent].worldMatrix._41,
                                       DAT_1115cfe8[rec.parent].worldMatrix._42,
                                       DAT_1115cfe8[rec.parent].worldMatrix._43);
            rec.startMatrix = anchor;
            rec.worldMatrix = anchor * rec.matrix;
        }
        else
            rec.worldMatrix = rec.matrix;
    }
    else
        rec.worldMatrix = rec.matrix;
    rec.m_0010++;

    Vec4 local;
    rec.velocity.Copy(rec.velocity + head.m_0264[6]);
    Matrix4 rotation;
    rotation.SetIdentity();
    int rotated = 0;
    if (rec.velocity.x != 0.0f)
    {
        rotation = RotationX(rec.velocity.x);
        rotated = 1;
    }
    if (rec.velocity.y != 0.0f)
    {
        rotation = rotation * RotationY(rec.velocity.y);
        rotated = 1;
    }
    if (rec.velocity.z != 0.0f)
    {
        rotation = rotation * RotationZ(rec.velocity.z);
        rotated = 1;
    }
    if (def.m_0104[2] == 1)
    {
        Vec4 turnKey;
        if (def.m_0104[3] == 1)
        {
            head.tracks[3].Sample(rec.m_0010, &turnKey.x, &turnKey.y, &turnKey.z, &turnKey.w, 0);
            turnKey.w = 0.0f;
            if (turnKey.x != 0.0f)
            {
                rotation = rotation * RotationX(turnKey.x);
                rotated = 1;
            }
            if (turnKey.y != 0.0f)
            {
                rotation = rotation * RotationY(turnKey.y);
                rotated = 1;
            }
            if (turnKey.z != 0.0f)
            {
                rotation = rotation * RotationZ(turnKey.z);
                rotated = 1;
            }
        }
        else
        {
            head.tracks[3].Sample(rec.m_0010, &turnKey.x, &turnKey.y, &turnKey.z, &turnKey.w, 0);
            turnKey.w = 0.0f;
            rotation.SetIdentity();
            if (turnKey.x != 0.0f)
            {
                rotation = RotationX(turnKey.x);
                rotated = 1;
            }
            if (turnKey.y != 0.0f)
            {
                rotation = rotation * RotationY(turnKey.y);
                rotated = 1;
            }
            if (turnKey.z != 0.0f)
            {
                rotation = rotation * RotationZ(turnKey.z);
                rotated = 1;
            }
        }
    }
    if (rotated)
        rec.worldMatrix = rotation * rec.worldMatrix;

    local.Copy(Project(rec.position, rec.worldMatrix));
    rec.acceleration.Copy(rec.acceleration + head.m_0264[3]);
    rec.localVelocity.Copy(rec.localVelocity + rec.acceleration);
    local.Copy(local + rec.localVelocity);

    // Where the effect moves to: the first two entries of the definition's table.
    for (int i = 0; i < 2; i++)
    {
        const EffectTarget &target = def.m_0114[i];
        if (target.type == EFFECT_TARGET_FOLLOW_OWNER)
        {
            if (rec.matrixOwner != 0)
                local.Copy(const_cast<Matrix4 *>(rec.matrixOwner)->GetTranslation());
        }
        else if (target.type == EFFECT_TARGET_FOLLOW_TARGET)
        {
            if (rec.m_0188 != 0)
                local.Copy(reinterpret_cast<Matrix4 *>(rec.m_0188)->GetTranslation());
        }
        else if (target.type == EFFECT_TARGET_STEER_OWNER)
        {
            float turn = (float)target.value / EFFECT_PERCENT;
            if (rec.matrixOwner != 0)
                SteerStep(rec, const_cast<Matrix4 *>(rec.matrixOwner)->GetTranslation() - local, turn);
        }
        else if (target.type == EFFECT_TARGET_STEER_TARGET)
        {
            float turn = (float)target.value / EFFECT_PERCENT;
            if (rec.m_0188 != 0)
                SteerStep(rec, reinterpret_cast<Matrix4 *>(rec.m_0188)->GetTranslation() - local, turn);
        }
        else if (target.type == EFFECT_TARGET_STEER_HOST)
        {
            float turn = (float)target.value / EFFECT_PERCENT;
            if (DAT_1101de40[EFFECT_HOST_AIM] != 0)
            {
                EffectOptions aimOptions;
                Vec4 aim;
                aimOptions.m_000e = rec.options.m_000e;
                aimOptions.m_0014 = rec.options.m_0014;
                aimOptions.m_0030 = &aim;
                int aimed = ((EffectHitHook)DAT_1101de40[EFFECT_HOST_AIM])(local.x, local.y, local.z,
                                                                          (float)rec.options.m_0018, &aimOptions);
                rec.options.m_0014 = aimOptions.m_0014;
                if (aimed > 0)
                    SteerStep(rec, *aimOptions.m_0030 - local, turn);
            }
        }
        else if (target.type == EFFECT_TARGET_SET_HEIGHT)
            local.z = (float)target.value;
    }

    rec.spin.Copy(rec.spin + head.m_0264[4]);
    rotation.SetIdentity();
    int spun = 0;
    if (rec.spin.x != 0.0f)
    {
        rotation = RotationX(rec.spin.x);
        spun = 1;
    }
    if (rec.spin.y != 0.0f)
    {
        rotation = rotation * RotationY(rec.spin.y);
        spun = 1;
    }
    if (rec.spin.z != 0.0f)
    {
        rotation = rotation * RotationZ(rec.spin.z);
        spun = 1;
    }
    if (def.m_0104[0] == 1)
    {
        Vec4 key;
        if (def.m_0104[1] == 1)
        {
            head.tracks[0].Sample(rec.m_0010, &key.x, &key.y, &key.z, &key.w, 0);
            key.w = 0.0f;
            local.Copy(local + key);
        }
        else
        {
            head.tracks[0].Sample(rec.m_0010, &key.x, &key.y, &key.z, &key.w, 0);
            key.w = 0.0f;
            local.Copy(key);
        }
    }
    rec.partPositions[part].Copy(local);
    rec.worldMatrix._41 = local.x;
    rec.worldMatrix._42 = local.y;
    rec.worldMatrix._43 = local.z;
    if (spun)
        rec.worldMatrix = rotation * rec.worldMatrix;

    if (def.kind == EFFECT_KIND_RIBBON_8 || def.kind == EFFECT_KIND_RIBBON_9)
    {
        DAT_11aea3f8 = def.lightFrames;
        if (DAT_11aea3f8 > 2)
        {
            Vec4 base;
            Vec4 offset;
            Vec4 offsetCopy;
            base.Copy(Project(rec.origin, rec.worldMatrix));
            offset.Copy(rec.partPositions[part]);
            offset.Copy(offset - base);
            offsetCopy.Copy(offset);
            for (int i = 0; i < DAT_11aea3f8; i++)
                DAT_11aeabfc[i + 1].Set(RandomJitter(), RandomJitter(), RandomJitter(), 0.0f);
            DAT_11aeabfc[1].Fill(0.0f);
            DAT_11aeabfc[DAT_11aea3f8].Fill(0.0f);
        }
        if ((def.spawnKind == EFFECT_SPAWN_ATTACHED || def.spawnKind == EFFECT_SPAWN_ATTACHED_POSITION) &&
            rec.parent != EFFECT_NONE)
            rec.origin.Copy(DAT_1115cfe8[rec.parent].worldMatrix.GetTranslation());
    }

    // The light of kinds 6, 7 and 0x16: one point per frame at the position of the current part.
    if (def.kind == 6 || def.kind == 7 || def.kind == EFFECT_KIND_LIGHT_22)
    {
        Vec4 point;
        Vec4 colour;
        Vec4 colour2;
        if (def.trackFlags[1] != 0)
            head.tracks[5].Sample(rec.m_0010, &colour.x, &colour.y, &colour.z, &colour.w, 0);
        else
            colour.Fill(1.0f);
        if (def.trackFlags[2] != 0)
            head.tracks[6].Sample(rec.m_0010, &colour2.x, &colour2.y, &colour2.z, &colour2.w, 0);
        else
            colour2.Fill(0.0f);
        float strength;       // its value is not used
        float strengthRest;
        if (def.trackFlags[6] != 0)
            head.tracks[10].Sample(rec.m_0010, &strength, &strengthRest, &strengthRest, &strengthRest, 0);
        else
            strength = 0.0f;
        point.Copy(rec.partPositions[rec.part]);
        float width;
        if (def.trackFlags[0] != 0)
        {
            float widthX;
            float widthY;
            float widthZ;
            float widthW;
            head.tracks[4].Sample(rec.m_0010, &widthX, &widthY, &widthZ, &widthW, 0);
            width = widthX * EFFECT_LIGHT_WIDTH_SCALE;
        }
        else
            width = EFFECT_LIGHT_WIDTH_DEFAULT;
        if (def.flat != 0)
            point.z = 0.0f;
        if (!AttachLight(rec, definition))
            return 0;
        if (DAT_118e99f8[rec.light].lastFrame != rec.m_0010)
        {
            int count = DAT_118e99f8[rec.light].pointCount;
            if (count < EFFECT_LIGHT_FILL_LIMIT)
            {
                DAT_118e99f8[rec.light].points[count].Copy(point);
                DAT_118e99f8[rec.light].widths[count].Fill(width);
                DAT_118e99f8[rec.light].colours[count].Copy(colour);
                DAT_118e99f8[rec.light].specular[count].Copy(colour2);
                DAT_118e99f8[rec.light].pointCount++;
            }
            DAT_118e99f8[rec.light].lastFrame = rec.m_0010;
        }
    }

    // The ribbon of kind 0x11 between the owner's and the target's position: two points a frame.
    if (def.kind == EFFECT_KIND_LIGHT_INTERLEAVED && rec.matrixOwner != 0 && rec.m_0188 != 0)
    {
        Vec4 point;
        Vec4 colour;
        Vec4 colour2;
        if (def.trackFlags[1] != 0)
            head.tracks[5].Sample(rec.m_0010, &colour.x, &colour.y, &colour.z, &colour.w, 0);
        else
            colour.Fill(1.0f);
        if (def.trackFlags[2] != 0)
            head.tracks[6].Sample(rec.m_0010, &colour2.x, &colour2.y, &colour2.z, &colour2.w, 0);
        else
            colour2.Fill(0.0f);
        float strength;       // its value is not used
        float strengthRest;
        if (def.trackFlags[6] != 0)
            head.tracks[10].Sample(rec.m_0010, &strength, &strengthRest, &strengthRest, &strengthRest, 0);
        else
            strength = 0.0f;
        point.Copy(rec.partPositions[rec.part]);
        float startWidth;
        float endWidth;
        if (def.trackFlags[0] != 0)
        {
            float widthX;
            float widthY;
            float widthZ;
            float widthW;
            head.tracks[4].Sample(rec.m_0010, &widthX, &widthY, &widthZ, &widthW, 0);
            startWidth = widthX;
            endWidth = widthY;
        }
        else
        {
            startWidth = 0.0f;
            endWidth = 0.0f;
        }
        if (def.flat != 0)
            point.z = 0.0f;
        if (!AttachLight(rec, definition))
            return 0;
        Vec4 axis;
        if (DAT_118e99f8[rec.light].lastFrame != rec.m_0010)
        {
            if (DAT_118e99f8[rec.light].pointCount < EFFECT_LIGHT_FILL_LIMIT)
            {
                axis.Copy(const_cast<Matrix4 *>(rec.matrixOwner)->GetTranslation() -
                          reinterpret_cast<Matrix4 *>(rec.m_0188)->GetTranslation());
                axis.Normalized3();
                int count = DAT_118e99f8[rec.light].pointCount;
                if (rec.matrixOwner != 0)
                    DAT_118e99f8[rec.light].points[count].Copy(
                        const_cast<Matrix4 *>(rec.matrixOwner)->GetTranslation() + axis * startWidth);
                DAT_118e99f8[rec.light].widths[count].Fill(startWidth);
                DAT_118e99f8[rec.light].colours[count].Copy(colour);
                DAT_118e99f8[rec.light].specular[count].Copy(colour2);
                DAT_118e99f8[rec.light].pointCount++;
                count++;
                if (rec.m_0188 != 0)
                    DAT_118e99f8[rec.light].points[count].Copy(
                        reinterpret_cast<Matrix4 *>(rec.m_0188)->GetTranslation() + axis * endWidth);
                DAT_118e99f8[rec.light].widths[count].Fill(endWidth);
                DAT_118e99f8[rec.light].colours[count].Copy(colour);
                DAT_118e99f8[rec.light].specular[count].Copy(colour2);
                DAT_118e99f8[rec.light].pointCount++;
            }
            DAT_118e99f8[rec.light].lastFrame = rec.m_0010;
        }
    }

    // The hit test: the host tests a ball (or a box) the effect covers and tells whether it hit something.
    int hit = 0;
    int ending = 0;
    if (def.m_0018[0] != 0 && DAT_1101de40[EFFECT_HOST_HIT_TEST] != 0)
    {
        float intensity = 0.0f;
        float intensityRest = 0.0f;
        head.tracks[1].Sample(rec.m_0010, &intensity, &intensityRest, &intensityRest, &intensityRest, 0);
        if (def.kind == EFFECT_KIND_SWEEP_BOX)
        {
            if (intensity != 0.0f)
            {
                float length;
                float depth;
                float radius;
                float sizeRest;
                if (def.trackFlags[0] != 0)
                    head.tracks[4].Sample(rec.m_0010, &length, &depth, &radius, &sizeRest, 0);
                else
                {
                    length = EFFECT_BOX_LENGTH_DEFAULT;
                    depth = EFFECT_BOX_LENGTH_DEFAULT;
                    radius = EFFECT_BOX_RADIUS_DEFAULT;
                }
                Vec4 corners[2];
                corners[0].Set(0.0f, 0.0f, 0.0f, 0.0f);
                corners[1].Set(0.0f, 0.0f, 0.0f, 0.0f);
                Matrix4 boxMatrix;        // set up but not used
                boxMatrix.SetIdentity();
                Vec4 boxScale;
                boxScale.x = radius;
                boxScale.y = radius;
                boxScale.z = radius;
                corners[0].Copy(Project(corners[0], rec.startMatrix));
                corners[1].Copy(Project(corners[1], rec.worldMatrix));
                rec.options.m_001c = def.m_0018[2];
                rec.options.m_0020 = def.m_0018[4];
                corners[0].w = radius;
                corners[1].w = radius;
                rec.options.m_0038 = &corners[1];
                rec.options.m_0034 = &corners[0];
                rec.options.settings[3] = 1;
                Vec4 aimed;
                rec.options.m_0030 = &aimed;
                hit = ((EffectHitHook)DAT_1101de40[EFFECT_HOST_HIT_TEST])(0.0f, 0.0f, 0.0f, 0.0f, &rec.options);
                rec.options.settings[3] = 0;
                rec.options.m_0034 = 0;
                rec.options.m_0038 = 0;
            }
        }
        else if (def.kind == EFFECT_KIND_LINE_BOX)
        {
            if (intensity != 0.0f)
            {
                float length;
                float depth;
                float radius;
                float sizeRest;
                if (def.trackFlags[0] != 0)
                    head.tracks[4].Sample(rec.m_0010, &length, &depth, &radius, &sizeRest, 0);
                else
                {
                    length = EFFECT_BOX_LENGTH_DEFAULT;
                    depth = EFFECT_BOX_LENGTH_DEFAULT;
                    radius = EFFECT_BOX_RADIUS_DEFAULT;
                }
                Vec4 corners[2];
                corners[0].Set(-length, 0.0f, 0.0f, 0.0f);
                corners[1].Set(depth, 0.0f, 0.0f, 0.0f);
                Matrix4 boxMatrix;        // set up but not used
                boxMatrix.SetIdentity();
                Vec4 boxScale;
                boxScale.x = radius;
                boxScale.y = radius;
                boxScale.z = radius;
                corners[0].Copy(Project(corners[0], rec.worldMatrix));
                corners[1].Copy(Project(corners[1], rec.worldMatrix));
                rec.options.m_001c = def.m_0018[2];
                rec.options.m_0020 = def.m_0018[4];
                corners[0].w = radius;
                corners[1].w = radius;
                rec.options.m_0034 = &corners[0];
                rec.options.m_0038 = &corners[1];
                rec.options.settings[3] = 1;
                Vec4 aimed;
                rec.options.m_0030 = &aimed;
                hit = ((EffectHitHook)DAT_1101de40[EFFECT_HOST_HIT_TEST])(0.0f, 0.0f, 0.0f, 0.0f, &rec.options);
                rec.options.settings[3] = 0;
                rec.options.m_0034 = 0;
                rec.options.m_0038 = 0;
            }
        }
        else if (intensity > 0.0f)
        {
            rec.options.m_001c = def.m_0018[2];
            rec.options.m_0020 = def.m_0018[4];
            Vec4 axis;
            rec.options.m_0030 = &axis;
            rec.options.settings[3] = 1;
            hit = ((EffectHitHook)DAT_1101de40[EFFECT_HOST_HIT_TEST])(local.x, local.y, local.z, intensity,
                                                                      &rec.options);
            if (hit == -1)
            {
                Vec4 bounced;        // its value is not used
                float length = rec.step.Length();
                Matrix4 inverse = InverseMatrix(rec.worldMatrix);
                Vec4 heading = rec.step;
                heading.Normalized3();
                axis.Normalized3();
                axis.Copy(TransformNormal(axis, inverse));
                axis.Normalized3();
                heading.RotateByDirection(axis);
                heading.Normalized3();
                rec.step.Copy(heading * length);
                if (def.m_0174[0].type == EFFECT_TRIGGER_HIT_FLAG)
                    ending = 1;
                if (def.m_0174[1].type == EFFECT_TRIGGER_HIT_FLAG)
                    ending = 1;
                if (def.m_0174[2].type == EFFECT_TRIGGER_HIT_FLAG)
                    ending = 1;
                hit = 0;
            }
        }
    }

    // The events of the definition (the first three of the table).
    for (int i = 0; i < 3; i++)
    {
        const EffectTrigger &trigger = def.m_0174[i];
        if (trigger.type == 0)
            continue;
        if (trigger.type == EFFECT_TRIGGER_HIT_ENDS && hit != 0)
            ending = 1;
        if (trigger.type == EFFECT_TRIGGER_AREA_ABOVE_ENDS)
        {
            if (DAT_10f668a8.min.z + EFFECT_AREA_HEIGHT_MARGIN < local.z && local.x >= DAT_10f668a8.min.x &&
                local.y >= DAT_10f668a8.min.y && local.x <= DAT_10f668a8.max.x && local.y <= DAT_10f668a8.max.y)
            {
                rec.options.list = 0;
                ending = 1;
            }
        }
        if (trigger.type == EFFECT_TRIGGER_AREA_BELOW_ENDS)
        {
            if (DAT_10f668a8.min.z + EFFECT_AREA_HEIGHT_MARGIN > local.z && local.x >= DAT_10f668a8.min.x &&
                local.y >= DAT_10f668a8.min.y && local.x <= DAT_10f668a8.max.x && local.y <= DAT_10f668a8.max.y)
            {
                rec.options.list = 0;
                ending = 1;
            }
        }
        if (trigger.type == EFFECT_TRIGGER_SCREEN_FLASH && trigger.frame == rec.m_0010 &&
            DAT_1101de40[EFFECT_HOST_SCREEN] != 0)
            ((EffectScreenHook)DAT_1101de40[EFFECT_HOST_SCREEN])(EFFECT_SCREEN_FLASH, 0.0f, 0.0f,
                                                                 (float)trigger.value, 0.0f);
        if (trigger.type == EFFECT_TRIGGER_SCREEN_TINT && trigger.frame == rec.m_0010 &&
            DAT_1101de40[EFFECT_HOST_SCREEN] != 0)
        {
            int packed = trigger.value;
            ((EffectScreenHook)DAT_1101de40[EFFECT_HOST_SCREEN])(
                EFFECT_SCREEN_TINT, (float)(packed / (EFFECT_TINT_BIG_STEP)) * EFFECT_TINT_UNIT,
                (float)(packed % EFFECT_TINT_BIG_STEP / EFFECT_TINT_STEP) * EFFECT_TINT_UNIT,
                (float)(packed % EFFECT_TINT_STEP) * EFFECT_TINT_UNIT, 0.0f);
        }
        if (trigger.type == EFFECT_TRIGGER_SCREEN_FADE && trigger.frame == rec.m_0010 &&
            DAT_1101de40[EFFECT_HOST_SCREEN] != 0)
            ((EffectScreenHook)DAT_1101de40[EFFECT_HOST_SCREEN])(EFFECT_SCREEN_FADE, 0.0f, 0.0f,
                                                                 (float)trigger.value, 0.0f);
        if (trigger.type == EFFECT_TRIGGER_PARENT_ENDS && rec.parent >= 0 && DAT_1115cfe8[rec.parent].id < 0)
            ending = 1;
        if (trigger.type == EFFECT_TRIGGER_HOST_ENDS && DAT_1101de40[EFFECT_HOST_WANTED] != 0)
        {
            if (((EffectWantedHook)DAT_1101de40[EFFECT_HOST_WANTED])(local.x, local.y, local.z,
                                                                     (float)trigger.frame, 0.0f) == 0)
                ending = 1;
        }
        if (trigger.type == EFFECT_TRIGGER_SCREEN_SHAKE && trigger.frame == rec.m_0010 &&
            DAT_1101de40[EFFECT_HOST_SCREEN] != 0)
            ((EffectScreenHook)DAT_1101de40[EFFECT_HOST_SCREEN])(EFFECT_SCREEN_SHAKE, 0.0f, 0.0f, 0.0f, 0.0f);
    }

    // Children started when the hit test hit, and at their frame.
    if (hit != 0)
    {
        for (int i = 0; i < 4; i++)
        {
            if (def.m_01b4[i].type == EFFECT_CHILD_ON_HIT)
                rec.m_018c += (short)StartChildEffects(this, record, def.m_01b4[i], 1);
        }
    }
    for (int i = 0; i < 4; i++)
    {
        if (def.m_01b4[i].type == EFFECT_CHILD_AT_FRAME && def.m_01b4[i].frame == rec.m_0010)
            rec.m_018c += (short)StartChildEffects(this, record, def.m_01b4[i], 1);
    }
    if (ending == 0 && def.m_0164 > rec.m_0010)
    {
        for (int i = 0; i < 4; i++)
        {
            const EffectChild &child = def.m_01b4[i];
            if (child.type == EFFECT_CHILD_EVERY)
            {
                int every = child.frame > 1 ? child.frame : 1;
                if ((unsigned int)rec.m_001c % every == 0)
                    rec.m_018c += (short)StartChildEffects(this, record, child, 0);
            }
        }
        return 0;
    }

    // The effect ends: the children of "at the end" start, and the record waits for its children to go.
    for (int i = 0; i < 4; i++)
    {
        if (def.m_01b4[i].type == EFFECT_CHILD_AT_END)
        {
            rec.m_018c += (short)StartChildEffects(this, record, def.m_01b4[i], 1);
            rec.id = EFFECT_ENDED;
        }
    }
    rec.id = EFFECT_ENDED;
    rec.m_001c = 0;
    if (rec.light != 0)
        DAT_118e99f8[rec.light].frame = 1;
    return 0;
}
