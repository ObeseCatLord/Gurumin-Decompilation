// The constructor of AnimModel (the CharModel of each character record): its vector and matrix members are
// constructed in declaration order, then the scale values set to 1 and the motion fields cleared. Fields are named
// by offset (see anim_reset.cpp for the full list of what AnimModel::ResetState clears).

#include "vec4.h"

#include "matrix4.h"

Matrix4 IdentityMatrix();                        // 0x101b0ba0

#include "anim_model.h"

// 0x101629f0 (AnimModel::AnimModel). Constructs the members, sets the scale values (0x260, 0x29c) to 1 and clears
// the motion and placement fields, setting the world matrix to the identity twice (two inlined resets).
AnimModel::AnimModel()
{
    uniformScale = 1.0f;
    speedScale = 1.0f;
    m_0268 = 0;
    motion = 0;
    shadowHidden = 0;
    m_231c = 0;
    trackA.playing = 0;
    root = 0;
    heading = 0;
    motionFrame = 0;
    verticalSpeed = 0;
    collider = 0;
    matrix = IdentityMatrix();
    m_2418 = 0;
    m_22a0 = 0;
    root = 0;
    loadedRoot = 0;
    heading = 0;
    motionFrame = 0;
    verticalSpeed = 0;
    collider = 0;
    matrix = IdentityMatrix();
    m_2418 = 0;
    headNode = 0;
    leftToeNode = 0;
    rightToeNode = 0;
}
