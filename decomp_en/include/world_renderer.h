// WorldRenderer: declared here once. The world object (global instance DAT_10c68000): the current map's models,
// textures, draw lists, fades and character messages (world_ctor.cpp, world_frame.cpp, world_model_state.cpp,
// scene_start.cpp).
#ifndef GURUMIN_EN_WORLD_RENDERER_H
#define GURUMIN_EN_WORLD_RENDERER_H

#include "vec4.h"
#include "key_bank.h"
#include "node.h"
#include "font_renderer.h"

struct AnimModel;

const int WORLD_MODEL_SLOTS = 0x80;      // model records of the world (the character table)
const int WORLD_NODE_COUNT = 0x20;       // map model nodes

// Host notification callback of the world (codes such as 0x5e0 on release, 9999 when a character is loaded).
typedef int (*WorldCallback)(int code, int value);

// Number of picture slots (WorldRenderer::picture*).
#define PICTURE_COUNT 0x20

// Keys below this number have their lowest line id cached (WorldRenderer::lowestIdCache).
const int MESSAGE_ID_CACHE_SIZE = 1000;
// Value of an id cache entry that has not been worked out yet.
const int MESSAGE_ID_CACHE_EMPTY = -9999;
// FindMessageLine result for a key that has no lines.
const int MESSAGE_LINE_NOT_FOUND = -1;
// EffectLevelKeyKind results: the key has an effect-level key line of the "K" form (RFK/AFK) or the "Q" form (RFQ/AFQ).
const int EFFECT_LEVEL_KEY_K = 1;
const int EFFECT_LEVEL_KEY_Q = 2;

// What a picture shows (WorldRenderer::pictureKind).
enum PictureKind
{
    PICTURE_TEXTURE = 0,         // its texture slot with its own texture rectangle
    PICTURE_SCREEN = 1,          // the screen copy, a 2-texel border cut off
    PICTURE_SCREEN_FLIPPED = 2,  // the screen copy upside down, a 2-texel border cut off
    PICTURE_SCREEN_EXACT = 3     // the screen copy with half-texel edges
};

// How a picture is blended (WorldRenderer::pictureBlend).
enum PictureBlend
{
    PICTURE_BLEND_ALPHA = 0,     // BLEND_ALPHA
    PICTURE_BLEND_ADD = 1,       // BLEND_ADD
    PICTURE_BLEND_MULTIPLY = 2,  // BLEND_MULTIPLY
    PICTURE_BLEND_SHADOWED = 3,  // drawn twice: darkened 2 pixels down and right, then added on top
    PICTURE_BLEND_DARKEN = 4     // BLEND_DARKEN
};

struct WorldRenderer
{
    Matrix4 m_00000;
    int bgm;                                         // 0x40: background music number of the scene (BGM command)
    char characterNames[0x20][0x20];                 // 0x44: "chara%d" (in Japanese) until a character is loaded
    float scaleStep[0x80];                           // 0x444: per frame change of scaleValue
    int scaleFrames[0x80];                           // 0x644: frames left of the scale change
    char m_00844[0x80];                              // 0x844: `important` of each model record
    char m_008c4[0x80];                              // 0x8c4: m_0267 of each model record
    float levelValue[4];                             // 0x944: the four level values (ROL/RGL)
    int levelFrames[4];                              // 0x954: frames left of each level change
    float levelStep[4];                              // 0x964: per frame change of levelValue
    char m_00974;                                    // 0x974: set for costume 6
    char m_00975;                                    // 0x975: non-zero = no screen fade
    char transitionsOff;                             // 0x976: non-zero = the CFD command starts no screen transition
    char pad_00977[0x1];
    int fadeOnTop;                                   // 0x978: 1 = the fade is drawn below the cinema bars
    int effectLevel;                                 // 0x97c: 0..3, selected by GGQ (which level key bank applies)
    int lowestIdCache[MESSAGE_ID_CACHE_SIZE];       // 0x980: LowestMessageId of each key below the table size (-9999 = not worked out yet)
    int lineTagIdCache[MESSAGE_ID_CACHE_SIZE];      // 0x1920: cache of EN 0x72e6b0's per-key id result for keys below the table size (-9999 = not worked out yet)
    int highestIdCache[MESSAGE_ID_CACHE_SIZE];      // 0x28c0: HighestMessageId of each key below the table size (-9999 = not worked out yet)
    int m_03860;
    int eventsMuted;                                 // 0x3864: non-zero = script events are not sent to the host
    int frameLimit;                                  // 0x3868: number of frames of the scene (LEG), 900 by default
    FontRenderer *font;                              // 0x386c: the text renderer (captions)
    int captionVisible;                              // 0x3870: 1 = the caption is drawn
    int captionSize;                                 // 0x3874: font size in percent, less 25
    int captionX;                                    // 0x3878: left of the caption, -1 = centred
    int captionY;                                    // 0x387c: baseline of the caption
    char caption[0x100];                             // 0x3880: caption text; a literal backslash-n splits two lines, an opening bracket starts the speech
    float postStrength;                              // 0x3980: strength of the post effect (PIA)
    float postStep;                                  // 0x3984: per frame change of postStrength
    int postFrames;                                  // 0x3988: frames left of the change
    char moviePicture;                               // 0x398c: picture slot that shows the playing movie
    char pad_0398d[0x3];
    int fadeColour;                                  // 0x3990: colour of the full-screen fade
    float fadeStep;                                  // 0x3994: added to fadeAmount each frame
    int fadeFrames;                                  // 0x3998: frames left of the fade
    float fadeTarget;
    float fadeAmount;                                // 0x39a0: fade strength in percent (0 = off)
    int fadeMode;
    int lastScriptFrame;                             // 0x39a8: frame the lip text was last advanced for
    char lipNames[0x20][0x10][0x10];                 // 0x39ac: the 16 texture names of each character's lip animation
    char lipWeights[0x20][0x10];                     // 0x59ac: frames each lip texture is shown for
    char lipActive[0x20];                            // 0x5bac: 1 while the lip animation of the character runs
    char lipRepeat[0x20];                            // 0x5bcc: repeats left
    short lipDuration[0x20];                         // 0x5bec: frames of one pass
    char lipCountdown[0x20];                         // 0x5c2c: frames left of the current lip texture
    char lipIndex[0x20];                             // 0x5c4c: current lip texture
    short lipPause[0x20];                            // 0x5c6c: frames to wait before the first texture is applied
    float modelAlphaStep[0x20];                      // 0x5cac: per frame change of the model's alpha (CTR)
    float modelAlphaTarget[0x20];                    // 0x5d2c: alpha the model ends at
    int modelAlphaFrames[0x20];                      // 0x5dac: frames left of the alpha change
    char levelKeyHit;                                // 0x5e2c: set when a level key bank was sampled this call
    char pad_05e2d[0x3];
    Vec4 levelKey;                                   // 0x5e30: value sampled from the level key bank
    KeyBank levelKeys[4];                            // 0x5e40: key bank of each effect level (AFK/AFQ)
    int levelKeyStart[4];                            // 0x8850: frame the keys of each bank start at
    AnimModel *models;                               // 0x8860: the map's model records (the character table)
    char pad_08864[0x4];
    Node nodes[0x20];                                // 0x8868: map model nodes
    // Per node tweens: value, per frame step and frames left, for each of the 0x20 map nodes.
    float nodeA[0x20];                               // 0x39c68: ORA (the frame handed to the proximity pass)
    float nodeAStep[0x20];
    int nodeAFrames[0x20];
    float nodeScale[0x20];                           // 0x39de8: ORS (1.0 by default)
    float nodeScaleStep[0x20];
    int nodeScaleFrames[0x20];
    float nodeT[0x20];                               // 0x39f68: ORT (1.0 by default)
    float nodeTStep[0x20];
    int nodeTFrames[0x20];
    float nodeAngleX[0x20];                          // 0x3a0e8: ORX (radians)
    float nodeAngleXStep[0x20];
    int nodeAngleXFrames[0x20];
    float nodeAngleY[0x20];                          // 0x3a268: ORY
    float nodeAngleYStep[0x20];
    int nodeAngleYFrames[0x20];
    float nodeAngleZ[0x20];                          // 0x3a3e8: ORZ
    float nodeAngleZStep[0x20];
    int nodeAngleZFrames[0x20];
    KeyBank nodePositionKeys[0x20];                  // 0x3a568: position keys of each node (AOP)
    int nodePositionStart[0x20];                     // 0x4f5e8: frame the position keys of the node were cleared at
    char nodeFiles[0x20][0x20];                      // 0x4f668: model file loaded under each node (LDO)
    char loaded;                                     // 0x4fa68: 1 while a map is loaded
    char pad_4fa69[3];
    WorldCallback callback;                          // 0x4fa6c: host notification callback
    short messageIds[2400];                          // 0x4fa70: id of each line of the character's .itm file
    char messages[2400][0x100];                      // 0x50d30: the lines ("##nnnnn text")
    int messageCount;
    int m_e6d34[0x14];
    int m_e6d84[0x14];
    char pad_e6dd4[0x58];
    int enabled;                                     // 0xe6e2c: frame updates on
    int frame;                                       // 0xe6e30: current world frame
    int lastFrame;                                   // 0xe6e34: frame the draw lists were last built for
    int scaleMode[0x80];                             // 0xe6e38: -1 = the model's heading follows its path (SRM "PATH")
    float scaleValue[0x80];                          // 0xe7038: heading of each model in degrees (SRS)
    int pathStart[0x80];                             // 0xe7238: frame the model's path keys start at
    Vec4 previousPosition[0x80];                     // 0xe7438: position of each model at the last frame (heading follows the path)
    char faceTextureNames[0x20][0x20];               // 0xe7c38: face texture name of each model (FCC)
    char pad_e8038[0xc00];
    char costumeNames[0x80][0x20];                   // 0xe8c38: costume name of each model (VIC)
    char mapModelName[0x20];                         // 0xe9c38: map model name (LMP)
    char modelFileNames[0x60][0x20];                 // 0xe9c58: model file loaded for each model (LMU/LMT)
    char pad_ea858[0x400];
    int modelAlias[0x80];                            // 0xeac58: model that stands for the slot (-1 = itself)
    int modelPartner[0x80];                          // 0xeae58: second model that mirrors the slot (-1 = none)
    short m_eb058[0x20];
    char pad_eb098[0xc0];
    int motionStart[0x80];                           // 0xeb158: frame the motion keys of the model start at
    KeyBank motionKeys[0x80];                        // 0xeb358: motion key bank of each model
    Vec4 targets[0x80];                              // 0x13f558: target of each model's state
    char active[0x80];                               // 0x13fd58: 1 while the model's state runs
    char barsFull;                                   // 0x13fdd8: non-zero = show the cinema bars fully at once
    char pad_13fdd9[0x3];
    int textures[0x20];                              // 0x13fddc: texture slots used by the map (-1 = none)
    // The 0x20 pictures (2D images placed by the scripts, drawn by DrawPictures 0x10167440): a picture shows a
    // texture slot (`textures`) or, for a non-zero kind, the copy of the screen (`screenTexture`).
    float pictureAlpha[0x20];                        // 0x13fe5c: opacity of each picture
    float pictureAlphaStep[PICTURE_COUNT];           // 0x13fedc: per frame change of pictureAlpha
    int pictureAlphaFrames[PICTURE_COUNT];           // 0x13ff5c: frames left of the alpha change
    Vec4 pictureUVs[0x20];                           // 0x13ffdc: texture rectangle u0, v0, u1, v1 (kind 0)
    Vec4 pictureUVStep[PICTURE_COUNT];               // 0x1401dc: per frame change of pictureUVs
    int pictureUVFrames[PICTURE_COUNT];              // 0x1403dc
    float pictureAngle[0x20];                        // 0x14045c: rotation of each picture
    float pictureAngleStep[PICTURE_COUNT];           // 0x1404dc
    int pictureAngleFrames[PICTURE_COUNT];           // 0x14055c
    float pictureGrey[0x20];                         // 0x1405dc: grey level of the specular colour (0..1)
    float pictureGreyStep[PICTURE_COUNT];            // 0x14065c
    int pictureGreyFrames[PICTURE_COUNT];            // 0x1406dc
    Vec4 picturePositions[0x20];                     // 0x14075c: centre (640x480 screen, or world when in-world)
    Vec4 picturePositionStep[PICTURE_COUNT];         // 0x14095c
    int picturePositionFrames[PICTURE_COUNT];        // 0x140b5c
    Vec4 pictureSizes[0x20];                         // 0x140bdc: width (x) and height (y) on the 640x480 screen
    Vec4 pictureSizeStep[PICTURE_COUNT];             // 0x140ddc
    int pictureSizeFrames[PICTURE_COUNT];            // 0x140fdc
    char pictureInWorld[0x20];                       // 0x14105c: non-zero = placed in the world (not drawn here)
    char pictureBlend[0x20];                         // 0x14107c: PictureBlend
    char pictureKind[0x20];                          // 0x14109c: PictureKind
    int screenTexture;                               // 0x1410bc: texture slot holding the copy of the screen
    int screenWidth;                                 // 0x1410c0: width of the screen copy in texels
    int screenHeight;                                // 0x1410c4: height of the screen copy in texels
    int movieActive;                                 // 0x1410c8: 1 while a movie (kind 1) plays on a picture
    int notifyPending;                               // 0x1410cc: a 0x5e0 notification must be sent on release
    WorldRenderer();
    int Bind(AnimModel *models, void *mapRoot, int *state, WorldCallback callback, void *data);  // 0x1017c9c0
    int LoadCharacter(const char *path);                                                       // 0x10166950
    int M_10166910(int slot, int character, int value);                                        // 0x10166910 (CharacterBank::M_10166910 in model_misc.cpp)
    int StartFade(int mode, int target, int frames);
    void TickFade();
    void TickCaption();                      // 0x101687c0 (model_misc.cpp)
    int SetModelState(int index, int mode);  // 0x10166f20 (world_model_state.cpp)
    int GetMessageField(int line, int field, char *out);  // 0x101672c0 (text_fields.cpp); EN 0x733400 (not ported yet): copies the comma-separated field `field` of line `line` to `out`
    int GetMessageId(int index);             // EN 0x7334a0: id of line `index` of the character's .itm file
    int SetMessageId(int index, int id);     // EN 0x746ba0: stores the id of line `index`
    int SetMessage(int index, int id, const char *text);   // EN 0x745510: stores the id and the text of line `index`
    int FindMessageLine(int key);            // EN 0x72c8b0: line number for `key` (not recovered yet)
    // Finders and writers of the message editor (EN-only; world_message_find.cpp, world_message_highest.cpp, world_message_store.cpp).
    int FindKeyLineOffset(int key, const char *first, const char *second, const char *third);   // EN 0x72c360: position within key `key` of the first line whose first three fields equal the strings (ignoring case; null = any), -1 when none
    int FindKeyLine(int key, const char *first, const char *second, const char *third);         // EN 0x72c450: the same, as a line number of the table
    int KeyMoveCommandKind(int key);         // EN 0x735390: 1 when a line of key `key` starts with "RCM"/"RCA"/"RM2"/"RA2", 2 for "RGM"/"RGA"/"RMT"/"RAT", else 0
    int AppendKeyLinesFrom(WorldRenderer *source, int key);   // EN 0x70ac30: appends the lines of key `key` of `source` to this table and forgets every cached id and the key count; returns 0
    int SaveMessages(const char *path);      // EN 0x7420b0: writes every line as "##nnnn,text" to the file `path`; returns 0
    // Callees of HighestMessageId (provisional names: only what they return is known).
    int KeyIdOffset(int key, int variant);            // EN 0x732440 (not ported yet): id offset of key `key`: the id of its first line when it has no move commands
    int KeyCommandCount(int key, int kind);           // EN 0x731ec0 (not ported yet): how many move commands of kind `kind` (0..15) key `key` has
    int KeyCommandId(int key, int kind, int index);   // EN 0x732150 (not ported yet): id offset of the `index`-th move command of kind `kind` of key `key`
    int HighestMessageId(int key);           // EN 0x72e5d0 (not ported yet): largest id among the lines of key `key` (-10000 when it has none), cached for keys below 1000
    int SetMessageField(int line, int field, const char *text);   // EN 0x746a50 (not ported yet): replaces the comma-separated field `field` of line `line` with `text`
    int ShiftKeyIds(int key, int firstId, int shift);   // EN 0x746d80 (not ported yet): adds `shift` to the ids of key `key` that are at least `firstId`
    int SetMessageNumber(int key, int number);   // EN 0x745aa0 (world_message_edit.cpp): writes `number` as the second field of the first line of key `key`
    int ShiftIdsFrom(int firstId, int shift);    // EN 0x747cc0 (world_message_edit.cpp): ShiftKeyIds on every key whose highest id is at least `firstId`
    // Script command scanners of the message editor (EN-only). ScanLineCommand<X> classifies the script command of line
    // `line` (not recovered; -1 = not its kind). FindKeyCommand<X> returns the first line of key `key` that it accepts,
    // -1 when none (world_message_scan.cpp).
    int ScanLineCommandA(int line, int *position);   // EN 0x72ce80
    int ScanLineCommandB(int line, int *position);   // EN 0x72d3b0
    int ScanLineCommandC(int line, int *position);   // EN 0x72da60
    int ScanLineCommandD(int line, int *position);   // EN 0x72dbd0
    int ScanLineCommandE(int line, int *position);   // EN 0x72dcd0
    int ScanLineCommandF(int line, int *position);   // EN 0x72de80
    int ScanLineCommandG(int line, int *position);   // EN 0x72e030
    int ScanLineCommandH(int line, int *position);   // EN 0x72e170
    int ScanLineCommandI(int line, int *position);   // EN 0x72e270
    int ScanLineCommandJ(int line, int *position);   // EN 0x732640
    int ScanLineCommandK(int line, int *position);   // EN 0x7331b0
    int FindKeyCommandA(int key);   // EN 0x72e3a0
    int FindKeyCommandB(int key);   // EN 0x72e3f0
    int FindKeyCommandC(int key);   // EN 0x72e440
    int FindKeyCommandD(int key);   // EN 0x72e490
    int FindKeyCommandE(int key);   // EN 0x72e4e0
    int FindKeyCommandF(int key);   // EN 0x72e530
    int FindKeyCommandG(int key);   // EN 0x72e580
    int FindKeyCommandH(int key);   // EN 0x72e7e0
    int FindKeyCommandI(int key);   // EN 0x72e870
    int FindKeyCommandJ(int key);   // EN 0x72e8f0
    int FindKeyCommandK(int key);   // EN 0x72e940
    int FindKeyCommandAWithPosition(int key);   // EN 0x72e340 (EN-only, world_message_scan.cpp): the first line of key `key` for which ScanLineCommandA finds both a result and a position (-1 when none)
    int KeyHasRlkLine(int key);                 // EN 0x735470 (EN-only, world_message_scan.cpp): 1 when a line of key `key` starts with "RLK"
    int KeyHasRopLine(int key);                 // EN 0x735700 (EN-only, world_message_scan.cpp): 1 when a line of key `key` starts with "ROP"
    int ShiftKeyIdsClamped(int key, int shift);   // EN 0x73e270 (EN-only, world_message_edit.cpp): forgets the cached ids of key `key` and adds `shift` to the ids of its lines, less when that would take an id below 0
    long GetKeyZimNumber(int key);              // EN 0x72ea40 (EN-only, world_message_edit.cpp): the number in the second field of the first "ZIM" line of key `key` that is not -1, -1 when there is none
    int EffectLevelKeyKind(int key);           // EN 0x7355d0 (EN-only, world_message_keys.cpp): EFFECT_LEVEL_KEY_K when a line of key `key` starts with "RFK"/"AFK", EFFECT_LEVEL_KEY_Q for "RFQ"/"AFQ", else 0
    int KeyHasRpkLine(int key);                // EN 0x7354e0 (EN-only, world_message_keys.cpp): 1 when a line of key `key` starts with "RPK" or "RP2"
    int ApplyKeyOffsetsA(int key, int firstId, float x, float y, float z);   // EN 0x747010 (not ported yet): edits the movement commands of key `key` (three floats)
    int ApplyKeyOffsetsB(int key, int firstId, float a, float b, float c, float d);   // EN 0x7472b0 (not ported yet): the same with the four-value form
    int OffsetKeysFromA(int firstId, float x, float y, float z);   // EN 0x73f770 (EN-only, world_message_keys.cpp): ApplyKeyOffsetsA on every key whose highest id is at least `firstId`
    int OffsetKeysFromB(int firstId, float a, float b, float c, float d);   // EN 0x742020 (EN-only, world_message_keys.cpp): ApplyKeyOffsetsB on those keys
    int SetKeyCommandBField(int key, int value);   // EN 0x71ad50 (EN-only, world_message_keys.cpp): writes `value` into the field ScanLineCommandB found, on every line of key `key` that has one
    int SetKeyCommandAField(int key, int value);   // EN 0x71acd0 (EN-only, world_message_keys.cpp): the same for ScanLineCommandA
    int CountMessageKeys();                  // EN 0x731db0 (not ported yet): number of keys in the message lines, cached in the table (-9999 = not counted yet)
    int InsertMessageLine(int position);     // EN 0x73ef10 (not ported yet): inserts an empty line (id -1) before line `position` and counts it; returns 1
    int RemoveMessageLine(int line);         // EN 0x71c9e0 (world_message_lines.cpp, EN-only): takes line `line` out of the table, moves the later lines up and counts it; returns 0
    int LowestMessageId(int key);            // EN 0x72e990 (world_message_lines.cpp): smallest id among the lines of key `key` (10000 when it has none), cached for keys below 1000
    int DeleteMessageKey(int key);           // EN 0x71ca80 (not ported yet): removes the lines of key `key` and moves the later ones up
    int GetMessageNumber(int key);           // EN 0x72e830 (EN-only, world_message_edit.cpp): second field of the first line of key `key` as a number
    int AddMessageLine(int key);             // EN 0x73efb0 (EN-only, world_message_edit.cpp): adds an empty line at the end of key `key`'s lines; returns the number of lines the key had
    int SetKeyedMessage(int key, int offset, int id, const char *text);   // EN 0x745a50 (EN-only, world_message_edit.cpp): stores id and text of line `offset` of key `key`
    int DeleteMessageKeysWithIds(int lowest, int highest);   // EN 0x71cc00 (EN-only, world_message_edit.cpp): deletes every key whose lowest id lies in lowest..highest
    int GetMessageIdOfKey(int key);          // EN 0x72ea20: id of the line FindMessageLine(key) finds
    int CopyKeyLinesTo(WorldRenderer *target, int key);   // EN 0x71bfa0 (EN-only, world_message_copy.cpp): replaces target's message lines with the lines of key `key` (up to the next key); returns 0
    int RunScriptFrames(int silent);        // EN 0x70c120 (JP 0x1017cc90) (world_script.cpp): runs the scene script up to the current frame and animates the world
    int AdvanceFrame();
    int Release();
    int Seek(int target);                    // 0x10189000
    void DrawCaption();                      // 0x101687e0 (world_caption.cpp)
    void DrawScreenOverlays();               // 0x10184160 (world_caption.cpp)
    void DrawOverlay();
    void DrawPictures(const Matrix4 &view);  // 0x10167440 (world_pictures.cpp)
    void DrawWorldPictures(const Matrix4 *view, const Matrix4 *camera);  // 0x10167d90 (world_in_pictures.cpp)
};

// Frame number of the world, published for the renderers (EN 0x01664430 .. 0x0166443c; JP DAT_1101de08..14).
// Number of world frame slots (the worlds the renderers can show at once).
const int WORLD_FRAME_SLOTS = 4;
extern "C" int g_worldFrame[WORLD_FRAME_SLOTS];   // EN 0x01664430..0x0166443c (JP DAT_1101de08..14)
// Two key banks per world slot (positions over the frames of a scripted move); sampled at the world frame. EN 0x01ee4e60 and 0x01da54a0.
extern "C" KeyBank g_frameKeysA[WORLD_FRAME_SLOTS];
extern "C" KeyBank g_frameKeysB[WORLD_FRAME_SLOTS];
// Stores in `*first` and `*second` the values of the two key banks of world slot `index` at that slot's world frame.
// Returns 0. EN 0x731b80 (world_frame_keys.cpp).
int World_SampleFrameKeys(int index, Vec4 *first, Vec4 *second);

extern "C" int g_scriptedCameraSlot;   // EN 0x01664670 (JP DAT_1101de34): scripted camera slot in use, reset when a scene starts
extern "C" float g_lightScale;         // EN 0x00908b00 (JP DAT_10203420): strength of the scene light, 1.0 when a scene starts

extern "C" WorldRenderer g_world;  // the world (EN 0x013390f8, JP DAT_10c68000)

#endif
