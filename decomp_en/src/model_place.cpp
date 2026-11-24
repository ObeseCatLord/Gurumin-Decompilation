// Placing a model (EN build). Ported from the matched JP source decomp/src/model_place.cpp.
#include "anim_model.h"
#include "node.h"

// AnimModel::SetPosition: moves the model to (x, y, z) with an identity orientation and stops it: the movement and
// speed vectors and the other motion state are cleared, the root and collision nodes lose their collision flag.
// g_placementMatrix is reset to the identity too. Returns 0. Called when a character or object is put on the map.
int AnimModel::SetPosition(float x, float y, float z)
{
    Vec4 position;
    position.z = z;
    placedHeight = z;
    placeHeight = z;
    position.x = x;
    position.y = y;
    pushVector.Fill(0);
    pushFlag = 0;
    m_2408.Fill(0);
    m_01d4 = 0;
    collisionFilter = 0;
    m_023c = 0;
    m_0290 = 0;
    blendFrames = 0;
    matrix = IdentityMatrix();
    matrix.SetTranslation(position);
    g_placementMatrix = IdentityMatrix();
    m_2394 = 1;
    m_237c = 1.0f;
    verticalSpeed = 0;
    move.Set(0, 0, 0, 0);
    velocity.Set(0, 0, 0, 0);
    moveStep.Set(0, 0, 0, 0);
    m_23c8.Set(0, 0, 0, 0);
    m_23f8.Set(0, 0, 0, 0);
    m_23e8.Set(0, 0, 0, 0);
    m_2374 = 0;
    if (collider != 0)
        collider->hasCollision = 0;
    if (root != 0)
        root->hasCollision = 0;
    collisionResult = 0;
    m_2379 = 0;
    m_2378 = 0;
    shadowOpacity = SHADOW_OPACITY_PLACED;
    return 0;
}

// AnimModel::Teleport: like SetPosition, but the root node drops its saved states and the shadow matrix moves to
// (x, y, z) as well. Returns 0. Called for scripted jumps of a model to another place.
int AnimModel::Teleport(float x, float y, float z)
{
    Vec4 position;
    position.z = z;
    placedHeight = z;
    placeHeight = z;
    position.x = x;
    position.y = y;
    m_023c = 0;
    collisionFilter = 0;
    pushVector.Fill(0);
    pushFlag = 0;
    blendFrames = 0;
    m_0290 = 0;
    matrix = IdentityMatrix();
    m_2408.Fill(0);
    m_01d4 = 0;
    if (root != 0)
        root->ReleaseSavedStates();
    matrix.SetTranslation(position);
    shadowMatrix.SetTranslation(position);
    g_placementMatrix = IdentityMatrix();
    m_2394 = 1;
    move.Set(0, 0, 0, 0);
    velocity.Set(0, 0, 0, 0);
    moveStep.Set(0, 0, 0, 0);
    m_23c8.Set(0, 0, 0, 0);
    m_23f8.Set(0, 0, 0, 0);
    m_23e8.Set(0, 0, 0, 0);
    verticalSpeed = 0;
    m_237c = 1.0f;
    m_2374 = 0;
    if (collider != 0)
        collider->hasCollision = 0;
    if (root != 0)
        root->hasCollision = 0;
    collisionResult = 0;
    m_2379 = 0;
    m_2378 = 0;
    shadowOpacity = SHADOW_OPACITY_PLACED;
    return 0;
}
