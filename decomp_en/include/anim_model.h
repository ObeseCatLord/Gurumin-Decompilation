// An animated model of the EN build (JP: decomp/include/anim_model.h, 0x249c bytes there). Only the members EN code
// has used so far are named; offsets are proven by matched EN functions. This is the one declaration of AnimModel and
// MotionChannels in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_ANIM_MODEL_H
#define GURUMIN_EN_ANIM_MODEL_H

#include "key_bank.h"
#include "vec4.h"

struct Node;

// Layers of a Playback (layer numbers above PLAYBACK_LAYER_COUNT - 1 are rejected).
const int PLAYBACK_LAYER_COUNT = 0x90;
// Motion slots per layer.
const int PLAYBACK_SLOT_COUNT = 16;
// Bytes of a Playback block.
const int PLAYBACK_SIZE = 0x7c84;
// Playback::endState value when the playback has reached its end.
const int PLAYBACK_END_REACHED = 1;
// Bytes of an AnimModel.
const int ANIM_MODEL_SIZE = 0x249c;

// Angles in radians as the game writes them (not the exact values of pi).
const float ANGLE_FULL_TURN = 6.283f;
const float ANGLE_HALF_TURN = 3.1415f;
// Height above the model's position that markers and balloons are drawn at.
const float ANIM_MARKER_HEIGHT = 50.0f;
// The 640x480 screen the 2D layer is laid out on.
const float ANIM_SCREEN_WIDTH = 640.0f;
const float ANIM_SCREEN_HEIGHT = 480.0f;
// Scales of the motion speed changes (AddMotionSpeed, PushMotionSpeed).
const double MOTION_SPEED_SCALE = 0.961875;
const float MOTION_SPEED_DAMPING = 0.9f;
const float PUSH_SPEED_SCALE = 0.2f;
// Blend frames a model gets when it has none, and the count below which a push still adds speed.
const int DEFAULT_BLEND_FRAMES = 8;
const int PUSH_BLEND_LIMIT = 16;
// Highest vertical speed a model can get.
const float MAX_VERTICAL_SPEED = 60.0f;
// Limit of each axis of MoveTowards' step.
const float MOVE_STEP_LIMIT = 40.0f;
// Factors of Push: while a speed change is pending, when the model is nearly at rest, and otherwise.
const float PUSH_MOVING_FACTOR = 0.7f;
const float PUSH_AT_REST_FACTOR = 1.4f;
const float PUSH_IDLE_FACTOR = 0.55f;
// Snapping works in tenths of a unit; four candidate slots (three used).
const float SNAP_UNITS_PER_WORLD_UNIT = 10.0f;
const int SNAP_CANDIDATE_COUNT = 4;
// Highest hold weight.
const float HOLD_WEIGHT_MAX = 0.9f;

// Entries of the model file table (one root node per loaded model file).
const int MODEL_FILE_ENTRIES = 0x100;
// Models and pushes one collision test can report.
const int MODEL_TOUCH_LIMIT = 0x40;
// CollideModel test mode: any model collision (-1).
const int COLLIDE_ANY_MODE = -1;
// CollideProbesWithMesh masks: the map collision kind, and every kind.
const unsigned int MAP_COLLISION_KIND = 0x4000000;
const unsigned int MAP_COLLISION_ANY = 0xffffffff;

// Root node of each loaded model file (0 = free entry). EN 0x01663970 (JP DAT_10faaa88).
extern "C" Node *g_modelFileRoots[MODEL_FILE_ENTRIES];
// World matrix of each model file entry (the AnimModel::matrix of its model). EN 0x01663d70 (JP DAT_10ff19e8).
extern "C" Matrix4 *g_modelFileMatrices[MODEL_FILE_ENTRIES];
// Collision filter of the model-against-world test that runs now (a model's collisionFilter). EN 0x0217fb00 (JP DAT_11b002c0).
extern "C" int g_collisionFilter;
// 1 while a model-against-world test runs. EN 0x0217fafe (JP DAT_11b002c8).
extern "C" char g_modelCollisionActive;

// Values of AnimModel::holdState.
enum HoldState
{
    HOLD_FREE = 0,      // not holding
    HOLD_HOLDING = 1,   // holding on to a model
    HOLD_RELEASED = 2   // let go, the weight fades out
};

// First frame of an empty channel: later than any real key, so the first key added becomes the start.
const int CHANNEL_EMPTY_FIRST_FRAME = 10000;

// One key-framed motion channel of a model: a key bank and its playback range.
struct MotionChannels
{
    KeyBank keys;     // 0x000: the keys
    int last;         // 0xa84: last frame (exclusive for track C)
    int first;        // 0xa88: first frame (CHANNEL_EMPTY_FIRST_FRAME when empty)
    int frame;        // 0xa8c: current frame while playing
    int playing;      // 0xa90: 1 while playing
    int looping;      // 0xa94: 1 = loop back to `first` at the end
};

struct AnimModel;

// The motion playback state of a model (0x7c84 bytes): layers of frame ranges and per-slot offsets. Allocated on
// first use through AnimModel::GetPlayback. Only the members EN code has used are named.
struct Playback
{
    int headerFlag;                    // 0x0000: first word of the state (cleared when the player gets a helmet)
    char m_0004[0x1c - 4];             // 0x0004: not yet named
    int layer;                         // 0x001c: the layer that plays now
    char m_0020[0x2c - 0x20];          // 0x0020: not yet named
    void *slotSource[16];              // 0x002c: motion source of each slot
    int slotLayer[16];                 // 0x006c: layer each slot plays
    char m_00ac[0xbec - 0xac];         // 0x00ac: not yet named
    int endState;                      // 0x0bec: PLAYBACK_END_REACHED once the playback has reached its end
    char m_0bf0[0x1070 - 0xbf0];       // 0x0bf0: not yet named
    int slotOffsetX[PLAYBACK_LAYER_COUNT][PLAYBACK_SLOT_COUNT];  // 0x1070: x offset per layer and slot
    int slotOffsetY[PLAYBACK_LAYER_COUNT][PLAYBACK_SLOT_COUNT];  // 0x3470: y offset per layer and slot
    char m_5870[0x7c74 - 0x5870];      // 0x5870: not yet named
    int blending;                      // 0x7c74: non-zero while blending into nextLayer
    int m_7c78;                        // 0x7c78: not yet named
    int nextLayer;                     // 0x7c7c: the layer being blended into
    char m_7c80[PLAYBACK_SIZE - 0x7c80];  // 0x7c80: not yet named

    void Init();                                                       // EN 0x7412b0 (JP 0x10169f90)
    int BindModel(AnimModel *owner);                                   // EN 0x71c060 (JP 0x1016a020)
    int SetLayer(int layer, int a, int first, int last, int b, int c); // EN 0x7437e0 (JP 0x1016a100)
    int SetSlotOffset(int layer, int slot, int x, int y);              // EN 0x746490 (JP 0x1016a260)
    int Play(int layer, int a, int b, float speed);                    // EN 0x71add0 (JP 0x1016a2e0): starts a motion on a layer
};

struct AnimModel
{
    char m_0000[0x70];            // 0x000: not yet named
    int collisionFilter;          // 0x070: collision filter of the model's world tests (g_collisionFilter while they run)
    char m_0074[0x76 - 0x74];     // 0x074: not yet named
    char uiAnimStarted;           // 0x076: set to 1 by UiAnim_Play (read on the first model only)
    char m_0077[0x84 - 0x77];     // 0x077: not yet named
    unsigned char speedDivisor;   // 0x084: slows every motion of this model down (0 = no division)
    char m_0085[0x21c - 0x85];    // 0x085: not yet named
    Vec4 scale;                   // 0x21c: model scale (x, y, z)
    char m_022c[0x248 - 0x22c];   // 0x22c: not yet named
    float holdWeight;             // 0x248: blend weight while holding (at most HOLD_WEIGHT_MAX)
    char m_024c[0x260 - 0x24c];   // 0x24c: not yet named
    float uniformScale;           // 0x260: scale of the whole model (1 for bank models)
    char m_0264;                  // 0x264: not yet named
    char important;               // 0x265: set when a character is assigned to a slot (CharacterBank::Assign)
    char m_0266;                  // 0x266: not yet named
    char idling;                  // 0x267: 1 while an NPC idles; set every frame by the kind updates
    char m_0268[0x270 - 0x268];   // 0x268: not yet named
    union
    {
        int switchNode;           // 0x270: node id of the switch this character works (switch characters only)
        int bankIndex;            // 0x270: a model-bank record's own index in the bank (bank models only)
    };
    char m_0274[0x294 - 0x274];   // 0x274: not yet named
    int walkingRoute;             // 0x294: 1 while the character walks a route (cleared when the player gets a helmet)
    Playback *playback;           // 0x298: the motion playback state (allocated on first use)
    float speedScale;             // 0x29c: motion speed scale
    char m_02a0[3];               // 0x2a0: not yet named
    unsigned char loadCount;      // 0x2a3: number of model files loaded into this model so far (stops counting at 3)
    char holdState;               // 0x2a4: HoldState
    char m_02a5[0x2ac - 0x2a5];   // 0x2a5: not yet named
    MotionChannels motion;        // 0x2ac: the motion channel (track A)
    char m_0d44[8];               // 0xd44: not yet named
    float alpha;                  // 0xd4c: model alpha (1 = fully visible; set when an enemy is respawned)
    MotionChannels trackB;        // 0xd50: track B
    char m_17e8;                  // 0x17e8: not yet named
    char locked;                  // 0x17e9: non-zero while the model ignores queued pushes
    char pad_17ea[2];
    MotionChannels trackC;        // 0x17ec: track C
    int m_2284;                   // 0x2284: not yet named
    char pushFlag;                // 0x2288: flag of the queued push
    char pad_2289[3];
    Vec4 pushVector;              // 0x228c: queued push (x, y, z)
    int blendFrames;              // 0x229c: frames a motion change blends over (0 = unset, DEFAULT_BLEND_FRAMES is used)
    char m_22a0[0x22ec - 0x22a0]; // 0x22a0: not yet named
    Node *root;                   // 0x22ec: the model's node tree (0 = free entry of the model bank)
    Node *loadedRoot;             // 0x22f0: the node tree the model file loaded (a bank model's root)
    Node *headNode;               // 0x22f4: the model's head node (0 = none)
    char m_22f8[0x2300 - 0x22f8]; // 0x22f8: not yet named
    Node *swapNode1;              // 0x2300: first node of the material swap
    Node *swapNode2;              // 0x2304: second node of the material swap
    Node *collider;               // 0x2308: the map collision this model moves against (0 = none)
    char m_230c[0x2318 - 0x230c]; // 0x230c: not yet named
    float colliderReach;          // 0x2318: reach of the model's probes against the collider
    Node *dropCollider;           // 0x231c: collision node the model drops onto (set by SetDropCollider)
    char m_2320[0x2328 - 0x2320]; // 0x2320: not yet named
    Matrix4 matrix;               // 0x2328: world matrix; the translation row (m[12..14]) is the position
    int motionFrame;              // 0x2368: frame of the motion file shown now
    float verticalSpeed;          // 0x236c: vertical speed (positive = rising)
    char m_2370[0x2384 - 0x2370]; // 0x2370: not yet named
    int collisionResult;          // 0x2384: collision result, cleared before each update (UpdateCollision)
    int speedChangeFrames;        // 0x2388: frames since the speed change (1 once one is pending)
    int idleMotionEnabled;        // 0x238c: non-zero = the model plays its idle motion now and then (Char_RandomIdle)
    int collisionLimit;           // 0x2390: limit of the map collision tests (99999 = none)
    char m_2394[0x2398 - 0x2394]; // 0x2394: not yet named
    Vec4 holdOffset;              // 0x2398: position relative to the held model
    Vec4 move;                    // 0x23a8: movement impulse applied by the next update (Push)
    Vec4 velocity;                // 0x23b8: current movement
    char m_23c8[0x23d8 - 0x23c8]; // 0x23c8: not yet named
    Vec4 moveStep;                // 0x23d8: this frame's movement: x, y scaled by speedScale, z the vertical step
    char m_23e8[0x247c - 0x23e8]; // 0x23e8: not yet named
    float heading;                // 0x247c: angle the character faces now
    float targetHeading;         // 0x2480: angle the character turns towards
    char m_2484[0x2494 - 0x2484]; // 0x2484: not yet named
    float eventCameraHeight;      // 0x2494: height the event camera adds to its eye and target (cleared by LoadModel)
    int mode;                     // 0x2498: behaviour mode (1 = the walking mode that slows movement impulses)

    int SetTrackCOptions(int playing, int looping);                      // EN 0x743960 (JP 0x10161b40)
    int SetMotionOptions(int playing, int looping);                      // EN 0x745c00 (JP 0x10161be0)
    int SetTrackBOptions(int playing, int looping);                      // EN 0x7469b0 (JP 0x10161f60)
    int AddTrackCKey(int frame, float x, float y, float z);              // EN 0x70ad20 (JP 0x10161af0)
    int AddMotionKey(int frame, float value);                            // EN 0x70af70 (JP 0x10161b90)
    int SetPendingPush(float x, float y, float z, char flag);            // EN 0x73f940 (JP 0x101628b0)
    int ClearTrackC();                                                   // EN 0x741bd0 (JP 0x10161a60)
    int ClearMotionKeys();                                               // EN 0x741c10 (JP 0x10161b60)
    int ClearTrackB();                                                   // EN 0x741fe0 (JP 0x10161f30)
    int ApplyMaterialSwap(int count);                                    // EN 0x71ac90 (JP 0x10161a20)
    int SnapHeadingToSide();                                             // EN 0x70b730 (JP 0x10162d50)
    int FaceAway(const Vec4 &point);                                     // EN 0x739110 (JP 0x10162ea0)
    int FaceAwayFrom(AnimModel *other);                                  // EN 0x7391a0 (JP 0x10162e00)
    int ScreenPosition(int *x, int *y, const Matrix4 &viewProjection);   // EN 0x733100 (JP 0x10162f50)
    int PushMotionSpeed(float speed);                                    // EN 0x73f7f0 (JP 0x10163000)
    int MoveTowards(float x, float y, float z);                          // EN 0x73fac0 (JP 0x10162800)
    int Push(float x, float y, float z);                                 // EN 0x73f9d0 (JP 0x101628f0)
    int SnapHeight(float step, float offset);                            // EN 0x70bb00 (JP 0x10161e40)
    int SnapPosition(float step, float offset);                          // EN 0x70b980 (JP 0x10161cb0)
    int Hold(int on, float amount, AnimModel *other);                    // EN 0x7467c0 (JP 0x10163d60)
    int HoldAtSpine(int on, float amount);                               // EN 0x7468c0 (JP 0x10163c70)
    int SetDropCollider(Node *node);                                     // EN 0x746bc0 (JP 0x10163e70)
    int IsClear(const Vec4 *offset);                                     // EN 0x71a790 (JP 0x10163e90)
    int IsClearFlagged(const Vec4 *offset);                              // EN 0x71a880 (JP 0x10163f90)
    int IsClearAny(const Vec4 *offset);                                  // EN 0x71a980 (JP 0x101640b0)
    int IsClearOfMap(const Vec4 *offset);                                // EN 0x71aa70 (JP 0x101642c0)
    int IsClearOfMapLimited(const Vec4 *offset);                         // EN 0x71ab70 (JP 0x101641b0)
    int SetMoveStep(float x, float y, float z);                          // EN 0x73f990 (JP model_params.cpp)
    int IsPlaybackAtEnd();                                               // EN 0x7356b0 (JP 0x1017b160)
    int LoadModel(const char *path);                                     // EN 0x738f80 (JP 0x1017aeb0)
    int AddMotionSpeed(float speed);                                     // EN 0x73f8a0 (JP 0x101630b0)
    int Teleport(float x, float y, float z);                             // EN 0x745f30 (JP 0x10162420): puts the model at (x, y, z)
    int SetPosition(float x, float y, float z);                          // EN 0x745c60 (JP 0x10162610)
    void PlayMotion(int layer, int a, int b, float speed);               // EN 0x71af60 (JP 0x1017b0e0)
    int SetSlotSource(void *source, int slot, int layer);                // EN 0x7454a0 (JP Playback::SetSlotSource through the model): creates the playback on first use
    int GetPlaybackLayer();                                              // EN 0x72ce30 (JP 0x1017b1c0): the layer that plays now
    int UpdateCollision();                                               // EN 0x741b40 (JP 0x10162390): collides the model's position with the map
    int SetLayer(int layer, int a, int first, int last, int b, int c);   // EN 0x7438f0: Playback::SetLayer on the playback
};

static_assert(sizeof(AnimModel) == ANIM_MODEL_SIZE, "AnimModel must be 0x249c bytes");

#endif
