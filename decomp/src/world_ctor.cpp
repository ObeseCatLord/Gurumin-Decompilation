// The world renderer (the map scene object DAT_10c68000, see world_frame.cpp): its constructor and Bind, which resets
// it for a new scene. Members are named where their role is known, the others by offset; the gaps are padding.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vec4.h"

#include "matrix4.h"

#include "key_track.h"

// The engine's file object (save_slot.cpp GameFile).
struct GameFile
{
    FILE *handle;
    char name[0x24];
    GameFile();                                      // 0x101ad9e0
    ~GameFile();                                     // 0x101b2570
    int Open(const char *path, const char *mode);    // 0x101b2310
    void Close();                                    // 0x101b22f0
    int Size();                                      // 0x101b2370
    int Seek(int offset, int origin);                // 0x101b2380
    int Read(void *buffer, int size, int count);     // 0x101b23a0
};

// A scene node (scene_node.cpp), 0x18a0 bytes.
struct WorldNode
{
    char data[0x18a0];
    WorldNode();                                     // 0x1019ebc0
    ~WorldNode();                                    // 0x10191480
};

typedef void (*WorldCallback)(int code, int value);

// A model record of the map (AnimModel, 0x249c bytes), only the two flags copied here.
struct ModelRecord
{
    char pad_0000[0x265];
    char m_0265;
    char m_0266;
    char m_0267;
    char pad_0268[0x2234];
};

struct WorldRenderer
{
    Matrix4 m_00000;
    int m_00040;
    char characterNames[0x20][0x20];                 // 0x44: "キャラ%d" until a character is loaded
    int m_00444[0x80];
    int m_00644[0x80];
    char m_00844[0x80];                              // 0x844: field 0x265 of each model record
    char m_008c4[0x80];                              // 0x8c4: field 0x267 of each model record
    int m_00944[3][4];
    char m_00974;                                    // 0x974: set for costume 6
    char m_00975;                                    // 0x975: non-zero = no screen fade
    char m_00976;
    char pad_00977[0x1];
    int fadeOnTop;                                   // 0x978: 1 = the fade is drawn below the cinema bars
    int m_0097c;
    int m_00980[1000];
    int m_01920[1000];
    int m_028c0[1000];
    int m_03860;
    int m_03864;
    int m_03868;
    void *data;                                      // 0x386c: passed to the callback
    int m_03870;
    char pad_03874[0x10c];
    int m_03980;
    int m_03984;
    int m_03988;
    char m_0398c;
    char pad_0398d[0x3];
    int fadeColour;                                  // 0x3990: colour of the full-screen fade
    float fadeStep;                                  // 0x3994: added to fadeAmount each frame
    int fadeFrames;                                  // 0x3998: frames left of the fade
    float fadeTarget;
    float fadeAmount;                                // 0x39a0: fade strength in percent (0 = off)
    int fadeMode;
    int m_039a8;
    char m_039ac[0x20][0x10][0x10];
    char pad_059ac[0x200];
    char m_05bac[0x20];
    char pad_05bcc[0xe0];
    int m_05cac[0x20];
    char pad_05d2c[0x80];
    int m_05dac[0x20];
    char pad_05e2c[0x4];
    Vec4 m_05e30;
    KeyBank m_05e40[4];
    int m_08850;
    int m_08854;
    int m_08858;
    int m_0885c;
    ModelRecord *models;                             // 0x8860: the map's model records
    char pad_08864[0x4];
    WorldNode nodes[0x20];                           // 0x8868: map model nodes
    float m_39c68[0x20];
    float m_39ce8[0x20];
    float m_39d68[0x20];
    float m_39de8[0x20];
    float m_39e68[0x20];
    float m_39ee8[0x20];
    float m_39f68[0x20];
    float m_39fe8[0x20];
    float m_3a068[0x20];
    float m_3a0e8[0x20];
    float m_3a168[0x20];
    float m_3a1e8[0x20];
    float m_3a268[0x20];
    float m_3a2e8[0x20];
    float m_3a368[0x20];
    float m_3a3e8[0x20];
    float m_3a468[0x20];
    float m_3a4e8[0x20];
    KeyBank m_3a568[0x20];
    char pad_4f5e8[0x80];
    char m_4f668[0x20][0x20];
    char pad_4fa68[0x4];
    WorldCallback callback;                          // 0x4fa6c: host notification callback
    short messageIds[2400];                          // 0x4fa70: id of each line of the character's .itm file
    char messages[2400][0x100];                      // 0x50d30: the lines ("##nnnnn text")
    int messageCount;
    int m_e6d34[0x14];
    int m_e6d84[0x14];
    char pad_e6dd4[0x58];
    int enabled;                                     // 0xe6e2c: frame updates on
    int frame;                                       // 0xe6e30: current world frame
    char pad_e6e34[0x4];
    int m_e6e38[0x80];
    int m_e7038[0x80];
    char pad_e7238[0x200];
    Vec4 m_e7438[0x80];
    char m_e7c38[0x20][0x20];
    char pad_e8038[0xc00];
    char m_e8c38[0x80][0x20];
    char m_e9c38[0x20];
    char m_e9c58[0x60][0x20];
    char pad_ea858[0x400];
    int m_eac58[0x80];
    int m_eae58[0x80];
    short m_eb058[0x20];
    char pad_eb098[0x2c0];
    KeyBank m_eb358[0x80];
    Vec4 targets[0x80];                              // 0x13f558: target of each model's state
    char active[0x80];                               // 0x13fd58: 1 while the model's state runs
    char barsFull;                                   // 0x13fdd8: non-zero = show the cinema bars fully at once
    char pad_13fdd9[0x3];
    int textures[0x20];                              // 0x13fddc: texture slots used by the map (-1 = none)
    char pad_13fe5c[0x180];
    Vec4 m_13ffdc[0x20];
    Vec4 m_1401dc[0x20];
    char pad_1403dc[0x380];
    Vec4 m_14075c[0x20];
    Vec4 m_14095c[0x20];
    char pad_140b5c[0x80];
    Vec4 m_140bdc[0x20];
    Vec4 m_140ddc[0x20];
    char pad_140fdc[0xc0];
    char m_14109c[0x20];
    char pad_1410bc[0xc];
    int m_1410c8;
    int notifyPending;                               // 0x1410cc: a 0x5e0 notification must be sent on release
    WorldRenderer();
    int Bind(ModelRecord *models, void *mapRoot, int *state, WorldCallback callback, void *data);
    int LoadCharacter(const char *path);
    int StartFade(int mode, int target, int frames);
    void TickFade();
};

// 0x101670e0 (WorldRenderer::WorldRenderer). Constructs the members, clears the two 0x14-entry tables, the names and
// flags of the 0x20 map slots and sets their texture slots to -1.
WorldRenderer::WorldRenderer()
{
    for (int i = 0; i < 0x14; i++)
    {
        m_e6d34[i] = 0;
        m_e6d84[i] = 0;
    }
    for (int i = 0; i < 0x20; i++)
    {
        m_e7c38[i][0] = 0;
        m_14109c[i] = 0;
        textures[i] = -1;
        m_05bac[i] = 0;
        m_4f668[i][0] = 0;
    }
}

extern "C" int DAT_1101de34;
extern "C" float DAT_10203420;
extern "C" int FUN_10161120(char *out, const char *path);   // Path_GetBaseName
extern "C" int FUN_101615d0(char *data, int size, const char *name, int sum, unsigned int check);  // ItmDescramble

// 0x1017c9c0 (WorldRenderer::Bind). Resets the scene for a new map: the character slots get their default names
// ("キャラ0".. as Shift-JIS), the three 1000-entry tables -9999, the map slots, textures, node tables and fade state
// are cleared, StartFade(0, 0, 0) runs, and the model records, callback and callback data are stored. mapRoot and
// state are not used. Returns 1.
int WorldRenderer::Bind(ModelRecord *models, void *mapRoot, int *state, WorldCallback callback, void *data)
{
    m_00040 = -1;
    m_00976 = 0;
    m_00975 = 0;
    for (int i = 0; i < 0x20; i++)
    {
        m_eb058[i] = -1;
        sprintf(characterNames[i], "\x83\x4c\x83\x83\x83\x89%d", i);
    }
    m_0097c = 0;
    DAT_1101de34 = 0;
    m_00974 = 0;
    fadeOnTop = 1;
    barsFull = 0;
    DAT_10203420 = 1.0f;
    for (int i = 0; i < 1000; i++)
    {
        m_00980[i] = -9999;
        m_01920[i] = -9999;
        m_028c0[i] = -9999;
    }
    m_03860 = -9999;
    m_0398c = 0;
    for (int i = 0; i < 0x20; i++)
    {
        for (int j = 0; j < 0x10; j++)
            m_039ac[i][j][0] = 0;
        m_e7c38[i][0] = 0;
        m_14109c[i] = 0;
        textures[i] = -1;
        m_05bac[i] = 0;
        m_4f668[i][0] = 0;
    }
    for (int i = 0; i < 0x20; i++)
    {
        m_05bac[i] = 0;
        m_3a0e8[i] = 0;
        m_3a268[i] = 0;
        m_3a3e8[i] = 0;
        m_3a1e8[i] = 0;
        m_3a368[i] = 0;
        m_3a4e8[i] = 0;
        m_39c68[i] = 0;
        m_39de8[i] = 1.0f;
        m_39f68[i] = 1.0f;
        m_39d68[i] = 0;
        m_39ee8[i] = 0;
        m_3a068[i] = 0;
        textures[i] = -1;
        m_4f668[i][0] = 0;
    }
    m_1410c8 = 0;
    notifyPending = 0;
    m_039a8 = -1;
    m_03868 = 900;
    this->data = data;
    m_03870 = 0;
    fadeTarget = 0;
    fadeAmount = 0;
    StartFade(0, 0, 0);
    this->callback = callback;
    this->models = models;
    m_03864 = 0;
    for (int i = 0; i < 0x60; i++)
    {
        m_eae58[i] = -1;
        m_eac58[i] = -1;
        m_e8c38[i][0] = 0;
        m_e9c58[i][0] = 0;
    }
    m_e9c38[0] = 0;
    for (int i = 0; i < 0x20; i++)
    {
        m_05dac[i] = 0;
        m_05cac[i] = 0;
    }
    return 1;
}

// 0x10166950 (WorldRenderer::LoadCharacter). Resets the scene like Bind (and tells the host with callback code 9999),
// clears the key banks, copies two flags of each of the 0x60 model records, then reads the character's message file:
// `path` with its extension replaced by "itm", descrambled with its base name (ItmDescramble). Each line "##nnnnn text"
// adds message id nnnnn and its text (up to the end of the line). Returns 0 when the file does not pass the check,
// else 1 (also when there is no file).
int WorldRenderer::LoadCharacter(const char *path)
{
    m_00040 = -1;
    m_00976 = 0;
    m_00975 = 0;
    for (int i = 0; i < 0x20; i++)
    {
        m_eb058[i] = -1;
        sprintf(characterNames[i], "\x83\x4c\x83\x83\x83\x89%d", i);
    }
    m_0097c = 0;
    DAT_1101de34 = 0;
    m_00974 = 0;
    fadeOnTop = 1;
    barsFull = 0;
    DAT_10203420 = 1.0f;
    for (int i = 0; i < 1000; i++)
    {
        m_00980[i] = -9999;
        m_01920[i] = -9999;
        m_028c0[i] = -9999;
    }
    m_03860 = -9999;
    m_0398c = 0;
    if (callback != 0)
        callback(9999, 0);
    m_039a8 = -1;
    m_1410c8 = 0;
    notifyPending = 0;
    m_03980 = 0;
    m_03984 = 0;
    m_03988 = 0;
    frame = 0;
    m_05e40[0].Clear();
    m_05e40[1].Clear();
    m_05e40[2].Clear();
    m_05e40[3].Clear();
    m_08850 = 0;
    m_08854 = 0;
    m_08858 = 0;
    m_0885c = 0;
    for (int i = 0; i < 0x20; i++)
    {
        for (int j = 0; j < 0x10; j++)
            m_039ac[i][j][0] = 0;
        m_e7c38[i][0] = 0;
        m_05bac[i] = 0;
        m_3a0e8[i] = 0;
        m_3a268[i] = 0;
        m_3a3e8[i] = 0;
        m_3a1e8[i] = 0;
        m_3a368[i] = 0;
        m_3a4e8[i] = 0;
        m_39c68[i] = 0;
        m_39de8[i] = 1.0f;
        m_39f68[i] = 1.0f;
        m_39d68[i] = 0;
        m_39ee8[i] = 0;
        m_3a068[i] = 0;
        textures[i] = -1;
        m_14109c[i] = 0;
        m_4f668[i][0] = 0;
    }
    for (int i = 0; i < 0x80; i++)
    {
        m_e6e38[i] = 0;
        m_e7038[i] = 0;
        m_00444[i] = 0;
        m_00644[i] = 0;
    }
    int model;
    for (model = 0; model < 0x60; model++)
    {
        if (models != 0)
        {
            m_00844[model] = models[model].m_0265;
            m_008c4[model] = models[model].m_0267;
        }
        m_eae58[model] = -1;
        m_eac58[model] = -1;
        m_e8c38[model][0] = 0;
        m_e9c58[model][0] = 0;
    }
    m_e9c38[model] = 0;
    for (int i = 0; i < 0x20; i++)
    {
        m_3a568[i].Clear();
        m_05dac[i] = 0;
        m_05cac[i] = 0;
    }
    for (int i = 0; i < 0xc; i++)
        m_eb358[i].Clear();
    m_00944[0][0] = 0;
    m_00944[1][0] = 0;
    m_00944[2][0] = 0;
    m_00944[0][1] = 0;
    m_00944[1][1] = 0;
    m_00944[2][1] = 0;
    m_00944[0][2] = 0;
    m_00944[1][2] = 0;
    m_00944[2][2] = 0;
    m_00944[0][3] = 0;
    m_00944[1][3] = 0;
    m_00944[2][3] = 0;
    enabled = 0;
    GameFile file;
    messageCount = 0;
    char itmPath[0x104];
    strcpy(itmPath, path);
    strcpy(&itmPath[strlen(itmPath) - 3], "itm");
    int opened = file.Open(itmPath, "rb");
    if (opened != 0)
    {
        char baseName[0x100];
        FUN_10161120(baseName, path);
        file.Seek(0, SEEK_END);
        int size = file.Size() - 0xc;
        file.Seek(0, SEEK_SET);
        char *text = (char *)operator new(size);
        int bodySize;
        int sum;
        unsigned int check;
        file.Read(text, 1, size);
        file.Read(&bodySize, 1, 4);
        file.Read(&sum, 1, 4);
        file.Read(&check, 1, 4);
        file.Close();
        if (FUN_101615d0(text, size, baseName, sum, check) == 0)
        {
            free(text);
            return 0;
        }
        messageCount = 0;
        char *p = text;
        char c = *p++;
        int position = 1;
        while (c != 0 && position < size)
        {
            if (c == '#')
            {
                c = *p++;
                position++;
                if (c == '#')
                {
                    char number[8];  // the five digits; not terminated (atol stops at what follows)
                    number[0] = *p++;
                    number[1] = *p++;
                    number[2] = *p++;
                    number[3] = *p++;
                    number[4] = *p++;
                    messageIds[messageCount] = (short)atol(number);
                    int length = 0;
                    c = *p++;
                    position += 6;
                    while (c != 0 && c != '\n' && c != '\r' && position < size)
                    {
                        messages[messageCount][length] = c;
                        messages[messageCount][length + 1] = 0;
                        length++;
                        c = *p++;
                        position++;
                    }
                    messageCount++;
                }
            }
            c = *p++;
            position++;
        }
        free(text);
    }
    return 1;
}

// 0x10167390 (WorldRenderer::StartFade). Fades the screen to `target` percent over `frames` frames (0 = at once),
// remembering `mode`. TickFade advances it each frame.
int WorldRenderer::StartFade(int mode, int target, int frames)
{
    float value = (float)target;
    fadeMode = mode;
    fadeTarget = value;
    if (frames == 0)
    {
        fadeAmount = value;
        fadeTarget = value;
        fadeStep = 0;
        fadeFrames = 0;
        return 0;
    }
    fadeFrames = frames;
    fadeStep = (value - fadeAmount) / frames;
    return 0;
}

// 0x10167400 (WorldRenderer::TickFade). Adds the step to the fade each frame and snaps to the target on the last.
void WorldRenderer::TickFade()
{
    if (fadeFrames != 0)
    {
        fadeAmount += fadeStep;
        if (--fadeFrames == 0)
            fadeAmount = fadeTarget;
    }
}
