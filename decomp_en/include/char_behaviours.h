// The NPC and enemy behaviours Char_RunBehaviour dispatches to (EN build), and the globals its test reads.
// Declarations of behaviours that no other header declares; each function is declared once in decomp_en.
// Functions that are not ported yet have their linkage guessed as C (JP: all of them are extern "C").
#ifndef GURUMIN_EN_CHAR_BEHAVIOURS_H
#define GURUMIN_EN_CHAR_BEHAVIOURS_H

// Non-zero while an event plays: only a few behaviours keep running. EN 0x009e3360 (JP DAT_104c0094).
extern "C" char g_eventPlaying;
// Non-zero while the event of maps 0x25c and 0x12d lets the listed behaviours run. EN 0x01335388 (JP DAT_10c655e0).
extern "C" int g_eventAllowsBehaviours;

// Behaviour 1: a guard that idles until it sees the player. EN 0x6b37c0 (JP 0x100b1520, npc_watch_player.cpp).
extern "C" int Npc_WatchForPlayer(int index);
// Behaviours 2, 4 and 0x15: `mode` 0 or 1 picks the variant (not recovered yet). EN 0x6b2950 (JP 0x100b2900).
extern "C" int Npc_ModeBehaviour(int index, int mode);
// Behaviour 8: watches the player at a route point, then walks to the next one. EN 0x6bc9b0 (JP 0x10044b10).
extern "C" int Npc_PatrolRoute(int index);
// Behaviour 9: walks a map route at the route speed, pauses, fades out at marked points, carries a rider. EN 0x6d4590 (JP 0x100451c0).
extern "C" int Npc_WalkRoute(int index);
// Behaviour 10: sits above the neighbour whose talk id it holds. EN 0x6d2d90 (JP 0x100b3770).
extern "C" int Npc_PerchOnNeighbour(int index);
// Behaviour 0xb: lookout that raises the alarm. EN 0x6cc8d0 (JP 0x100b4300).
extern "C" int Npc_RaiseAlarm(int index);
// Behaviour 0xc: lookout riding behind the nearest marker character. EN 0x6cc180 (JP 0x100b4930).
extern "C" int Npc_RideAlarm(int index);
// Behaviour 0xd: carrier on a gated route. EN 0x6ae170 (JP 0x1007b870).
extern "C" int Npc_GatedRouteCarrier(int index);
// Behaviour 0xf: a child that runs to the player. EN 0x6b4610 (JP 0x100b23b0, see char_move.cpp).
extern "C" int Npc_ChildFollow(int index);
// Behaviour 0x11: sleeper woken by sight or noise. EN 0x6bc630 (JP 0x10045830).
extern "C" int Npc_SleepUntilWoken(int index);
// Behaviour 0x13: bobs in place, pops out when the player comes near. EN 0x6aeda0 (JP 0x100b4f00, see char_move.cpp).
int Npc_PopUp(int index);
// Behaviour 0x17: a character near a link line is shocked. EN 0x6a32b0 (JP 0x100460d0).
extern "C" int Npc_ShockByLink(int index);
// Behaviour 0x1b: an enemy that appears near the player. EN 0x6d6a10 (JP 0x10046f10, see enemy_blink.cpp).
extern "C" int Enemy_AppearNearby(int index);
// Behaviour 0x1e: pushes a cart back and forth. EN 0x6a2c70 (JP 0x10047440).
extern "C" int Npc_PushCart(int index);
// Behaviour 0x29: turns to the player and fidgets. EN 0x6bbea0 (JP 0x100b7360).
extern "C" int Npc_GreetWhenFaced(int index);
// Behaviour 0x2a: lights the eye of the nearest lamp. EN 0x6d52c0 (JP 0x100b7b10).
extern "C" int Npc_Lookout(int index);
// Behaviour 0x2d: performs to its neighbour. EN 0x6af890 (JP 0x100b84a0).
extern "C" int Npc_PerformTricks(int index);
// Behaviour 0x2e: gestures with its neighbour. EN 0x6b0590 (JP 0x100b9150).
extern "C" int Npc_GestureWithNeighbour(int index);
// Behaviour 0x31: an enemy dragged into a pit. EN 0x6abe50 (JP 0x100b9930, see enemy_drag.cpp).
extern "C" int Enemy_DragToPit(int index);

// Runs the behaviour of character `index`: every 30 frames (staggered by index) a character that stands idle moves its
// shadow to the ground, then the behaviour number is unpacked on first use and its routine runs (while an event plays only
// behaviours 0x21, 10 and 0x24 on maps 0x25c and 0x12d). Unknown numbers reset the character's state. Returns 0. EN 0x6d1b40 (JP 0x100dfcf0).
extern "C" int Char_RunBehaviour(int index);

#endif
