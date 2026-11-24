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

// Collects the camera paths of move `moveId` into *pathCount. EN 0x744e40 (JP 0x1016ac90).
extern "C" int Camera_CollectPaths(int moveId, int moveIdAgain, int *pathCount);

// Builds the view and projection matrices for a camera at `eye` looking at `target`. EN 0x759300 (JP 0x10194360).
int Camera_SetMatrices(const Vec4 *eye, const Vec4 *target, Matrix4 *view, Matrix4 *projection, float roll);

// Positions the camera keeps in view (at most CAMERA_TRACKED_COUNT).
const int CAMERA_TRACKED_COUNT = 0x30;

extern "C" float *g_trackedPositions[CAMERA_TRACKED_COUNT];   // EN 0x0091f2b0 (JP DAT_1032d5b8): translation rows kept in view
extern "C" char g_trackedFlags[CAMERA_TRACKED_COUNT];         // EN 0x0091f410 (JP DAT_1021bbc0): a flag per tracked position
extern "C" int g_trackedCount;                                // EN 0x0091e76c (JP DAT_1049f0f0): entries of the two lists

// Adds character `index` position to the list of positions the camera keeps in view. Returns 1. EN 0x645730.
extern "C" int Camera_AddTrackedPosition(int index);


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

// Camera state cleared with the player (Player_ResetMotion) and the camera (Camera_Reset).
const int CAMERA_GROUND_UNKNOWN = -100;          // g_cameraGroundHeight value before a ground height was measured
extern "C" int g_cameraGroundHeight;             // EN 0x00908a20 (JP DAT_10203444): ground height cache, CAMERA_GROUND_UNKNOWN = unknown
extern "C" float g_cameraTargetLift;             // EN 0x01664678 (JP DAT_1101dde4): how far the camera target is lifted (0 or up to 120)
extern "C" float g_cameraEyePitch;               // EN 0x01664440 (JP DAT_1101de18): pitch the free camera lifts its eye by (radians, 0 = none)
extern "C" char g_cameraPushRightFrames;         // EN 0x0162ef62 (JP DAT_10fabe8c): frames the follow camera keeps pushing right
extern "C" char g_cameraPushLeftFrames;          // EN 0x0162ef63 (JP DAT_10faaa80): frames the follow camera keeps pushing left
extern "C" int g_cameraState1664448;             // EN 0x01664448 (JP DAT_1101d888): not recovered, cleared with the camera
extern "C" int g_cameraState1664fe0;             // EN 0x01664fe0 (JP DAT_1101d898): not recovered, cleared with the camera

// Zeroes the camera vectors and the per-frame camera state of a new player position. EN 0x70bc30 (JP 0x1016a8f0).
extern "C" int Player_ResetMotion(void);

#endif
