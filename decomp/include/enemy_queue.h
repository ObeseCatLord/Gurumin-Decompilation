// The enemy queue (enemy_busy.cpp): up to four enemies wait in a line (DAT_10c66cf4 at the front, then DAT_10c66cf0,
// DAT_10c66cec, DAT_10c66ce8; 0 = empty place). Each step an enemy moves up one place when the place ahead is free,
// else it follows the enemy ahead (FUN_100e0320). DAT_10c66cd8 is the enemy that leaves the line and fades out.
#ifndef GURUMIN_ENEMY_QUEUE_H
#define GURUMIN_ENEMY_QUEUE_H

extern "C" int DAT_10c66cf4;   // character slot at the front of the queue
extern "C" int DAT_10c66cf0;   // second place
extern "C" int DAT_10c66cec;   // third place
extern "C" int DAT_10c66ce8;   // fourth place
extern "C" int DAT_10c66cd8;   // enemy leaving the queue (fades out once it reaches its last position)
extern "C" int DAT_10c66cd4;   // frames until the front enemy starts to leave (0 = not counting)

#define QUEUE_FADE_FRAMES 20            // a leaving enemy fades out over this many frames
#define QUEUE_LEAVE_DISTANCE 50.0f      // it fades once it is this close (x/y) to its last recorded position
#define QUEUE_PARKED_X -200000.0f       // where a faded enemy is put
#define QUEUE_ROOT_KIND_MASK 0x400      // node kinds whose colour is changed while it fades
#define QUEUE_ROOT_KIND_MASK2 0x200
#define TALK_TARGET_NONE (-1)           // DAT_10c6351c when nobody is being talked to
#define TALK_TARGET_HELD 100            // DAT_10c6351c = slot + this: that character is held still
#define QUEUE_FREE_MAP 999              // on this map every enemy's AnimModel::mode is cleared each frame
#define QUEUE_KEEP_TARGET 10000         // DAT_104bf888 = slot + this: the slot is kept as a target

// 0x100e0320: moves enemy `index` along the queue behind enemy `ahead` (0 = it is at the front). Returns 0.
extern "C" int FUN_100e0320(int index, int ahead);
// 0x100e1c40 (enemy_busy.cpp): non-zero = skip the enemy's AI this frame.
extern "C" int Enemy_IsBusy(int index);

#endif
