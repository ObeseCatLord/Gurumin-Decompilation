// Camera and caption globals of the EN build (declared once here; JP equivalents in decomp/include/camera_vars.h).
#ifndef GURUMIN_EN_CAMERA_H
#define GURUMIN_EN_CAMERA_H

// Camera control modes stored in g_cameraMode.
enum CameraMode
{
    CAMERA_MODE_FOLLOW = 0,   // normal follow camera
    CAMERA_MODE_1 = 1,        // mode 1 (set by Camera_SetMode1)
    CAMERA_MODE_2 = 2         // mode 2 (set by Camera_SetMode2)
};

// Camera mode setters (camera.cpp). Camera_SetMode2 is EN 0x70bc20.
extern "C" int Camera_SetMode2(void);

const int CAPTION_TEXT_SIZE = 0x100;       // bytes of the title caption buffer

extern "C" int g_cameraMode;                           // EN 0x01664a34 (JP DAT_1101de1c): camera control mode (CameraMode)
extern "C" char g_titleCaptionText[CAPTION_TEXT_SIZE]; // EN 0x01664ac0 (JP DAT_1101d8b8): title caption text
extern "C" int g_titleCaptionTimer;                    // EN 0x01664abc (JP DAT_1101d9b8): title caption timer, 0 = just started
// Sets the title caption text and restarts its timer. Returns 0. EN 0x746be0 (camera.cpp).
extern "C" int Caption_SetTitle(const char *text);


#include "containers.h"
#include "node.h"
#include "vec4.h"

const int CAMERA_MOVE_FRAMES = 10;          // frames a started camera move runs

extern "C" int g_cameraMoveReset;           // EN 0x01664fdc (JP DAT_10fabe88): cleared when a camera move starts
extern "C" int g_cameraMoveSelected;        // EN 0x01664fd8 (JP DAT_11018738): selected path of the move, -1 = none
extern "C" int g_cameraMoveStep;            // EN 0x01664698 (JP DAT_1101d8a0): step of the move, 0 = first
extern "C" int g_cameraPathCount;           // EN 0x01664694 (JP DAT_1101d89c): number of camera paths
extern "C" int g_cameraMoveFramesLeft;      // EN 0x016646a0 (JP DAT_1101d8a8): frames left in the camera move
extern "C" Vec4 g_cameraEye;                // EN 0x021ffed8 (JP DAT_10f65688): camera position
extern "C" Vec4 g_cameraTarget;             // EN 0x02200ee8 (JP DAT_10f668e8): camera target
extern "C" Vec4 g_cameraTargetMapPass;      // EN 0x021ffe68 (JP DAT_10f66908): camera target while the map pass is running

// Adds every camera path in the node tree under `node` to the path tables at *pathCount (`root` is the tree names are
// looked up under; the function calls itself for the children). Returns 0. EN 0x744e40 (JP 0x1016ac90).
int Camera_CollectPaths(Node *root, Node *node, int *pathCount);

// Builds the view and projection matrices for a camera at `eye` looking at `target`. EN 0x759300 (JP 0x10194360).
int Camera_SetMatrices(const Vec4 *eye, const Vec4 *target, Matrix4 *view, Matrix4 *projection, float roll);

// Camera_SetMatrices, then stores `eye` and `target` as the camera position and target (g_cameraEye, g_cameraTarget).
// Returns 0. EN 0x744a40 (see camera_apply.cpp). No direct caller.
int Camera_SetAndRemember(const Vec4 *eye, const Vec4 *target, Matrix4 *view, Matrix4 *projection, float roll);

// Positions the camera keeps in view (at most CAMERA_TRACKED_COUNT).
const int CAMERA_TRACKED_COUNT = 0x30;

extern "C" float *g_trackedPositions[CAMERA_TRACKED_COUNT];   // EN 0x0091f2b0 (JP DAT_1032d5b8): translation rows kept in view
extern "C" char g_trackedFlags[CAMERA_TRACKED_COUNT];         // EN 0x0091f410 (JP DAT_1021bbc0): a flag per tracked position
extern "C" int g_trackedCount;                                // EN 0x0091e76c (JP DAT_1049f0f0): entries of the two lists

// Adds character `index` position to the list of positions the camera keeps in view. Returns 1. EN 0x645730.
extern "C" int Camera_AddTrackedPosition(int index);
extern "C" int Camera_SetPushOffset(float x, float y);   // EN 0x70b8b0 (camera_push_offset.cpp): the player's ground push turned into the camera frame (g_cameraPushOffset)


// Shot cameras (camera_follow.cpp): the goals and the smoothed vectors of the script-driven shots.
extern "C" int g_cameraShotStarted;              // EN 0x009159e8 (JP DAT_104c0358): 1 once the shot's vectors were set up
extern "C" Node *g_cameraFollowNode;             // EN 0x0091da28 (JP DAT_104c039c): the node the camera follows (0 = none)
extern "C" int g_cameraSmoothingLocked;      // EN 0x009159e4 (JP DAT_104c0354): non-zero = the listener update leaves the camera vectors alone
extern "C" Vec4 g_cameraPositionGoal;            // EN 0x015ad640 (JP DAT_10866d98): where the camera position moves to
extern "C" Vec4 g_cameraTargetGoal;              // EN 0x0147ba88 (JP DAT_10edbd48): where the camera target moves to
extern "C" Vec4 g_cameraTargetNow;               // EN 0x01d8f690 (JP DAT_118e6cb8): the camera target this frame
extern "C" Vec4 g_cameraPositionNow;             // EN 0x020f17a0 (JP DAT_118e69e8): the camera position this frame
extern "C" Vec4 g_cameraTargetCopy;              // EN 0x01ee4900 (JP DAT_118e6c78): copy of the target for the other systems
extern "C" Vec4 g_cameraPositionCopy;            // EN 0x0217b5b0 (JP DAT_11af3e28): copy of the position
extern "C" Vec4 g_cameraPreviousPosition;        // EN 0x01337488 (JP DAT_104d29d8): the position of the previous frame
extern "C" Vec4 g_cameraPreviousTarget;          // EN 0x01611c58 (JP DAT_10f08aa0): the target of the previous frame

extern "C" Vec4 g_cameraOrbitCentre;            // EN 0x015e9da8 (JP DAT_108393d8): the point the orbit shot circles (w: its heading)
extern "C" Vec4 g_cameraPlayerPosition;         // EN 0x0147ba78 (JP DAT_10da93f0): the orbit centre, the player's position
extern "C" Node *g_cameraDriftNode;             // EN 0x0091500c (JP DAT_1021c624): a node the orbit target drifts towards (0 = none)

// Two-character shot (Camera_Between): the script's parameters.
const float BETWEEN_HEAD_OFFSET = 35.0f;         // goal height above a character's head node
const float BETWEEN_BODY_OFFSET = 75.0f;         // goal height above a character without a head node
const float BETWEEN_AHEAD = -600.0f;             // y of the target goal behind the first character when there is no second one
const float BETWEEN_BELOW = -60.0f;              // z of that target goal
const float BETWEEN_NO_POSITION = -20000.0f;     // position goal x at or below this: the position goal was not set
const float BETWEEN_RISE_PER_CENT = 0.01f;       // one per cent
extern "C" Vec4 g_cameraBetweenParams;           // EN 0x00daa850 (JP DAT_10da93e0): x = first character, y = second (negative = none), z = per cent of the distance the target goal rises
// Camera_Between: the shot between two characters. EN 0x64d540 (JP 0x10019d80).
extern "C" int Camera_Between(void);

// Item stand shot (Camera_ItemStand).
const int ITEM_STAND_SPARKLE_EFFECT = 0x75f;      // effect started above the target goal every ITEM_STAND_SPARKLE_PERIOD frames
const int ITEM_STAND_SPARKLE_PERIOD = 65;
const float ITEM_STAND_SPARKLE_BASE = 90.0f;      // height of the sparkle matrix above the target goal: 90 plus 25 (two adds)
const float ITEM_STAND_SPARKLE_REST = 25.0f;
const int ITEM_STAND_SPARKLE_INT = 115;           // the same height in the integer shot point
const float ITEM_STAND_DROP = 50.0f;              // the stand's parent follows the target goal this far lower
const float ITEM_STAND_ORBIT_STEP = -0.012f;      // radians the position goal turns about the target goal each frame
const int ITEM_STAND_SINK_START = 90;             // frame the target goal starts to sink
const int ITEM_STAND_SINK_FULL = 120;             // frame it sinks at the full speed
const float ITEM_STAND_SINK_SPEED = 4.0f;         // units per frame at the full speed
const int ITEM_STAND_SINK_RAMP = 30;              // frames the sink speed takes to build up
// Node flag the item stand's parent gets (and its "itemkagu" child loses).
const unsigned int ITEM_STAND_NODE_FLAG = 0x20;
extern "C" Node *g_itemStandParent;             // EN 0x009147e0 (JP DAT_1021bdf8): the item stand's parent node
extern "C" int g_shotPointX;                    // EN 0x01333314 (JP DAT_10c6356c): the shot's point as integers
extern "C" int g_shotPointY;                    // EN 0x01333318 (JP DAT_10c63570)
extern "C" int g_shotPointZ;                    // EN 0x0133331c (JP DAT_10c63574): z + ITEM_STAND_SPARKLE_INT

// Camera_ItemStand: the shot of the item stand rising. EN 0x64bd10.
extern "C" int Camera_ItemStand(void);
// Camera_Orbit: the shot that circles the player. EN 0x64d890.
extern "C" int Camera_Orbit(void);
// Places the camera at heading `angle` around the target at its current ground distance and height. EN 0x7465f0 (JP 0x1016bdf0).
extern "C" int Camera_PlaceAtHeading(float angle);
// Camera_Ease: the follow shot that eases towards the followed node. EN 0x64c790.
extern "C" int Camera_Ease(void);

// Camera shift shot (Camera_Shift, camera_shift.cpp): the offsets g_cameraPlayerPosition (position) and g_cameraOrbitCentre
// (target) move towards the shift goals while a shift runs and shrink otherwise; the goals follow the offsets.
extern "C" int g_cameraShiftFrames;              // EN 0x009159ec (JP DAT_104c035c): frames left of the running shift, 0 = none
extern "C" Vec4 g_cameraShiftPosition;           // EN 0x013388d8 (JP DAT_10802be8): the camera position the shift moves to
extern "C" Vec4 g_cameraShiftTarget;             // EN 0x00daa720 (JP DAT_10c67ff0): the camera target the shift moves to
// Camera_Shift: the shot that shifts the camera by offsets. Returns 0. EN 0x64caf0 (JP 0x10019310).
extern "C" int Camera_Shift(void);

// Camera_PullToItemStand: the camera shot that pulls back from the item stand while the player walks to it. Returns 0.
// EN 0x64c090 (JP 0x10018870, camera_follow.cpp). Switches back to the follow mode (Camera_SetModeFollow, EN 0x7485b0)
// when g_cameraShiftFrames is 1.
extern "C" int Camera_PullToItemStand(void);
extern "C" int Camera_SetModeFollow(void);

// Camera state cleared with the player (Player_ResetMotion) and the camera (Camera_Reset).
const int CAMERA_GROUND_UNKNOWN = -100;          // g_cameraGroundHeight value before a ground height was measured
extern "C" int g_cameraGroundHeight;             // EN 0x00908a20 (JP DAT_10203444): ground height cache, CAMERA_GROUND_UNKNOWN = unknown
extern "C" float g_cameraTargetLift;             // EN 0x01664678 (JP DAT_1101dde4): how far the camera target is lifted (0 or up to 120)
extern "C" float g_cameraEyePitch;               // EN 0x01664440 (JP DAT_1101de18): pitch the free camera lifts its eye by (radians, 0 = none)
extern "C" char g_cameraPushRightFrames;         // EN 0x0162ef62 (JP DAT_10fabe8c): frames the follow camera keeps pushing right
extern "C" char g_cameraPushLeftFrames;          // EN 0x0162ef63 (JP DAT_10faaa80): frames the follow camera keeps pushing left
extern "C" int g_cameraState1664448;             // EN 0x01664448 (JP DAT_1101d888): not recovered, cleared with the camera
extern "C" int g_cameraState1664fe0;             // EN 0x01664fe0 (JP DAT_1101d898): not recovered, cleared with the camera

// More camera state cleared by Camera_Reset (uses not recovered yet; the JP twin's names are given where the JP order
// identifies them).
const float CAMERA_RESET_RATE = 12.0f;           // g_cameraRate after the reset
const int CAMERA_SHOT_SLOT_COUNT = 4;            // scripted-camera slots (one easing exponent each)
extern "C" int g_cameraState1664418;             // EN 0x01664418 (JP DAT_1101de20)
extern "C" char g_cameraFlag1664468;             // EN 0x01664468 (JP DAT_10faa678)
extern "C" char g_cameraFlag162ef61;             // EN 0x0162ef61 (JP DAT_1101de38)
extern "C" float g_cameraRate;                   // EN 0x009089e8 (JP DAT_10203450): 12 after the reset
extern "C" float g_cameraScale;                  // EN 0x00908a1c (JP DAT_10203448): 1 after the reset
extern "C" int g_cameraState1664444;             // EN 0x01664444 (JP DAT_10fe01d0)
extern "C" float g_cameraShotEase[CAMERA_SHOT_SLOT_COUNT];   // EN 0x009089f0 (JP DAT_10203454..60): easing exponent of each scripted camera slot, 1 after the reset
extern "C" int g_cameraState166444c;             // EN 0x0166444c (JP DAT_1101de28)
extern "C" int g_cameraState1664450;             // EN 0x01664450 (JP DAT_1101de2c)
extern "C" int g_cameraCounter9089ec;            // EN 0x009089ec (JP DAT_10203440): 1 after the reset
extern "C" int g_cameraState1664fd0;             // EN 0x01664fd0 (JP DAT_1101d890): 1 while a path camera runs
extern "C" int g_cameraState1664178;             // EN 0x01664178 (JP DAT_1101d894)
extern "C" int g_cameraState166469c;             // EN 0x0166469c (JP DAT_1101d8a4): the current path
extern "C" int g_cameraState16646a4;             // EN 0x016646a4 (JP DAT_1101d8ac)
extern "C" int g_cameraState1664204;             // EN 0x01664204 (JP DAT_1101d8b0)
extern "C" int g_cameraState1664ab8;             // EN 0x01664ab8 (JP DAT_1101d8b4)
extern "C" int g_cameraState1664fe4;             // EN 0x01664fe4 (JP DAT_1101ddd0)
extern "C" int g_cameraState166cfec;             // EN 0x0166cfec (JP DAT_1101ddd4)
extern "C" int g_cameraState166cff0;             // EN 0x0166cff0 (JP DAT_1101ddd8)
extern "C" float g_cameraState1664210;           // EN 0x01664210 (JP DAT_1101dddc)
extern "C" int g_cameraState166cff4;             // EN 0x0166cff4 (JP DAT_1101dde0[0].m_0000)
extern "C" int g_cameraState1664200;             // EN 0x01664200 (JP DAT_1101dde8)
extern "C" int g_cameraState1664208;             // EN 0x01664208 (JP DAT_1101ddec)
extern "C" float g_cameraScale9089e4;            // EN 0x009089e4 (JP DAT_1020344c): 1 after the reset
extern "C" int g_cameraState1664a30;             // EN 0x01664a30 (JP DAT_1101ddf0)
extern "C" float g_cameraPathFrame;              // EN 0x016646a8 (JP DAT_1101c800): frame of the path
extern "C" int g_cameraState1664214;             // EN 0x01664214 (JP DAT_10ff19e0)
extern "C" int g_cameraState166cfe8;             // EN 0x0166cfe8 (JP DAT_1101872c)
extern "C" int g_cameraState1664fd4;             // EN 0x01664fd4 (JP DAT_11018734)
extern "C" int g_cameraState166cff8;             // EN 0x0166cff8 (JP DAT_11018728)
extern "C" int g_cameraState166cffc;             // EN 0x0166cffc (JP DAT_10ff2ff4)
extern "C" int g_cameraState1664680;             // EN 0x01664680 (JP DAT_10feb3d8)
extern "C" float g_cameraTargetPathDistance;     // EN 0x0166420c (JP DAT_10ff2ff0): distance of the camera target to the path frame
// Vec4 values cleared by Camera_Reset (JP names in the comments).
extern "C" Vec4 g_cameraPushOffset;              // EN 0x0217b640 (JP DAT_11af3f58): the camera-space push of the player (x, y; z is 0)
extern "C" Vec4 g_cameraPositionCopyB;           // EN 0x0217b5c0 (JP DAT_11af3e38)
extern "C" Vec4 g_cameraTargetCopyB;             // EN 0x01ee4910 (JP DAT_118e6c88)
extern "C" Vec4 g_cameraPositionCopyC;           // EN 0x0217b5d0 (JP DAT_11af3e48)
extern "C" Vec4 g_cameraTargetCopyC;             // EN 0x01ee4920 (JP DAT_118e6c98)
extern "C" Vec4 g_cameraPositionCopyD;           // EN 0x0217b5e0 (JP DAT_11af3e58)
extern "C" Vec4 g_cameraTargetCopyD;             // EN 0x01ee4930 (JP DAT_118e6ca8)
extern "C" Vec4 g_cameraVectorSpare;             // EN 0x01da7eb0 (JP DAT_118e6cd8)

// Clears the camera globals: vectors, goals, blend and cut counters. EN 0x741c50 (JP 0x1016a980).
extern "C" int Camera_Reset(void);

// Zeroes the camera vectors and the per-frame camera state of a new player position. EN 0x70bc30 (JP 0x1016a8f0).
extern "C" int Player_ResetMotion(void);

// Camera path tables (filled by Camera_CollectPaths, one entry per path).
// Settings of a camera path (0x10 bytes).
struct PathSettings
{
    int m_0000;      // 0x00: copied from Node::m_187c of the path node
    int m_0004;      // 0x04: copied from Node::m_187d of the path node
    int m_0008;      // 0x08: 1 when the path or its target node has the flag 0x20
    int m_000c;      // 0x0c: 1 when the target node has the flag 0x10
};
extern "C" KeyTrack g_pathCameraTracks[];       // EN 0x01ee4630 (JP DAT_11af3928): per path: the camera position track
extern "C" KeyTrack g_pathTargetTracks[];       // EN 0x01ee4960 (JP DAT_118e69f8): the position track of each path's target
extern "C" KeyTrack g_pathSecondTracks[];       // EN 0x01ee4be0 (JP DAT_118e96f8): the track of the "...b" node
extern "C" KeyTrack g_pathSecondTargetTracks[]; // EN 0x0217b650 (JP DAT_11af3ba8): the second target track
extern "C" int g_pathIds[];                     // EN 0x01664a38 (JP DAT_10f8d938): id of each path
extern "C" int g_pathActive[];                  // EN 0x01664180 (JP DAT_11018740): 1 = active
extern "C" int g_pathHasSecondTarget[];         // EN 0x01664930 (JP DAT_11018628): 1 = has a second target
extern "C" int g_pathHasSecondNode[];           // EN 0x016649b0 (JP DAT_10fdeb10): 1 = has a "...b" node
extern "C" PathSettings g_pathSettings[];       // EN 0x01664730 (JP DAT_10ff17e0)

// Quads the camera keeps (the tables hold CAMERA_QUAD_COUNT entries of CAMERA_QUAD_SIZE bytes; the entry count is
// g_cameraState1664fe4, reset by Camera_Reset).
const int CAMERA_QUAD_COUNT = 64;
const int CAMERA_QUAD_SIZE = 0x200;
// Factor that turns the span between two opposite corners into the half diagonal.
const float CAMERA_QUAD_HALF = 0.5f;

// A corner of a camera quad.
struct CameraQuadPoint
{
    float x;    // 0x00: x
    float y;    // 0x04: y
};

// One camera quad: four corners and the half diagonal from the first corner to the third (and its negation).
struct CameraQuad
{
    CameraQuadPoint corner[4];      // 0x00: the four corners
    float halfDiagonalX;            // 0x20: (corner 2 x - corner 0 x) / 2
    float halfDiagonalY;            // 0x24: (corner 2 y - corner 0 y) / 2
    float negHalfDiagonalX;         // 0x28: -halfDiagonalX
    float negHalfDiagonalY;         // 0x2c: -halfDiagonalY
    char m_0030[CAMERA_QUAD_SIZE - 0x30];   // 0x30: not recovered
};
extern "C" CameraQuad g_cameraQuads[CAMERA_QUAD_COUNT];   // EN 0x01664fe8: the camera quads (g_cameraState1664fe4 are in use)

// Appends a camera quad from the corners (x0, y0), (x1, y1), (x2, y2), (x3, y3) and works out its half diagonal.
// Returns 0. EN 0x70a700 (EN-only, camera_quads.cpp). No direct caller recovered.
extern "C" int Camera_AddQuad(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3);

#endif
