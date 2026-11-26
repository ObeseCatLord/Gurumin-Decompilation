// Shaga pieces (EN build). Ported from the matched JP source decomp/src/shaga_spawn.cpp.
#include <stdio.h>
#include <string.h>
#include "shaga.h"
#include "anti_tamper.h"
#include "char.h"
#include "engine.h"
#include "node.h"
#include "texture_mgr.h"
#include "visibility.h"

// Motion name the spawned piece's colour plugs are restarted with.
static const char SHAGA_STOP_MOTION[] = "stop";

// Shaga_Spawn: creates a shaga piece with Shaga_Create, clears the slot's two targets (-1) and its counter, and
// restarts the model's colour plugs with "stop". Returns 0. Called by the equipment-break scripts.
extern "C" int Shaga_Spawn(const Vec4 *position, const char *model, int radius, float d, float e)
{
    int slot = Shaga_Create(position, model, radius, d, e);
    g_textureManager.ResetFrameTimer();
    if (slot > -1)
    {
        g_shagaTargetA[slot] = SHAGA_NO_TARGET;
        g_shagaTargetB[slot] = SHAGA_NO_TARGET;
        g_shagaCounter[slot] = 0;
        g_shagaNode[slot]->RestartColourPlugs(SHAGA_STOP_MOTION, g_colourPlugFrames);
    }
    g_textureManager.ResetFrameTimer();
    return 0;
}

// Node mode of the nodes a piece is built from.
const int SHAGA_NODE_POSITION = 0x40;
// Value g_shagaTargetB starts with for a piece that has just broken off.
const int SHAGA_BREAK_TIMER = 0x23;
// Piece scale and size class for the base kinds that break off bigger.
const int SHAGA_KIND_BIG = 0x10;
const int SHAGA_KIND_MEDIUM = 0x31;
const float SHAGA_SCALE_BIG = 2.0f;
const float SHAGA_SCALE_MEDIUM = 1.2f;
const int SHAGA_SIZE_NORMAL = 1;
const int SHAGA_SIZE_MEDIUM = 2;
const int SHAGA_SIZE_BIG = 4;
// A launch force below this launches nothing (the piece falls straight down).
const float SHAGA_MIN_FORCE = 0.01f;
// Launch speed: along y (random 0..4.95 + 20) times the force; z: 45 minus random 0..9.9.
const float SHAGA_LAUNCH_SPEED_STEP = 20.0f;
const float SHAGA_LAUNCH_SPEED_BASE = 20.0f;
const float SHAGA_LAUNCH_LIFT = 45.0f;
const float SHAGA_LAUNCH_LIFT_STEP = 10.0f;
// Launch direction spread: +-50 hundredths of a degree tenth, i.e. +-5 degrees about the character's facing.
const float SHAGA_SPREAD_STEP = 1800.0f;
const float SHAGA_PI = 3.1415f;
const float SHAGA_HALF_TURN_DEGREES = 180.0f;
const int SHAGA_TURN_DEGREES = 360;
const int SHAGA_RANDOM_RANGE = 100;
const int SHAGA_SPREAD_HALF = 50;

// Shaga_BreakOff: like Shaga_Create for a piece breaking off character `index` (with `tired`, the guarded stamina value
// first rises by 2): the piece starts from `matrix` turned at random about z and is launched away from the character's
// facing (+-5 degrees). With `scaled`, base kinds 0x10 and 0x31 make it twice or 1.2 times as big. It is raised so that it
// starts at least its radius + 1 above the character. A piece without room for its model is removed again and the next
// slot tried. Returns the slot, -1 when all are used, 0 without a map. Called by the equipment-break scripts.
int Shaga_BreakOff(int index, const char *model, int radius, const Vec4 *position, const Matrix4 *matrix, int scaled,
                   float force, int tired)
{
    Vec4 velocity;
    float scale = 1.0f;
    if (tired != 0)
    {
        g_guardedValue7 += 2;
        AntiTamper_StoreValues();
    }
    if (g_mapRootNode == 0)
        return 0;
    for (int slot = 0; slot < SHAGA_SLOTS; slot++)
    {
        if (g_shagaNode[slot] != 0)
            continue;
        Node *shaga = g_mapRootNode->Find("@shaga_000", 0);
        if (shaga == 0)
        {
            shaga = g_mapRootNode->AddChild(0, SHAGA_NODE_POSITION);
            strcpy(shaga->name, "shaga_000");
            shaga->parent = g_mapRootNode;
        }
        Node *piece = shaga->AddChild(0, SHAGA_NODE_POSITION);
        if (piece == 0)
            continue;
        piece->Load(Path_Data("EEQR.it3", 0), 0, 0, 0);
        sprintf(piece->name, "soubihagare%02d", slot);
        piece->parent = shaga;
        Node *part = piece->AddChild(0, SHAGA_NODE_POSITION);
        if (part == 0)
        {
            piece->parent->RemoveChild(piece, 0);
            continue;
        }
        part->Load(Path_Data(model, 0), 0, 0, 0);
        part->parent = piece;
        piece->BuildMesh(1, 0);
        piece->matrix = *matrix;
        piece->matrix = piece->matrix * RotationZ((float)(Random() % SHAGA_TURN_DEGREES) * SHAGA_PI / SHAGA_HALF_TURN_DEGREES);
        piece->matrix.SetTranslation(*position);
        g_shagaNode[slot] = piece;
        g_shagaRadius[slot] = (float)radius;
        g_shagaTargetB[slot] = SHAGA_BREAK_TIMER;
        g_shagaCounter[slot] = 0;
        if (force < SHAGA_MIN_FORCE)
            velocity.Fill(0.0f);
        else
        {
            velocity.Fill(0.0f);
            velocity.y = ((float)(Random() % SHAGA_RANDOM_RANGE) / SHAGA_LAUNCH_SPEED_STEP + SHAGA_LAUNCH_SPEED_BASE) * force;
            velocity.z = SHAGA_LAUNCH_LIFT - (float)(Random() % SHAGA_RANDOM_RANGE) / SHAGA_LAUNCH_LIFT_STEP;
            velocity.RotateZ((float)(Random() % SHAGA_RANDOM_RANGE - SHAGA_SPREAD_HALF) * SHAGA_PI / SHAGA_SPREAD_STEP -
                             g_characterModels[index].heading);
        }
        g_shagaVelocity[slot].Copy(velocity);
        piece->RestartColourPlugs("anime", g_colourPlugFrames);
        g_shagaSizeClass[slot] = SHAGA_SIZE_NORMAL;
        if (scaled != 0)
        {
            switch (g_charBaseKind[index])
            {
            case SHAGA_KIND_BIG:
                scale = SHAGA_SCALE_BIG;
                g_shagaSizeClass[slot] = SHAGA_SIZE_BIG;
                break;
            case SHAGA_KIND_MEDIUM:
                scale = SHAGA_SCALE_MEDIUM;
                g_shagaSizeClass[slot] = SHAGA_SIZE_MEDIUM;
                break;
            }
            g_shagaRadius[slot] = scale * g_shagaRadius[slot];
            part->matrix = part->matrix * ScaleMatrix(scale, scale, scale);
        }
        float above = position->z - g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Z];
        float lowest = g_shagaRadius[slot] + 1.0f;
        if (above < lowest)
            piece->matrix.m[MATRIX_TRANSLATION_Z] += lowest - above;
        g_shagaSettled[slot] = 0;
        Shaga_PushOut(slot);
        g_textureManager.ResetFrameTimer();
        return slot;
    }
    return -1;
}
