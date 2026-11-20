// Model bank: the global object at 0x10804bf8 that holds up to 16 extra models (map objects, effects) with their
// own model records (same 0x249c-byte layout as the model part of a Character), plus a list of the scene's light
// nodes. Loaded per map; the bank is only active when `enabled` is set.
#include <string.h>

struct Node
{
    Node *Find(const char *name, int index);  // 0x1019ece0: the index-th node called `name` in the subtree, or 0
};

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    Vec4();
    void Copy(const Vec4 &source);
};

struct Matrix4
{
    float m[16];
};

Vec4 Project(const Vec4 &point, const Matrix4 &matrix);  // 0x101ae060 (vec4.cpp)
// 0x101ae4d0 (View_ContainsPoint, vec4.cpp): 1 when `point` transformed by `matrix` lies inside the view volume, widened by `margin`.
extern "C" int FUN_101ae4d0(const Vec4 *point, const Matrix4 *matrix, float margin);

// One model record (the CharModel part of a Character, see world.h / texture_mgr.cpp SceneOwner).
struct BankModel
{
    char pad_0000[0x260];
    float scale;            // 0x260: model scale, 1.0 when added
    char pad_0264[0xc];
    int index;              // 0x270: the record's own index in the bank
    char pad_0274[0x2078];
    int id;                 // 0x22ec: model id (0 = free record)
    int id2;                // 0x22f0: copy of the id
    char pad_22f4[0x9c];
    int timer;              // 0x2390: 99999 = no timeout
    char pad_2394[0x108];
    BankModel();            // 0x101629f0 (the model record constructor)
    ~BankModel();           // 0x10007e10
};

struct ModelBank
{
    Node *lights[0x20];     // 0x0000: light nodes of the scene (Omn..., then Spot...)
    BankModel *litModels[0x80];  // 0x0080: models that take part in the scene lighting
    int shadows[10];        // 0x0280: the shadow render targets "shadow0".."shadow9"
    char pad_02a8[0x1d8];
    int shadowTexture;      // 0x0480: kage.bmp
    Vec4 m_0484[0x80];      // 0x0484
    Vec4 m_0c84[0x80];      // 0x0c84
    char pad_1484[0x400];
    int m_1884;             // 0x1884
    Vec4 lighting[0xb00];   // 0x1888: 22 vectors of light parameters for each entry of litModels
    Vec4 m_c888[0x16];      // 0xc888
    BankModel models[16];   // 0xc9e8
    int params[16];         // 0x313a8: per-model parameter given when the model was added
    int m_313e8;            // 0x313e8
    int enabled;            // 0x313ec: 1 when the current map uses the bank
    int lens[4];            // 0x313f0: lens01.bmp .. lens04.bmp (lens flare)
    char pad_31400[0x70];
    float flareAlpha[0x20]; // 0x31470: brightness of each flare point (0..1)
    int crossfade[3];       // 0x314f0: "crossfade0".."crossfade2" render targets (screen size)
    int mirror;             // 0x314fc: 256x256 render targets
    int saveBuffer;         // 0x31500
    int minimap;            // 0x31504
    int mirror2;            // 0x31508
    int saveBuffer2;        // 0x3150c
    int minimap2;           // 0x31510
    char pad_31514[0xc];
    int flareCount;         // 0x31520: flare points collected this frame
    Vec4 flareScreen[0x20]; // 0x31524: screen position of each flare point (x, y, depth, w)
    ModelBank();
    int Reset();
    int CollectLights(Node *root);
    int Add(int id, int param);
    int AddLitModel(BankModel *model);
    int AddFlarePoint(Vec4 position, const Matrix4 *viewProjection);
    int CreateTextures();
    int DrawLensFlares();
};

// Empties the bank: clears the light list, the 0x80-entry table, every model record's id and the counters.
int ModelBank::Reset()
{
    for (int j = 0; j < 0x80; j++)
        litModels[j] = 0;
    m_1884 = 0;
    for (int i = 0; i < 16; i++)
        models[i].id = 0;
    m_313e8 = 0;
    memset(lights, 0, sizeof(lights));
    return 1;
}

// Rebuilds the light list from the scene below `root`: up to 32 nodes whose names start with "Omn" (omni lights)
// followed by up to 32 starting with "Spot" (spot lights); the leading '@' makes Node::Find a prefix search. Missing lights are skipped, so the list stays packed. Returns 0 when the bank
// is not in use, else 1. (The list has 32 entries; with more than 32 lights in total it overruns into litModels.)
int ModelBank::CollectLights(Node *root)
{
    if (enabled == 0)
        return 0;
    memset(lights, 0, sizeof(lights));
    int count = 0;
    int i;
    for (i = 0; i < 0x20; i++)
    {
        lights[count] = root->Find("@Omn", i);
        if (lights[count] != 0)
            count++;
    }
    Node **slot;
    for (i = 0, slot = &lights[count]; i < 0x20; i++)
    {
        *slot = root->Find("@Spot", i);
        if (*slot != 0)
            slot++;
    }
    return 1;
}

// Adds model `id` to the first free record (scale 1, no timeout), remembers `param` for it and sets the record up
// (AddLitModel). Returns 0 when all 16 records are in use.
int ModelBank::Add(int id, int param)
{
    for (int i = 0; i < 16; i++)
    {
        if (models[i].id == 0)
        {
            models[i].id = id;
            models[i].id2 = id;
            models[i].scale = 1.0f;
            models[i].timer = 99999;
            models[i].index = i;
            params[i] = param;
            AddLitModel(&models[i]);
            return 1;
        }
    }
    return 0;
}

// 0x10165140. Adds a lens-flare point for the world position `position` (the scene code passes the light/sun
// position): when it is on screen (inside the view volume widened by 24) and in front of the far plane, its screen
// position is recorded with a brightness that fades in over the last 1/70 of the depth range. Returns 1.
int ModelBank::AddFlarePoint(Vec4 position, const Matrix4 *viewProjection)
{
    if (FUN_101ae4d0(&position, viewProjection, 24.0f) != 0)
    {
        flareScreen[flareCount].Copy(Project(position, *viewProjection));
        if (flareScreen[flareCount].z > 0.0f && flareScreen[flareCount].z < 1.0f)
        {
            float alpha = (1.0f - flareScreen[flareCount].z) * 70.0f;
            flareAlpha[flareCount] = 1.0f > alpha ? alpha : 1.0f;  // clamped to 1
            flareCount++;
        }
    }
    return 1;
}

// 0x1017c8e0. Constructs the bank: the vector tables and model records are constructed (zeroed vectors) and the bank
// starts disabled.
ModelBank::ModelBank()
{
    enabled = 0;
}

// 0x10164af0. Adds `model` to the lit models (once; only models with an id, and only while the bank is in use)
// and gives it default light parameters. Returns 1 when it was added, else 0.
int ModelBank::AddLitModel(BankModel *model)
{
    if (enabled == 0)
        return 0;
    if (model == 0 || model->id2 == 0)
        return 0;
    int i;
    for (i = 0; i < 0x80; i++)
    {
        if (litModels[i] == model)
            return 0;
    }
    for (i = 0; i < 0x80; i++)
    {
        if (litModels[i] == 0)
        {
            litModels[i] = model;
            Vec4 *light = &lighting[i * 22];  // this model's 22 light vectors
            light[0].x = 0;
            light[0].y = 0;
            light[0].z = 1.0f;
            light[0].w = 1.0f;
            light[1].x = 1.0f;
            light[1].y = 1.0f;
            light[1].z = 1.0f;
            light[1].w = 1.0f;
            light[2].x = 0;
            light[2].y = -1.0f;
            light[2].z = 0;
            light[2].w = 1.0f;
            light[3].x = 1.0f;
            light[3].y = 1.0f;
            light[3].z = 1.0f;
            light[3].w = 1.0f;
            light[4].x = 0;
            light[4].y = 0.5f;
            light[4].z = 0;
            light[4].w = 1.0f;
            light[5].x = 1.0f;
            light[5].y = 1.0f;
            light[5].z = 1.0f;
            light[5].w = 1.0f;
            return 1;
        }
    }
    return 0;
}

#include <stdio.h>

// The texture slots (texture_mgr.cpp TextureObject), only the image update.
struct TextureSlot
{
    char pad_0000[0x38];
    int UpdateImage(int flags);                      // 0x10169f70
    int DrawScreenCopy(int left, int top, int right, int bottom, float z, float alpha);  // 0x1014b750
};

extern "C" TextureSlot DAT_118d0ba8[1600];
extern "C" int DAT_10203434;                         // 1 when shadows are drawn
extern "C" int DAT_1101d864;
extern "C" int DAT_1101d878;                         // set while loading the shared textures
extern "C" int DAT_10f8d9b8[10];                     // copy of the shadow targets
extern "C" int DAT_10f655ac;                         // cel.bmp (cel shading ramp)
extern "C" int DAT_10f655b0;                         // cel2.bmp
extern "C" int DAT_10f654cc;                         // render-to-texture available (RenderTarget_Probe)
extern "C" int DAT_10f654d0;
extern "C" int DAT_10f654e4;                         // 1 = screen-size crossfade targets
extern "C" float DAT_10202b18;                       // size of the crossfade targets
extern "C" int DAT_10f6559c;
extern "C" int DAT_10f655a0;
extern "C" int DAT_10f65484;
extern "C" const char *FUN_1013f8a0(const char *name, int folder);   // builds the data path of `name`
extern "C" int FUN_10173550(const char *path, const char *alphaPath, int flags);  // Texture_LoadBmp
extern "C" int FUN_10175c90(const char *name, int width, int height, const unsigned char *pixels, int alpha,
                            int flags);                              // Texture_CreateFromPixels
extern "C" void FUN_1013f070(int mode, int a, int b);                // sets a 2D/3D render state
extern "C" void FUN_101439e0(void);
extern "C" void FUN_1014b480(void);

// Loads BMP texture `name` from the data folder (the BMP branch of LoadTexture, texture_load.h).
inline int LoadBmp(const char *name)
{
    return FUN_10173550(FUN_1013f8a0(name, 0), 0, 0);
}

// 0x1017c160 (ModelBank::CreateTextures). Creates the bank's textures for a new game: the ten shadow targets (when
// shadows are on), the lens-flare, shadow and cel-shading images, and, when render-to-texture works, the crossfade,
// mirror, save-thumbnail and minimap targets; the shadow targets are cleared once. Returns 0 when one of the needed
// targets could not be created, else 1.
int ModelBank::CreateTextures()
{
    int pixel;
    char name[0x80];
    m_313e8 = 0;
    DAT_1101d864 = 0;
    if (DAT_10203434 != 0)
    {
        for (int i = 0; i < 10; i++)
        {
            sprintf(name, "shadow%0d", i);
            shadows[i] = FUN_10175c90(name, 0x100, 0x100, (unsigned char *)&pixel, 1, 0);
            DAT_10f8d9b8[i] = shadows[i];
        }
    }
    int loading = DAT_1101d878;
    DAT_1101d878 = 1;
    lens[0] = LoadBmp("lens01.bmp");
    lens[1] = LoadBmp("lens02.bmp");
    lens[2] = LoadBmp("lens03.bmp");
    lens[3] = LoadBmp("lens04.bmp");
    shadowTexture = LoadBmp("kage.bmp");
    DAT_10f655ac = LoadBmp("cel.bmp");
    DAT_10f655b0 = LoadBmp("cel2.bmp");
    DAT_1101d878 = loading;
    if (DAT_10f654cc != 0 && DAT_10f654d0 != 0)
    {
        if (DAT_10f654e4 != 0)
        {
            crossfade[0] = FUN_10175c90("crossfade0", (int)DAT_10202b18, (int)DAT_10202b18, (unsigned char *)&pixel,
                                        1, 0);
            crossfade[1] = FUN_10175c90("crossfade1", (int)DAT_10202b18, (int)DAT_10202b18, (unsigned char *)&pixel,
                                        1, 0);
            crossfade[2] = FUN_10175c90("crossfade2", (int)DAT_10202b18, (int)DAT_10202b18, (unsigned char *)&pixel,
                                        1, 0);
        }
        mirror = FUN_10175c90("mirror", 0x100, 0x100, (unsigned char *)&pixel, 1, 0);
        saveBuffer = FUN_10175c90("savebuf", 0x100, 0x100, (unsigned char *)&pixel, 1, 0);
        minimap = FUN_10175c90("minimap", 0x100, 0x100, (unsigned char *)&pixel, 1, 0);
        mirror2 = FUN_10175c90("mirror2", 0x100, 0x100, (unsigned char *)&pixel, 1, 0);
        saveBuffer2 = FUN_10175c90("savebuf2", 0x100, 0x100, (unsigned char *)&pixel, 1, 0);
        minimap2 = FUN_10175c90("minimap2", 0x100, 0x100, (unsigned char *)&pixel, 1, 0);
        DAT_10f655a0 = DAT_10f6559c = mirror;
        if (DAT_10203434 != 0)
        {
            for (int i = 0; i < 10; i++)
            {
                DAT_118d0ba8[shadows[i]].UpdateImage(0);
                FUN_1013f070(4, -1, 0);
                FUN_1013f070(6, 0, 0);
            }
        }
        if (DAT_10f65484 != 0)
            FUN_101439e0();
        else
            FUN_1014b480();
        if (DAT_10f654e4 != 0)
        {
            if (crossfade[0] < 0 || crossfade[1] < 0 || crossfade[2] < 0 || mirror < 0 || saveBuffer < 0 ||
                minimap < 0)
                return 0;
        }
        else if (minimap2 < 0 || saveBuffer2 < 0 || mirror2 < 0 || minimap < 0 || saveBuffer < 0 || mirror < 0)
            return 0;
    }
    return 1;
}

extern "C" int DAT_1020343c;                         // lens flares switched on
extern "C" int FUN_1013f880(void);                   // the game's random number

// 0x10166520 (ModelBank::DrawLensFlares). For every flare point collected this frame draws eight discs from the lens
// textures on the line from the point through the screen centre: each with its size (half-width in 640x480 pixels,
// scaled by the point's brightness), position along the line and opacity; the first disc flickers a little.
// Returns 0 when the bank or the flares are off, else 1.
int ModelBank::DrawLensFlares()
{
    if (enabled == 0)
        return 0;
    if (DAT_1020343c == 0)
        return 0;
    FUN_1013f070(0, 1, 0);
    FUN_1013f070(9, 0, 0);
    int sizes[10] = {220, 100, 50, 30, 20, 20, 80, 160};
    int steps[10] = {0, -4, 4, 6, 8, 11, 15, 19};
    int lenses[10] = {0, 3, 1, 2, 3, 2, 2, 1};
    float alphas[10] = {0.0f, 0.4f, 0.4f, 0.4f, 0.3f, 0.2f, 0.2f, 0.15f};
    alphas[0] = (float)(FUN_1013f880() % 10) / 200.0f + 0.25f;
    for (int flare = 0; flare < flareCount; flare++)
    {
        for (int i = 0; i < 8; i++)
        {
            float size = (float)sizes[i] * flareAlpha[flare];
            float step = (float)steps[i];
            DAT_118d0ba8[lens[lenses[i]]].DrawScreenCopy(
                (int)(flareScreen[flare].x * 320.0f + 320.0f - size - step * flareScreen[flare].x * 32.0f),
                (int)(240.0f - flareScreen[flare].y * 240.0f - size + step * flareScreen[flare].y * 24.0f),
                (int)(size - step * flareScreen[flare].x * 32.0f + flareScreen[flare].x * 320.0f + 320.0f),
                (int)(step * flareScreen[flare].y * 24.0f + size - flareScreen[flare].y * 240.0f + 240.0f),
                0.0f, alphas[i]);
        }
    }
    FUN_1013f070(0, 0, 0);
    FUN_1013f070(9, 1, 0);
    return 1;
}
