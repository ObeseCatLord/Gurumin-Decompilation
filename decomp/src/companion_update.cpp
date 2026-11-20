// The player's three companions (run from 0x100d24a0): each companion k of character `index` lives in character
// slot index + 2 + k and keeps its health and revival state in the owner's records. The three updates are the
// same code for k = 0, 1 and 2.

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    Vec4();                                          // 0x101ac990: zero
    void Copy(const Vec4 &source);                   // 0x101acc20
    Vec4 operator-(const Vec4 &other) const;         // 0x101aca40
};

struct Matrix4
{
    float m[16];
    Matrix4();                                       // 0x101ae770
    ~Matrix4();                                      // 0x1013eda0 (empty)
    Vec4 GetTranslation();                           // 0x101ade80
};

Matrix4 RotationZ(float angle);                      // 0x101b1b40

// A model node (scene_node.cpp Node): only what the companions use.
struct Node
{
    char pad_0000[0x24c];
    Matrix4 worldMatrix;                             // 0x24c
    void M_1019b130(int flags);
    void M_1019b1b0(int flags);
    void M_10194730(int value, int mask);
};

// A character record (world.h Character), 0x249c bytes.
struct Character
{
    char pad_0000[0x22fc];
    Node *root;                                      // 0x22fc: root node of the model
    char pad_2300[0x38];
    Matrix4 matrix;                                  // 0x2338: world matrix
    char pad_2378[0x114];
    float heading;                                   // 0x248c
    char pad_2490[0xc];
};

// The last position and step of each character (route_walk.cpp StepRecord).
struct LastPosition
{
    Vec4 position;
    Vec4 step;
    char pad_0020[0x20];
};

extern "C" Character DAT_10db6ed8[0x80];
extern "C" LastPosition DAT_10802bf8[0x80];
extern "C" int DAT_1020b430[0x80][16];               // AI state (world.h CharState)
extern "C" Node *DAT_1021bdf8[0x80][9];              // [1], [2]: the companion's attached effect nodes
extern "C" short DAT_103153b0[0x80];
extern "C" int DAT_10219770[0x80];
extern "C" unsigned char DAT_10315350[0x80];
extern "C" unsigned char DAT_102550e0[0x80];         // level of each character
extern "C" unsigned char DAT_104686b0[0x80];         // display state (2 = down, 7 = idle, 0x1e = revived)
extern "C" unsigned char DAT_1020d4c0[0x80];
extern "C" char DAT_1042d9e8[0x80];                  // set when the character was hit this frame
extern "C" char DAT_1021f3c0[0x80];                  // kind of each character slot
// The owner's companion records (inside the owner's 0x188-byte character slot, one set per companion).
extern "C" int DAT_10daadfc[0x80][0x62];             // companion 0: health (2000 = down, 70000 = full)
extern "C" int DAT_10daae00[0x80][0x62];             // companion 0: maximum health
extern "C" int DAT_10daae0c[0x80][0x62];
extern "C" int DAT_10daaaf0[0x80][0x62];
extern "C" int DAT_10daaafc[0x80][0x62];
extern "C" char DAT_10daaf39[0x80][0x188];           // companion 0: 1 while down
extern "C" char DAT_10daaf3a[0x80][0x188];           // companion 0: 1 while waiting outside the play area
extern "C" Vec4 DAT_10f08080[0x80];
extern "C" Vec4 DAT_10836c74;
extern "C" int DAT_104c00e0;
extern "C" int DAT_104c00e8;                         // map area
extern "C" int DAT_104c00f8;
extern "C" int DAT_10c655f0;                         // current costume
extern "C" int DAT_10c635f8;
extern "C" int DAT_10c64ad8;
extern "C" int FUN_10065b10(int index, int sound, int always, int optional);  // Character_PlaySound
extern "C" int FUN_1000e750(void);
extern "C" int FUN_1001d8b0(int index, int effect, Vec4 position, int parent, int unused, const Matrix4 *owner,
                            int value, int frame, const Matrix4 *matrix, const void *options);

// 0x100b0250 (Companion0_Update, a C++ function: it has an EH frame for its matrices). Keeps companion 0 of character `index` (slot index + 2) in range and runs its
// down/revive cycle (state slot 4): 0 = active (checks health, waits outside the areas 0x13 and 0x30, goes down
// below its maximum), 1 = counting to 60 then revives with effect 0x36d2, sound 0x194 and 30 points, 2 = counts
// down to active, 10 and above counts down with effect 0x36d5 at 70 and comes back at 11. Returns 0.
int Companion0_Update(int index)
{
    if (DAT_103153b0[index + 2] > 15)
        DAT_103153b0[index + 2] = 15;
    if (DAT_10219770[index + 2] > 5)
        DAT_10219770[index + 2] = 5;
    if (DAT_10315350[index + 2] > 0)
        DAT_10315350[index + 2] = 0;
    unsigned char level = DAT_102550e0[index + 2];
    if (level != 0)
    {
        if (DAT_102550e0[index] >= level)
            level = DAT_102550e0[index];
        DAT_102550e0[index] = level;
    }
    int mode = DAT_1020b430[index + 2][4];
    if (mode != 0)
    {
        if (mode == 1)
        {
            DAT_104686b0[index + 2] = 2;
            if (DAT_1020b430[index + 2][3] == 1)
                FUN_10065b10(index, 0x192, 1, 0);
            if (++DAT_1020b430[index + 2][3] == 0x3c)
            {
                DAT_1020b430[index + 2][4] = 0;
                Vec4 position;
                Matrix4 matrix;
                if (DAT_1021bdf8[index + 2][1] != 0)
                {
                    DAT_1021bdf8[index + 2][1]->M_1019b130(0x20);
                    matrix = DAT_1021bdf8[index + 2][1]->worldMatrix;
                }
                else
                    matrix = RotationZ(DAT_10db6ed8[index].heading);
                if (DAT_1021bdf8[index + 2][2] != 0)
                    FUN_1001d8b0(index, 0x36d2, position, -1, -1, &DAT_1021bdf8[index + 2][2]->worldMatrix, 0, 0, &matrix,
                                 0);
                else
                    FUN_1001d8b0(index, 0x36d2, position, -1, -1, 0, 0, 0, &matrix, 0);
                FUN_10065b10(index, 0x194, 1, 0);
                DAT_10c64ad8 += 0x1e;
                FUN_1000e750();
            }
        }
        else if (mode == 2)
        {
            DAT_104686b0[index + 2] = 2;
            if (--DAT_1020b430[index + 2][3] == 0)
                DAT_1020b430[index + 2][4] = 0;
        }
        else if (mode >= 10)
        {
            DAT_1020b430[index + 2][4] = mode - 1;
            if (mode - 1 == 0x46)
            {
                Vec4 position;
                Matrix4 matrix;
                if (DAT_1021bdf8[index + 2][1] != 0)
                    matrix = DAT_1021bdf8[index + 2][1]->worldMatrix;
                else
                    matrix = RotationZ(DAT_10db6ed8[index].heading);
                if (DAT_1021bdf8[index + 2][2] != 0)
                    FUN_1001d8b0(index, 0x36d5, position, -1, -1, &DAT_1021bdf8[index + 2][2]->worldMatrix, 0, 0, &matrix,
                                 0);
                else
                    FUN_1001d8b0(index, 0x36d5, position, -1, -1, 0, 0, 0, &matrix, 0);
            }
            if (DAT_1020b430[index + 2][4] == 0xb)
            {
                if (DAT_1021bdf8[index + 2][1] != 0)
                {
                    DAT_1021bdf8[index + 2][1]->M_1019b1b0(0x20);
                    int health = DAT_10daadfc[index][0] + 200;
                    if (DAT_1020b430[index][3] + 2000 <= health)
                        health = DAT_1020b430[index][3] + 2000;
                    DAT_1020b430[index + 2][5] = health;
                    DAT_10daadfc[index][0] = DAT_10daae00[index][0];
                    DAT_10db6ed8[index + 2].root->M_10194730(0x2f, 0x400);
                    DAT_10daaf39[index][0] = 0;
                    DAT_1021f3c0[index + 2] = 3;
                }
                DAT_104686b0[index + 2] = 2;
                DAT_1020b430[index + 2][4] = 2;
            }
        }
    }
    else
    {
        int health = DAT_10daadfc[index][0];
        if (health > 70000)
        {
            int special = DAT_104c00e0;
            int area = DAT_104c00e8;
            if ((special == 0 || (area != 0x13 && area != 0x30)) && DAT_1020b430[index + 2][2] == 0 &&
                DAT_10c655f0 != 6)
            {
                DAT_10daaf3a[index][0] = 1;
                DAT_10daaf39[index][0] = 0;
                DAT_10802bf8[index + 2].position.z = 0;
            }
            else
            {
                DAT_10daaf3a[index][0] = 0;
                DAT_10daae0c[index][0] = 999;
                DAT_10daaf39[index][0] = 0;
                if (DAT_1020b430[index][2] == 0)
                    DAT_10802bf8[index + 2].position.z = -50.0f;
                if (special != 0 && (area == 0x13 || area == 0x30))
                    DAT_1020b430[index + 2][2] = 2;
                else
                    DAT_1020b430[index + 2][2]--;
            }
            if (DAT_1042d9e8[index + 2] != 0)
            {
                unsigned char shown = DAT_104686b0[index + 2];
                DAT_1042d9e8[index + 2] = 0;
                DAT_10daadfc[index][0] = health - 1;
                if (shown > 6)
                    DAT_104686b0[index + 2] = 6;
            }
            if (DAT_10daadfc[index][0] < DAT_10daae00[index][0] - 5 ||
                (DAT_10c635f8 == 1 && DAT_10daadfc[index][0] < DAT_10daae00[index][0]))
            {
                int revived = DAT_1020b430[index + 2][5];
                DAT_1020b430[index + 2][4] = 1;
                DAT_10daaf3a[index][0] = 0;
                if (revived == 0)
                    revived = DAT_10daaaf0[index][0] + 2000;
                DAT_10daadfc[index][0] = revived;
                DAT_10daae0c[index][0] = DAT_10daaafc[index][0];
                DAT_104686b0[index + 2] = 0x1e;
                DAT_10802bf8[index + 2].position.z = 0;
            }
        }
        else if (health > 2000)
        {
            DAT_10daaf39[index][0] = 0;
            if (DAT_104686b0[index + 2] == 7)
            {
                if (DAT_1020d4c0[index + 2] > 4)
                {
                    if (DAT_104c00f8 != 0)
                        DAT_10f08080[index].Copy(DAT_10836c74 - DAT_10db6ed8[index].matrix.GetTranslation());
                    DAT_1020d4c0[index + 2] = 6;
                }
                else
                {
                    if (DAT_104c00f8 != 0)
                        DAT_10f08080[index].Copy(DAT_10836c74 - DAT_10db6ed8[index].matrix.GetTranslation());
                    DAT_1020d4c0[index + 2] = 4;
                }
            }
        }
        else
        {
            DAT_10daadfc[index][0] = 2000;
            DAT_10daaf39[index][0] = 1;
        }
    }
    return 0;
}

// 0x100b0880 (Companion1_Update, a C++ function: it has an EH frame for its matrices). Keeps companion 1 of character `index` (slot index + 3) in range and runs its
// down/revive cycle (state slot 4): 0 = active (checks health, waits outside the areas 0x13 and 0x30, goes down
// below its maximum), 1 = counting to 60 then revives with effect 0x36d1, sound 0x194 and 30 points, 2 = counts
// down to active, 10 and above counts down with effect 0x36d4 at 70 and comes back at 11. Returns 0.
int Companion1_Update(int index)
{
    int slot = index + 1;
    if (DAT_103153b0[slot + 2] > 15)
        DAT_103153b0[slot + 2] = 15;
    if (DAT_10219770[slot + 2] > 5)
        DAT_10219770[slot + 2] = 5;
    if (DAT_10315350[index + 3] > 0)
        DAT_10315350[index + 3] = 0;
    unsigned char level = DAT_102550e0[index + 3];
    if (level != 0)
    {
        if (DAT_102550e0[index] >= level)
            level = DAT_102550e0[index];
        DAT_102550e0[index] = level;
    }
    int mode = DAT_1020b430[slot + 2][4];
    if (mode != 0)
    {
        if (mode == 1)
        {
            DAT_104686b0[index + 3] = 2;
            if (DAT_1020b430[slot + 2][3] == 1)
                FUN_10065b10(index, 0x192, 1, 0);
            if (++DAT_1020b430[slot + 2][3] == 0x3c)
            {
                DAT_1020b430[slot + 2][4] = 0;
                Vec4 position;
                Matrix4 matrix;
                if (DAT_1021bdf8[slot + 2][1] != 0)
                {
                    DAT_1021bdf8[slot + 2][1]->M_1019b130(0x20);
                    matrix = DAT_1021bdf8[slot + 2][1]->worldMatrix;
                }
                else
                    matrix = RotationZ(DAT_10db6ed8[index].heading);
                if (DAT_1021bdf8[slot + 2][2] != 0)
                    FUN_1001d8b0(index, 0x36d1, position, -1, -1, &DAT_1021bdf8[slot + 2][2]->worldMatrix, 0, 0, &matrix,
                                 0);
                else
                    FUN_1001d8b0(index, 0x36d1, position, -1, -1, 0, 0, 0, &matrix, 0);
                FUN_10065b10(index, 0x194, 1, 0);
                DAT_10c64ad8 += 0x1e;
                FUN_1000e750();
            }
        }
        else if (mode == 2)
        {
            DAT_1020b430[slot + 2][3]--;
            DAT_104686b0[index + 3] = 2;
            if (DAT_1020b430[slot + 2][3] == 0)
                DAT_1020b430[slot + 2][4] = 0;
        }
        else if (mode >= 10)
        {
            DAT_1020b430[slot + 2][4] = mode - 1;
            if (mode - 1 == 0x46)
            {
                Vec4 position;
                Matrix4 matrix;
                if (DAT_1021bdf8[slot + 2][1] != 0)
                    matrix = DAT_1021bdf8[slot + 2][1]->worldMatrix;
                else
                    matrix = RotationZ(DAT_10db6ed8[index].heading);
                if (DAT_1021bdf8[slot + 2][2] != 0)
                    FUN_1001d8b0(index, 0x36d4, position, -1, -1, &DAT_1021bdf8[slot + 2][2]->worldMatrix, 0, 0, &matrix,
                                 0);
                else
                    FUN_1001d8b0(index, 0x36d4, position, -1, -1, 0, 0, 0, &matrix, 0);
                FUN_10065b10(index, 0x193, 1, 0);
            }
            if (DAT_1020b430[slot + 2][4] == 0xb)
            {
                if (DAT_1021bdf8[slot + 2][1] != 0)
                {
                    DAT_1021bdf8[slot + 2][1]->M_1019b1b0(0x20);
                    int health = DAT_10daadfc[slot][0] + 200;
                    if (DAT_1020b430[index][3] + 2000 <= health)
                        health = DAT_1020b430[index][3] + 2000;
                    DAT_1020b430[slot + 2][5] = health;
                    DAT_10daadfc[slot][0] = DAT_10daae00[slot][0];
                    DAT_10db6ed8[slot + 2].root->M_10194730(0x2f, 0x400);
                    DAT_10daaf39[slot][0] = 0;
                    DAT_1021f3c0[index + 3] = 3;
                }
                DAT_1020b430[slot + 2][4] = 2;
                DAT_104686b0[index + 3] = 2;
            }
        }
    }
    else
    {
        int health = DAT_10daadfc[slot][0];
        if (health > 70000)
        {
            int special = DAT_104c00e0;
            int area = DAT_104c00e8;
            DAT_10daaf39[slot][0] = 0;
            if ((special == 0 || (area != 0x13 && area != 0x30)) && DAT_1020b430[slot + 2][2] == 0 &&
                DAT_10c655f0 != 6)
            {
                DAT_10daaf3a[slot][0] = 1;
                DAT_10daaf39[slot][0] = 0;
                DAT_10802bf8[slot + 2].position.z = 0;
            }
            else
            {
                DAT_10daaf3a[slot][0] = 0;
                DAT_10daae0c[slot][0] = 999;
                if (DAT_1020b430[index][2] == 0)
                    DAT_10802bf8[slot + 2].position.z = -50.0f;
                if (special != 0 && (area == 0x13 || area == 0x30))
                    DAT_1020b430[slot + 2][2] = 2;
                else
                    DAT_1020b430[slot + 2][2]--;
            }
            if (DAT_1042d9e8[index + 3] != 0)
            {
                unsigned char shown = DAT_104686b0[index + 3];
                DAT_1042d9e8[index + 3] = 0;
                DAT_10daadfc[slot][0] = health - 1;
                if (shown > 6)
                    DAT_104686b0[index + 3] = 6;
            }
            if (DAT_10daadfc[slot][0] < DAT_10daae00[slot][0] - 5 ||
                (DAT_10c635f8 == 1 && DAT_10daadfc[slot][0] < DAT_10daae00[slot][0]))
            {
                int revived = DAT_1020b430[slot + 2][5];
                DAT_1020b430[slot + 2][4] = 1;
                DAT_10daaf3a[slot][0] = 0;
                if (revived != 0)
                    DAT_10daadfc[slot][0] = revived;
                else
                    DAT_10daadfc[slot][0] = DAT_10daaaf0[index][0] + 2000;
                DAT_10daae0c[slot][0] = DAT_10daaafc[index][0];
                DAT_104686b0[index + 3] = 0x1e;
                DAT_10802bf8[slot + 2].position.z = 0;
            }
        }
        else if (health > 2000)
        {
            DAT_10daaf39[slot][0] = 0;
            if (DAT_104686b0[index + 3] == 7)
            {
                if (DAT_1020d4c0[index + 3] > 4)
                {
                    if (DAT_104c00f8 != 0)
                        DAT_10f08080[index].Copy(DAT_10836c74 - DAT_10db6ed8[index].matrix.GetTranslation());
                    DAT_1020d4c0[index + 3] = 6;
                }
                else
                {
                    if (DAT_104c00f8 != 0)
                        DAT_10f08080[index].Copy(DAT_10836c74 - DAT_10db6ed8[index].matrix.GetTranslation());
                    DAT_1020d4c0[index + 3] = 4;
                }
            }
        }
        else
        {
            DAT_10daadfc[slot][0] = 2000;
            DAT_10daaf39[slot][0] = 1;
        }
    }
    return 0;
}

// 0x100b0f00 (Companion2_Update, a C++ function: it has an EH frame for its matrices). Keeps companion 2 of character `index` (slot index + 4) in range and runs its
// down/revive cycle (state slot 4): 0 = active (checks health, waits outside the areas 0x13 and 0x30, goes down
// below its maximum), 1 = counting to 60 then revives with effect 0x36d0, sound 0x194 and 30 points, 2 = counts
// down to active, 10 and above counts down with effect 0x36d4 at 70 and comes back at 11. Returns 0.
int Companion2_Update(int index)
{
    int slot = index + 2;
    if (DAT_103153b0[slot + 2] > 15)
        DAT_103153b0[slot + 2] = 15;
    if (DAT_10219770[slot + 2] > 5)
        DAT_10219770[slot + 2] = 5;
    if (DAT_10315350[index + 4] > 0)
        DAT_10315350[index + 4] = 0;
    unsigned char level = DAT_102550e0[index + 4];
    if (level != 0)
    {
        if (DAT_102550e0[index] >= level)
            level = DAT_102550e0[index];
        DAT_102550e0[index] = level;
    }
    int mode = DAT_1020b430[slot + 2][4];
    if (mode != 0)
    {
        if (mode == 1)
        {
            DAT_104686b0[index + 4] = 2;
            if (DAT_1020b430[slot + 2][3] == 1)
                FUN_10065b10(index, 0x192, 1, 0);
            if (++DAT_1020b430[slot + 2][3] == 0x3c)
            {
                DAT_1020b430[slot + 2][4] = 0;
                Vec4 position;
                Matrix4 matrix;
                if (DAT_1021bdf8[slot + 2][1] != 0)
                {
                    DAT_1021bdf8[slot + 2][1]->M_1019b130(0x20);
                    matrix = DAT_1021bdf8[slot + 2][1]->worldMatrix;
                    FUN_1001d8b0(index, 0x36d0, position, -1, -1, &DAT_1021bdf8[slot + 2][1]->worldMatrix, 0, 0, &matrix, 0);
                }
                else
                {
                    matrix = RotationZ(DAT_10db6ed8[index].heading);
                    FUN_1001d8b0(index, 0x36d0, position, -1, -1, 0, 0, 0, &matrix, 0);
                }
                FUN_10065b10(index, 0x194, 1, 0);
                DAT_10c64ad8 += 0x1e;
                FUN_1000e750();
            }
        }
        else if (mode == 2)
        {
            if (--DAT_1020b430[slot + 2][3] == 0)
                DAT_1020b430[slot + 2][4] = 0;
            DAT_104686b0[index + 4] = 2;
        }
        else if (mode >= 10)
        {
            DAT_1020b430[slot + 2][4] = mode - 1;
            if (mode - 1 == 0x46)
            {
                Vec4 position;
                Matrix4 matrix;
                if (DAT_1021bdf8[slot + 2][1] != 0)
                {
                    matrix = DAT_1021bdf8[slot + 2][1]->worldMatrix;
                    FUN_1001d8b0(index, 0x36d3, position, -1, -1, &DAT_1021bdf8[slot + 2][1]->worldMatrix, 0, 0, &matrix, 0);
                }
                else
                {
                    matrix = RotationZ(DAT_10db6ed8[index].heading);
                    FUN_1001d8b0(index, 0x36d3, position, -1, -1, 0, 0, 0, &matrix, 0);
                }
            }
            if (DAT_1020b430[slot + 2][4] == 0xb)
            {
                if (DAT_1021bdf8[slot + 2][1] != 0)
                {
                    DAT_1021bdf8[slot + 2][1]->M_1019b1b0(0x20);
                    int health = DAT_10daadfc[slot][0] + 200;
                    if (DAT_1020b430[index][3] + 2000 <= health)
                        health = DAT_1020b430[index][3] + 2000;
                    DAT_1020b430[slot + 2][5] = health;
                    DAT_10daadfc[slot][0] = DAT_10daae00[slot][0];
                    DAT_10db6ed8[slot + 2].root->M_10194730(0x2f, 0x400);
                    DAT_10daaf39[slot][0] = 0;
                    DAT_1021f3c0[index + 4] = 3;
                }
                DAT_1020b430[slot + 2][4] = 2;
                DAT_104686b0[index + 4] = 2;
            }
        }
    }
    else
    {
        int health = DAT_10daadfc[slot][0];
        if (health > 70000)
        {
            int special = DAT_104c00e0;
            int area = DAT_104c00e8;
            DAT_10daaf39[slot][0] = 0;
            if ((special == 0 || (area != 0x13 && area != 0x30)) && DAT_1020b430[slot + 2][2] == 0 &&
                DAT_10c655f0 != 6)
            {
                DAT_10daaf3a[slot][0] = 1;
                DAT_10daaf39[slot][0] = 0;
            }
            else
            {
                DAT_10daaf3a[slot][0] = 0;
                DAT_10daae0c[slot][0] = 999;
                if (special != 0 && (area == 0x13 || area == 0x30))
                    DAT_1020b430[slot + 2][2] = 2;
                else
                    DAT_1020b430[slot + 2][2]--;
            }
            if (DAT_1042d9e8[index + 4] != 0)
            {
                unsigned char shown = DAT_104686b0[index + 4];
                DAT_1042d9e8[index + 4] = 0;
                DAT_10daadfc[slot][0] = health - 1;
                if (shown > 6)
                    DAT_104686b0[index + 4] = 6;
            }
            if (DAT_10daadfc[slot][0] < DAT_10daae00[slot][0] - 10 ||
                (DAT_10c635f8 == 1 && DAT_10daadfc[slot][0] < DAT_10daae00[slot][0]))
            {
                DAT_1020b430[slot + 2][4] = 1;
                int revived = DAT_1020b430[slot + 2][5];
                DAT_10daaf3a[slot][0] = 0;
                if (revived != 0)
                    DAT_10daadfc[slot][0] = revived;
                else
                    DAT_10daadfc[slot][0] = DAT_10daaaf0[index][0] + 2000;
                DAT_10daae0c[slot][0] = DAT_10daaafc[index][0];
                DAT_104686b0[index + 4] = 0x3c;
            }
        }
        else if (health > 2000)
        {
            DAT_10daaf39[slot][0] = 0;
            if (DAT_104686b0[index + 4] == 7)
            {
                if (DAT_1020d4c0[index + 4] > 4)
                {
                    if (DAT_104c00f8 != 0)
                        DAT_10f08080[index].Copy(DAT_10836c74 - DAT_10db6ed8[index].matrix.GetTranslation());
                    DAT_1020d4c0[index + 4] = 6;
                }
                else
                {
                    if (DAT_104c00f8 != 0)
                        DAT_10f08080[index].Copy(DAT_10836c74 - DAT_10db6ed8[index].matrix.GetTranslation());
                    DAT_1020d4c0[index + 4] = 4;
                }
            }
        }
        else
        {
            DAT_10daadfc[slot][0] = 2000;
            DAT_10daaf39[slot][0] = 1;
        }
    }
    return 0;
}

typedef char check_character[sizeof(Character) == 0x249c ? 1 : -1];
