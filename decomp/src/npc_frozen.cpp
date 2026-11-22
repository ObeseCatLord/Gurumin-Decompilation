// A character held in place by a spell (0x100469d0, behaviour 0x18 of Char_RunBehaviour 0x100dfcf0): it sparkles
// until the scripts release it, then it is freed with a flash and its state is reset.

#include "vec4.h"

#include "matrix4.h"

Matrix4 RotationZ(float angle);                  // 0x101b1b40

// A model node, only what is used here.
struct Node
{
    char pad_0000[0x24c];
    Matrix4 worldMatrix;      // 0x24c
    Node *Find(const char *name, int index);     // 0x1019ece0
};

// Effect start parameters (model_state.cpp ModelState), only the fields set here.
struct ModelState
{
    int m_0000;
    int m_0004;
    float m_0008;
    char m_000c;
    unsigned char m_000d_0 : 1;
    unsigned char followOwner : 1;   // 0x0d bit 1: the effect follows its owner
    unsigned char m_000d_2 : 6;
    char m_000e;
    char pad_000f[0x1d];
    int m_002c;               // 0x2c: owner (character index + 100)
    char pad_0030[0x14];
    ModelState();             // 0x101668b0
};

struct EffectManager
{
    int Start(int effect, Vec4 position, int parent, int unused, const Matrix4 *owner, int value, int frame,
              const Matrix4 *matrix, const ModelState *options, int *pause);  // 0x10172350
    int RemoveOwnedBy(int owner);                    // 0x101883b0
};

// A character record seen from its model part (char_move.cpp CharRecord), only the fields used here.
struct CharRecord
{
    void PlayMotion(int motion, int a, int b, float speed);  // 0x1017b0e0 (world.h CharModel::M_1017b0e0)
    char pad_0000[0x267];
    char m_0267;              // 0x267: 1 while held (idle)
    char pad_0268[0x2c];
    int m_0294;               // 0x294: 1 while held in its pose
    char pad_0298[0x2054];
    Node *root;               // 0x22ec: root node of the model
    char pad_22f0[0x38];
    Matrix4 matrix;           // 0x2328: world matrix
    char pad_2368[0x130];
    int m_2498;               // 0x2498: set to 1 by the scene start for the player
};

typedef char check_charrecord[sizeof(CharRecord) == 0x249c ? 1 : -1];

// Per-slot state table at 0x10daaaec (actor_scan.cpp ActorSlot).
struct ActorSlot
{
    int health;               // 0x00
    int maxHealth;            // 0x04
    char pad_0008[0x78];
    int m_0080;               // 0x80: 1 = released by its group
    char pad_0084[0x10];
    int m_0094;               // 0x94: 0x1d while held
    char pad_0098[0xa6];
    char m_013e;              // 0x13e: 1 while held
    char pad_013f;
    char m_0140;              // 0x140: kept in CharState m_000c while held
    char pad_0141[0x47];
};

// Per-character AI state (world.h CharState).
struct NpcState
{
    int state;                // 0x00: 0 = start, 1 = held
    int wait;                 // 0x04: frames held
    int talk;
    int saved;                // 0x0c: the slot's m_0140 while held
    char pad_0010[0x30];
};

extern "C" CharRecord DAT_10db6ee8[0x80];
extern "C" NpcState DAT_1020b430[0x80];
extern "C" ActorSlot DAT_10daaaec[0x80];
extern "C" EffectManager DAT_10daaa80;
extern "C" short DAT_103153b0[0x80];           // non-zero = release now
extern "C" char DAT_10255a88[0x80];
extern "C" int DAT_1032d69c;                  // 1 once the game built-ins are registered
extern "C" int FUN_1013f880(void);            // the game's random number
extern "C" int FUN_10043cc0(int index);       // Char_ResetState

// 0x100469d0 (Npc_Held). On its first frame it saves the slot's m_0140 and starts effect 0x36c6 (turned at random,
// following the model's "Bip01 Spine" node or else the character); every eighth frame after that it adds sparkle
// 0x36c7. While the scripts run it stays held (m_0267, m_0294, slot m_013e and m_0094 0x1d) until DAT_103153b0 is
// set or its group releases it (slot m_0080 1, with flash 0x2265 50 units above it): then its effects stop, effect
// 0x1390 plays on it, the slot gets its m_0140 back and its state is reset. Returns 0.
int FUN_100469d0(int index)
{
    if (DAT_1020b430[index].state == 0)
    {
        DAT_1020b430[index].saved = DAT_10daaaec[index].m_0140;
        DAT_1020b430[index].state = 1;
        ModelState options;
        Matrix4 matrix;
        Vec4 position;
        options.m_002c = index + 100;
        options.followOwner = 1;
        matrix = RotationZ((float)(FUN_1013f880() % 360) * 3.1415f / 180.0f);
        matrix.SetTranslation(DAT_10db6ee8[index].matrix.GetTranslation());
        if (DAT_10db6ee8[index].root->Find("Bip01 Spine", 0) != 0)
            DAT_10daaa80.Start(0x36c6, position, -1, -1, &DAT_10db6ee8[index].root->Find("Bip01 Spine", 0)->worldMatrix,
                               0, 0, &matrix, &options, 0);
        else
            DAT_10daaa80.Start(0x36c6, position, -1, -1, &DAT_10db6ee8[index].matrix, 0, 0, &matrix, &options, 0);
        DAT_10db6ee8[index].m_2498 = 0;
        DAT_10db6ee8[index].m_0267 = 1;
        DAT_10db6ee8[index].m_0294 = 1;
        DAT_10daaaec[index].m_013e = 1;
        DAT_10daaaec[index].m_0094 = 0x1d;
        DAT_10255a88[index] = 0;
    }
    else if (++DAT_1020b430[index].wait % 8 == 0)
    {
        Matrix4 matrix;
        Vec4 position;
        ModelState options;
        options.followOwner = 1;
        matrix = RotationZ((float)(FUN_1013f880() % 360) * 3.1415f / 180.0f);
        matrix.SetTranslation(DAT_10db6ee8[index].matrix.GetTranslation());
        DAT_10daaa80.Start(0x36c7, position, -1, -1, 0, 0, 0, &matrix, &options, 0);
    }
    if (DAT_1032d69c != 0)
    {
        DAT_10db6ee8[index].m_2498 = 0;
        DAT_10db6ee8[index].m_0267 = 1;
        DAT_10db6ee8[index].m_0294 = 1;
        DAT_10daaaec[index].m_013e = 1;
        DAT_10daaaec[index].m_0094 = 0x1d;
        DAT_10255a88[index] = 0;
        if (DAT_103153b0[index] == 0 && DAT_10daaaec[index].m_0080 != 1)
            return 0;
        if (DAT_103153b0[index] == 0)
        {
            Vec4 position;
            position.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
            position.z += 50.0f;
            DAT_10daaa80.Start(0x2265, position, -1, -1, 0, 0, 0, 0, 0, 0);
        }
        DAT_10daaaec[index].m_0140 = (char)DAT_1020b430[index].saved;
        DAT_10db6ee8[index].m_2498 = 1;
        DAT_10db6ee8[index].m_0267 = 0;
        DAT_10daaaec[index].m_013e = 0;
        DAT_10daaaec[index].m_0094 = 0;
        DAT_10daaa80.RemoveOwnedBy(index + 100);
        Vec4 position;
        DAT_10daaa80.Start(0x1390, position, -1, -1, 0, 0, 0, &DAT_10db6ee8[index].matrix, 0, 0);
        DAT_103153b0[index] = 0;
        DAT_10db6ee8[index].m_0294 = 0;
        DAT_10db6ee8[index].PlayMotion(2, 1, 5, 1.0f);
        FUN_10043cc0(index);
        DAT_10255a88[index] = 1;
    }
    return 0;
}
