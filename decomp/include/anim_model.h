// An animated model (0x249c bytes): a loaded node tree with its motion playback, three key-framed motion channels, a
// world matrix, movement vectors and collision state. Every character is one: the character models are the array
// DAT_10db6ee8 (world.h's Character views the same records 0x10 bytes earlier). Methods: anim_model.cpp (channels),
// anim_reset.cpp (ResetState), anim_unload.cpp (Unload), anim_ctor.cpp (constructor), model_params.cpp (movement,
// facing, holding, collision tests), model_place.cpp (Teleport, SetPosition), path_anim.cpp (StepPath),
// texture_mgr.cpp (LoadModel, PlayMotion, UpdateCollision) and scene_misc.cpp (playback settings).
// This is the one declaration of these types: include this header, never redeclare them.
#ifndef GURUMIN_ANIM_MODEL_H
#define GURUMIN_ANIM_MODEL_H

#include "vec4.h"
#include "matrix4.h"
#include "key_track.h"

struct Node;

// What a motion slot points at: a source that is in use while `busy` is non-zero.
struct MotionSource
{
    int busy;
};

// Number of motion layers a Playback tracks.
#define PLAYBACK_LAYER_COUNT 0x90
// Playback::layer while blending into nextLayer.
#define PLAYBACK_BLENDING_LAYER 0x90

// The motion playback state of a model (0x7c84 bytes, `AnimModel::motion`): which layer (motion) plays, the frame,
// and per-layer frame ranges and values. Init, SetLayer, BindNode, BindModel and Play are in model_state.cpp; the
// motion-slot setters (SetSlotSource, SetSlotOffset, SetLayerParam) in mesh_cache.cpp.
struct Playback
{
    int m_0000;
    float m_0004;                                  // 0x04: frame position (-1 while blending)
    float m_0008;                                  // 0x08: playback speed (1.0)
    unsigned char m_000c;
    char pad_000d[3];
    int m_0010;
    int m_0014;
    int m_0018;
    int layer;                                     // 0x1c: layer playing (PLAYBACK_BLENDING_LAYER while blending, -1 none)
    int frame;                                     // 0x20: current frame
    int m_0024;
    int m_0028;
    MotionSource *slotSource[16];                        // 0x2c: source of each motion slot (SetSlotSource)
    int slotLayer[16];                             // 0x6c: layer of each motion slot (-1 = unused)
    int m_00ac[PLAYBACK_LAYER_COUNT];              // 0xac: 1 while the layer is active
    int m_02ec[PLAYBACK_LAYER_COUNT];              // 0x2ec: first frame
    int m_052c[PLAYBACK_LAYER_COUNT];              // 0x52c: last frame
    int m_076c[PLAYBACK_LAYER_COUNT];
    int m_09ac[PLAYBACK_LAYER_COUNT];
    int m_0bec;
    int m_0bf0[PLAYBACK_LAYER_COUNT];              // 0xbf0: playback direction (+1 forwards, -1 backwards)
    int m_0e30[PLAYBACK_LAYER_COUNT];
    int slotOffsetX[PLAYBACK_LAYER_COUNT][16];     // 0x1070: per layer and slot: x offset (cleared when the layer
                                                   // starts; SetSlotOffset)
    int slotOffsetY[PLAYBACK_LAYER_COUNT][16];     // 0x3470: y offset
    int layerParams[PLAYBACK_LAYER_COUNT][16];     // 0x5870: per-layer parameters, 8 used (-2 = unused, set when
                                                   // the layer starts; SetLayerParam)
    int frameCount;                                // 0x7c70: frames of the bound node's animation (0 when 8 or
                                                   // fewer)
    int blending;                                  // 0x7c74: non-zero while blending into nextLayer
    int m_7c78;
    int nextLayer;                                 // 0x7c7c: layer being blended into
    Node *m_7c80;

    void Init();                                                       // 0x10169f90
    int BindModel(struct AnimModel *owner);                            // 0x1016a020: BindNode on owner's root
    int BindNode(Node *node);                                          // 0x1016a090: binds to `node`, resets
    int SetLayer(int layer, int a, int first, int last, int b, int c); // 0x1016a100
    int Play(int layer, int restart, int blendFrames, float speed);    // 0x1016a2e0
    int Advance();                                                     // 0x1016a480: one frame step
    // Motion-slot setters (mesh_cache.cpp); each returns 0 for an out-of-range layer or slot, else 1.
    int SetSlotSource(void *source, int slot, int layer);              // 0x1016a220: slot's source and layer
    int SetSlotOffset(int layer, int slot, int x, int y);              // 0x1016a260
    int SetLayerParam(int layer, int slot, int value);                 // 0x1016a2a0: slot 0..7
};

// One key-framed motion channel of a model (0xa98 bytes): a key bank and its playback range.
struct MotionChannels
{
    KeyBank keys;             // 0x000
    int last;                 // 0xa84: last frame (exclusive for track C)
    int first;                // 0xa88: first frame (10000 when empty)
    int frame;                // 0xa8c: current frame while playing
    int playing;              // 0xa90: 1 while playing
    int looping;              // 0xa94: 1 = loop back to `first` at the end
};

// Values of AnimModel::holdState.
#define HOLD_FREE 0
#define HOLD_HOLDING 1
#define HOLD_RELEASED 2

struct AnimModel
{
    Vec4 m_0000;
    char contactState;        // 0x10: 0 = not recording, set to 3 when another model blocks, 2 when the map does (LiftProbes)
    char mapTested;           // 0x11: 0 until the first map test of LiftProbes, then 1
    char m_0012;              // 0x12: set to 2 every frame by many characters (Enemy_FlyAndStrafe)
    char m_0013;
    char hitSolid;            // 0x14: 1 when the probe MoveWithCollisions settled on was not blocked by a camera node
    char pad_0015[3];
    Matrix4 m_0018;
    Vec4 m_0058;
    float m_0068;             // 0x68: -1000.0 on objects set up by a map trigger (FLOAT, LIFT, BUTTON...)
    char m_006c;
    char pad_006d[3];
    union
    {
        char *nearCells;      // 0x70: characters: row of DAT_1021d338 (map cells near it), 0 = no visibility data
        int collisionFilter;  // 0x70: collision filter of the model's tests (DAT_11b002c0, model_params.cpp)
    };
    char m_0074;
    char m_0075;
    char m_0076;
    unsigned char m_0077;     // 0x77: model variant loaded (Costume_LoadPlayerModel 0x1003b6e0)
    union
    {
        struct
        {
            int m_0078;
            int m_007c;
        };
        int footState[2];     // 0x78: frames each foot has been planted (0 = in the air; UpdateFrame)
    };
    int m_0080;
    unsigned char speedDivisor;  // 0x84: slows every motion of this model down (0 = no division)
    char pad_0085[3];
    union
    {
        int m_0088;           // 0x88: cleared by CloneModel
        float spinAngle;      // 0x88: angle the drawn model is turned about y (0 = not turned; UpdateFrame)
    };
    unsigned char m_008c;
    char m_008d;              // 0x8d: set while a child runs to the player (Npc_ChildFollow)
    char pad_008e[2];
    Vec4 m_0090;
    Vec4 m_00a0;
    float m_00b0;             // 0xb0: height the model was placed at
    Vec4 m_00b4[12];
    Vec4 m_0174[6];
    int m_01d4;
    Matrix4 m_01d8;
    char lifted;              // 0x218: 1 when LiftProbes had to raise a probe
    char pad_0219[3];
    Vec4 scale;               // 0x21c: model scale
    Vec4 m_022c;
    int m_023c;
    char m_0240;
    char idleFrames;          // 0x241: frames m_238c has been non-zero (stops at 100; UpdateFrame)
    char activeFrames;        // 0x242: frames m_238c has been zero (stops at 100)
    char pad_0243;
    Node *spineNode;          // 0x244: the "Bip01 Spine" bone (loadedRoot when the model has none; CloneModel)
    float holdWeight;         // 0x248: blend weight while holding (at most 0.9)
    Vec4 m_024c;
    int m_025c;               // 0x25c: random value 0..499 picked by CloneModel
    float uniformScale;       // 0x260: uniform model scale (1.0; model_bank.cpp passes it to the shadow drawing)
    char m_0264;
    char important;           // 0x265: motion changes are flagged important (DAT_11b04508); set by Enemy_ShoveAndBlast
    char m_0266;
    char m_0267;              // 0x267: 1 while an NPC idles; set every frame by the kind updates
    Vec4 *pullTarget;         // 0x268: position the model is drawn towards while pullFrames last (0 = none; MoveWithCollisions)
    int pullFrames;           // 0x26c: frames left of the pull towards pullTarget
    // 0x270-0x290: values whose meaning depends on the character's kind (each struct names one kind's use).
    union
    {
        struct
        {
            int bankIndex;    // 0x270: a model-bank record's own index in the bank (model_bank.cpp)
            int m_0274;
            int m_0278;
            int m_027c;
        };
        struct
        {
            int switchNode;   // 0x270: node id of the switch this character works (switches.cpp)
        };
        struct
        {
            int zone;         // 0x270: the zone a zone character runs (damage_zone.cpp)
        };
        struct
        {
            int scriptFlag;   // 0x270: script flag set when it is destroyed, 0 = none (enemy_destroy.cpp)
            int leftModel;    // 0x274: index of the model left behind (path table DAT_10894698)
        };
        struct
        {
            int flameCode;    // 0x270: flame layer group * 10000 + route variant (flame_layers.cpp)
        };
        struct
        {
            int pad_burst;
            int burst;        // 0x274: 1 once the balloon has burst (balloon_burst.cpp)
            int blastAngle;   // 0x278: direction of the blast (1/1000 radians)
            int blastPush;    // 0x27c: strength of the push
        };
    };
    int m_0280;               // 0x280: 0 or 1 while the character has not been defeated or moved on
    int m_0284;               // 0x284: set to a random number below 1000 by the "PCON" trigger
    int m_0288;               // 0x288: set to 6 when destroyed
    int m_028c;               // 0x28c: cleared when destroyed
    float m_0290;             // 0x290: set to a count by LiftProbes on the model it collided with
    union
    {
        int m_0294;           // 0x294: 1 while held in its pose (npc_frozen.cpp)
        int holding;          // 0x294: 1 while a sleeper holds its sleeping frame (npc_sleep.cpp)
        int walkingRoute;     // 0x294: 1 while the character walks a route (route_walk.cpp)
    };
    Playback *motion;         // 0x298: the motion playback state (freed by Unload)
    float speedScale;         // 0x29c: motion speed scale (1.0)
    char m_02a0;
    char m_02a1;
    char m_02a2;
    unsigned char loadCount;  // 0x2a3: number of models loaded so far (stops counting at 3)
    char holdState;           // 0x2a4: HOLD_*
    char pad_02a5[3];
    union
    {
        int m_02a8;
        float holdAngle;      // 0x2a8: angle the held model is turned about the hold point (0 to 2 pi; UpdateFrame)
    };
    MotionChannels trackA;    // 0x2ac: motion keys (KEY_FLAG_MOTION)
    int m_0d44;
    int m_0d48;
    float alpha;              // 0xd4c: 1.0 when spawned (model alpha)
    MotionChannels trackB;    // 0xd50
    char m_17e8;
    char locked;              // 0x17e9: non-zero = SetPendingPush ignores pushes
    char pad_17ea[2];
    MotionChannels trackC;    // 0x17ec: path keys (path_anim.cpp StepPath)
    int shadowHidden;         // 0x2284: non-zero = no shadow is drawn (model_bank.cpp)
    char m_2288;              // 0x2288: flag of the pending push
    char pad_2289[3];
    Vec4 m_228c;              // 0x228c: pending push (x, y, z), see SetPendingPush
    int m_229c;               // 0x229c: blend frames, 8 when unset
    int m_22a0;
    char pad_22a4[8];
    char m_22ac;
    char pad_22ad[0x3f];
    Node *root;               // 0x22ec: the model's node tree (its entry in the model table)
    Node *loadedRoot;         // 0x22f0: the same node tree as root while the model is in use, 0 when free (key
                              // prompts are drawn only for loaded characters; model_bank.cpp reads its model data)
    Node *headNode;           // 0x22f4: the head bone node (0 = none)
    union
    {
        struct
        {
            Node *leftToeNode;    // 0x22f8: the "Bip01 L Toe0" bone (loadedRoot when the model has none)
            Node *rightToeNode;   // 0x22fc: the "Bip01 R Toe0" bone (loadedRoot when the model has none)
        };
        Node *toeNodes[2];
    };
    Node *swapNode1;          // 0x2300: nodes whose materials M_10161a20 swaps (0 = none)
    Node *swapNode2;          // 0x2304
    Node *collider;           // 0x2308: the map collision this model moves against
    char pad_230c[4];
    Node *eyeNode;            // 0x2310: node whose animation places the event camera's eye (0 = none)
    Node *targetNode;         // 0x2314: node whose animation places the event camera's target (0 = none)
    float colliderReach;      // 0x2318: 0.578 * the bounding radius of the collision node (CloneModel)
    union
    {
        int m_231c;
        Node *dropCollider;   // 0x231c: the collision the dropped pieces bounce on (debris_update.cpp)
    };
    union
    {
        int m_2320;           // 0x2320: cleared by CloneModel
        Node *followNode;     // 0x2320: node whose world matrix and frame this model copies (0 = none; UpdateFrame)
    };
    Node *bipNode;            // 0x2324: the "Bip01" bone (0 = no node tree)
    Matrix4 matrix;           // 0x2328: world matrix; matrix.translation is the position
    union
    {
        int motionFrame;      // 0x2368: frame of the motion file shown now (held while sleeping)
        float radius;         // 0x2368: collision radius (UpdateCollision)
    };
    float verticalSpeed;      // 0x236c: vertical speed (positive = rising)
    float m_2370;             // 0x2370: height the model was placed at
    int m_2374;
    char m_2378;
    char m_2379;
    unsigned char shadowOpacity; // 0x237a: shadow opacity (15 when placed, 0 = no shadow)
    char pad_237b;
    float m_237c;
    int m_2380;
    int m_2384;               // 0x2384: collision result, cleared before each update
    int m_2388;               // 0x2388: 1 once a speed change is pending
    int m_238c;               // 0x238c: non-zero = play the idle motion (app_0011.cpp)
    int m_2390;               // 0x2390: limit for map collision tests; model_bank.cpp uses 99999 for "no timeout"
    int m_2394;
    Vec4 holdOffset;          // 0x2398: position relative to the held model
    Vec4 move;                // 0x23a8: movement impulse applied by the next update (Character_SetMove)
    Vec4 velocity;            // 0x23b8: current movement
    Vec4 m_23c8;
    Vec4 m_23d8;              // 0x23d8: x, y = target of MoveTowards / step of SetMoveStep; z = MoveTowards' int `mode` or SetMoveStep's vertical step
    Vec4 m_23e8;
    Vec4 m_23f8;
    Vec4 m_2408;
    AnimModel *m_2418;        // 0x2418: the model this one is carried by or follows (0 = none)
    char pad_241c[4];
    Vec4 m_2420;
    int m_2430;
    Matrix4 shadowMatrix;     // 0x2434: the matrix the shadow is drawn with; receives the ground position
                              // (char_ground.cpp)
    char m_2474;
    char pad_2475[3];
    int updateCount;          // 0x2478: frames updated (UpdateFrame)
    float heading;            // 0x247c: facing angle in radians
    float targetHeading;      // 0x2480: angle the character turns towards
    int m_2484;
    char pad_2488[4];
    int nextSlotId;           // 0x248c: id of the next slot's character (read as DAT_10db6ee8[i - 1].nextSlotId)
    int pad_2490;
    float m_2494;             // 0x2494: cleared by LoadModel; the height the event camera adds to its eye and target
    int mode;                 // 0x2498: 0 = the game may move the character, 1 = normal (set by the scene start for
                              //         the player), 2 = over the pit area

    AnimModel();                                                   // 0x101629f0 (anim_ctor.cpp)
    ~AnimModel();                                                  // 0x10007e10 (the shared empty destructor)
    int M_10161a20(int texture);                                   // 0x10161a20: material swap
    void M_10161a60();                                             // clears track C
    int M_10161a90(int frame, float x, float y, float z, char flag);
    int M_10161af0(int frame, float x, float y, float z);          // adds a key to track C
    int M_10161b40(int a, int b);                                  // track C's playback options
    void M_10161b60();                                             // clears track A
    int M_10161b90(int frame, float value);                        // adds a motion key to track A
    int M_10161be0(int a, int b);
    int StepTrackA();                                              // 0x10161c00
    int SnapPosition(float step, float offset);                    // 0x10161cb0
    int SnapHeight(float step, float offset);                      // 0x10161e40
    void M_10161f30();
    int M_10161f60(int a, int b);
    int ResetState();                                              // 0x10161f80 (anim_reset.cpp)
    int UpdateCollision();                                         // 0x10162390 (texture_mgr.cpp)
    int Teleport(float x, float y, float z);                       // 0x10162420 (model_place.cpp)
    int SetPosition(float x, float y, float z);                    // 0x10162610 (model_place.cpp)
    int SetMoveStep(float x, float y, float z);  // 0x101627d0 (model_params.cpp)
    int MoveTowards(float x, float y, int mode);                   // 0x10162800
    // 0x101628b0: queues a push (x, y, z) with `flag` unless the model is locked.
    int SetPendingPush(float x, float y, float z, char flag);
    int Push(float x, float y, float z);                           // 0x101628f0
    int M_101629b0(float y, float x);
    int Unload();                                                  // 0x10162c50 (anim_unload.cpp)
    int SnapHeadingToSide();                                       // 0x10162d50
    int FaceAwayFrom(AnimModel *other);                            // 0x10162e00
    int FaceAway(const Vec4 &point);                               // 0x10162ea0
    int ScreenPosition(int *x, int *y, const Matrix4 &viewProjection);  // 0x10162f50
    int PushMotionSpeed(float speed);                              // 0x10163000
    int AddMotionSpeed(float speed);                               // 0x101630b0
    int HoldAtSpine(int on, float amount);                         // 0x10163c70
    int Hold(int on, float amount, AnimModel *other);              // 0x10163d60
    int Draw(const Matrix4 *viewProjection, const Vec4 *lights, int flags);  // 0x10163150: queues the model under `viewProjection` with the light set `lights`
    int IsClear(const Vec4 *offset);                               // 0x10163e90
    int IsClearFlagged(const Vec4 *offset);                        // 0x10163f90
    int IsClearAny(const Vec4 *offset);                            // 0x101640b0: 0 when the move collides
    int IsClearOfMapLimited(const Vec4 *offset);                   // 0x101641b0
    int IsClearOfMap(const Vec4 *offset);                          // 0x101642c0
    int LiftProbes(float *nearest, Vec4 *probes, int probeCount, AnimModel **hitIds, Vec4 *hits, int *hitOther, int unused1,
                   unsigned int flags, int unused2, int mode, int extra, int *outHit, int *outFlag);  // 0x101643c0
    int StepPath();                                                // 0x10175e80 (path_anim.cpp): follows track C
    int LoadModel(const char *path);                               // 0x1017aeb0 (texture_mgr.cpp)
    int SetPlayback(int a, int b, int c, int d, int e, int f);     // 0x1017af40 (scene_misc.cpp)
    int SetLayerOffset(int layer, int slot, int x, int y);         // 0x1017afb0
    int SetLayerSource(void *source, int slot, int value);         // 0x1017b020
    int SetLayerParameter(int layer, int slot, int value);         // 0x1017b080
    void PlayMotion(int motion, int a, int b, float speed);        // 0x1017b0e0 (texture_mgr.cpp)
    int M_1017b160();
    int M_1017b1c0();
    int CloneModel(AnimModel *source, int unused);                 // 0x1017b210 (anim_clone.cpp)
    int LoadFile(const char *path);                                // 0x1017b9b0 (anim_clone.cpp)
    // 0x1017a010 (anim_model_step.cpp): the per-frame update of a model: advances its motion, fades the shadow,
    // plays footsteps, moves it by its pending movement (collision tested by MoveWithCollisions), turns it towards its target
    // heading and refreshes its node tree under `view`; see the definition.
    int UpdateFrame(Matrix4 *view, int withView, int force, int frame, Matrix4 *reference);
    int MoveWithCollisions(float x, float y, float z, int wantHitOther, int unused);  // 0x10175f20 (anim_move.cpp): moves by (x, y, z) against the collisions
    int SetDropCollider(Node *collider);                           // 0x10163e70: sets dropCollider (the same code as ModelObject::M_10163e70)
};

// The character models (one per character slot; slot 0 is the player).
#define PLAYER_SLOT 0   // the player's character slot

extern "C" AnimModel DAT_10db6ee8[0x80];

// The model table: root node of each loaded model file (0 = free entry).
extern "C" Node *DAT_10faaa88[0x100];

#endif
