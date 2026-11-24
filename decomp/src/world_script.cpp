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

static inline bool IsCommand(const char *line, char a, char b, char c)
{
    return line[0] == a && line[1] == b && line[2] == c;
}

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

// Whether `name` ends in `extension` (case-insensitive).
static int HasExtension(const char *name, const char *extension)
{
    int length = strlen(name);
    int extensionLength = strlen(extension);
    return length >= extensionLength && _stricmp(name + length - extensionLength, extension) == 0;
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
    bool skipping = false;
    levelKeyHit = 0;
    if (silent == 0 && callback != 0)
    {
        int pastFirstSecond = lastFrame > 30;
        if (callback(WORLD_EVENT_QUERY_JUMP, (int)&pastFirstSecond) != 0)
        {
            skipping = true;
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
            const char *command = messages[line];
            if (IsCommand(command, COMMAND_BLANK, COMMAND_BLANK, COMMAND_BLANK))
                continue;
            ScriptFields args(this, line);
            if (IsCommand(command, 'G', 'G', 'Q'))
            {
                effectLevel = ClampLevel(args.Int());
                DAT_1101de34 = effectLevel;
            }
            else if (IsCommand(command, 'N', 'A', 'M'))
            {
                long n = args.Int();
                int character = n > 0x1f ? 0x1f : (n < 0 ? 0 : n);
                strcpy(characterNames[character], args.Text());
            }
            else if (IsCommand(command, 'C', 'O', 'T'))
            {
                DAT_10203420 = (float)args.Float();
            }
            else if (IsCommand(command, 'P', 'I', 'A'))
            {
                long mode = args.Int();
                float strength = (float)args.Float();
                long frames = args.Int();
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
            else if (IsCommand(command, 'O', 'R', 'Z'))
            {
                long n = args.Int();
                float angle = DegreesToRadians(args.Float());
                StartChange(nodeAngleZ[n], nodeAngleZStep[n], nodeAngleZFrames[n], angle, args.Int());
            }
            else if (IsCommand(command, 'O', 'R', 'A'))
            {
                long n = args.Int();
                float value = (float)args.Float();
                StartChange(nodeA[n], nodeAStep[n], nodeAFrames[n], value, args.Int());
            }
            else if (IsCommand(command, 'O', 'R', 'T'))
            {
                long n = args.Int();
                float value = (float)args.Float();
                StartChange(nodeT[n], nodeTStep[n], nodeTFrames[n], value, args.Int());
            }
            else if (IsCommand(command, 'O', 'R', 'S'))
            {
                long n = args.Int();
                float value = (float)args.Float();
                StartChange(nodeScale[n], nodeScaleStep[n], nodeScaleFrames[n], value, args.Int());
            }
            else if (IsCommand(command, 'O', 'R', 'Y'))
            {
                long n = args.Int();
                float angle = DegreesToRadians(args.Float());
                StartChange(nodeAngleY[n], nodeAngleYStep[n], nodeAngleYFrames[n], angle, args.Int());
            }
            else if (IsCommand(command, 'O', 'R', 'X'))
            {
                long n = args.Int();
                float angle = DegreesToRadians(args.Float());
                StartChange(nodeAngleX[n], nodeAngleXStep[n], nodeAngleXFrames[n], angle, args.Int());
            }
            else if (IsCommand(command, 'A', 'L', 'N'))
            {
                int model = ModelOf(this, args.Int());
                char newName[0x100];
                char parentName[0x100];
                char fileName[0x100];
                strcpy(newName, args.Text());
                strcpy(parentName, args.Text());
                strcpy(fileName, args.Text());
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
            else if (IsCommand(command, 'D', 'E', 'N'))
            {
                int model = ModelOf(this, args.Int());
                Node *node = models[model].root->Find(args.Text(), 0);
                if (node != 0)
                    node->parent->RemoveChild(node, 0);
            }
            else if (IsCommand(command, 'A', 'O', 'P'))
            {
                long n = args.Int();
                long keyFrame = args.Int();
                long x = args.Int();
                long y = args.Int();
                long z = args.Int();
                nodePositionKeys[n].Set(keyFrame, (float)x, (float)y, (float)z, 0);
            }
            else if (IsCommand(command, 'S', 'P', 'L'))
            {
                long n = args.Int();
                args.Text();
                pictureKind[n] = PICTURE_TEXTURE;
                int movieCode = 0;
                if (HasExtension(args.text, "mmv"))
                {
                    pictureKind[n] = PICTURE_SCREEN;
                    movieActive = 1;
                    moviePicture = n;
                    movieCode = WORLD_EVENT_MOVIE_AVI;
                }
                else if (HasExtension(args.text, "mv3"))
                {
                    pictureKind[n] = PICTURE_SCREEN_FLIPPED;
                    moviePicture = n;
                    notifyPending = 1;
                    movieCode = WORLD_EVENT_MOVIE_MV3;
                }
                else if (HasExtension(args.text, ".mpg") || HasExtension(args.text, "mpeg"))
                {
                    pictureKind[n] = PICTURE_SCREEN_EXACT;
                    moviePicture = n;
                    notifyPending = 2;
                    movieCode = WORLD_EVENT_MOVIE_MPEG;
                }
                else if (HasExtension(args.text, "avi"))
                {
                    pictureKind[n] = PICTURE_SCREEN_EXACT;
                    notifyPending = 2;
                    moviePicture = n;
                    movieCode = WORLD_EVENT_MOVIE_MPEG;
                }
                else
                {
                    textures[n] = FUN_10174ce0(FUN_1013f8a0(args.text, 0), 0, 0);
                }
                if (movieCode != 0)
                {
                    unsigned int size = callback(movieCode, (int)args.text);
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
            else if (IsCommand(command, 'S', 'P', 'M'))
            {
                long n = args.Int();
                pictureBlend[n] = (char)args.Int();
            }
            else if (IsCommand(command, 'S', 'U', 'V'))
            {
                long n = args.Int();
                Vec4 target;
                target.x = (float)args.Float();
                target.y = (float)args.Float();
                target.z = (float)args.Float();
                target.w = (float)args.Float();
                long frames = args.Int();
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
            else if (IsCommand(command, 'S', 'R', 'O'))
            {
                long n = args.Int();
                float value = (float)args.Float();
                StartChange(pictureAngle[n], pictureAngleStep[n], pictureAngleFrames[n], value, args.Int());
            }
            else if (IsCommand(command, 'S', 'S', 'P'))
            {
                long n = args.Int();
                float value = (float)args.Float();
                StartChange(pictureGrey[n], pictureGreyStep[n], pictureGreyFrames[n], value, args.Int());
            }
            else if (IsCommand(command, 'S', 'P', 'T'))
            {
                long n = args.Int();
                float value = (float)args.Float();
                StartChange(pictureAlpha[n], pictureAlphaStep[n], pictureAlphaFrames[n], value, args.Int());
            }
            else if (IsCommand(command, 'S', 'P', 'S'))
            {
                long n = args.Int();
                Vec4 target;
                target.x = (float)args.Float();
                target.y = (float)args.Float();
                long frames = args.Int();
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
            else if (IsCommand(command, 'S', 'P', '2') || IsCommand(command, 'S', 'P', '3'))
            {
                char inWorld = command[2] == '3';
                long n = args.Int();
                Vec4 target;
                target.x = (float)args.Float();
                target.y = (float)args.Float();
                target.z = (float)args.Float();
                long frames = args.Int();
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
            else if (IsCommand(command, 'N', 'P', 'G'))
            {
                args.Skip();
                args.Text();
                if (eventsMuted == 0)
                    callback(WORLD_EVENT_NPG, (int)args.text);
            }
            else if (IsCommand(command, 'E', 'S', 'P'))
            {
                int value = args.Int();
                if (eventsMuted == 0)
                    callback(WORLD_EVENT_ESP, (int)&value);
            }
            else if (IsCommand(command, 'R', 'O', 'P'))
            {
                long n = args.Int();
                nodePositionKeys[n].Clear();
                nodePositionStart[n] = current;
            }
            else if (IsCommand(command, 'L', 'D', 'O'))
            {
                long n = args.Int();
                args.Text();
                if (_stricmp(nodeFiles[n], args.text) != 0)
                {
                    strcpy(nodeFiles[n], args.text);
                    nodes[n].Release();
                    nodes[n].Load(FUN_1013f8a0(args.text, 0), 0, 0, 0);
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
            else if (IsCommand(command, 'L', 'O', 'P'))
            {
                frame = -1;
            }
            else if (IsCommand(command, 'D', 'N', 'D'))
            {
                callback(WORLD_EVENT_DND, (int)args.Text());
            }
            else if (IsCommand(command, 'S', 'E', 'F'))
            {
                int event[5];
                event[1] = args.Int();
                event[2] = args.Int();
                event[3] = args.Int();
                event[4] = args.Int();
                event[0] = (int)args.Text();
                if (eventsMuted == 0 && !skipping)
                    callback(WORLD_EVENT_SOUND_EFFECT, (int)event);
            }
            else if (IsCommand(command, 'S', 'P', 'E'))
            {
                int event[2];
                event[0] = args.Int();
                event[1] = args.Int();
                if (eventsMuted == 0)
                    callback(WORLD_EVENT_PLAY_EFFECT, (int)event);
            }
            else if (IsCommand(command, 'S', 'P', 'K'))
            {
                int event[2];
                event[0] = args.Int();
                event[1] = args.Int();
                if (eventsMuted == 0)
                    callback(WORLD_EVENT_PLAY_SKILL, (int)event);
            }
            else if (IsCommand(command, 'C', 'S', '2'))
            {
                int event[3];
                event[2] = args.Int();
                event[0] = args.Int();
                event[1] = args.Int();
                if (DAT_1101d870 == 0)
                {
                    if (event[1] >= 0 && modelAlias[event[1]] >= 0)
                        event[1] = modelAlias[event[1]];
                    if (event[0] == 0 || args.text[0] == '0')
                    {
                        event[0] = (int)args.Text();
                        if (eventsMuted == 0 && !skipping)
                            callback(WORLD_EVENT_VOICE_TEXT, (int)event);
                    }
                    else if (eventsMuted == 0 && !skipping)
                        callback(WORLD_EVENT_VOICE, (int)event);
                }
            }
            else if (IsCommand(command, 'C', 'S', '3'))
            {
                int event[3];
                event[2] = args.Int();
                event[0] = args.Int();
                event[1] = args.Int();
                if (event[1] >= 0 && modelAlias[event[1]] >= 0)
                    event[1] = modelAlias[event[1]];
                if (event[0] == 0 || args.text[0] == '0')
                {
                    event[0] = (int)args.Text();
                    if (eventsMuted == 0 && !skipping)
                        callback(WORLD_EVENT_VOICE_TEXT, (int)event);
                }
                else if (eventsMuted == 0 && !skipping)
                    callback(WORLD_EVENT_VOICE, (int)event);
            }
            else if (IsCommand(command, 'C', 'S', '4'))
            {
                int event[6];
                event[2] = args.Int();
                event[0] = args.Int();
                event[3] = args.Int();
                event[4] = args.Int();
                event[5] = args.Int();
                if (event[0] == 0 || args.text[0] == '0')
                {
                    event[0] = (int)args.Text();
                    if (eventsMuted == 0 && !skipping)
                        callback(WORLD_EVENT_VOICE_ALT_TEXT, (int)event);
                }
                else if (eventsMuted == 0 && !skipping)
                    callback(WORLD_EVENT_VOICE_ALT, (int)event);
            }
            else if (IsCommand(command, 'C', 'S', 'E'))
            {
                int event[3];
                event[0] = args.Int();
                event[1] = args.Int();
                event[2] = 0x32;
                if (event[1] < 0)
                {
                    long voice = args.Int();
                    if (voice == 0 || args.text[0] == '0')
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
                    if (event[0] == 0 || args.text[0] == '0')
                    {
                        event[0] = (int)args.Text();
                        if (eventsMuted == 0 && !skipping)
                            callback(WORLD_EVENT_VOICE_TEXT, (int)event);
                    }
                    else if (eventsMuted == 0 && !skipping)
                        callback(WORLD_EVENT_VOICE, (int)event);
                }
            }
            else if (IsCommand(command, 'T', 'R', 'E'))
            {
                int event[3];
                event[1] = args.Int();
                if (modelAlias[event[1]] >= 0)
                    event[1] = modelAlias[event[1]];
                event[0] = args.Int();
                event[2] = (int)args.Text();
                callback(WORLD_EVENT_TRIGGER_PREPARE, (int)event);
                if (eventsMuted == 0 && !skipping)
                    callback(WORLD_EVENT_TRIGGER, (int)event);
            }
            else if (IsCommand(command, 'R', 'E', 'F'))
            {
                int event[6];
                event[1] = args.Int();
                event[2] = args.Int();
                event[3] = args.Int();
                event[0] = args.Int();
                event[4] = 0;
                event[5] = 0;
                callback(WORLD_EVENT_REF_PREPARE, (int)event);
                if (eventsMuted == 0 && !skipping)
                    callback(WORLD_EVENT_REF, (int)event);
            }
            else if (IsCommand(command, 'R', 'E', '2'))
            {
                int event[6];
                event[1] = args.Int();
                event[2] = args.Int();
                event[3] = args.Int();
                event[0] = args.Int();
                event[5] = args.Int();
                event[4] = 1;
                callback(WORLD_EVENT_REF_PREPARE, (int)event);
                if (eventsMuted == 0 && !skipping)
                    callback(WORLD_EVENT_REF, (int)event);
            }
            else if (IsCommand(command, 'B', 'G', 'M'))
            {
                int music = args.Int();
                if (!skipping && eventsMuted == 0)
                    callback(WORLD_EVENT_BGM, (int)&music);
                bgm = music;
            }
            else if (IsCommand(command, 'P', 'S', 'E'))
            {
                int voice = args.Int();
                if (voice == 0 || args.text[0] == '0')
                {
                    if (eventsMuted == 0 && !skipping)
                        callback(WORLD_EVENT_VOICE_STOP, (int)&voice);
                }
                else if (eventsMuted == 0 && !skipping)
                    callback(WORLD_EVENT_VOICE_STOP_MODEL, (int)&voice);
            }
            else if (IsCommand(command, 'M', 'P', 'A'))
            {
                args.Skip();
                args.Text();
                callback(WORLD_EVENT_MAP_ARGS, (int)args.text);
            }
            else if (IsCommand(command, 'C', 'F', 'D'))
            {
                long mode = args.Int();
                if (transitionsOff == 0)
                    FUN_1015c9e0(mode);
            }
            else if (IsCommand(command, 'F', 'C', 'S'))
            {
                long character = args.Int();
                long entry = args.Int();
                long weight = args.Int();
                lipWeights[character][entry] = weight < 2 ? 1 : (char)weight;
                args.Text();
                if (character == 0 && m_00974 != 0)
                {
                    args.text[2] = '1';
                    args.text[3] = '4';
                }
                strcpy(lipNames[character][entry], args.text);
                lipRepeat[character] = LipRepeatCount(this, character);
            }
            else if (IsCommand(command, 'F', 'L', '2'))
            {
                long character = args.Int();
                lipDuration[character] = ParseLipDuration(args.Text());
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
            else if (IsCommand(command, 'F', 'S', 'T'))
            {
                long character = args.Int();
                lipPause[character] = args.Int();
            }
            else if (IsCommand(command, 'F', 'C', 'L'))
            {
                long character = args.Int();
                lipDuration[character] = ParseLipDuration(args.Text());
                lipCountdown[character] = 0;
                lipActive[character] = 1;
                lipIndex[character] = 0;
                for (int e = 0; e < LIP_ENTRIES; e++)
                    lipNames[character][e][0] = 0;
                lipPause[character] = 0;
            }
            else if (IsCommand(command, 'S', 'R', 'S'))
            {
                int model = ModelOf(this, args.Int());
                long value = args.Int();
                long frames = args.Int();
                if (frames < 1)
                    frames = 1;
                scaleFrames[model] = frames;
                scaleStep[model] = ((float)value - scaleValue[model]) / (float)frames;
            }
            else if (IsCommand(command, 'L', 'E', 'G'))
            {
                frameLimit = args.Int();
            }
            else if (IsCommand(command, 'Z', 'I', 'M'))
            {
                args.Text();
                FUN_1013e790(caption, args.text);
                callback(WORLD_EVENT_CAPTION, (int)caption);
                args.Text();
                if (strrchr(args.text, '.') != 0)
                    captionVisible = (int)(atof(args.text) * SCRIPT_FRAME_RATE);
                else
                    captionVisible = atol(args.text) + 5;
                captionX = args.Int();
                captionY = args.Int();
                captionSize = args.Int();
            }
            else if (IsCommand(command, 'E', 'T', 'Q'))
            {
                args.Skip();
                if (eventsMuted == 0)
                    FUN_1016b1e0(0, args.text);
            }
            else if (IsCommand(command, 'E', 'G', 'Q'))
            {
                long index = args.Int();
                args.Text();
                if (eventsMuted == 0)
                    FUN_1016b1e0(index, args.text);
            }
            else if (IsCommand(command, 'B', 'L', 'S'))
            {
                unsigned long red = args.Int();
                unsigned long green = args.Int();
                unsigned long blue = args.Int();
                fadeColour = (red << 8 | green) << 8 | blue;
            }
            else if (IsCommand(command, 'B', 'L', 'K'))
            {
                long target = args.Int();
                long frames = args.Int();
                StartFade(0, target, frames);
            }
            else if (IsCommand(command, 'C', 'T', 'R'))
            {
                long n = args.Int();
                float target = (float)args.Float();
                long frames = args.Int();
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
            else if (IsCommand(command, 'E', 'N', 'D'))
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
            else if (IsCommand(command, 'F', 'C', 'C'))
            {
                if (models != 0)
                {
                    long n = args.Int();
                    args.Text();
                    if (eventsMuted == 0 && !skipping)
                    {
                        if (n == 0 && m_00974 != 0)
                        {
                            args.text[2] = '1';
                            args.text[3] = '4';
                        }
                        if (strcmp(faceTextureNames[n], args.text) != 0)
                        {
                            int texture = FUN_10174ce0(FUN_1013f8a0(args.text, 0), 0, 0);
                            if (texture > 0)
                            {
                                models[n].M_10161a20(texture);
                                if (modelPartner[n] >= 0)
                                    models[modelPartner[n]].M_10161a20(texture);
                            }
                        }
                    }
                    strcpy(faceTextureNames[n], args.text);
                }
            }
            else if (IsCommand(command, 'V', 'I', 'C'))
            {
                long n = args.Int();
                args.Text();
                if (strcmp(costumeNames[n], args.text) == 0)
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
                    callback(n, (int)args.text);
                    strcpy(costumeNames[n], args.text);
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
            else if (IsCommand(command, 'L', 'M', 'U'))
            {
                if (models != 0)
                {
                    long n = args.Int();
                    args.Text();
                    int model = ModelOf(this, n);
                    m_eb058[n] = -1;
                    char fileName[0x100];
                    sprintf(fileName, "up%s", args.text);
                    if (strcmp(modelFileNames[model], fileName) != 0)
                    {
                        models[model].root->PreScanFile(FUN_1013f8a0(fileName, 0), 0);
                        strcpy(modelFileNames[model], fileName);
                    }
                }
            }
            else if (IsCommand(command, 'L', 'M', 'T'))
            {
                if (!skipping && models != 0)
                {
                    long n = args.Int();
                    args.Text();
                    m_eb058[n] = -1;
                    int model = ModelOf(this, n);
                    if (strcmp(modelFileNames[model], args.text) != 0)
                    {
                        models[model].m_0294 = 0;
                        models[model].M_10161be0(0, 0);
                        models[model].M_10161f60(0, 0);
                        models[model].LoadModel(FUN_1013f8a0(args.text, 0));
                        if (modelPartner[n] >= 0)
                            models[modelPartner[n]].LoadModel(FUN_1013f8a0(args.text, 0));
                        strcpy(modelFileNames[model], args.text);
                    }
                }
            }
            else if (IsCommand(command, 'S', 'A', 'N'))
            {
                if (!skipping && models != 0)
                {
                    long n = args.Int();
                    long motion = args.Int();
                    long speed = args.Int();
                    long a = args.Int();
                    long b = args.Int();
                    models[n].SetPlayback(motion, speed, a, b, -1, 0);
                    if (modelPartner[n] >= 0)
                        models[modelPartner[n]].SetPlayback(motion, speed, a, b, -1, 0);
                }
            }
            else if (IsCommand(command, 'S', 'A', 'E'))
            {
                if (!skipping && models != 0)
                {
                    long n = args.Int();
                    long motion = args.Int();
                    long speed = args.Int();
                    long a = args.Int();
                    long b = args.Int();
                    long c = args.Int();
                    long d = args.Int();
                    models[n].SetPlayback(motion, speed, a, b, c, d);
                    if (modelPartner[n] >= 0)
                        models[modelPartner[n]].SetPlayback(motion, speed, a, b, c, d);
                }
            }
            else if (IsCommand(command, 'R', 'I', 'K'))
            {
                if (models != 0 && silent == 0)
                {
                    long n = args.Int();
                    models[n].UpdateCollision();
                    if (modelPartner[n] >= 0)
                        models[modelPartner[n]].UpdateCollision();
                }
            }
            else if (IsCommand(command, 'C', 'S', 'D'))
            {
                if (models != 0 && silent == 0)
                {
                    long n = args.Int();
                    int model = ModelOf(this, n);
                    long value = args.Int();
                    models[model].m_17e8 = value == 0;
                    if (modelPartner[n] >= 0)
                        models[model].m_17e8 = value == 0;
                }
            }
            else if (IsCommand(command, 'A', 'C', 'G'))
            {
                if (models != 0)
                {
                    long n = args.Int();
                    long motion = args.Int();
                    long frames = args.Int();
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
            else if (IsCommand(command, 'A', 'C', 'E'))
            {
                if (models != 0)
                {
                    long n = args.Int();
                    long motion = args.Int();
                    long frames = args.Int();
                    float speed = (float)args.Float();
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
            else if (IsCommand(command, 'L', 'M', 'P'))
            {
                args.Text();
                if (strcmp(mapModelName, args.text) != 0)
                {
                    strcpy(mapModelName, args.text);
                    callback(WORLD_EVENT_COSTUME, (int)args.text);
                    for (int c = 0; c < 0x80; c++)
                        costumeNames[c][0] = 0;
                }
            }
            else if (IsCommand(command, 'R', 'F', 'K'))
            {
                levelKeys[0].Clear();
            }
            else if (IsCommand(command, 'R', 'F', 'Q'))
            {
                long n = args.Int();
                levelKeys[n].Clear();
            }
            else if (IsCommand(command, 'R', 'L', 'K'))
            {
                if (models != 0)
                {
                    long n = args.Int();
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
            else if (IsCommand(command, 'R', 'L', 'S'))
            {
                if (models != 0)
                {
                    long n = args.Int();
                    SetModelState(ModelOf(this, n), -5);
                    if (modelPartner[n] >= 0)
                        SetModelState(modelPartner[n], -5);
                }
            }
            else if (IsCommand(command, 'A', 'F', 'K'))
            {
                if (models != 0)
                {
                    long keyFrame = args.Int();
                    long a = args.Int();
                    long b = args.Int();
                    long c = args.Int();
                    levelKeys[0].Set(keyFrame, (float)a, (float)b, (float)c, 0);
                    if (keyFrame == 0)
                        levelKeyStart[0] = current;
                }
            }
            else if (IsCommand(command, 'A', 'F', 'Q'))
            {
                if (models != 0)
                {
                    int level = args.Int();
                    if (level < 0)
                        level = 0;
                    if (level > 2)
                        level = 3;
                    long keyFrame = args.Int();
                    long a = args.Int();
                    long b = args.Int();
                    long c = args.Int();
                    long flag = args.Int();
                    int key = levelKeys[level].Set(keyFrame, (float)a, (float)b, (float)c, 0);
                    levelKeys[level].flags[key] = (char)flag;
                    if (keyFrame == 0)
                        levelKeyStart[level] = current;
                }
            }
            else if (IsCommand(command, 'A', 'L', 'K'))
            {
                if (models != 0)
                {
                    long n = args.Int();
                    long keyFrame = args.Int();
                    float a = (float)args.Int();
                    long b = args.Int();
                    long c = args.Int();
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
            else if (IsCommand(command, 'R', 'P', 'K') || IsCommand(command, 'R', 'P', '2'))
            {
                if (models != 0)
                {
                    long n = args.Int();
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
            else if (IsCommand(command, 'A', 'P', 'K'))
            {
                if (models != 0)
                {
                    long n = args.Int();
                    long keyFrame = args.Int();
                    float a = (float)args.Int();
                    long b = args.Int();
                    long c = args.Int();
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
            else if (IsCommand(command, 'A', 'P', '2'))
            {
                if (models != 0)
                {
                    long n = args.Int();
                    long keyFrame = args.Int();
                    float a = (float)args.Int();
                    long b = args.Int();
                    long c = args.Int();
                    long flag = args.Int();
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
            else if (IsCommand(command, 'R', 'C', 'M') || IsCommand(command, 'R', 'C', 'A'))
            {
                DAT_1101ddf4 = 1;
                DAT_1101ddf8[0] = 1;
                CameraScriptFrame(0) = 0;
                if (command[2] == 'M')
                    DAT_118e6ce8[0].Clear();
                else
                    DAT_111322b8[0].Clear();
            }
            else if (IsCommand(command, 'R', 'G', 'M') || IsCommand(command, 'R', 'G', 'A'))
            {
                int slot = ClampLevel(args.Int());
                DAT_1101ddf4 = 1;
                DAT_1101ddf8[slot] = 1;
                CameraScriptFrame(slot) = 0;
                if (command[2] == 'M')
                    DAT_118e6ce8[slot].Clear();
                else
                    DAT_111322b8[slot].Clear();
            }
            else if (IsCommand(command, 'R', 'M', '2') || IsCommand(command, 'R', 'A', '2'))
            {
                DAT_1101ddf4 = 1;
                DAT_1101ddf8[0] = 2;
                CameraScriptFrame(0) = 0;
                if (command[1] == 'M')
                {
                    DAT_118e6ce8[0].Clear();
                    CameraScriptExponent(0) = (float)args.Float();
                }
                else
                {
                    DAT_111322b8[0].Clear();
                    DAT_10203464[0] = (float)args.Float();
                }
            }
            else if (IsCommand(command, 'R', 'M', 'T') || IsCommand(command, 'R', 'A', 'T'))
            {
                int slot = ClampLevel(args.Int());
                DAT_1101ddf4 = 1;
                DAT_1101ddf8[slot] = 2;
                CameraScriptFrame(slot) = 0;
                if (command[1] == 'M')
                {
                    DAT_118e6ce8[slot].Clear();
                    CameraScriptExponent(slot) = (float)args.Float();
                }
                else
                {
                    DAT_111322b8[slot].Clear();
                    DAT_10203464[slot] = (float)args.Float();
                }
            }
            else if (IsCommand(command, 'S', 'R', 'M'))
            {
                long n = args.Int();
                long value = args.Int();
                int model = ModelOf(this, n);
                scaleStep[model] = 0;
                scaleFrames[model] = 0;
                if (strncmp(args.text, "PATH", 4) == 0)
                {
                    scaleMode[model] = -1;
                }
                else
                {
                    scaleMode[model] = 0;
                    scaleValue[model] = (float)value;
                }
            }
            else if (IsCommand(command, 'A', 'C', 'M') || IsCommand(command, 'A', 'C', 'A'))
            {
                DAT_1101ddf4 = 1;
                long keyFrame = args.Int();
                long a = args.Int();
                long b = args.Int();
                long c = args.Int();
                if (command[2] == 'M')
                    DAT_118e6ce8[0].Set(keyFrame + current, (float)a, (float)b, (float)c, 0);
                else
                    DAT_111322b8[0].Set(keyFrame + current, (float)a, (float)b, (float)c, 0);
            }
            else if (IsCommand(command, 'A', 'G', 'M') || IsCommand(command, 'A', 'G', 'A'))
            {
                DAT_1101ddf4 = 1;
                int slot = ClampLevel(args.Int());
                long keyFrame = args.Int();
                long a = args.Int();
                long b = args.Int();
                long c = args.Int();
                long flag = args.Int();
                KeyBank &bank = command[2] == 'M' ? DAT_118e6ce8[slot] : DAT_111322b8[slot];
                int key = bank.Set(keyFrame + current, (float)a, (float)b, (float)c, 0);
                bank.flags[key] = (char)flag;
            }
            else if (IsCommand(command, 'R', 'O', 'L'))
            {
                if (DAT_1101ddf4 == 0)
                    DAT_1101ddf4 = 1;
                long value = args.Int();
                long frames = args.Int();
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
            else if (IsCommand(command, 'R', 'G', 'L'))
            {
                if (DAT_1101ddf4 == 0)
                    DAT_1101ddf4 = 1;
                int slot = ClampLevel(args.Int());
                long value = args.Int();
                long frames = args.Int();
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
