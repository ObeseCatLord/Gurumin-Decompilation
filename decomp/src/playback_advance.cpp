// Playback stepping (one frame of a model's motion).

#include "node.h"
#include "anim_model.h"
#include "char_vars.h"
#include "game_vars.h"

extern "C" int FUN_1013f880(void);

#define PLAYBACK_SLOT_COUNT 16
#define PLAYBACK_CHANCE_RANGE 1000
#define PLAYBACK_IDLE_COOLDOWN 0x82
#define PLAYBACK_IDLE_BLEND_FRAMES 14
#define PLAYBACK_CHAIN_BLEND_FRAMES 5
#define PLAYBACK_CHAIN_BLEND_FRAMES_END 8
// Follow-up values of m_076c: stay on the last frame / keep stepping back by one step.
#define PLAYBACK_FOLLOW_HOLD (-2)
#define PLAYBACK_FOLLOW_NONE (-1)

// 0x1016a480 (Playback::Advance). Steps the playback by one frame. While blending it counts the frame down and, once
// the blend is over, switches to nextLayer. Otherwise a random idle motion may start first (each slot's chance, out
// of 1000, is slotOffsetX; the motion is slotOffsetY, only when no slot source of that layer is busy). Then the
// frame moves by the layer's speed (m_0bf0) in the current direction (m_0024: 0 forwards, 1 backwards). Passing the
// end of the layer plays the layer in m_0e30 (blended), holds (m_076c = -2), loops, or continues on layer m_076c.
// Returns 1.
int Playback::Advance()
{
    int i = 0;
    if (m_0018 != 0)
        return 1;
    int current = layer;
    if (current == -1)
        return 1;
    if (blending != 0)
    {
        frame--;
        m_0004 = (float)frame;
        if (frame <= -1 - m_0014)
        {
            current = nextLayer;
            blending = 0;
            layer = current;
            frame = m_02ec[current];
            m_0004 = (float)frame;
            m_0bec = m_09ac[current];
        }
        return 1;
    }
    if (m_000c > 0)
        m_000c--;
    if (DAT_1101d87c == 0 && m_000c == 0)
    {
        for (i = 0; i < PLAYBACK_SLOT_COUNT; i++)
        {
            if (slotOffsetX[current][i] != 0 && FUN_1013f880() % PLAYBACK_CHANCE_RANGE <= slotOffsetX[current][i])
            {
                int slot = 0;
                while (slotSource[slot] == 0 || slotSource[slot]->busy != 0 ||
                       slotLayer[slot] != slotOffsetY[current][i])
                {
                    slot++;
                    if (slot >= PLAYBACK_SLOT_COUNT)
                    {
                        m_000c = PLAYBACK_IDLE_COOLDOWN;
                        m_0bec = 0;
                        m_7c78 = current;
                        Play(slotOffsetY[current][i], 1, PLAYBACK_IDLE_BLEND_FRAMES, 1.0f);
                        m_7c78 = current;
                        return 1;
                    }
                }
            }
        }
    }
    float step = (float)m_0bf0[current];
    if (m_0024 == 0)
    {
        m_0004 = step * m_0008 + m_0004;
        frame = (int)m_0004;
        if (m_0bf0[current] < 0 && frame < m_052c[current])
        {
            if (m_0e30[current] != -1)
            {
                m_0bec = 0;
                int rewound = frame;
                rewound -= m_0bf0[current];
                frame = rewound;
                layer = -1;
                m_0004 = (float)rewound;
                Playback *saved = DAT_1101d880;
                DAT_1101d880 = 0;
                Play(m_0e30[current], 0, PLAYBACK_CHAIN_BLEND_FRAMES, 1.0f);
                DAT_1101d880 = saved;
                return 1;
            }
            int follow = m_076c[current];
            if (follow == PLAYBACK_FOLLOW_HOLD)
            {
                current = layer;
                frame = m_052c[current];
                m_0024 = 1;
                m_0004 = (float)frame;
                m_0bec = m_09ac[current];
            }
            else
            {
                m_0024 = 0;
                if (follow == PLAYBACK_FOLLOW_NONE)
                {
                    frame = frame - m_0bf0[current];
                    m_0004 = (float)frame;
                }
                else
                {
                    current = m_076c[current];
                    layer = current;
                    int first = m_02ec[current];
                    frame = first;
                    m_0004 = (float)first;
                    m_0bec = m_09ac[current];
                }
            }
        }
        if (m_0bf0[current] > 0 && frame > m_052c[current])
        {
            if (m_0e30[current] != -1)
            {
                m_0bec = 0;
                int rewound = frame;
                rewound -= m_0bf0[current];
                frame = rewound;
                layer = -1;
                m_0004 = (float)rewound;
                Playback *saved = DAT_1101d880;
                DAT_1101d880 = 0;
                Play(m_0e30[current], 0, PLAYBACK_CHAIN_BLEND_FRAMES_END, 1.0f);
                DAT_1101d880 = saved;
                return 1;
            }
            int follow = m_076c[current];
            if (follow == PLAYBACK_FOLLOW_HOLD)
            {
                int last = m_052c[layer];
                m_0024 = 1;
                frame = last;
                m_0004 = (float)frame;
                m_0bec = m_09ac[layer];
                return 1;
            }
            m_0024 = 0;
            if (follow == PLAYBACK_FOLLOW_NONE)
            {
                frame = frame - m_0bf0[current];
                m_0004 = (float)frame;
                return 1;
            }
            layer = m_076c[current];
            frame = m_02ec[layer];
            m_0004 = (float)frame;
            m_0bec = m_09ac[layer];
        }
    }
    else
    {
        m_0004 = m_0004 - step * m_0008;
        frame = (int)m_0004;
        int speed = m_0bf0[current];
        if (speed < 0 && frame > m_02ec[current])
        {
            int first = m_02ec[layer];
            m_0024 = 0;
            frame = first;
            m_0004 = (float)frame;
            m_0bec = m_09ac[layer];
            return 1;
        }
        if (speed > 0 && frame < m_02ec[current])
        {
            frame = m_02ec[layer];
            m_0024 = 0;
            m_0004 = (float)frame;
            m_0bec = m_09ac[layer];
        }
    }
    return 1;
}
