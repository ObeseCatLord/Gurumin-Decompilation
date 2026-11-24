// Camera modes run by the scene frame (FUN_100f5a80): each moves the camera target and position one step.
#ifndef GURUMIN_CAMERA_MODES_H
#define GURUMIN_CAMERA_MODES_H

extern "C" int FUN_10018870(void);   // Camera_PullToItemStand: orbits the player while it is drawn to the item stand (camera_item_stand_pull.cpp)
// 0x101b0cd0 (vector math): a zero matrix, returned by value (declared here, not in matrix4.h: adding it there
// changes the code of UpdateDrillLevel).
Matrix4 ZeroMatrix();
extern "C" int FUN_10019670(void);   // Camera_ShiftOrbit: a camera shift that turns about orbit centres (camera_shift_orbit.cpp)
struct Vec4;
extern "C" int FUN_10035050(Vec4 *player, Vec4 *target, int index);   // Camera_ChaseTarget: camera mode 0x11, follows the player and swings behind it (camera_chase.cpp)

extern "C" int FUN_100184e0(void);   // camera mode 9 (camera_follow.cpp)
extern "C" int FUN_10018fb0(void);   // camera mode 0xb (camera_follow.cpp)
extern "C" int FUN_10019210(void);   // camera mode 0xc (head_look.cpp)
extern "C" int FUN_10019310(void);   // camera mode 0xe (camera_follow.cpp)
extern "C" int FUN_10019d80(void);   // camera mode 0x10 (camera_follow.cpp)
extern "C" int FUN_1001a0d0(void);   // camera mode 0x12 (camera_follow.cpp)

#endif
