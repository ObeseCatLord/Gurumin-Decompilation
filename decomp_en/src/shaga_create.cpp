// Creating a shaga piece (EN build). Ported from the matched JP source decomp/src/shaga_spawn.cpp (0x100425f0).
#include <stdio.h>
#include <string.h>
#include "shaga.h"
#include "engine.h"
#include "node.h"
#include "visibility.h"

// Node mode of the nodes a piece is built from.
const int SHAGA_CREATE_NODE_POSITION = 0x40;
// Value g_shagaTargetB starts with for a new piece.
const int SHAGA_CREATE_TIMER = 0x23;
// Size class of a new piece.
const int SHAGA_CREATE_SIZE_NORMAL = 1;
// A launch force below this launches nothing (the piece falls straight down).
const float SHAGA_CREATE_MIN_FORCE = 0.01f;
// Launch speed: along y (random 0..4.95 + 20) times the force; z: 45 minus random 0..9.9.
const float SHAGA_CREATE_SPEED_STEP = 20.0f;
const float SHAGA_CREATE_SPEED_BASE = 20.0f;
const float SHAGA_CREATE_LIFT = 45.0f;
const float SHAGA_CREATE_LIFT_STEP = 10.0f;
// A heading below this means "launch in a random direction".
const float SHAGA_CREATE_RANDOM_HEADING = -100.0f;
// Launch direction spread: +-5 degrees about the given heading.
const float SHAGA_CREATE_SPREAD_STEP = 1800.0f;
const float SHAGA_CREATE_PI = 3.1415f;
const float SHAGA_CREATE_HALF_TURN_DEGREES = 180.0f;
const int SHAGA_CREATE_TURN_DEGREES = 360;
const int SHAGA_CREATE_RANDOM_RANGE = 100;
const int SHAGA_CREATE_SPREAD_HALF = 50;

// Shaga_Create: takes the first free shaga slot (of 0x28; -1 when all are used, 0 without a map) and builds its model
// under the map's "shaga_000" node (made when missing): a "soubihagareNN" node with EEQR.it3 that holds model `model`,
// turned at random about z, x and z again and placed at `position`. The slot gets the radius, counter 0x23 and a
// velocity: none when `force` is below 0.01, else 20..25 * force outwards and 35..45 up, turned to `heading` (+-5
// degrees) or at random when `heading` is below -100. Restarts the "anime" colour plugs, pushes the model out of the
// world collision and returns the slot. Called by Shaga_Spawn.
int Shaga_Create(const Vec4 *position, const char *model, int radius, float heading, float force)
{
    Vec4 velocity;
    if (g_mapRootNode == 0)
        return 0;
    for (int slot = 0; slot < SHAGA_SLOTS; slot++)
    {
        if (g_shagaNode[slot] == 0)
        {
            g_shagaSizeClass[slot] = SHAGA_CREATE_SIZE_NORMAL;
            Node *shaga = g_mapRootNode->Find("@shaga_000", 0);
            if (shaga == 0)
            {
                shaga = g_mapRootNode->AddChild(0, SHAGA_CREATE_NODE_POSITION);
                strcpy(shaga->name, "shaga_000");
                shaga->parent = g_mapRootNode;
            }
            Node *piece = shaga->AddChild(0, SHAGA_CREATE_NODE_POSITION);
            piece->Load(Path_Data("EEQR.it3", 0), 0, 0, 0);
            sprintf(piece->name, "soubihagare%02d", slot);
            piece->parent = shaga;
            Node *part = piece->AddChild(0, SHAGA_CREATE_NODE_POSITION);
            part->Load(Path_Data(model, 0), 0, 0, 0);
            part->parent = piece;
            piece->BuildMesh(1, 0);
            piece->matrix = RotationX((float)(Random() % SHAGA_CREATE_TURN_DEGREES) * SHAGA_CREATE_PI / SHAGA_CREATE_HALF_TURN_DEGREES) *
                            RotationZ((float)(Random() % SHAGA_CREATE_TURN_DEGREES) * SHAGA_CREATE_PI / SHAGA_CREATE_HALF_TURN_DEGREES);
            piece->matrix = piece->matrix * RotationZ((float)(Random() % SHAGA_CREATE_TURN_DEGREES) * SHAGA_CREATE_PI / SHAGA_CREATE_HALF_TURN_DEGREES);
            piece->matrix.SetTranslation(*position);
            g_shagaNode[slot] = piece;
            g_shagaTargetB[slot] = SHAGA_CREATE_TIMER;
            g_shagaCounter[slot] = 0;
            g_shagaRadius[slot] = (float)radius;
            if (force < SHAGA_CREATE_MIN_FORCE)
                velocity.Fill(0.0f);
            else
            {
                velocity.Fill(0.0f);
                velocity.y = ((float)(Random() % SHAGA_CREATE_RANDOM_RANGE) / SHAGA_CREATE_SPEED_STEP + SHAGA_CREATE_SPEED_BASE) * force;
                velocity.z = SHAGA_CREATE_LIFT - (float)(Random() % SHAGA_CREATE_RANDOM_RANGE) / SHAGA_CREATE_LIFT_STEP;
            }
            if (heading < SHAGA_CREATE_RANDOM_HEADING)
                velocity.RotateZ((float)(Random() % SHAGA_CREATE_TURN_DEGREES) * SHAGA_CREATE_PI / SHAGA_CREATE_HALF_TURN_DEGREES);
            else
                velocity.RotateZ((float)(Random() % SHAGA_CREATE_RANDOM_RANGE - SHAGA_CREATE_SPREAD_HALF) * SHAGA_CREATE_PI /
                                     SHAGA_CREATE_SPREAD_STEP +
                                 heading);
            g_shagaVelocity[slot].Copy(velocity);
            piece->RestartColourPlugs("anime", g_colourPlugFrames);
            g_shagaSettled[slot] = 0;
            Shaga_PushOut(slot);
            return slot;
        }
    }
    return -1;
}
