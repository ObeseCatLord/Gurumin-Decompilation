// Character step scripts (char_script.cpp): a script is a map record (map_records.h) with one RoutePoint per step:
// x * 10000 + y, z * 10000 + extra, the step kind and the step's length (frames) or parameter.
#ifndef GURUMIN_CHAR_SCRIPT_H
#define GURUMIN_CHAR_SCRIPT_H

#define SCRIPT_STEP_FADE_IN_OUT 0     // fades in and back out over the step
#define SCRIPT_STEP_HIDDEN 1          // invisible for the step's length
#define SCRIPT_STEP_KEEP 2            // keeps the opacity; ends the script (back to step 0)
#define SCRIPT_STEP_FADE_IN 3         // fades in over SCRIPT_MIN_STEP_FRAMES frames
#define SCRIPT_STEP_FADE_OUT 4        // fades out over the step
#define SCRIPT_STEP_SHOW 5            // visible at once; starts the speaker's voice event (the step's length)
#define SCRIPT_MIN_STEP_FRAMES 30     // shortest step (and full opacity value)

// 0x10029cf0 (Char_RunScriptStep): runs step `step` of the script of character `index`.
extern "C" int FUN_10029cf0(int index, int *doneStep, int step, int *nextStep, Vec4 *position, int *kind, int *value,
                            int *extra);
// 0x10028fa0 (Spirit_StepRoute, spirit_route.cpp): moves the spirit `index` one step along its route (see there).
extern "C" int FUN_10028fa0(int index, int *lastPoint, int point, int *nextPoint, Vec4 *target, int *kind, int *value,
                            int *extra);
// 0x100286d0 (Char_StepRouteTarget, route_target.cpp): the route target of character `slot` heading for `point`; writes the next
// point, the target position, the point's kind, code and extra. Returns 1 at a hold, turn or wait point.
extern "C" int FUN_100286d0(int slot, int *lastPoint, int point, int *nextPoint, Vec4 *target, int *kind, int *value,
                            int *extra);
// 0x100286b0 (app_0012.cpp): wraps a packed route part into -4999..5000.
extern "C" int FUN_100286b0(int angle);
// 0x1001dbd0 (app_0012.cpp): sets parameter 5 of every NPC with `id` to `value` and restarts its behaviour.
extern "C" int FUN_1001dbd0(int id, int value);

#endif
