// The scene script of the world (WorldRenderer::RunScriptFrames, 0x1017cc90, 29,886 bytes, the largest function in
// APP.dll). The lines of the character's .itm file ("##nnnnn text") carry the scene script: each line has a frame
// number and a three letter command with comma separated fields (messageIds, messages). RunScriptFrames executes the
// lines of every frame from the last built one up to the current one, then steps the animation of the world.

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "world_renderer.h"
#include "world_script.h"
#include "anim_model.h"
#include "camera_vars.h"
#include "camera_update_vars.h"
#include "render_vars.h"
#include "game_vars.h"
#include "map_setup.h"
#include "football_state.h"
#include "name_paths.h"
#include "field_item_menu.h"
#include "texture_load.h"

#define COMMAND_BLANK '-'                  // a line that starts with "---" is empty
#define DEGREES_TO_RADIANS_NUMERATOR 3.1415f
#define SCRIPT_FRAME_RATE 30.0             // seconds to frames in the lip and caption durations
#define LEVEL_COUNT 4                      // effect levels and scripted camera slots
#define LIP_ENTRIES 0x10                   // lip textures of a character
#define MODEL_SLOTS 0x10                   // model slots stepped each frame
#define PICTURE_HALF_SIZE 128.0f           // default width and height of a picture
#define GROUND_LIFT 10.0f                  // the ground probe starts this far above the model
#define GROUND_PROBE_DEPTH 1000.0f
#define NO_GROUND -3500.0f
#define GROUND_PROBE_PERIOD 30
#define GROUND_PROBE_MASK 0x4000000
#define ALPHA_MIN 0.0f
#define POST_STRENGTH_MAX 1.0f
#define NODE_PROXIMITY_FLOOR -10000.0f

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

// Field `n` of the script line being run (field 0 is the command), read into `field`.
#define FIELD_TEXT(n) (GetMessageField(line, (n), field), field)
#define FIELD_INT(n) atol(FIELD_TEXT(n))
#define FIELD_FLOAT(n) atof(FIELD_TEXT(n))
#define FIELD_INTO(n, buffer) GetMessageField(line, (n), buffer)
// Whether the last field read ends in the literal `extension` (case-insensitive).
#define FIELD_ENDS_WITH(extension) (_strnicmp(field + strlen(field) - (sizeof(extension) - 1), extension, sizeof(extension) - 1) == 0)
#define FIELD_SKIP(n) FIELD_INTO(n, field)

// Whether the script line being run is the three letter command a b c.
#define IS_COMMAND(a, b, c) (messages[line][0] == (a) && messages[line][1] == (b) && messages[line][2] == (c))

// Starts a change of `value` to `target` over `frames` frames (at once when fewer than 2).
static inline void StartChange(float &value, float &step, int &framesLeft, float target, long frames)
{
    if (frames < 2)
    {
        value = target;
        step = 0;
        framesLeft = 0;
    }
    else
    {
        framesLeft = frames;
        step = (target - value) / (float)frames;
    }
}

static inline float DegreesToRadians(double degrees)
{
    return (float)(degrees * DEGREES_TO_RADIANS_NUMERATOR / 180.0);
}

static inline int ClampLevel(long value)
{
    if (value > LEVEL_COUNT - 1)
        return LEVEL_COUNT - 1;
    if (value < 0)
        return 0;
    return value;
}

// The model that stands for slot `slot` (its alias, when it has one).
static inline int ModelOf(WorldRenderer *world, int slot)
{
    int alias = world->modelAlias[slot];
    return alias >= 0 ? alias : slot;
}

// Duration of a lip text: seconds when it has a decimal point, frames otherwise.
static inline int ParseLipDuration(const char *text)
{
    if (strchr(text, '.') != 0)
        return (int)(atof(text) * SCRIPT_FRAME_RATE);
    return atol(text);
}

// Frames of one pass divided by the weights of the lip textures set so far (at least 1).
static int LipRepeatCount(WorldRenderer *world, int character)
{
    int total = 0;
    for (int e = 0; e < LIP_ENTRIES; e++)
    {
        if (world->lipNames[character][e][0] == 0)
            break;
        total += world->lipWeights[character][e];
    }
    if (total > 0)
        total = world->lipDuration[character] / total;
    if (total < 1)
        total = 1;
    return total;
}

static int SameName(const char *a, const char *b)
{
    return strcmp(a, b) == 0;
}

static inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

// 0x1017cc90. Runs the scene script from the frame after the last built one up to the current frame (`frame`):
// every line whose id equals the frame being built is executed (cameras, models, motions, pictures, captions,
// lip texts, sound and host notifications), then the tweens, node transforms, pictures, key banks and models of
// that frame are stepped. `silent` = 1 when the world is only being rebuilt (Seek): the host is not asked to jump
// and no motions are started. Returns 1.
int WorldRenderer::RunScriptFrames(int silent)
{
    char field[0x100];
    char newName[0x100];
    char fileName[0x100];
    char parentName[0x100];
    char removeName[0x100];
    int skipping = 0;
    levelKeyHit = 0;
    if (silent == 0 && callback != 0)
    {
        int pastFirstSecond = lastFrame > 30;
        if (callback(WORLD_EVENT_QUERY_JUMP, (int)&pastFirstSecond) != 0)
        {
            skipping = 1;
            frame = frameLimit - 1;
        }
    }
    if (frame == 0)
    {
        effectLevel = 0;
        DAT_1101de34 = 0;
        FUN_1016b1e0(0, "");
        FUN_1016b1e0(1, "");
        FUN_1016b1e0(2, "");
        FUN_1016b1e0(3, "");
        DAT_118e6ce8[0].Clear();
        DAT_111322b8[0].Clear();
        DAT_118e6ce8[1].Clear();
        DAT_111322b8[1].Clear();
        DAT_118e6ce8[2].Clear();
        DAT_111322b8[2].Clear();
        DAT_118e6ce8[3].Clear();
        DAT_111322b8[3].Clear();
        captionVisible = 0;
    }
    for (int current = lastFrame + 1; current <= frame; current++)
    {
        TickFade();
        TickCaption();
        for (int line = 0; line < messageCount; line++)
        {
            if (current != messageIds[line])
                continue;
            if (IS_COMMAND(COMMAND_BLANK, COMMAND_BLANK, COMMAND_BLANK))
                continue;
            if (IS_COMMAND('G', 'G', 'Q'))
            {
                FIELD_SKIP(1);
                effectLevel = MIN(LEVEL_COUNT - 1, MAX(0, atol(field)));
                DAT_1101de34 = effectLevel;
            }
            else if (IS_COMMAND('N', 'A', 'M'))
            {
                FIELD_SKIP(1);
                int character = MIN(0x1f, MAX(0, atol(field)));
                strcpy(characterNames[character], FIELD_TEXT(2));
            }
            else if (IS_COMMAND('C', 'O', 'T'))
            {
                DAT_10203420 = (float)FIELD_FLOAT(1);
            }
            else if (IS_COMMAND('P', 'I', 'A'))
            {
                long mode = FIELD_INT(1);
                float strength = (float)FIELD_FLOAT(2);
                long frames = FIELD_INT(3);
                DAT_10f654f8 = mode;
                if (frames < 2)
                {
                    postStrength = strength;
                    DAT_10f654fc = strength;
                    postStep = 0;
                    postFrames = 0;
                }
                else
                {
                    postFrames = frames;
                    postStep = (strength - postStrength) / (float)frames;
                }
            }
            else if (IS_COMMAND('O', 'R', 'Z'))
            {
                long n = FIELD_INT(1);
                float angle = DegreesToRadians(FIELD_FLOAT(2));
                long frames = FIELD_INT(3);
                if (frames < 2)
                {
                    nodeAngleZ[n] = angle;
                    nodeAngleZStep[n] = 0;
                    nodeAngleZFrames[n] = 0;
                }
                else
                {
                    nodeAngleZFrames[n] = frames;
                    nodeAngleZStep[n] = (angle - nodeAngleZ[n]) / (float)frames;
                }
            }
            else if (IS_COMMAND('O', 'R', 'A'))
            {
                long n = FIELD_INT(1);
                float value = (float)FIELD_FLOAT(2);
                long frames = FIELD_INT(3);
                if (frames < 2)
                {
                    nodeA[n] = value;
                    nodeAStep[n] = 0;
                    nodeAFrames[n] = 0;
                }
                else
                {
                    nodeAFrames[n] = frames;
                    nodeAStep[n] = (value - nodeA[n]) / (float)frames;
                }
            }
            else if (IS_COMMAND('O', 'R', 'T'))
            {
                long n = FIELD_INT(1);
                float value = (float)FIELD_FLOAT(2);
                long frames = FIELD_INT(3);
                if (frames < 2)
                {
                    nodeT[n] = value;
                    nodeTStep[n] = 0;
                    nodeTFrames[n] = 0;
                }
                else
                {
                    nodeTFrames[n] = frames;
                    nodeTStep[n] = (value - nodeT[n]) / (float)frames;
                }
            }
            else if (IS_COMMAND('O', 'R', 'S'))
            {
                long n = FIELD_INT(1);
                float value = (float)FIELD_FLOAT(2);
                long frames = FIELD_INT(3);
                if (frames < 2)
                {
                    nodeScale[n] = value;
                    nodeScaleStep[n] = 0;
                    nodeScaleFrames[n] = 0;
                }
                else
                {
                    nodeScaleFrames[n] = frames;
                    nodeScaleStep[n] = (value - nodeScale[n]) / (float)frames;
                }
            }
            else if (IS_COMMAND('O', 'R', 'Y'))
            {
                long n = FIELD_INT(1);
                float angle = DegreesToRadians(FIELD_FLOAT(2));
                long frames = FIELD_INT(3);
                if (frames < 2)
                {
                    nodeAngleY[n] = angle;
                    nodeAngleYStep[n] = 0;
                    nodeAngleYFrames[n] = 0;
                }
                else
                {
                    nodeAngleYFrames[n] = frames;
                    nodeAngleYStep[n] = (angle - nodeAngleY[n]) / (float)frames;
                }
            }
            else if (IS_COMMAND('O', 'R', 'X'))
            {
                long n = FIELD_INT(1);
                float angle = DegreesToRadians(FIELD_FLOAT(2));
                long frames = FIELD_INT(3);
                if (frames < 2)
                {
                    nodeAngleX[n] = angle;
                    nodeAngleXStep[n] = 0;
                    nodeAngleXFrames[n] = 0;
                }
                else
                {
                    nodeAngleXFrames[n] = frames;
                    nodeAngleXStep[n] = (angle - nodeAngleX[n]) / (float)frames;
                }
            }
            else if (IS_COMMAND('A', 'L', 'N'))
            {
                int model = ModelOf(this, FIELD_INT(1));
                FIELD_INTO(2, newName);
                FIELD_INTO(3, parentName);
                FIELD_INTO(4, fileName);
                Node *root = models[model].root;
                if (root->Find(newName, 0) == 0)
                {
                    Node *parent = root->Find(parentName, 0);
                    if (parent != 0)
                    {
                        Node *child = parent->AddChild(0, 0x40);
                        child->Load(FUN_1013f8a0(fileName, 0), 0, 0, 0);
                        strcpy(child->name, newName);
                        child->parent = parent;
                        child->BuildMesh(1, 0);
                    }
                }
            }
            else if (IS_COMMAND('D', 'E', 'N'))
            {
                int model = ModelOf(this, FIELD_INT(1));
                FIELD_INTO(2, removeName);
                Node *node = models[model].root->Find(removeName, 0);
                if (node != 0)
                    node->parent->RemoveChild(node, 0);
            }
            else if (IS_COMMAND('A', 'O', 'P'))
            {
                long n = FIELD_INT(1);
                long keyFrame = FIELD_INT(2);
                long x = FIELD_INT(3);
                long y = FIELD_INT(4);
                long z = FIELD_INT(5);
                nodePositionKeys[n].Set(keyFrame, (float)x, (float)y, (float)z, 0);
            }
            else if (IS_COMMAND('S', 'P', 'L'))
            {
                long n = FIELD_INT(1);
                FIELD_TEXT(2);
                pictureKind[n] = PICTURE_TEXTURE;
                int movieCode = 0;
                if (FIELD_ENDS_WITH("mmv"))
                {
                    pictureKind[n] = PICTURE_SCREEN;
                    movieActive = 1;
                    moviePicture = n;
                    movieCode = WORLD_EVENT_MOVIE_AVI;
                }
                else if (FIELD_ENDS_WITH("mv3"))
                {
                    pictureKind[n] = PICTURE_SCREEN_FLIPPED;
                    moviePicture = n;
                    notifyPending = 1;
                    movieCode = WORLD_EVENT_MOVIE_MV3;
                }
                else if (FIELD_ENDS_WITH(".mpg") || FIELD_ENDS_WITH("mpeg"))
                {
                    pictureKind[n] = PICTURE_SCREEN_EXACT;
                    moviePicture = n;
                    notifyPending = 2;
                    movieCode = WORLD_EVENT_MOVIE_MPEG;
                }
                else if (FIELD_ENDS_WITH("avi"))
                {
                    pictureKind[n] = PICTURE_SCREEN_EXACT;
                    notifyPending = 2;
                    moviePicture = n;
                    movieCode = WORLD_EVENT_MOVIE_MPEG;
                }
                else
                {
                    textures[n] = FUN_10174ce0(FUN_1013f8a0(field, 0), 0, 0);
                }
                if (movieCode != 0)
                {
                    unsigned int size = callback(movieCode, (int)field);
                    screenHeight = size & 0x3ff;
                    screenWidth = (size >> 10) & 0x3ff;
                    screenTexture = (size >> 20) & 0x3ff;
                }
                pictureAlpha[n] = 0;
                pictureAlphaStep[n] = 0;
                pictureAlphaFrames[n] = 0;
                pictureUVs[n].Set(0, 0, 1.0f, 1.0f);
                pictureUVStep[n].Fill(0);
                pictureUVFrames[n] = 0;
                pictureAngle[n] = 0;
                pictureAngleStep[n] = 0;
                pictureAngleFrames[n] = 0;
                picturePositions[n].Fill(0);
                picturePositionStep[n].Fill(0);
                picturePositionFrames[n] = 0;
                pictureInWorld[n] = 0;
                pictureBlend[n] = PICTURE_BLEND_ALPHA;
                pictureSizes[n].Fill(PICTURE_HALF_SIZE);
                pictureSizeStep[n].Fill(0);
                pictureSizeFrames[n] = 0;
            }
            else if (IS_COMMAND('S', 'P', 'M'))
            {
                long n = FIELD_INT(1);
                pictureBlend[n] = (char)FIELD_INT(2);
            }
            else if (IS_COMMAND('S', 'U', 'V'))
            {
                long n = FIELD_INT(1);
                Vec4 target;
                target.x = (float)FIELD_FLOAT(2);
                target.y = (float)FIELD_FLOAT(3);
                target.z = (float)FIELD_FLOAT(4);
                target.w = (float)FIELD_FLOAT(5);
                long frames = FIELD_INT(6);
                if (frames < 2)
                {
                    pictureUVs[n].Copy(target);
                    pictureUVStep[n].Fill(0);
                    pictureUVFrames[n] = 0;
                }
                else
                {
                    float count = (float)frames;
                    pictureUVStep[n].x = (target.x - pictureUVs[n].x) / count;
                    pictureUVStep[n].y = (target.y - pictureUVs[n].y) / count;
                    pictureUVStep[n].z = (target.z - pictureUVs[n].z) / count;
                    pictureUVFrames[n] = frames;
                    pictureUVStep[n].w = (target.w - pictureUVs[n].w) / count;
                }
            }
            else if (IS_COMMAND('S', 'R', 'O'))
            {
                long n = FIELD_INT(1);
                float value = (float)FIELD_FLOAT(2);
                long frames = FIELD_INT(3);
                if (frames < 2)
                {
                    pictureAngle[n] = value;
                    pictureAngleStep[n] = 0;
                    pictureAngleFrames[n] = 0;
                }
                else
                {
                    pictureAngleFrames[n] = frames;
                    pictureAngleStep[n] = (value - pictureAngle[n]) / (float)frames;
                }
            }
            else if (IS_COMMAND('S', 'S', 'P'))
            {
                long n = FIELD_INT(1);
                float value = (float)FIELD_FLOAT(2);
                long frames = FIELD_INT(3);
                if (frames < 2)
                {
                    pictureGrey[n] = value;
                    pictureGreyStep[n] = 0;
                    pictureGreyFrames[n] = 0;
                }
                else
                {
                    pictureGreyFrames[n] = frames;
                    pictureGreyStep[n] = (value - pictureGrey[n]) / (float)frames;
                }
            }
            else if (IS_COMMAND('S', 'P', 'T'))
            {
                long n = FIELD_INT(1);
                float value = (float)FIELD_FLOAT(2);
                long frames = FIELD_INT(3);
                if (frames < 2)
                {
                    pictureAlpha[n] = value;
                    pictureAlphaStep[n] = 0;
                    pictureAlphaFrames[n] = 0;
                }
                else
                {
                    pictureAlphaFrames[n] = frames;
                    pictureAlphaStep[n] = (value - pictureAlpha[n]) / (float)frames;
                }
            }
            else if (IS_COMMAND('S', 'P', 'S'))
            {
                long n = FIELD_INT(1);
                Vec4 target;
                target.x = (float)FIELD_FLOAT(2);
                target.y = (float)FIELD_FLOAT(3);
                long frames = FIELD_INT(4);
                if (frames < 2)
                {
                    pictureSizes[n].Copy(target);
                    pictureSizeStep[n].Fill(0);
                    pictureSizeFrames[n] = 0;
                }
                else
                {
                    pictureSizeStep[n].x = (target.x - pictureSizes[n].x) / (float)frames;
                    pictureSizeFrames[n] = frames;
                    pictureSizeStep[n].y = (target.y - pictureSizes[n].y) / (float)frames;
                }
            }
            else if (IS_COMMAND('S', 'P', '2') || IS_COMMAND('S', 'P', '3'))
            {
                char inWorld = messages[line][2] == '3';
                long n = FIELD_INT(1);
                Vec4 target;
                target.x = (float)FIELD_FLOAT(2);
                target.y = (float)FIELD_FLOAT(3);
                target.z = (float)FIELD_FLOAT(4);
                long frames = FIELD_INT(5);
                if (frames < 2)
                {
                    picturePositions[n].Copy(target);
                    picturePositionStep[n].Fill(0);
                    picturePositionFrames[n] = 0;
                }
                else
                {
                    float count = (float)frames;
                    picturePositionStep[n].x = (target.x - picturePositions[n].x) / count;
                    picturePositionStep[n].y = (target.y - picturePositions[n].y) / count;
                    picturePositionFrames[n] = frames;
                    picturePositionStep[n].z = (target.z - picturePositions[n].z) / count;
                }
                pictureInWorld[n] = inWorld;
            }
            else if (IS_COMMAND('N', 'P', 'G'))
            {
                FIELD_SKIP(1);
                FIELD_TEXT(2);
                if (eventsMuted == 0)
                    callback(WORLD_EVENT_NPG, (int)field);
            }
            else if (IS_COMMAND('E', 'S', 'P'))
            {
                int value = FIELD_INT(1);
                if (eventsMuted == 0)
                    callback(WORLD_EVENT_ESP, (int)&value);
            }
            else if (IS_COMMAND('R', 'O', 'P'))
            {
                long n = FIELD_INT(1);
                nodePositionKeys[n].Clear();
                nodePositionStart[n] = current;
            }
            else if (IS_COMMAND('L', 'D', 'O'))
            {
                long n = FIELD_INT(1);
                FIELD_TEXT(2);
                if (_stricmp(nodeFiles[n], field) != 0)
                {
                    strcpy(nodeFiles[n], field);
                    nodes[n].Release();
                    nodes[n].Load(FUN_1013f8a0(field, 0), 0, 0, 0);
                    nodes[n].BuildMesh(1, 0);
                    nodeAngleX[n] = 0;
                    nodeAngleXStep[n] = 0;
                    nodeAngleXFrames[n] = 0;
                    nodeAngleY[n] = 0;
                    nodeAngleYStep[n] = 0;
                    nodeAngleYFrames[n] = 0;
                    nodeAngleZ[n] = 0;
                    nodeAngleZStep[n] = 0;
                    nodeAngleZFrames[n] = 0;
                }
            }
            else if (IS_COMMAND('L', 'O', 'P'))
            {
                frame = -1;
            }
            else if (IS_COMMAND('D', 'N', 'D'))
            {
                callback(WORLD_EVENT_DND, (int)FIELD_TEXT(1));
            }
            else if (IS_COMMAND('S', 'E', 'F'))
            {
                int event[5];
                event[1] = FIELD_INT(1);
                event[2] = FIELD_INT(2);
                event[3] = FIELD_INT(3);
                event[4] = FIELD_INT(4);
                event[0] = (int)FIELD_TEXT(5);
                if (eventsMuted == 0 && !skipping)
                    callback(WORLD_EVENT_SOUND_EFFECT, (int)event);
            }
            else if (IS_COMMAND('S', 'P', 'E'))
            {
                int event[2];
                event[0] = FIELD_INT(1);
                event[1] = FIELD_INT(2);
                if (eventsMuted == 0)
                    callback(WORLD_EVENT_PLAY_EFFECT, (int)event);
            }
            else if (IS_COMMAND('S', 'P', 'K'))
            {
                int event[2];
                event[0] = FIELD_INT(1);
                event[1] = FIELD_INT(2);
                if (eventsMuted == 0)
                    callback(WORLD_EVENT_PLAY_SKILL, (int)event);
            }
            else if (IS_COMMAND('C', 'S', '2'))
            {
                int event[3];
                event[2] = FIELD_INT(1);
                event[0] = FIELD_INT(2);
                event[1] = FIELD_INT(3);
                if (DAT_1101d870 == 0)
                {
                    if (event[1] >= 0 && modelAlias[event[1]] >= 0)
                        event[1] = modelAlias[event[1]];
                    if (event[0] == 0 || field[0] == '0')
                    {
                        event[0] = (int)FIELD_TEXT(4);
                        if (eventsMuted == 0 && !skipping)
                            callback(WORLD_EVENT_VOICE_TEXT, (int)event);
                    }
                    else if (eventsMuted == 0 && !skipping)
                        callback(WORLD_EVENT_VOICE, (int)event);
                }
            }
            else if (IS_COMMAND('C', 'S', '3'))
            {
                int event[3];
                event[2] = FIELD_INT(1);
                event[0] = FIELD_INT(2);
                event[1] = FIELD_INT(3);
                if (event[1] >= 0 && modelAlias[event[1]] >= 0)
                    event[1] = modelAlias[event[1]];
                if (event[0] == 0 || field[0] == '0')
                {
                    event[0] = (int)FIELD_TEXT(4);
                    if (eventsMuted == 0 && !skipping)
                        callback(WORLD_EVENT_VOICE_TEXT, (int)event);
                }
                else if (eventsMuted == 0 && !skipping)
                    callback(WORLD_EVENT_VOICE, (int)event);
            }
            else if (IS_COMMAND('C', 'S', '4'))
            {
                int event[6];
                event[2] = FIELD_INT(1);
                event[0] = FIELD_INT(2);
                event[3] = FIELD_INT(3);
                event[4] = FIELD_INT(4);
                event[5] = FIELD_INT(5);
                if (event[0] == 0 || field[0] == '0')
                {
                    event[0] = (int)FIELD_TEXT(6);
                    if (eventsMuted == 0 && !skipping)
                        callback(WORLD_EVENT_VOICE_ALT_TEXT, (int)event);
                }
                else if (eventsMuted == 0 && !skipping)
                    callback(WORLD_EVENT_VOICE_ALT, (int)event);
            }
            else if (IS_COMMAND('C', 'S', 'E'))
            {
                int event[3];
                event[0] = FIELD_INT(1);
                event[1] = FIELD_INT(2);
                event[2] = 0x32;
                if (event[1] < 0)
                {
                    long voice = FIELD_INT(3);
                    if (voice == 0 || field[0] == '0')
                    {
                        if (eventsMuted == 0 && !skipping)
                            callback(WORLD_EVENT_VOICE_STOP, (int)event);
                    }
                    else if (eventsMuted == 0 && !skipping)
                        callback(WORLD_EVENT_VOICE_STOP_MODEL, (int)event);
                }
                else
                {
                    if (modelAlias[event[1]] >= 0)
                        event[1] = modelAlias[event[1]];
                    if (event[0] == 0 || field[0] == '0')
                    {
                        event[0] = (int)FIELD_TEXT(3);
                        if (eventsMuted == 0 && !skipping)
                            callback(WORLD_EVENT_VOICE_TEXT, (int)event);
                    }
                    else if (eventsMuted == 0 && !skipping)
                        callback(WORLD_EVENT_VOICE, (int)event);
                }
            }
            else if (IS_COMMAND('T', 'R', 'E'))
            {
                int event[3];
                event[1] = FIELD_INT(1);
                if (modelAlias[event[1]] >= 0)
                    event[1] = modelAlias[event[1]];
                event[0] = FIELD_INT(2);
                event[2] = (int)FIELD_TEXT(3);
                callback(WORLD_EVENT_TRIGGER_PREPARE, (int)event);
                if (eventsMuted == 0 && !skipping)
                    callback(WORLD_EVENT_TRIGGER, (int)event);
            }
            else if (IS_COMMAND('R', 'E', 'F'))
            {
                int event[6];
                event[1] = FIELD_INT(1);
                event[2] = FIELD_INT(2);
                event[3] = FIELD_INT(3);
                event[0] = FIELD_INT(4);
                event[4] = 0;
                event[5] = 0;
                callback(WORLD_EVENT_REF_PREPARE, (int)event);
                if (eventsMuted == 0 && !skipping)
                    callback(WORLD_EVENT_REF, (int)event);
            }
            else if (IS_COMMAND('R', 'E', '2'))
            {
                int event[6];
                event[1] = FIELD_INT(1);
                event[2] = FIELD_INT(2);
                event[3] = FIELD_INT(3);
                event[0] = FIELD_INT(4);
                event[5] = FIELD_INT(5);
                event[4] = 1;
                callback(WORLD_EVENT_REF_PREPARE, (int)event);
                if (eventsMuted == 0 && !skipping)
                    callback(WORLD_EVENT_REF, (int)event);
            }
            else if (IS_COMMAND('B', 'G', 'M'))
            {
                int music = FIELD_INT(1);
                if (!skipping && eventsMuted == 0)
                    callback(WORLD_EVENT_BGM, (int)&music);
                bgm = music;
            }
            else if (IS_COMMAND('P', 'S', 'E'))
            {
                int voice = FIELD_INT(1);
                if (voice == 0 || field[0] == '0')
                {
                    if (eventsMuted == 0 && !skipping)
                        callback(WORLD_EVENT_VOICE_STOP, (int)&voice);
                }
                else if (eventsMuted == 0 && !skipping)
                    callback(WORLD_EVENT_VOICE_STOP_MODEL, (int)&voice);
            }
            else if (IS_COMMAND('M', 'P', 'A'))
            {
                FIELD_SKIP(1);
                FIELD_TEXT(2);
                callback(WORLD_EVENT_MAP_ARGS, (int)field);
            }
            else if (IS_COMMAND('C', 'F', 'D'))
            {
                long mode = FIELD_INT(1);
                if (transitionsOff == 0)
                    FUN_1015c9e0(mode);
            }
            else if (IS_COMMAND('F', 'C', 'S'))
            {
                long character = FIELD_INT(1);
                long entry = FIELD_INT(2);
                long weight = FIELD_INT(3);
                lipWeights[character][entry] = weight < 2 ? 1 : (char)weight;
                FIELD_TEXT(4);
                if (character == 0 && m_00974 != 0)
                {
                    field[2] = '1';
                    field[3] = '4';
                }
                strcpy(lipNames[character][entry], field);
                lipRepeat[character] = LipRepeatCount(this, character);
            }
            else if (IS_COMMAND('F', 'L', '2'))
            {
                long character = FIELD_INT(1);
                lipDuration[character] = ParseLipDuration(FIELD_TEXT(2));
                lipCountdown[character] = 0;
                lipActive[character] = 1;
                lipIndex[character] = 0;
                int total = 0;
                for (int e = 0; e < LIP_ENTRIES; e++)
                {
                    if (lipNames[character][e][0] == 0)
                        break;
                    total += lipWeights[character][e];
                }
                if (total == 0)
                {
                    lipCountdown[character] = 0;
                    lipActive[character] = 0;
                    lipIndex[character] = 0;
                    lipPause[character] = 0;
                }
                else
                {
                    lipRepeat[character] = LipRepeatCount(this, character);
                    lipPause[character] = 0;
                }
            }
            else if (IS_COMMAND('F', 'S', 'T'))
            {
                long character = FIELD_INT(1);
                lipPause[character] = FIELD_INT(2);
            }
            else if (IS_COMMAND('F', 'C', 'L'))
            {
                long character = FIELD_INT(1);
                lipDuration[character] = ParseLipDuration(FIELD_TEXT(2));
                lipCountdown[character] = 0;
                lipActive[character] = 1;
                lipIndex[character] = 0;
                for (int e = 0; e < LIP_ENTRIES; e++)
                    lipNames[character][e][0] = 0;
                lipPause[character] = 0;
            }
            else if (IS_COMMAND('S', 'R', 'S'))
            {
                int model = ModelOf(this, FIELD_INT(1));
                long value = FIELD_INT(2);
                long frames = FIELD_INT(3);
                if (frames < 1)
                    frames = 1;
                scaleFrames[model] = frames;
                scaleStep[model] = ((float)value - scaleValue[model]) / (float)frames;
            }
            else if (IS_COMMAND('L', 'E', 'G'))
            {
                frameLimit = FIELD_INT(1);
            }
            else if (IS_COMMAND('Z', 'I', 'M'))
            {
                FIELD_TEXT(1);
                FUN_1013e790(caption, field);
                callback(WORLD_EVENT_CAPTION, (int)caption);
                FIELD_TEXT(2);
                if (strrchr(field, '.') != 0)
                    captionVisible = (int)(atof(field) * SCRIPT_FRAME_RATE);
                else
                    captionVisible = atol(field) + 5;
                captionX = FIELD_INT(3);
                captionY = FIELD_INT(4);
                captionSize = FIELD_INT(5);
            }
            else if (IS_COMMAND('E', 'T', 'Q'))
            {
                FIELD_SKIP(1);
                if (eventsMuted == 0)
                    FUN_1016b1e0(0, field);
            }
            else if (IS_COMMAND('E', 'G', 'Q'))
            {
                long index = FIELD_INT(1);
                FIELD_TEXT(2);
                if (eventsMuted == 0)
                    FUN_1016b1e0(index, field);
            }
            else if (IS_COMMAND('B', 'L', 'S'))
            {
                unsigned long red = FIELD_INT(1);
                unsigned long green = FIELD_INT(2);
                unsigned long blue = FIELD_INT(3);
                fadeColour = (red << 8 | green) << 8 | blue;
            }
            else if (IS_COMMAND('B', 'L', 'K'))
            {
                long target = FIELD_INT(1);
                long frames = FIELD_INT(2);
                StartFade(0, target, frames);
            }
            else if (IS_COMMAND('C', 'T', 'R'))
            {
                long n = FIELD_INT(1);
                float target = (float)FIELD_FLOAT(2);
                long frames = FIELD_INT(3);
                int model = ModelOf(this, n);
                modelAlphaFrames[n] = frames;
                if (frames < 1)
                {
                    models[model].alpha = target;
                    if (modelPartner[n] >= 0)
                        models[modelPartner[n]].alpha = target;
                }
                else
                {
                    modelAlphaStep[n] = (target - models[model].alpha) / (float)frames;
                }
                modelAlphaTarget[n] = target;
            }
            else if (IS_COMMAND('E', 'N', 'D'))
            {
                if (bgm != -1 && eventsMuted == 0)
                    callback(WORLD_EVENT_BGM, (int)&bgm);
                callback(WORLD_EVENT_END, 0);
                movieActive = 0;
                if (notifyPending != 0)
                {
                    notifyPending = 0;
                    callback(WORLD_EVENT_MOVIE_END, 0);
                }
                DAT_1101ddf4 = 0;
                for (int i = 0; i < PICTURE_COUNT; i++)
                {
                    if (textures[i] != -1)
                    {
                        FUN_10174c70(textures[i], 0);
                        textures[i] = -1;
                    }
                }
                for (int s = 0; s < 0x60; s++)
                {
                    if (active[s] != 0)
                        SetModelState(s, -5);
                    models[s].m_0267 = m_008c4[s];
                    models[s].important = m_00844[s];
                    models[s].alpha = 1.0f;
                    models[s].m_0012 = 0;
                    if (!skipping)
                    {
                        models[s].M_10161b60();
                        models[s].M_10161f30();
                        models[s].M_10161a60();
                        if (modelPartner[s] >= 0)
                        {
                            models[modelPartner[s]].M_10161b60();
                            models[modelPartner[s]].M_10161f30();
                            models[modelPartner[s]].M_10161a60();
                        }
                    }
                    costumeNames[s][0] = 0;
                }
                for (int j = 0; j < 0x20; j++)
                {
                    nodes[j].Release();
                    loaded = 0;
                }
            }
            else if (IS_COMMAND('F', 'C', 'C'))
            {
                if (models != 0)
                {
                    long n = FIELD_INT(1);
                    FIELD_TEXT(2);
                    if (eventsMuted == 0 && !skipping)
                    {
                        if (n == 0 && m_00974 != 0)
                        {
                            field[2] = '1';
                            field[3] = '4';
                        }
                        if (strcmp(faceTextureNames[n], field) != 0)
                        {
                            int texture = FUN_10174ce0(FUN_1013f8a0(field, 0), 0, 0);
                            if (texture > 0)
                            {
                                models[n].M_10161a20(texture);
                                if (modelPartner[n] >= 0)
                                    models[modelPartner[n]].M_10161a20(texture);
                            }
                        }
                    }
                    strcpy(faceTextureNames[n], field);
                }
            }
            else if (IS_COMMAND('V', 'I', 'C'))
            {
                long n = FIELD_INT(1);
                FIELD_TEXT(2);
                if (strcmp(costumeNames[n], field) == 0)
                {
                    int model = ModelOf(this, n);
                    faceTextureNames[n][0] = 0;
                    models[model].m_0294 = 0;
                    models[model].m_0267 = 1;
                    models[model].m_0012 = 2;
                    models[model].M_10161be0(0, 0);
                    models[model].M_10161f60(0, 0);
                    models[model].matrix._41 = 0;
                    models[model].matrix._42 = 0;
                    models[model].matrix._43 = 0;
                    m_eb058[n] = -1;
                }
                else
                {
                    callback(n, (int)field);
                    strcpy(costumeNames[n], field);
                    faceTextureNames[n][0] = 0;
                    lipActive[n] = 0;
                    int model = ModelOf(this, n);
                    modelFileNames[model][0] = 0;
                    models[model].m_0294 = 0;
                    models[model].m_0267 = 1;
                    models[model].m_0012 = 2;
                    models[model].M_10161be0(0, 0);
                    models[model].M_10161f60(0, 0);
                    SetModelState(model, -5);
                    if (modelPartner[n] >= 0)
                        SetModelState(modelPartner[n], -5);
                    m_eb058[n] = -1;
                }
            }
            else if (IS_COMMAND('L', 'M', 'U'))
            {
                if (models != 0)
                {
                    long n = FIELD_INT(1);
                    FIELD_TEXT(2);
                    int model = ModelOf(this, n);
                    m_eb058[n] = -1;
                    char fileName[0x100];
                    sprintf(fileName, "up%s", field);
                    if (strcmp(modelFileNames[model], fileName) != 0)
                    {
                        models[model].root->PreScanFile(FUN_1013f8a0(fileName, 0), 0);
                        strcpy(modelFileNames[model], fileName);
                    }
                }
            }
            else if (IS_COMMAND('L', 'M', 'T'))
            {
                if (!skipping && models != 0)
                {
                    long n = FIELD_INT(1);
                    FIELD_TEXT(2);
                    m_eb058[n] = -1;
                    int model = ModelOf(this, n);
                    if (strcmp(modelFileNames[model], field) != 0)
                    {
                        models[model].m_0294 = 0;
                        models[model].M_10161be0(0, 0);
                        models[model].M_10161f60(0, 0);
                        models[model].LoadModel(FUN_1013f8a0(field, 0));
                        if (modelPartner[n] >= 0)
                            models[modelPartner[n]].LoadModel(FUN_1013f8a0(field, 0));
                        strcpy(modelFileNames[model], field);
                    }
                }
            }
            else if (IS_COMMAND('S', 'A', 'N'))
            {
                if (!skipping && models != 0)
                {
                    long n = FIELD_INT(1);
                    long motion = FIELD_INT(2);
                    long speed = FIELD_INT(3);
                    long a = FIELD_INT(4);
                    long b = FIELD_INT(5);
                    models[n].SetPlayback(motion, speed, a, b, -1, 0);
                    if (modelPartner[n] >= 0)
                        models[modelPartner[n]].SetPlayback(motion, speed, a, b, -1, 0);
                }
            }
            else if (IS_COMMAND('S', 'A', 'E'))
            {
                if (!skipping && models != 0)
                {
                    long n = FIELD_INT(1);
                    long motion = FIELD_INT(2);
                    long speed = FIELD_INT(3);
                    long a = FIELD_INT(4);
                    long b = FIELD_INT(5);
                    long c = FIELD_INT(6);
                    long d = FIELD_INT(7);
                    models[n].SetPlayback(motion, speed, a, b, c, d);
                    if (modelPartner[n] >= 0)
                        models[modelPartner[n]].SetPlayback(motion, speed, a, b, c, d);
                }
            }
            else if (IS_COMMAND('R', 'I', 'K'))
            {
                if (models != 0 && silent == 0)
                {
                    long n = FIELD_INT(1);
                    models[n].UpdateCollision();
                    if (modelPartner[n] >= 0)
                        models[modelPartner[n]].UpdateCollision();
                }
            }
            else if (IS_COMMAND('C', 'S', 'D'))
            {
                if (models != 0 && silent == 0)
                {
                    long n = FIELD_INT(1);
                    int model = ModelOf(this, n);
                    long value = FIELD_INT(2);
                    models[model].m_17e8 = value == 0;
                    if (modelPartner[n] >= 0)
                        models[model].m_17e8 = value == 0;
                }
            }
            else if (IS_COMMAND('A', 'C', 'G'))
            {
                if (models != 0)
                {
                    long n = FIELD_INT(1);
                    long motion = FIELD_INT(2);
                    long frames = FIELD_INT(3);
                    m_eb058[n] = motion;
                    if (silent == 0)
                    {
                        int model = ModelOf(this, n);
                        models[model].important = 1;
                        models[model].PlayMotion(motion, 1, frames, 1.0f);
                        if (modelPartner[n] >= 0)
                        {
                            models[modelPartner[n]].important = 1;
                            models[modelPartner[n]].PlayMotion(motion, 1, frames, 1.0f);
                        }
                    }
                }
            }
            else if (IS_COMMAND('A', 'C', 'E'))
            {
                if (models != 0)
                {
                    long n = FIELD_INT(1);
                    long motion = FIELD_INT(2);
                    long frames = FIELD_INT(3);
                    float speed = (float)FIELD_FLOAT(4);
                    m_eb058[n] = (short)motion;
                    if (silent == 0)
                    {
                        int model = ModelOf(this, n);
                        models[model].important = 1;
                        models[model].PlayMotion(motion, 1, frames, speed);
                        if (modelPartner[n] >= 0)
                        {
                            models[modelPartner[n]].important = 1;
                            models[modelPartner[n]].PlayMotion(motion, 1, frames, speed);
                        }
                    }
                }
            }
            else if (IS_COMMAND('L', 'M', 'P'))
            {
                FIELD_TEXT(1);
                if (strcmp(mapModelName, field) != 0)
                {
                    strcpy(mapModelName, field);
                    callback(WORLD_EVENT_COSTUME, (int)field);
                    for (int c = 0; c < 0x80; c++)
                        costumeNames[c][0] = 0;
                }
            }
            else if (IS_COMMAND('R', 'F', 'K'))
            {
                levelKeys[0].Clear();
            }
            else if (IS_COMMAND('R', 'F', 'Q'))
            {
                long n = FIELD_INT(1);
                levelKeys[n].Clear();
            }
            else if (IS_COMMAND('R', 'L', 'K'))
            {
                if (models != 0)
                {
                    long n = FIELD_INT(1);
                    int model = ModelOf(this, n);
                    motionKeys[model].Clear();
                    SetModelState(model, 1);
                    motionStart[model] = current;
                    if (modelPartner[n] >= 0)
                    {
                        motionKeys[modelPartner[n]].Clear();
                        SetModelState(modelPartner[n], 1);
                        motionStart[model] = current;
                    }
                }
            }
            else if (IS_COMMAND('R', 'L', 'S'))
            {
                if (models != 0)
                {
                    long n = FIELD_INT(1);
                    SetModelState(ModelOf(this, n), -5);
                    if (modelPartner[n] >= 0)
                        SetModelState(modelPartner[n], -5);
                }
            }
            else if (IS_COMMAND('A', 'F', 'K'))
            {
                if (models != 0)
                {
                    long keyFrame = FIELD_INT(1);
                    long a = FIELD_INT(2);
                    long b = FIELD_INT(3);
                    long c = FIELD_INT(4);
                    levelKeys[0].Set(keyFrame, (float)a, (float)b, (float)c, 0);
                    if (keyFrame == 0)
                        levelKeyStart[0] = current;
                }
            }
            else if (IS_COMMAND('A', 'F', 'Q'))
            {
                if (models != 0)
                {
                    int level = FIELD_INT(1);
                    if (level < 0)
                        level = 0;
                    if (level > 2)
                        level = 3;
                    long keyFrame = FIELD_INT(2);
                    long a = FIELD_INT(3);
                    long b = FIELD_INT(4);
                    long c = FIELD_INT(5);
                    long flag = FIELD_INT(6);
                    int key = levelKeys[level].Set(keyFrame, (float)a, (float)b, (float)c, 0);
                    levelKeys[level].flags[key] = (char)flag;
                    if (keyFrame == 0)
                        levelKeyStart[level] = current;
                }
            }
            else if (IS_COMMAND('A', 'L', 'K'))
            {
                if (models != 0)
                {
                    long n = FIELD_INT(1);
                    long keyFrame = FIELD_INT(2);
                    float a = (float)FIELD_INT(3);
                    long b = FIELD_INT(4);
                    long c = FIELD_INT(5);
                    int model = ModelOf(this, n);
                    motionKeys[model].Set(keyFrame, a, (float)b, (float)c, 0);
                    if (keyFrame == 0)
                        motionStart[model] = current;
                    if (modelPartner[n] >= 0)
                    {
                        motionKeys[modelPartner[n]].Set(keyFrame, a, (float)b, (float)c, 0);
                        if (keyFrame == 0)
                            motionStart[modelPartner[n]] = current;
                    }
                }
            }
            else if (IS_COMMAND('R', 'P', 'K') || IS_COMMAND('R', 'P', '2'))
            {
                if (models != 0)
                {
                    long n = FIELD_INT(1);
                    int model = ModelOf(this, n);
                    pathStart[model] = current;
                    models[model].M_10161a60();
                    if (modelPartner[n] >= 0)
                    {
                        pathStart[modelPartner[n]] = current;
                        models[modelPartner[n]].M_10161a60();
                    }
                }
            }
            else if (IS_COMMAND('A', 'P', 'K'))
            {
                if (models != 0)
                {
                    long n = FIELD_INT(1);
                    long keyFrame = FIELD_INT(2);
                    float a = (float)FIELD_INT(3);
                    long b = FIELD_INT(4);
                    long c = FIELD_INT(5);
                    int model = ModelOf(this, n);
                    models[model].M_10161af0(keyFrame, a, (float)b, (float)c);
                    models[model].M_10161b40(1, 0);
                    if (keyFrame == 0)
                        pathStart[model] = current;
                    if (modelPartner[n] >= 0)
                    {
                        models[modelPartner[n]].M_10161af0(keyFrame, a, (float)b, (float)c);
                        models[modelPartner[n]].M_10161b40(1, 0);
                        if (keyFrame == 0)
                            pathStart[modelPartner[n]] = current;
                    }
                }
            }
            else if (IS_COMMAND('A', 'P', '2'))
            {
                if (models != 0)
                {
                    long n = FIELD_INT(1);
                    long keyFrame = FIELD_INT(2);
                    float a = (float)FIELD_INT(3);
                    long b = FIELD_INT(4);
                    long c = FIELD_INT(5);
                    long flag = FIELD_INT(6);
                    int model = ModelOf(this, n);
                    models[model].M_10161a90(keyFrame, (float)b, a, (float)c, (char)flag);
                    models[model].M_10161b40(1, 0);
                    if (keyFrame == 0)
                        pathStart[model] = current;
                    if (modelPartner[n] >= 0)
                    {
                        models[modelPartner[n]].M_10161a90(keyFrame, (float)b, a, (float)c, (char)flag);
                        models[modelPartner[n]].M_10161b40(1, 0);
                        if (keyFrame == 0)
                            pathStart[modelPartner[n]] = current;
                    }
                }
            }
            else if (IS_COMMAND('R', 'C', 'M'))
            {
                DAT_1101ddf4 = 1;
                DAT_1101ddf8[0] = 1;
                CameraScriptFrame(0) = 0;
                DAT_118e6ce8[0].Clear();
            }
            else if (IS_COMMAND('R', 'C', 'A'))
            {
                DAT_1101ddf4 = 1;
                DAT_1101ddf8[0] = 1;
                CameraScriptFrame(0) = 0;
                DAT_111322b8[0].Clear();
            }
            else if (IS_COMMAND('R', 'G', 'M'))
            {
                FIELD_SKIP(1);
                int slot = MIN(LEVEL_COUNT - 1, MAX(0, atol(field)));
                DAT_1101ddf4 = 1;
                DAT_1101ddf8[slot] = 1;
                CameraScriptFrame(slot) = 0;
                DAT_118e6ce8[slot].Clear();
            }
            else if (IS_COMMAND('R', 'G', 'A'))
            {
                FIELD_SKIP(1);
                int slot = MIN(LEVEL_COUNT - 1, MAX(0, atol(field)));
                DAT_1101ddf4 = 1;
                DAT_1101ddf8[slot] = 1;
                CameraScriptFrame(slot) = 0;
                DAT_111322b8[slot].Clear();
            }
            else if (IS_COMMAND('R', 'M', '2'))
            {
                DAT_1101ddf4 = 1;
                DAT_1101ddf8[0] = 2;
                CameraScriptFrame(0) = 0;
                DAT_118e6ce8[0].Clear();
                CameraScriptExponent(0) = (float)FIELD_FLOAT(1);
            }
            else if (IS_COMMAND('R', 'A', '2'))
            {
                DAT_1101ddf4 = 1;
                DAT_1101ddf8[0] = 2;
                CameraScriptFrame(0) = 0;
                DAT_111322b8[0].Clear();
                DAT_10203464[0] = (float)FIELD_FLOAT(1);
            }
            else if (IS_COMMAND('R', 'M', 'T'))
            {
                long slot = FIELD_INT(1);
                if (slot < 0)
                    slot = 0;
                if (slot > LEVEL_COUNT - 1)
                    slot = LEVEL_COUNT - 1;
                DAT_1101ddf4 = 1;
                DAT_1101ddf8[slot] = 2;
                CameraScriptFrame(slot) = 0;
                DAT_118e6ce8[slot].Clear();
                CameraScriptExponent(slot) = (float)FIELD_FLOAT(2);
            }
            else if (IS_COMMAND('R', 'A', 'T'))
            {
                long slot = FIELD_INT(1);
                if (slot < 0)
                    slot = 0;
                if (slot > LEVEL_COUNT - 1)
                    slot = LEVEL_COUNT - 1;
                DAT_1101ddf4 = 1;
                DAT_1101ddf8[slot] = 2;
                CameraScriptFrame(slot) = 0;
                DAT_111322b8[slot].Clear();
                DAT_10203464[slot] = (float)FIELD_FLOAT(2);
            }
            else if (IS_COMMAND('S', 'R', 'M'))
            {
                long n = FIELD_INT(1);
                long value = FIELD_INT(2);
                int model = ModelOf(this, n);
                scaleStep[model] = 0;
                scaleFrames[model] = 0;
                if (strncmp(field, "PATH", 4) == 0)
                {
                    scaleMode[model] = -1;
                }
                else
                {
                    scaleMode[model] = 0;
                    scaleValue[model] = (float)value;
                }
            }
            else if (IS_COMMAND('A', 'C', 'M'))
            {
                DAT_1101ddf4 = 1;
                long keyFrame = FIELD_INT(1);
                long a = FIELD_INT(2);
                long b = FIELD_INT(3);
                long c = FIELD_INT(4);
                DAT_118e6ce8[0].Set(keyFrame + current, (float)a, (float)b, (float)c, 0);
            }
            else if (IS_COMMAND('A', 'C', 'A'))
            {
                DAT_1101ddf4 = 1;
                long keyFrame = FIELD_INT(1);
                long a = FIELD_INT(2);
                long b = FIELD_INT(3);
                long c = FIELD_INT(4);
                DAT_111322b8[0].Set(keyFrame + current, (float)a, (float)b, (float)c, 0);
            }
            else if (IS_COMMAND('A', 'G', 'M'))
            {
                DAT_1101ddf4 = 1;
                FIELD_SKIP(1);
                int slot = MIN(LEVEL_COUNT - 1, MAX(atol(field), 0));
                long keyFrame = FIELD_INT(2);
                long a = FIELD_INT(3);
                long b = FIELD_INT(4);
                long c = FIELD_INT(5);
                long flag = FIELD_INT(6);
                int key = DAT_118e6ce8[slot].Set(keyFrame + current, (float)a, (float)b, (float)c, 0);
                DAT_118e6ce8[slot].flags[key] = (char)flag;
            }
            else if (IS_COMMAND('A', 'G', 'A'))
            {
                DAT_1101ddf4 = 1;
                FIELD_SKIP(1);
                int slot = MIN(LEVEL_COUNT - 1, MAX(atol(field), 0));
                long keyFrame = FIELD_INT(2);
                long a = FIELD_INT(3);
                long b = FIELD_INT(4);
                long c = FIELD_INT(5);
                long flag = FIELD_INT(6);
                int key = DAT_111322b8[slot].Set(keyFrame + current, (float)a, (float)b, (float)c, 0);
                DAT_111322b8[slot].flags[key] = (char)flag;
            }
            else if (IS_COMMAND('R', 'O', 'L'))
            {
                if (DAT_1101ddf4 == 0)
                    DAT_1101ddf4 = 1;
                long value = FIELD_INT(1);
                long frames = FIELD_INT(2);
                if (frames < 1)
                {
                    levelStep[0] = 0;
                    levelValue[0] = (float)value;
                    levelFrames[0] = 0;
                }
                else
                {
                    levelFrames[0] = frames;
                    levelStep[0] = ((float)value - levelValue[0]) / (float)frames;
                }
            }
            else if (IS_COMMAND('R', 'G', 'L'))
            {
                if (DAT_1101ddf4 == 0)
                    DAT_1101ddf4 = 1;
                int slot = ClampLevel(FIELD_INT(1));
                long value = FIELD_INT(2);
                long frames = FIELD_INT(3);
                if (frames < 1)
                {
                    levelStep[slot] = 0;
                    levelValue[slot] = (float)value;
                    levelFrames[slot] = 0;
                }
                else
                {
                    levelFrames[slot] = frames;
                    levelStep[slot] = ((float)value - levelValue[slot]) / (float)frames;
                }
            }
        }
        if (postFrames != 0)
        {
            float strength = postStep + postStrength;
            postFrames--;
            postStrength = strength;
            if (strength >= POST_STRENGTH_MAX)
                DAT_10f654fc = POST_STRENGTH_MAX;
            else if (strength >= 0.0f)
                DAT_10f654fc = strength;
            else
                DAT_10f654fc = 0.0f;
        }
        bool newFrame = lastScriptFrame != current;
        if (newFrame)
            lastScriptFrame = current;
        for (int c = 0; c < 0x20; c++)
        {
            if (newFrame && lipActive[c] != 0)
            {
                if (lipCountdown[c] == 0)
                {
                    int entry = lipIndex[c];
                    if (lipPause[c] == 0)
                    {
                        if (eventsMuted == 0 && !skipping)
                        {
                            if (strcmp(faceTextureNames[c], lipNames[c][entry]) != 0)
                            {
                                int texture = FUN_10174ce0(FUN_1013f8a0(lipNames[c][entry], 0), 0, 0);
                                if (texture > 0)
                                    models[c].M_10161a20(texture);
                            }
                        }
                        strcpy(faceTextureNames[c], lipNames[c][entry]);
                    }
                    if (lipPause[c] > 0)
                        lipPause[c]--;
                    lipIndex[c]++;
                    if (lipNames[c][lipIndex[c]][0] == 0)
                    {
                        lipIndex[c] = 0;
                        lipRepeat[c]--;
                        if (lipRepeat[c] < 1)
                            lipActive[c] = 0;
                    }
                    lipCountdown[c] = lipWeights[c][entry] - 1;
                }
                else
                {
                    lipCountdown[c]--;
                }
            }
            Vec4 position;
            Matrix4 identity;
            identity.SetIdentity();
            nodePositionKeys[c].Sample(current - nodePositionStart[c], &position.x, &position.y, &position.z, &position.w, 0);
            Matrix4 rotationZ = RotationZ(nodeAngleZ[c]);
            Matrix4 rotationY = RotationY(nodeAngleY[c]);
            Matrix4 rotationX = RotationX(nodeAngleX[c]);
            Matrix4 scaling = Scaling(nodeScale[c], nodeScale[c], nodeScale[c]);
            nodes[c].matrix = ((scaling * rotationX) * rotationY) * rotationZ;
            nodes[c].matrix.SetTranslation(position);
            if (position.x > NODE_PROXIMITY_FLOOR)
                nodes[c].StartProximityPass(identity.m, &identity, &m_00000, (int)nodeA[c], 0);
            if (nodeAFrames[c] != 0)
            {
                nodeAFrames[c]--;
                nodeA[c] = nodeAStep[c] + nodeA[c];
            }
            if (nodeScaleFrames[c] != 0)
            {
                nodeScaleFrames[c]--;
                nodeScale[c] = nodeScaleStep[c] + nodeScale[c];
            }
            if (nodeTFrames[c] != 0)
            {
                nodeTFrames[c]--;
                nodeT[c] = nodeTStep[c] + nodeT[c];
            }
            if (nodeAngleXFrames[c] != 0)
            {
                nodeAngleXFrames[c]--;
                nodeAngleX[c] = nodeAngleXStep[c] + nodeAngleX[c];
            }
            if (nodeAngleYFrames[c] != 0)
            {
                nodeAngleYFrames[c]--;
                nodeAngleY[c] = nodeAngleYStep[c] + nodeAngleY[c];
            }
            if (nodeAngleZFrames[c] != 0)
            {
                nodeAngleZFrames[c]--;
                nodeAngleZ[c] = nodeAngleZStep[c] + nodeAngleZ[c];
            }
        }
        for (int k = 0; k < LEVEL_COUNT; k++)
        {
            if (levelFrames[k] > 0)
            {
                levelFrames[k]--;
                levelValue[k] = levelStep[k] + levelValue[k];
            }
        }
        if (eventsMuted == 0)
        {
            if (movieActive != 0 && pictureAlpha[moviePicture] > 0.0f && callback(WORLD_EVENT_MOVIE_POLL_MMV, 0) == 0)
                movieActive = 0;
            if (notifyPending == 1 && pictureAlpha[moviePicture] > 0.0f && callback(WORLD_EVENT_MOVIE_POLL_MV3, 0) == 0)
            {
                callback(WORLD_EVENT_MOVIE_END, 0);
                notifyPending = 0;
            }
            if (notifyPending == 2 && pictureAlpha[moviePicture] > 0.0f && callback(WORLD_EVENT_MOVIE_POLL_MPEG, 0) == 0)
            {
                callback(WORLD_EVENT_MOVIE_STOP_MPEG, 0);
                notifyPending = 0;
            }
        }
        for (int i = 0; i < PICTURE_COUNT; i++)
        {
            if (pictureKind[i] == 0 && textures[i] == -1)
                continue;
            if (pictureAlphaFrames[i] != 0)
            {
                pictureAlphaFrames[i]--;
                pictureAlpha[i] = pictureAlphaStep[i] + pictureAlpha[i];
            }
            if (pictureUVFrames[i] != 0)
            {
                pictureUVs[i].x = pictureUVStep[i].x + pictureUVs[i].x;
                pictureUVs[i].y = pictureUVStep[i].y + pictureUVs[i].y;
                pictureUVFrames[i]--;
                pictureUVs[i].z = pictureUVStep[i].z + pictureUVs[i].z;
                pictureUVs[i].w = pictureUVStep[i].w + pictureUVs[i].w;
            }
            if (pictureAngleFrames[i] != 0)
            {
                pictureAngleFrames[i]--;
                pictureAngle[i] = pictureAngleStep[i] + pictureAngle[i];
            }
            if (pictureGreyFrames[i] != 0)
            {
                pictureGreyFrames[i]--;
                pictureGrey[i] = pictureGreyStep[i] + pictureGrey[i];
            }
            if (picturePositionFrames[i] != 0)
            {
                picturePositions[i].x = picturePositionStep[i].x + picturePositions[i].x;
                picturePositions[i].y = picturePositionStep[i].y + picturePositions[i].y;
                picturePositionFrames[i]--;
                picturePositions[i].z = picturePositionStep[i].z + picturePositions[i].z;
            }
            if (pictureSizeFrames[i] != 0)
            {
                pictureSizes[i].x = pictureSizeStep[i].x + pictureSizes[i].x;
                pictureSizeFrames[i]--;
                pictureSizes[i].y = pictureSizeStep[i].y + pictureSizes[i].y;
            }
        }
        for (int k = 0; k < LEVEL_COUNT; k++)
        {
            if (current - levelKeyStart[k] >= 0 && levelKeys[k].count > 0 &&
                current <= levelKeys[k].frames[levelKeys[k].count - 1] + levelKeyStart[k] && effectLevel == k)
            {
                levelKeyHit = 1;
                float unusedW;
                levelKeys[k].Sample(current - levelKeyStart[k], &levelKey.x, &levelKey.y, &levelKey.z, &unusedW, 0);
            }
        }
        if (models != 0)
        {
            for (int s = 0; s < MODEL_SLOTS; s++)
            {
                int model = modelAlias[s] >= 0 ? modelAlias[s] : s;
                int partner = modelPartner[s];
                if (model > 0 && current % GROUND_PROBE_PERIOD == model % 10 && models[model].dropCollider != 0)
                {
                    Vec4 from;
                    Vec4 ground;
                    Vec4 top = models[model].matrix.GetTranslation();
                    from.Copy(top);
                    top.z += GROUND_LIFT;
                    Vec4 direction;
                    direction.Set(0, 0, -1.0f, 0);
                    direction.Normalized3();
                    float distance = GROUND_PROBE_DEPTH;
                    from.Copy(from - direction * distance);
                    if (models[model].dropCollider->CastSegment2(0, GROUND_PROBE_MASK, &from, &top, &ground, 0) == 0)
                        ground.z = NO_GROUND;
                    ground.x = from.x;
                    ground.y = from.y;
                    models[model].shadowMatrix.SetTranslation(ground);
                }
                if (modelAlphaFrames[s] != 0)
                {
                    models[model].alpha = models[model].alpha + modelAlphaStep[s];
                    if (partner >= 0)
                        models[partner].alpha = models[partner].alpha + modelAlphaStep[s];
                    modelAlphaFrames[s]--;
                    if (modelAlphaFrames[s] == 0)
                    {
                        models[model].alpha = modelAlphaTarget[s];
                        if (partner >= 0)
                            models[partner].alpha = modelAlphaTarget[s];
                    }
                }
                float unusedW;
                motionKeys[model].Sample(current - motionStart[model], &targets[model].x, &targets[model].y,
                                         &targets[model].z, &unusedW, 0);
                if (partner >= 0)
                    motionKeys[partner].Sample(current - motionStart[partner], &targets[partner].x,
                                               &targets[partner].y, &targets[partner].z, &unusedW, 0);
                models[model].trackC.frame = current - pathStart[model];
                if (partner >= 0)
                    models[partner].trackC.frame = current - pathStart[partner];
                if (scaleMode[model] == 0)
                {
                    if (scaleFrames[model] != 0)
                    {
                        scaleFrames[model]--;
                        scaleValue[model] = scaleStep[model] + scaleValue[model];
                    }
                    models[model].heading = scaleValue[model] * DEGREES_TO_RADIANS_NUMERATOR / 180.0f;
                    models[model].targetHeading = models[model].heading;
                    if (partner >= 0)
                    {
                        models[partner].heading = scaleValue[model] * DEGREES_TO_RADIANS_NUMERATOR / 180.0f;
                        models[partner].targetHeading = models[partner].heading;
                    }
                }
                else
                {
                    Vec4 moved;
                    moved.Copy(models[model].matrix.GetTranslation() - previousPosition[model]);
                    models[model].heading = Atan2(moved.x, moved.y) + DEGREES_TO_RADIANS_NUMERATOR;
                    models[model].targetHeading = models[model].heading;
                    if (partner >= 0)
                    {
                        models[partner].heading = Atan2(moved.x, moved.y) + DEGREES_TO_RADIANS_NUMERATOR;
                        models[partner].targetHeading = models[partner].heading;
                    }
                }
                previousPosition[model].Copy(models[model].matrix.GetTranslation());
            }
        }
    }
    return 1;
}
