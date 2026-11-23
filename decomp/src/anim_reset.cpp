// Resetting a model's motion state (AnimModel, the CharModel of each character record): the whole 0x249c-byte
// object is cleared and the scale and colour vectors set to 1 (AnimModel::Unload, anim_unload.cpp, calls it).
// The fields are named by offset; their roles are described in the files that use them.

#include <string.h>

#include "vec4.h"

#include "anim_model.h"

// 0x10161f80 (AnimModel::ResetState). A few flags are cleared, the whole object is zeroed, then the vectors are
// set (most to 0, the scale vectors to 1) and the remaining fields cleared again (shadowOpacity = 15, m_2380 = 40). Returns 0.
int AnimModel::ResetState()
{
    m_0012 = 0;
    m_0077 = 0;
    m_008c = 0;
    m_0244 = 0;
    m_008d = 0;
    m_006c = 0;
    m_00b0 = 0;
    m_0068 = 0;
    nearCells = 0;
    m_023c = 0;
    memset(this, 0, 0x249c);
    m_0074 = 0;
    m_022c.Fill(0.0f);
    m_0076 = 0;
    m_0240 = 0;
    m_2408.Fill(0.0f);
    speedDivisor = 0;
    uniformScale = 1.0f;
    m_0075 = 0;
    m_0078 = 0;
    m_007c = 0;
    m_0080 = 0;
    scale.Fill(1.0f);
    holdWeight = 0;
    m_024c.Fill(1.0f);
    m_025c = 0;
    m_0264 = 0;
    important = 0;
    m_0266 = 0;
    m_0267 = 0;
    m_0268 = 0;
    m_026c = 0;
    bankIndex = 0;
    m_0274 = 0;
    m_0278 = 0;
    m_027c = 0;
    m_0290 = 0;
    m_0294 = 0;
    speedScale = 1.0f;
    m_02a0 = 0;
    m_02a1 = 0;
    m_02a2 = 0;
    loadCount = 0;
    holdState = 0;
    m_02a8 = 0;
    trackA.last = 0;
    trackA.first = 0;
    trackA.frame = 0;
    trackA.playing = 0;
    trackA.looping = 0;
    m_0d44 = 0;
    m_0d48 = 0;
    alpha = 1.0f;
    trackB.last = 0;
    trackB.first = 0;
    trackB.frame = 0;
    trackB.playing = 0;
    trackB.looping = 0;
    m_17e8 = 0;
    locked = 0;
    trackC.last = 0;
    trackC.first = 0;
    trackC.frame = 0;
    trackC.playing = 0;
    trackC.looping = 0;
    shadowHidden = 0;
    m_2288 = 0;
    m_228c.Fill(0.0f);
    m_229c = 0;
    motionFrame = 0;
    motionSpeed = 0;
    m_2370 = 0;
    m_2374 = 0;
    m_2378 = 0;
    m_2379 = 0;
    shadowOpacity = 0;
    m_237c = 0;
    m_2380 = 0;
    m_2384 = 0;
    m_2388 = 0;
    m_238c = 0;
    m_2394 = 0;
    holdOffset.Fill(0.0f);
    move.Fill(0.0f);
    velocity.Fill(0.0f);
    m_23c8.Fill(0.0f);
    m_23d8.Fill(0.0f);
    m_23e8.Fill(0.0f);
    m_23f8.Fill(0.0f);
    m_2420.Fill(0.0f);
    m_2430 = 0;
    m_2478 = 0;
    targetHeading = 0;
    m_2484 = 0;
    speedScale = 1.0f;
    m_22a0 = 0;
    heading = 0;
    motionFrame = 0;
    m_2418 = 0;
    holdState = 0;
    bankIndex = 0;
    m_0274 = 0;
    m_0278 = 0;
    m_027c = 0;
    locked = 0;
    m_2379 = 0;
    m_2378 = 0;
    shadowOpacity = 15;
    loadCount = 0;
    m_0267 = 0;
    m_2474 = 0;
    m_0080 = 0;
    important = 0;
    m_02a1 = 0;
    m_0075 = 0;
    m_02a2 = 0;
    m_17e8 = 0;
    m_0266 = 0;
    m_0268 = 0;
    m_2288 = 0;
    alpha = 1.0f;
    m_02a0 = 0;
    m_237c = 1.0f;
    shadowHidden = 0;
    trackC.playing = 0;
    trackA.playing = 0;
    trackB.playing = 0;
    m_2380 = 40;
    m_2384 = 0;
    m_23d8.x = 0;
    m_23d8.y = 0;
    m_23d8.z = 0;
    m_2394 = 0;
    motionSpeed = 0;
    m_2370 = 0;
    m_229c = 0;
    m_0294 = 0;
    scale.Fill(1.0f);
    motionFrame = 0;
    return 0;
}
