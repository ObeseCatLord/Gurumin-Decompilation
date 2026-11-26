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
// Layer number meaning "none" (Playback::layer, slot layers).
const int PLAYBACK_NO_LAYER = -1;
// A node animation needs more than this many frames to be played.
const int PLAYBACK_MIN_FRAMES = 8;
// Follow-up layers a layer lists in Playback::layerParams (the first entries; the rest stay unused).
const int PLAYBACK_FOLLOWUP_COUNT = 8;
// Marks an empty follow-up entry.
const int PLAYBACK_NO_FOLLOWUP = -2;
// Motion slots per layer.
const int PLAYBACK_SLOT_COUNT = 16;
// Bytes of a Playback block.
const int PLAYBACK_SIZE = 0x7c84;
// Playback::endState value when the playback has reached its end.
const int PLAYBACK_END_REACHED = 1;
// endState while the playback holds its last frame (Play ignores a request for the layer it came from).
const int PLAYBACK_END_HELD = 2;
// The pseudo layer a playback shows while it blends into another layer.
const int PLAYBACK_BLEND_LAYER = 0x90;
// Most frames a blend between two layers is stretched over, minus one.
const int PLAYBACK_BLEND_MAX = 14;
// Blends between layers started so far (counted by Playback::Play). EN 0x0162ef5c (JP DAT_110175f8).
extern "C" int g_motionBlendCount;
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
// AnimModel::shadowOpacity of a model that has just been placed.
const unsigned char SHADOW_OPACITY_PLACED = 15;
// Highest hold weight.
const float HOLD_WEIGHT_MAX = 0.9f;

// Values loading, cloning and resetting a model give its fields.
const int M_025C_RANGE = 500;                    // AnimModel::m_025c is a random number below this
const int FRESH_M_2380 = 40;                     // AnimModel::m_2380 of a model that was just loaded
const float MODEL_REACH_SCALE = 0.578f;          // scale of a node's bounding radius to the reach of the model (colliderReach and the wobble vectors)
const float WOBBLE_DIRECTION_SCALE = 0.5f;       // wobble direction vectors: share of the reach
const float WOBBLE_OFFSET_SCALE = 0.3f;          // random wobble offsets: share of the reach
const float WOBBLE_WEIGHT_SCALE = 0.15f;         // weight carried by the wobble axis vectors: share of the reach
const float WOBBLE_LIFT = 50.0f;                 // height added to every wobble direction
const int WOBBLE_OFFSET_ENTRIES = 10;            // entries of AnimModel::m_00b4 a model file load fills in
const int OFFSET_RANDOM_RANGE = 100;             // random wobble offsets are drawn from 0..99 ...
const int OFFSET_RANDOM_HALF = 50;               // ... minus this
// Node::flags of the node a model file's tree is loaded as (the animated root) and of its collision node.
const unsigned int MODEL_FLAG_LOADED_ROOT = 0x80;
const unsigned int MODEL_FLAG_COLLIDER = 0x100;
// Colour value and node mask CloneModel and LoadFile give Node::SetColourA (every node, full colour).
const unsigned char NODE_COLOUR_OPAQUE = 0xff;
const unsigned int NODE_ALL_FLAGS = 0xffffffff;
// Node::SetRenderModeExcept arguments of a model file load while g_refreshRenderMode is set: back to the default render
// mode, except the nodes in the default mode or in mode 0x1f.
const int RENDER_MODE_DEFAULT = 0;
const int MODEL_LOAD_RENDER_MODE_EXCEPT = 0x1f;

// AnimModel::Draw: `flags` values (any other value is a pointer to the matrix of a mirror the model is reflected in).
const int DRAW_NORMAL = 0;                 // the plain draw
const int DRAW_OVERHEAD = 1;               // flipped about z below the floor (the overhead pass)
// shadowOpacity of a fully opaque model: Draw divides the opacity by it.
const float SHADOW_OPACITY_FULL = 15.0f;
// Root drawMode that turns the rainbow wobble on, and the entries of the triangle ramp it steps through.
const int WOBBLE_DRAW_MODE = 0x24;
const int WOBBLE_RAMP_LENGTH = 30;
const float WOBBLE_RAMP_PEAK = 9.0f;       // highest ramp value: dividing by it scales the ramp to 0..1
const int WOBBLE_COLOUR_COUNT = 10;        // colours of AnimModel::m_00b4 copied to g_wobbleColours
// Phase shift of the three channels of the first wobbling colour and of each following one.
const unsigned int WOBBLE_PHASE_X = 0;
const unsigned int WOBBLE_PHASE_Y = 10;
const unsigned int WOBBLE_PHASE_Z = 20;
const unsigned int WOBBLE_NEXT_COLOUR = 6;
// Matrix4::m index of the z scale (_33) that flips a model about z, and the factors that shift the flipped model:
// the overhead pass doubles the shadow matrix's z, a mirror's frame moves it by -2 times.
const int MATRIX_SCALE_Z = 10;
const float OVERHEAD_FLIP_SCALE = 2.0f;
const float MIRROR_SHADOW_SCALE = -2.0f;
const float MIRROR_OFFSET_SCALE = -2.0f;   // how far the mirrored model moves from the foot of its perpendicular

// Non-zero = models whose root has drawMode WOBBLE_DRAW_MODE are drawn with the rainbow wobble. EN 0x021a34a8 (JP DAT_10f6558c).
extern "C" int g_wobbleOn;
// The model's m_024c (all ones while the wobble runs), handed to the renderer. EN 0x021ffe58 (JP DAT_10f65888).
extern "C" Vec4 g_wobbleAmbient;
// Copy of the model's m_00b4 colours; the wobble overwrites x/y/z of the odd ones. EN 0x021ffd98 (JP DAT_10f655c8).
extern "C" Vec4 g_wobbleColours[WOBBLE_COLOUR_COUNT];

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
// Clears the collision flag (Node::hasCollision) of every loaded model file's root node. Returns 0. EN 0x734340.
extern "C" int ModelFiles_ClearCollision(void);
// Matrix reset to the identity every time a model is placed. EN 0x01ee48b0 (JP DAT_118e69a8).
extern "C" Matrix4 g_placementMatrix;
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

// What a motion slot points at: a source that is in use while `busy` is non-zero (JP MotionSource).
struct MotionSource
{
    int busy;   // 0x00: non-zero while the source is in use
};

// The motion playback state of a model (0x7c84 bytes): layers of frame ranges and per-slot offsets. Allocated on
// first use through AnimModel::GetPlayback. Only the members EN code has used are named.
struct Playback
{
    int headerFlag;                    // 0x0000: first word of the state (cleared when the player gets a helmet)
    float framePosition;               // 0x0004: frame position (-1 while blending)
    float speed;                       // 0x0008: playback speed (1 = normal)
    unsigned char m_000c;              // 0x000c: not yet named
    char pad_000d[3];                  // 0x000d
    int m_0010;                        // 0x0010: not yet named
    int m_0014;                        // 0x0014: not yet named
    int m_0018;                        // 0x0018: not yet named
    int layer;                         // 0x001c: the layer that plays now
    int frame;                         // 0x0020: current motion frame
    int m_0024;                        // 0x0024: not yet named
    int m_0028;                        // 0x0028: not yet named
    MotionSource *slotSource[16];      // 0x002c: motion source of each slot
    int slotLayer[16];                 // 0x006c: layer each slot plays
    int layerActive[PLAYBACK_LAYER_COUNT];   // 0x00ac: 1 while the layer is active
    int m_02ec[PLAYBACK_LAYER_COUNT];        // 0x02ec: not yet named
    int layerLast[PLAYBACK_LAYER_COUNT];     // 0x052c: last frame of each layer
    int m_076c[PLAYBACK_LAYER_COUNT];        // 0x076c: not yet named
    int m_09ac[PLAYBACK_LAYER_COUNT];        // 0x09ac: not yet named
    int endState;                      // 0x0bec: PLAYBACK_END_REACHED once the playback has reached its end
    int layerDirection[PLAYBACK_LAYER_COUNT];   // 0x0bf0: 1 when the layer plays forwards (first <= last), -1 backwards
    int m_0e30[PLAYBACK_LAYER_COUNT];           // 0x0e30: fifth SetLayer parameter of each layer
    int slotOffsetX[PLAYBACK_LAYER_COUNT][PLAYBACK_SLOT_COUNT];  // 0x1070: x offset per layer and slot
    int slotOffsetY[PLAYBACK_LAYER_COUNT][PLAYBACK_SLOT_COUNT];  // 0x3470: y offset per layer and slot
    int layerParams[PLAYBACK_LAYER_COUNT][PLAYBACK_SLOT_COUNT];   // 0x5870: per layer 16 follow-up layers (-2 = none)
    int frameCount;                    // 0x7c70: frame count of the bound node (0 when it has 8 frames or fewer)
    int blending;                      // 0x7c74: non-zero while blending into nextLayer
    int m_7c78;                        // 0x7c78: not yet named
    int nextLayer;                     // 0x7c7c: the layer being blended into
    Node *boundNode;                   // 0x7c80: the node tree this playback animates (BindNode)

    void Init();                                                       // EN 0x7412b0 (JP 0x10169f90)
    int BindNode(Node *node);                                          // EN 0x71c0e0 (JP 0x1016a090): binds to a node tree and resets
    int BindModel(AnimModel *owner);                                   // EN 0x71c060 (JP 0x1016a020)
    int SetLayer(int layer, int a, int first, int last, int b, int c); // EN 0x7437e0 (JP 0x1016a100)
    int SetSlotOffset(int layer, int slot, int x, int y);              // EN 0x746490 (JP 0x1016a260)
    int Play(int layer, int a, int b, float speed);                    // EN 0x71add0 (JP 0x1016a2e0): starts a motion on a layer
    int SetSlotSource(void *source, int slot, int layer);              // EN 0x745460: sets the motion source and layer of slot `slot`; 0 when a value is out of range
    int SetFollowUp(int layer, int index, int value);                  // EN 0x745550: sets follow-up entry `index` of layer `layer`; 0 when out of range
    int Advance();                                                     // EN 0x73eff0 (JP 0x1016a480): steps the playback one frame (model_state.cpp)
};

// Non-zero while the frame code suspends random idle motions (the helmet regeneration pause). EN 0x0162ef60 (JP DAT_1101d87c).
extern "C" char g_idleMotionsSuspended;

struct AnimModel
{
    int objectId;                 // 0x000: id the scripts give to find this model among the objects (FindObjectByModelId)
    char m_0004[0x12 - 4];        // 0x004: not yet named
    char m_0012;                // 0x012: set to 2 every frame by the hovering characters
    char m_0013;                  // 0x013: cleared when the model is loaded or cloned
    char m_0014[0x68 - 0x14];     // 0x014: not yet named
    float sortBias;               // 0x068: added to the z position when objects are depth sorted (-1000 on objects set up by a map trigger in the JP build)
    char m_006c;                  // 0x06c: cleared on reset
    char m_006d[0x70 - 0x6d];     // 0x06d: not yet named
    union
    {
        int collisionFilter;      // 0x070: collision filter of the model's world tests (g_collisionFilter while they run)
        char *nearCells;          // 0x070: characters: row of g_charNearCells (map cells near the character), 0 = no visibility data
    };
    char m_0074;                  // 0x074: cleared when the model is loaded or reset
    char carriedFlag;             // 0x075: cleared on the carried model and on the player when a knock-back drops the carried object
    char uiAnimStarted;           // 0x076: set to 1 by UiAnim_Play (read on the first model only)
    unsigned char m_0077;         // 0x077: cleared on reset (JP: the model variant the player costume loads)
    int m_0078;                   // 0x078: cleared on reset (JP: frames the left foot has been planted)
    int m_007c;                   // 0x07c: cleared on reset (JP: frames the right foot has been planted)
    int m_0080;                   // 0x080: cleared on reset and on load
    unsigned char speedDivisor;   // 0x084: slows every motion of this model down (0 = no division)
    char m_0085[0x88 - 0x85];       // 0x85: not yet named
    int m_0088;                   // 0x088: cleared when a model file is loaded or cloned (JP: spin angle of the drawn model)
    unsigned char m_008c;         // 0x08c: cleared on reset and on load
    char childRunning;              // 0x8d: set while a child runs to the player (Npc_ChildFollow, Enemy_DropIn)
    char m_008e[0x90 - 0x8e];       // 0x8e: not yet named
    Vec4 m_0090;                  // 0x090: cleared (Fill) when a model file is loaded or cloned
    char m_00a0[0xb0 - 0xa0];       // 0xa0: not yet named
    float placeHeight;       // 0xb0: height the model was placed at (SetPosition, Teleport)
    Vec4 m_00b4[12];         // 0x0b4: Draw's wobble colour offsets: loading a model fills the even entries 0..8 with random directions scaled by the model size and the odd ones with unit axes carrying a weight in w (entries 10 and 11 stay unused)
    Vec4 m_0174[6];          // 0x174: Draw's wobble directions: five pointing around the model, scaled by the model size (the sixth is unused by Draw)
    int m_01d4;              // 0x1d4: cleared when the model is placed
    char m_01d8[0x21c - 0x1d8];       // 0x1d8: not yet named
    Vec4 scale;                   // 0x21c: model scale (x, y, z)
    Vec4 m_022c;                  // 0x22c: colour offset handed to Node::QueueDraw (cleared on reset, load and clone)
    int m_023c;              // 0x23c: cleared when the model is placed
    char m_0240;                  // 0x240: cleared on reset, load and clone
    char m_0241[0x244 - 0x241];   // 0x241: not yet named
    Node *spineNode;              // 0x244: the "Bip01 Spine" bone (the loaded root when the model has none)
    float holdWeight;             // 0x248: blend weight while holding (at most HOLD_WEIGHT_MAX)
    Vec4 m_024c;                  // 0x24c: ambient colour Draw hands to the renderer (cleared to 1 on reset)
    int m_025c;                   // 0x25c: random phase 0..M_025C_RANGE-1 picked when a model file is loaded (Draw's wobble)
    float uniformScale;           // 0x260: scale of the whole model (1 for bank models)
    char m_0264;                  // 0x264: not yet named
    char important;               // 0x265: set when a character is assigned to a slot (CharacterBank::Assign)
    char m_0266;                  // 0x266: not yet named
    char idling;                  // 0x267: 1 while an NPC idles; set every frame by the kind updates
    Vec4 *pullTarget;             // 0x268: position the model is drawn towards while pullFrames last (0 = none)
    int pullFrames;               // 0x26c: frames left of the pull towards pullTarget
    union
    {
        int switchNode;           // 0x270: node id of the switch this character works (switch characters only)
        int bankIndex;            // 0x270: a model-bank record's own index in the bank (bank models only)
        int scriptFlag;           // 0x270: script flag set when an enemy is destroyed, 0 = none (Enemy_Destroy)
        int zone;                 // 0x270: damage zone a zone character works: its index in g_mapRecords (DamageZone_Update)
    };
    union
    {
        int burst;                    // 0x274: 1 once the balloon burst (balloon slots only)
        int leftModel;                // 0x274: index of the model an enemy leaves behind when destroyed (g_leftModelPaths)
    };
    int blastAngle;                   // 0x278: direction of the last blast on the balloon, in thousandths of a radian
    int blastPush;                    // 0x27c: strength of the last blast push on the balloon
    char m_0280[0x288 - 0x280];       // 0x280: not yet named
    int defeatState;                  // 0x288: set to 6 when the character is destroyed (Enemy_Destroy)
    int defeatCancel;                 // 0x28c: cleared when destroyed; set again by a script to cancel the removal
    float m_0290;            // 0x290: cleared when the model is placed
    int walkingRoute;             // 0x294: 1 while the character walks a route (cleared when the player gets a helmet)
    Playback *playback;           // 0x298: the motion playback state (allocated on first use)
    float speedScale;             // 0x29c: motion speed scale
    char m_02a0;                  // 0x2a0: cleared on reset, load and clone
    char m_02a1;                  // 0x2a1: cleared on reset, load and clone
    char m_02a2;                  // 0x2a2: cleared on reset, load and clone
    unsigned char loadCount;      // 0x2a3: number of model files loaded into this model so far (stops counting at 3)
    char holdState;               // 0x2a4: HoldState
    char m_02a5[0x2a8 - 0x2a5];   // 0x2a5: not yet named
    int m_02a8;                   // 0x2a8: cleared on reset (JP: angle the held model is turned about the hold point)
    MotionChannels motion;        // 0x2ac: the motion channel (track A)
    int m_0d44;                   // 0xd44: cleared on reset
    int m_0d48;                   // 0xd48: cleared on reset
    float alpha;                  // 0xd4c: model alpha (1 = fully visible; set when an enemy is respawned)
    MotionChannels trackB;        // 0xd50: track B
    char m_17e8;                  // 0x17e8: not yet named
    char locked;                  // 0x17e9: non-zero while the model ignores queued pushes
    char pad_17ea[2];
    MotionChannels trackC;        // 0x17ec: track C
    int shadowHidden;             // 0x2284: non-zero = no shadow is drawn for the model (model_bank.cpp)
    char pushFlag;                // 0x2288: flag of the queued push
    char pad_2289[3];
    Vec4 pushVector;              // 0x228c: queued push (x, y, z)
    int blendFrames;              // 0x229c: frames a motion change blends over (0 = unset, DEFAULT_BLEND_FRAMES is used)
    int m_22a0;                   // 0x22a0: cleared when the model is unloaded
    char m_22a4[0x22ac - 0x22a4]; // 0x22a4: not yet named
    char m_22ac;                  // 0x22ac: cleared when the model is unloaded
    char m_22ad[0x22ec - 0x22ad]; // 0x22ad: not yet named
    Node *root;                   // 0x22ec: the model's node tree (0 = free entry of the model bank)
    Node *loadedRoot;             // 0x22f0: the node tree the model file loaded (a bank model's root)
    Node *headNode;               // 0x22f4: the model's head node (0 = none)
    Node *leftToeNode;            // 0x22f8: the model's left toe node (0 = none)
    Node *rightToeNode;           // 0x22fc: the model's right toe node (0 = none)
    Node *swapNode1;              // 0x2300: first node of the material swap
    Node *swapNode2;              // 0x2304: second node of the material swap
    Node *collider;               // 0x2308: the map collision this model moves against (0 = none)
    char m_230c[0x2318 - 0x230c]; // 0x230c: not yet named
    float colliderReach;          // 0x2318: reach of the model's probes against the collider
    Node *dropCollider;           // 0x231c: collision node the model drops onto (set by SetDropCollider)
    int m_2320;                   // 0x2320: cleared when a model file is loaded or cloned (JP: the node whose world matrix and frame this model copies)
    Node *bipNode;                // 0x2324: the "Bip01" bone (0 = no node tree)
    Matrix4 matrix;               // 0x2328: world matrix; the translation row (m[12..14]) is the position
    int motionFrame;              // 0x2368: frame of the motion file shown now
    float verticalSpeed;          // 0x236c: vertical speed (positive = rising)
    float placedHeight;      // 0x2370: height the model was placed at (copy of placeHeight)
    int m_2374;              // 0x2374: cleared when the model is placed
    char m_2378;             // 0x2378: cleared when the model is placed
    char m_2379;             // 0x2379: cleared when the model is placed
    unsigned char shadowOpacity; // 0x237a: shadow opacity (SHADOW_OPACITY_PLACED when placed, 0 = no shadow)
    char m_237b;                  // 0x237b: not yet named
    float m_237c;            // 0x237c: 1 when the model is placed
    int m_2380;                   // 0x2380: set to FRESH_M_2380 on reset, load and clone (purpose not yet known)
    int collisionResult;          // 0x2384: collision result, cleared before each update (UpdateCollision)
    int speedChangeFrames;        // 0x2388: frames since the speed change (1 once one is pending)
    int idleMotionEnabled;        // 0x238c: non-zero = the model plays its idle motion now and then (Char_RandomIdle)
    int collisionLimit;           // 0x2390: limit of the map collision tests (99999 = none)
    int m_2394;              // 0x2394: 1 when the model is placed
    Vec4 holdOffset;              // 0x2398: position relative to the held model
    Vec4 move;                    // 0x23a8: movement impulse applied by the next update (Push)
    Vec4 velocity;                // 0x23b8: current movement
    Vec4 m_23c8;             // 0x23c8: cleared when the model is placed
    Vec4 moveStep;                // 0x23d8: this frame's movement: x, y scaled by speedScale, z the vertical step
    Vec4 m_23e8;             // 0x23e8: cleared when the model is placed
    Vec4 m_23f8;             // 0x23f8: cleared when the model is placed
    Vec4 m_2408;             // 0x2408: cleared when the model is placed
    int m_2418;                         // 0x2418: cleared when the model is unloaded
    char m_241c[0x2420 - 0x241c];       // 0x241c: not yet named
    Vec4 m_2420;                  // 0x2420: cleared on reset
    int m_2430;                   // 0x2430: cleared on reset
    Matrix4 shadowMatrix;    // 0x2434: matrix the shadow is drawn with (Teleport moves it with the model)
    char m_2474;                  // 0x2474: cleared on reset, load and clone
    char m_2475[0x2478 - 0x2475]; // 0x2475: not yet named
    int updateCount;              // 0x2478: cleared on reset (JP: frames the model has been updated)
    float heading;                // 0x247c: angle the character faces now
    float targetHeading;         // 0x2480: angle the character turns towards
    int m_2484;                   // 0x2484: cleared on reset
    char m_2488[0x2494 - 0x2488]; // 0x2488: not yet named
    float eventCameraHeight;      // 0x2494: height the event camera adds to its eye and target (cleared by LoadModel)
    int mode;                     // 0x2498: behaviour mode (1 = the walking mode that slows movement impulses)

    int SetTrackCOptions(int playing, int looping);                      // EN 0x743960 (JP 0x10161b40)
    int SetMotionOptions(int playing, int looping);                      // EN 0x745c00 (JP 0x10161be0)
    int RunRootCall(int a, int b);                                       // EN 0x73f750: calls Node::UnrecoveredCall on the node tree, 0 when there is none
    int SetTrackBOptions(int playing, int looping);                   // EN 0x7469b0 (JP 0x10161f60)
    int AddTrackBKey(int frame, float value);                            // EN 0x70b6d0 (EN-only, anim_model.cpp): adds a key to track B and widens its frame range
    int FaceDirection(float x, float y);                                 // EN 0x746590 (EN-only, anim_model.cpp): turns the model's target heading to the direction (x, y), pointing away from it
    int AddTrackCKey(int frame, float x, float y, float z);              // EN 0x70ad20 (JP 0x10161af0)
    int AddTrackCKeyFlagged(int frame, float x, float y, float z, char flag);   // EN 0x70ad90 (JP 0x10161a90)
    int StepMotion();                                                    // EN 0x70bea0 (JP 0x10161c00): advances the motion channel one frame
    int Draw(Matrix4 *viewProjection, const void *lights, int flag);   // EN 0x71d240 (JP 0x10163150): queues the model for drawing under `viewProjection` with the light set `lights`
    int AddMotionKey(int frame, float value);                            // EN 0x70af70 (JP 0x10161b90)
    int SetPendingPush(float x, float y, float z, char flag);            // EN 0x73f940 (JP 0x101628b0)
    int StepPath();                                                      // EN 0x70bf40 (JP 0x10161c80): moves the model along track C one frame
    int StepTrackB();                                                    // EN 0x70c030 (EN-only, anim_model_track_b.cpp): advances track B one frame (samples it, loops or stops at the end)
    int Unload();                                                        // EN 0x740c40 (JP 0x10162c50): frees the node tree and motion buffer and resets position and state, returns 1
    int ResetState();                                                    // EN 0x7413f0 (JP 0x10161f80): resets the model's motion and pose state
    int ClearTrackC();                                                // EN 0x741bd0 (JP 0x10161a60)
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
    int SetFollowUp(int layer, int index, int value);                    // EN 0x745590: Playback::SetFollowUp on the playback (creates it on first use)
    int SetSlotOffset(int layer, int slot, int x, int y);                // EN 0x7464d0: Playback::SetSlotOffset on the playback (creates it on first use)
    int LoadFile(const char *path);                                      // EN 0x735780 (JP 0x1017b9b0, anim_clone.cpp): loads the model file `path` into the model
    int CloneModel(AnimModel *source, int unused);                       // EN 0x71aff0 (JP 0x1017b210, anim_clone.cpp): gives the model a copy of `source`'s node tree, 0 when `source` has none
};

static_assert(sizeof(AnimModel) == ANIM_MODEL_SIZE, "AnimModel must be 0x249c bytes");

#endif
