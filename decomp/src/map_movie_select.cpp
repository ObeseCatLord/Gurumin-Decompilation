// Choosing the background movie of the map (run from 0x100ba2a0): the camera point nearest to the player picks
// the movie, which is opened into the movie player and rendered into the movie texture.

#include "vec4.h"

struct Matrix4
{
    float m[16];
    ~Matrix4();                                      // 0x1013eda0 (empty)
    Vec4 GetTranslation();                           // 0x101ade80
};

struct NodeExtra
{
    Matrix4 AnimatedMatrix(int frame);               // 0x101693f0
};

struct Node
{
    char pad_0000[0x2a8];
    NodeExtra *extra;                                // 0x2a8: the node's animation tracks
};

// A camera point of the map (0x100 bytes): the frame of the camera path at which it lies and its movie.
struct CameraPoint
{
    int frame;
    int m_0004;
    int movie;                                       // 0x08: movie number (index into DAT_10875e48), -1 = none
    char pad_000c[0xf4];
};

struct TextureObject
{
    char pad_0000[0x38];
};

struct MmvPlayer
{
    void M_10161050();                               // decodes the next frame
    int M_10160960(TextureObject *texture);          // renders the current frame into `texture`
};

extern "C" Matrix4 DAT_1086df38;                     // the player's world matrix
extern "C" Node *DAT_104c03ac;                       // the camera path node
extern "C" CameraPoint DAT_10210fc4[];
extern "C" int DAT_10216dc4;                         // number of camera points
extern "C" int DAT_1020c2c0;                         // the chosen camera point
extern "C" int DAT_1020c2d0;                         // the movie playing
extern "C" int DAT_104bfe70;                         // frames of the movie decoded so far (0x30 = all)
extern "C" char DAT_10875e48[0x40][0x20];            // movie names
extern "C" int DAT_1020b228;
extern "C" int DAT_10c64a5c;
extern "C" int DAT_10202478;                         // the movie texture slot
extern "C" TextureObject DAT_118d0ba8[];
extern "C" MmvPlayer DAT_10f08ab0;
extern "C" int FUN_10016580(const char *name);       // Movie_PlayToTexture
extern "C" int FUN_1001e240(void);                   // Map_IconTexture

// 0x1001f270 (Map_SelectMovie). A C++ function (an extern "C" one gets no EH frame for the temporary matrix). Picks the camera point nearest to the player (within 100 units, else point 0x5c) and, when its movie
// differs from the one playing, finishes decoding the old one's first 0x30 frames, opens the new movie, decodes as
// many frames as were shown of the old one (mirrored past 0x30) and renders it into the movie texture. Returns 0.
int Map_SelectMovie()
{
    Vec4 point;
    Vec4 player;
    float best = 10000.0f;
    int nearest = -1;
    player.Copy(DAT_1086df38.GetTranslation());
    for (int i = 0; i < DAT_10216dc4; i++)
    {
        point.Copy(DAT_104c03ac->extra->AnimatedMatrix(DAT_10210fc4[i].frame).GetTranslation());
        float distance = (player - point).LengthSquared3();
        if (distance < best)
        {
            nearest = i;
            best = distance;
        }
    }
    if (nearest == -1 || best > 10000.0f)
        nearest = 0x5c;
    DAT_1020c2c0 = nearest;
    int movie = DAT_10210fc4[nearest].movie;
    if (movie != DAT_1020c2d0)
    {
        if (DAT_104bfe70 != 0 && DAT_104bfe70 < 0x30)
        {
            for (int frame = DAT_104bfe70; frame < 0x2f; frame += 2)
                DAT_10f08ab0.M_10161050();
            movie = DAT_10210fc4[DAT_1020c2c0].movie;
            DAT_104bfe70 = 0x30;
            DAT_1020c2d0 = movie;
        }
        int shown = DAT_104bfe70;
        if (movie != DAT_1020c2d0)
        {
            DAT_1020c2d0 = movie;
            if (movie >= 0 && movie < 0x40)
            {
                FUN_10016580(DAT_10875e48[movie]);
                DAT_1020b228 = FUN_1001e240();
                if (shown != 0)
                    DAT_104bfe70 = shown;
                if (DAT_104bfe70 > 0x2f)
                {
                    DAT_104bfe70 = 0x61 - DAT_104bfe70;
                    if (DAT_104bfe70 <= 1)
                        DAT_104bfe70 = 1;
                    for (int frame = 1; frame < DAT_104bfe70; frame += 2)
                        DAT_10f08ab0.M_10161050();
                    DAT_10f08ab0.M_10160960(&DAT_118d0ba8[DAT_10202478]);
                }
            }
        }
    }
    DAT_10c64a5c = DAT_1020c2c0;
    return 0;
}
