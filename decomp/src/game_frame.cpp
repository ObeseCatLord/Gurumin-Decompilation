// The in-game frame (game mode 1, called by the frame step): the first frame resets the mode's globals, then one pass
// of the game logic runs for every engine tick that has elapsed (input, the main menu, the player, the characters, the
// effects, the camera); the scene is prepared and drawn once at the end.

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "vec4.h"

#include "matrix4.h"

#include "node.h"

#include "anim_model.h"

#include "model_bank.h"

#include "char_slot.h"

#include "game.h"

#include "tracker.h"

#include "dinput_min.h"

#include "demo_recording.h"

#include "bgm_track.h"

#include "item_dictionary.h"

#include "message_window.h"

#include "field_item_menu.h"

#include "partner_follow.h"

#include "bs003.h"

#include "hud_frame.h"

#include "menu_main.h"

#include "camera_refresh_vars.h"

#include "scene_draw.h"

#include "npc_lookout.h"

#include "char_vars.h"

#include "char_tables.h"

#include "camera_vars.h"

#include "map_vars.h"

#include "ui_vars.h"

#include "render_vars.h"

#include "game_vars.h"

#include "sound_vars.h"

#include "script_system.h"

#include "map_setup.h"

#include "drill_claws.h"

#include "field_update.h"

#include "title_screen.h"

#include "engine_memory.h"

#include "frame_vars.h"

#include "world.h"

#include "game_load_data.h"
#include "debris.h"
#include "char_equipment.h"

#include "player_frame.h"

#include "action_select_vars.h"
#include "football_state.h"
#include "target_race.h"

#include "game_main.h"

#include "script_vars.h"

#include "hop_spit_enemy.h"

#include "node_collision_vars.h"

#include "shaga.h"

#include "texture_manager.h"
#include "shadow_ground.h"
#include "gimmick_slot.h"
#include "depth_sort.h"

#include "name_entry.h"

#include "doppler_loops.h"

#include "scene_command_ids.h"

#include "texture_load.h"

#include "mesh_lit_vars.h"

#include "map_trigger_setup.h"

#include "tutorial_book.h"

#include "ground_height.h"

#include "ground_mesh.h"

#include "scene_callback.h"

#include "texture_object.h"

#include "mmv_player.h"

#include "dshow_stream.h"

#include "char_hikari.h"

#include "menu_save_load.h"

#include "map_placement.h"

#include "item_info.h"

#include "item_use_texts.h"

#include "game_frame.h"

// Hidden-part loop, light shafts: DAT_10f65848 holds SHAFT_COUNT shafts (x, y offsets and w opacity).
#define SHAFT_COUNT 2
#define SHAFT_LIMIT 2500.0f
#define SHAFT_FADE 0.05f
#define SHAFT_SPEED 20.0f
#define SHAFT_RESTART 0x5dc
// Effect holds that start an effect with a fire flag.
#define HOLD_VALUE_FIRE_A 0x10
#define HOLD_VALUE_FIRE_B 0x50
// Frames the HUD stays shown after one of its triggers.
#define HUD_HOLD_FRAMES 0x96
// Rain effect in the weather box (mode 2).
#define RAIN_EFFECT 0x82dc
// Levers ("@kskNN+") and their parts.
#define KSK_COUNT 0x40
#define KSK_PARTS 6
// Fish (DAT_10210f38): FISH_MAX swim, the other nodes up to FISH_NODE_COUNT are hidden far away.
#define FISH_MAX 8
#define FISH_NODE_COUNT 0x20
#define FISH_HIDDEN_X -200000.0f
#define FISH_START_Y 2000.0f
#define FISH_DEPTH_MIN 260
#define FISH_DEPTH_MAX 400
#define FISH_FLEE_START 250.0f
#define FISH_FLEE_END 400.0f
#define FISH_ARRIVE_DISTANCE 10.0f
#define FISH_SWIM_SPEED 5.0f
#define FISH_FLEE_SPEED 10.0f
#define FISH_WAVE_FRAMES 40
#define FISH_RADIUS 20.0f
#define FISH_TURN_TRIES 10
#define FISH_TURN_SMALL 0.785375f
#define FISH_TURN_LARGE 1.57075f
#define FISH_COURSE_KEEP 0.8f
#define FISH_COURSE_STEER 0.2f
#define FISH_PI 3.1415f
// The swing (DAT_104c0240): seats within SWING_REACH of the player (2D), beat of SWING_BEAT frames in two halves.
#define SWING_MIN_HEIGHT 5.0f
#define SWING_REACH 150.0f
#define SWING_START_POWER 13
#define SWING_LEVEL_STEP 10
#define SWING_BEAT 0x60
#define SWING_HALF_BEAT 0x30
#define SWING_SAVE_POWER_BEAT 0x28
// The player's scene state DAT_104c00d0: ranges and the steps inside them.
#define SCENE_TIMELINE_LIMIT 0xc9       // 2..200: timeline counting down from SCENE_TIMELINE_START to 1
#define SCENE_BLANK_LIMIT 1000          // 201..999 do nothing
#define SCENE_SPARK_LIMIT 2000          // 1000..1999: spark effect at the player
#define SCENE_HAND_LIMIT 5000           // 2000..4999: the left hand glows
#define SCENE_DAMAGE_LIMIT 0x2711       // 5000..10000: damage state; 10001..19999 do nothing
#define SCENE_SHOVE_WAIT 20000          // waits for the player to stand still, then becomes SCENE_SHOVE
#define SCENE_SHOVE 0x4e21              // the player is shoved aside
#define SCENE_SWAN_WAIT 30000           // starts the swan ride, then becomes SCENE_SWAN
#define SCENE_SWAN 0x7531
#define SCENE_TIMELINE_START 200
#define SCENE_TIMELINE_EFFECT 0xaa
#define SCENE_TIMELINE_VOICE 0x74
#define SCENE_TIMELINE_SHADE 0x6e
#define SCENE_TIMELINE_RELEASE 0xc6
#define SCENE_SPARK_START 0x3ec
#define SCENE_SPARK_END 0x41b
#define SCENE_SPARK_FINISH 1000
#define SCENE_HAND_START 0x7e3
#define SCENE_HAND_END 0x806
#define SCENE_HAND_FINISH 2000
#define SCENE_DAMAGE_TICK 0x13a6
#define SCENE_DAMAGE_RECOVER 0x13c4
#define SCENE_DAMAGE_END 0x13a0
// The second player model (a double of the player, shown unless the costume has three pointers).
#define SCENE_DOUBLE_MODEL 124
#define SCENE_HIDDEN_X -200000.0f
#define COSTUME_THREE_POINTERS 6
// Face bits of the worn helmet (DAT_104c02c8) that scale the effect strength, and the damage bits.
#define HELMET_FACE_400 0x400
#define HELMET_FACE_300 0x300
#define HELMET_FACE_200 0x200
#define HELMET_FACE_100 0x100
#define HELMET_NO_DAMAGE 0x8000000
#define HELMET_HALF_DAMAGE 0x4000000
// The shove: four directions of length SHOVE_SIDE sideways and SHOVE_DOWN down.
#define SHOVE_SIDE 10.0f
#define SHOVE_DOWN -5.0f
// The swan ride: frames before it ends, the player's heading offset to the swan and the sound period.
#define SWAN_RIDE_FRAMES 200
#define SWAN_HEADING_OFFSET 1.57075f
#define SWAN_SOUND_PERIOD 0xf
#define PLAYER_TRAIL_LENGTH 16
// Most engine ticks run in one frame; a longer backlog (or a halted engine) runs a single tick.
#define MAX_CATCH_UP_TICKS 20
// Menu windows closed when a queued event interrupts the open overlay mode.
#define WINDOW_ID_SAVE_MENU 0x22c
#define WINDOW_ID_CHOICE_MENU 0x218
#define WINDOW_ID_YES_NO 0x216
// Texture slots of the HUD (DAT_10202808) released by the first frame.
#define HUD_TEXTURE_SLOT_COUNT 15
// Texture slots of the overlay mode (DAT_1030b870).
#define OVERLAY_TEXTURE_SLOT_COUNT 0x22
// Characters whose models the frame updates (DAT_10db6ee8[0..MODEL_UPDATE_COUNT-1]).
#define MODEL_UPDATE_COUNT 124
#define FADE_OUT_FLOOR_HEIGHT 240.0f    // a fading model never rises above its slot's spot plus this
#define FADE_OUT_DIVISOR 6.0f           // DAT_10467f28 (the model's fade strength) scale
#define FADE_OUT_BASE 0.5
#define FADE_OUT_FRAMES 15.0            // frames the colour takes to dim
#define ENEMY_SPARK_PERIOD 10           // frames between the sparks of a hurt enemy
#define ENEMY_SPARK_BURST_PERIOD 0x3c
#define ENEMY_SPARK_OWNER 0x1226
#define EFFECT_ENEMY_SPARK 0x65
#define EFFECT_ENEMY_SPARK_BURST 0x226
#define EFFECT_ENEMY_SMOKE 0x868
#define HIT_FLASH_RISE 4.0f             // DAT_10f08080 z: the flashing enemy's offset rises by this each frame
#define HIT_FLASH_PULSE_DIVISOR 2.0f
#define HIT_FLASH_FRAMES 20.0
#define HIT_FLASH_STEP 0.4f
#define HIT_FLASH_MAX 1.3f
#define HIT_FLASH_START 0x14
#define ENEMY_GONE 2
#define ENEMY_DEFEATED 3                // the enemy has no health left; its death is handled this step
#define ENEMY_FADE_START 4              // states 5..13 fade the model out
#define ENEMY_FADE_END 14
#define FADE_ALPHA_DIVISOR 10.0f
#define GONE_X -200000.0f               // a gone enemy's model waits here (minus GONE_SPACING per slot)
#define GONE_SPACING 500.0f
#define FIRST_ENEMY_SLOT 0x3a
#define TOSS_LAST_TARGET_SLOT 0x5c
#define TOSS_TARGET_RANGE 3000          // the defeated enemy is thrown at another one within this distance
#define TOSS_TARGET_KIND_A 0x63
#define TOSS_TARGET_ALIGNMENT 0.7f      // dot product the direction to the target needs with the player's view
#define TOSS_NUDGE 0.1f
#define TOSS_HOLD_WEIGHT 0.1f
#define TOSS_CARRIER_LIMIT 200
#define DEFEATED_FROM_ABOVE (-0x65)     // DAT_10467ba0 marks of the way the enemy was defeated
#define DEFEATED_BURST (-100)
#define COMBO_MAX 99
#define COMBO_SHOW_A 8
#define COMBO_SHOW_B 0x3c
#define SOUND_ENEMY_DEFEATED 0x37a
#define COIN_BONUS_FACTOR 1.5f
#define COIN_FLAGS_BOTH 0x30000000      // DAT_104c02c8 bits of the helmet: both coin bonuses
#define COIN_FACTOR_BOTH 3.0f
#define COIN_FLAG_DOUBLE 0x20000000
#define COIN_FLAG_BONUS 0x10000000
#define COIN_GAME_OVER_LIMIT 5          // the coins drop by a fifth for each game over, down to the minimum
#define ENEMY_DYING 1                   // DAT_10210d30 states of an enemy slot
#define ENEMY_TOSSED 1000               // defeated and flying away
#define TOSS_STEPS_PER_FRAME 2
#define TOSS_SWEEP_COUNT 5              // directions tried to slide along
#define TOSS_DROP 0.1                   // the first direction points this much lower
#define TOSS_TURN 0.5236f               // the side directions are turned by this angle
#define TOSS_REACH 19.0f                // distance a toss step moves
#define TOSS_WALL_RADIUS 50.0f          // first radius of the wall test (grows by TOSS_WALL_RADIUS_STEP)
#define TOSS_WALL_RADIUS_STEP 10.0f
#define TOSS_STOP_SPEED 0.1             // the toss ends when every component of the direction is below this
#define TOSS_CEILING 6000.0f
#define TOSS_FLOOR_X -100000.0f
#define TOSS_PIECE_LIFT 50.0f
#define TOSS_SPIN 0.5f
// A prop the player talks to (slot kind 'E'): talk distance (squared), the phases of its state, the steps and sizes of
// its moves.
#define PROP_TALK_DISTANCE_SQUARED 1600.0f
#define PROP_PHASE_IDLE 100
#define PROP_FORWARD_FIRST 0xc9         // phases 201..300: steps forward
#define PROP_FORWARD_END 300
#define PROP_BACK_FIRST 0x12d           // phases 301..400: backs away
#define PROP_BACK_END 400
#define PROP_STEP 12.0f                 // height change per frame of a step
#define PROP_LEAN_SCALE 0.2f            // the push away from the player per frame
#define PROP_LEAN_TILT 0.005f           // tilt of the first child per unit of push
#define PROP_SPOT_OFFSET 500.0f         // the two spots beside it where the player starts a step
#define PROP_SPOT_RADIUS 150.0f
#define PROP_TALK_RISE_END 0x5a         // after the first talk it moves aside until here
#define PROP_TALK_END 0x5c
#define PROP_TALK_STEP_X 10.0f
#define PROP_TALK_STEP_Z 8.0f
// Swaying props (slot kinds 'B', 'z', 'P', ','): the sway's angle per frame left, its unit, and the divisors.
#define SWAY_FRAME_ANGLE 0.5f
#define SWAY_ANGLE_UNIT 1000.0f
#define SWAY_DIVISOR_FLAG -450.0f
#define SWAY_DIVISOR_POT 130.0f
#define FLAG_LIFT 20.0f                 // the effect of kind 'B' starts this far above the pit floor
#define FLAG_RISE_STEP 50.0f
#define FLAG_FALL_END 0x78              // phase at which the flag has gone up far enough to be dropped
#define FLAG_DROP_HEIGHT -1050.0f
#define FLAG_PROBE_HEIGHT 10.0f
#define FLAG_PHASE_RESET 0xdc
#define POT_SMASH_KIND 0x86             // the slot kind a smashed pot becomes
#define POT_SMASH_SOUND 0x33b
#define POT_SMASH_EVENT 0x1f1
#define POT_HIDE_X -250000.0f           // the smashed pot's model is parked far away
#define LAMP_TURN_OFFSET 45.0f          // kind ',': the effect stands this far in front of the player
#define LAMP_HEADING_SHIFT 1.57075f
#define LAMP_FIRE_PERIOD 0x5a
// Props the player can pick up (slot kinds 5, 6, '_'), floor buttons ('M') and markers ('\\').
#define PICKUP_REACH 300.0f             // the prop is a candidate within this distance
#define PICKUP_NEAR 150.0f              // and can be picked up within this one
#define PICKUP_HEIGHT 40.0f
#define PICKUP_AHEAD 30.0f              // the point ahead of the player the closest prop is chosen for
#define PICKUP_HEADING_SHIFT 1.57075f
#define PICKUP_GAP_MIN 40.0f            // m_0267: the prop hangs 40..140 above the player
#define PICKUP_GAP_MAX 140.0f
#define PICKUP_STAND_RADIUS 90.0f
#define PICKUP_CARRY_LINKS 16
#define PROP_SNAP_STEP 75.0f
#define PROP_SHADOW_PERIOD 0x1e          // the shadow ground check runs every 30th frame (staggered by slot % 10)
#define PROP_SHADOW_STAGGER 10
#define SWAY_FRAME_ANGLE_SLOW 0.3f
#define SWAY_DIVISOR_STALL 430.0f
#define SWAY_DIVISOR_SIGN 230.0f
#define BUTTON_TALK_DISTANCE_SQUARED 400.0f
#define BUTTON_LEAVE_DISTANCE_SQUARED 3600.0f
#define BUTTON_SOUND_DOWN 0x37b
#define BUTTON_SOUND_UP 0x37c
#define SPOUT_AHEAD 40.0f               // kind 2: the point ahead of the player that the model is tested against
#define SPOUT_REACH 100.0f
#define MARKER_FIRST_SLOT 0x5c          // kind '\\': markers are the slots 0x5c..0x7b
#define MARKER_LAST_SLOT 0x7b
#define MARKER_CLEAR_DISTANCE_SQUARED 2025.0f
#define MARKER_CLOSED_FRAME 0x1e
#define BUTTON_DIM 0.7f                 // the button's scale while it is not at rest
#define ENEMY_DISTANCE_START 99999.0f   // closestEnemy before any enemy was measured
// Tracks below this number drive the beat counter DAT_1032d6a0.
#define BGM_TRACK_LIMIT 0xff
// Beat counter: BGM_BEAT_STEPS steps per BGM_BEAT_SAMPLES / BgmTrack::m_002c samples.
#define BGM_BEAT_STEPS 15
#define BGM_BEAT_SAMPLES 0x285ff0
// The model of the second character (the partner), recorded in the demo.
#define SECOND_MODEL 75
// While model NEAR_MODEL is within NEAR_MODEL_DISTANCE of the player DAT_104c02f0 is 1.
#define NEAR_MODEL 16
#define NEAR_MODEL_DISTANCE 300.0f
// Recorded demo: the playback position at which it ends, and the music fades started on a desync or at the end.
#define DEMO_END_FRAME 0x3d22
#define DEMO_DESYNC_FADE_FRAMES 0xf
#define DEMO_END_FADE_FRAMES 0x1e
#define BGM_VOLUME_FULL 100
// The sun's lens flare is only added when DAT_10467798 is above this.
#define LENS_FLARE_MIN_COUNT 2

// Closes the event overlay mode if it is open (windows, caption, book model and its textures). The scene states
// run this code at the top of each timeline.
static __forceinline void CloseEventOverlay(void)
{
    int slot;
    if (DAT_104bfe1c != 0)
    {
        DAT_1030c54c = 0;
        DAT_104bfd78 = 0;
        DAT_104b7200.CloseWindow(WINDOW_ID_SAVE_MENU);
        DAT_104b7200.CloseWindow(WINDOW_ID_CHOICE_MENU);
        DAT_104b7200.CloseWindow(WINDOW_ID_YES_NO);
        DAT_1020e0f8[0] = 0;
        FUN_1000cb30("");
        FUN_1000cb90("");
        DAT_104bf8d4 = 1;
        if (DAT_104c03c8 != 0)
        {
            DAT_104c03dc = 0;
            DAT_104c03c8->Release();
            DAT_11b1ba40(DAT_104c03c8);
            DAT_104c03c8 = 0;
            DAT_104c03cc = 0;
            DAT_104c03d0 = 0;
            DAT_104c03d4 = 0;
            DAT_104c03d8 = 0;
            DAT_104c03e0 = 0;
            DAT_104c03e4 = 0;
            for (slot = 0; slot < OVERLAY_TEXTURE_SLOT_COUNT; slot++)
            {
                if (DAT_1030b870[slot] > -1)
                {
                    FUN_10174c70(DAT_1030b870[slot], 1);
                    DAT_1030b870[slot] = -1;
                }
            }
        }
        DAT_104b7200.CloseWindow(WINDOW_ID_CHOICE_MENU);
        DAT_104b7200.CloseWindow(WINDOW_ID_YES_NO);
        memset(DAT_1030c500, 0, sizeof(DAT_1030c500));
        DAT_1020e0f8[0] = 0;
        DAT_104bfe1c = 0;
    }
}

// Starts the player's talk with a prop that pulls the player in: the player turns, the event overlay closes, and
// the player's offset to the prop is stored (here ends up holding the player's position).
static __forceinline void StartPropPull(int slot, Vec4 &here)
{
    DAT_10db6ee8[0].PlayMotion(2, 1, 5, 1.0f);
    FUN_100114a0();
    DAT_104c0068 = 1;
    CloseEventOverlay();
    DAT_104c006c = slot;
    here.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    DAT_10daaad4.Copy(DAT_10db6ee8[0].matrix.GetTranslation() - here);
}
// Sways a prop: while `frames` lasts it counts down and tilts the matrix about x by sin(frames * frameAngle) *
// frames / divisor, turned by param / 1000 radians about z.
static __forceinline void SwayProp(Matrix4 &sway, int &frames, int param, float divisor, float frameAngle)
{
    sway = RotationX((float)sin(frames * frameAngle) * frames / divisor);
    frames--;
    sway = RotationZ(-(float)param / SWAY_ANGLE_UNIT) * sway * RotationZ((float)param / SWAY_ANGLE_UNIT);
}

// Offers model `slot` (a movable prop) as the object the player picks up next: when it is the closest one to a point
// just ahead of the player and within reach, DAT_104bfdd8 takes its index and DAT_104bfdd0 the side the player
// stands on (1 = along x, 2 = along y).
static __forceinline void OfferPickup(int slot)
{
    Vec4 propAt = DAT_10db6ee8[slot].matrix.GetTranslation();
    float distance = DAT_10db6ee8[0].matrix.GetTranslation().Distance(propAt);
    if (DAT_104bfdc8 == 0 && distance < PICKUP_REACH)
    {
        float apartX = (float)fabs(DAT_10db6ee8[0].matrix.GetTranslation().x - DAT_10db6ee8[slot].matrix.GetTranslation().x);
        float apartY = (float)fabs(DAT_10db6ee8[0].matrix.GetTranslation().y - DAT_10db6ee8[slot].matrix.GetTranslation().y);
        float apartZ = (float)fabs(DAT_10db6ee8[0].matrix.GetTranslation().z - DAT_10db6ee8[slot].matrix.GetTranslation().z);
        Vec4 ahead = DAT_10db6ee8[0].matrix.GetTranslation();
        ahead.x = (float)cos(-DAT_10db6ee8[DAT_104bf8b4].targetHeading - PICKUP_HEADING_SHIFT) * PICKUP_AHEAD + ahead.x;
        ahead.y = (float)sin(-DAT_10db6ee8[DAT_104bf8b4].targetHeading - PICKUP_HEADING_SHIFT) * PICKUP_AHEAD + ahead.y;
        float gap = DAT_10db6ee8[slot].matrix.GetTranslation().Distance(ahead);
        if (gap < DAT_102027e4)
        {
            DAT_102027e4 = gap;
            if (distance < PICKUP_NEAR && apartZ < PICKUP_HEIGHT)
            {
                if ((double)apartX <= (double)apartY + (double)apartY)
                {
                    if ((double)apartY <= (double)apartX + (double)apartX)
                        return;
                    DAT_104bfdd0 = 2;
                }
                else
                    DAT_104bfdd0 = 1;
                DAT_104bfdd8 = slot;
                DAT_104c0074 = 0;
            }
        }
    }
}

// Sets m_0267 of model `slot`: 1 while the player stands within PICKUP_STAND_RADIUS of it and 40..140 units below it.
static __forceinline void MarkPlayerUnderProp(int slot)
{
    Vec4 playerAt = DAT_10db6ee8[0].matrix.GetTranslation();
    Vec4 propAt = DAT_10db6ee8[slot].matrix.GetTranslation();
    if (propAt.z <= playerAt.z + PICKUP_GAP_MIN || playerAt.z + PICKUP_GAP_MAX <= propAt.z
        || playerAt.Distance2D(propAt) >= PICKUP_STAND_RADIUS)
        DAT_10db6ee8[slot].m_0267 = 0;
    else
        DAT_10db6ee8[slot].m_0267 = 1;
}

// Follows the model that carries model `slot` (up to 16 links): returns 1 when it is DAT_104bfdd8, the model the
// player holds. (The original re-reads the carrier of the model itself on each link.)
static __forceinline int IsCarriedByHeld(int slot)
{
    int carried = 0;
    if (DAT_104bfdd8 > 0)
    {
        AnimModel *link = &DAT_10db6ee8[slot];
        int links = 0;
        do
        {
            links++;
            if (link->m_2418 == 0)
                break;
            link = DAT_10db6ee8[slot].m_2418;
            if (&DAT_10db6ee8[DAT_104bfdd8] == link)
            {
                carried = 1;
                break;
            }
        } while (links < PICKUP_CARRY_LINKS);
    }
    return carried;
}

// Puts model `slot` on the ground below it: it is snapped down by PROP_SNAP_STEP and given half of the horizontal
// move as its step, then put back where it was.
static __forceinline void SnapPropToGround(int slot)
{
    Vec4 before = DAT_10db6ee8[slot].matrix.GetTranslation();
    DAT_10db6ee8[slot].SnapPosition(PROP_SNAP_STEP, 0);
    Vec4 after = DAT_10db6ee8[slot].matrix.GetTranslation();
    DAT_10db6ee8[slot].SetMoveStep((after.x - before.x) * 0.5f, (after.y - before.y) * 0.5f, 0);
    DAT_10db6ee8[slot].matrix.SetTranslation(before);
}
// Fills the ring of recent player positions with the current one.
static __forceinline void ResetPlayerTrail(void)
{
    DAT_104c0228 = 0;
    DAT_104c022c = 0;
    DAT_104c022d = 0;
    for (int step = PLAYER_TRAIL_LENGTH; step != 0; step--)
    {
        DAT_10802ad8[DAT_104c022c].Copy(DAT_10db6ee8[0].matrix.GetTranslation());
        DAT_104c022c = DAT_104c022c + 1 & 0xf;
    }
}

// Releases the model of the item book and its textures (the overlay mode's pages).
static __forceinline void ReleaseBookModel(void)
{
    int slot;
    if (DAT_104c03c8 != 0)
    {
        DAT_104c03dc = 0;
        DAT_104c03c8->Release();
        DAT_11b1ba40(DAT_104c03c8);
        DAT_104c03c8 = 0;
        DAT_104c03cc = 0;
        DAT_104c03d0 = 0;
        DAT_104c03d4 = 0;
        DAT_104c03d8 = 0;
        DAT_104c03e0 = 0;
        DAT_104c03e4 = 0;
        for (slot = 0; slot < OVERLAY_TEXTURE_SLOT_COUNT; slot++)
        {
            if (DAT_1030b870[slot] > -1)
            {
                FUN_10174c70(DAT_1030b870[slot], 1);
                DAT_1030b870[slot] = -1;
            }
        }
    }
}

// The first half of the sequences that close the item list: the dialogs, the caption and the book model.
static __forceinline void CloseBookModel(void)
{
    DAT_1030c54c = 0;
    DAT_104bfd78 = 0;
    DAT_104b7200.CloseWindow(WINDOW_ID_SAVE_MENU);
    FUN_100106a0();
    FUN_1000cb30("");
    FUN_1000cb90("");
    DAT_104bf8d4 = 1;
    ReleaseBookModel();
}

// The whole sequence that closes the item list (dialogs, caption, book model, state).
static __forceinline void CloseItemList(void)
{
    DAT_1030c54c = 0;
    DAT_104bfd78 = 0;
    DAT_104b7200.CloseWindow(WINDOW_ID_SAVE_MENU);
    DAT_104b7200.CloseWindow(WINDOW_ID_CHOICE_MENU);
    DAT_104b7200.CloseWindow(WINDOW_ID_YES_NO);
    DAT_1020e0f8[0] = 0;
    FUN_1000cb30("");
    FUN_1000cb90("");
    DAT_104bf8d4 = 1;
    ReleaseBookModel();
    DAT_104b7200.CloseWindow(WINDOW_ID_CHOICE_MENU);
    DAT_104b7200.CloseWindow(WINDOW_ID_YES_NO);
    DAT_104bfe1c = 0;
    DAT_1020e0f8[0] = 0;
    memset(DAT_1030c500, 0, sizeof(DAT_1030c500));
}

// Item list (overlay mode) with the mouse: the cursor area, rows of the list and the corner that opens it.
#define ITEM_MENU_LEFT 0x1e4
#define ITEM_MENU_TOP -0x52
#define ITEM_MENU_RIGHT 0x286
#define ITEM_MENU_BOTTOM 0x1ca
#define ITEM_MENU_ROW_TOP 0x4f
#define ITEM_MENU_ROW_HEIGHT 0x16
#define ITEM_MENU_ROWS 0xf
#define ITEM_LIST_CORNER 0x40f
#define SOUND_ITEM_LIST_OPEN 0x361
#define SOUND_ITEM_LIST_CLOSE 0x362
// Camera turn of one wheel notch and of a key press (radians per tick of the key's strength).
#define CAMERA_WHEEL_TURN 0.314f
#define CAMERA_KEY_TURN 8.0f
// Mouse look: the cursor's distance from the player (divided) is the steering, limited to AIM_LIMIT.
#define AIM_X_DIVISOR 16.0f
#define AIM_Y_DIVISOR 12.0f
#define AIM_LIMIT 15.0f
// DAT_104c0228: at or above TURN_STATE_BASE the character waits for a click behind it (TURN_PENDING) or has been refused.
#define TURN_STATE_BASE 10000
#define TURN_PENDING 0x271a
#define TURN_REFUSED 0x271b
#define TURN_AROUND_DOT -0.4f
#define HALF_PI 1.57075f
// Pad stick: history length, the flick (jump) detector, steering and the page switch limit.
#define PAD_HISTORY_LENGTH 10
#define PAD_FLICK_PEAK 0.9f
#define PAD_FLICK_ALIGN 0.5
#define PAD_FLICK_LOW_DIVISOR 10.0f
#define PAD_STEER_DIVISOR 10.0f
#define PAD_PAGE_LIMIT 50.0
#define STEER_FULL 16.0f
#define SOUND_ITEM_LIST_MOVE 0x363

// Lock-on: models LOCK_FIRST_MODEL..DOOR_FIRST_MODEL - 1 are the targets; a target far away at x below TARGET_GONE_X is gone.
#define LOCK_FIRST_MODEL 0x3a
#define TARGET_GONE_X -100000.0f
#define TARGET_SEARCH_START 4000000.0f

// Doors (switch models of kind 0x13/0x14, models DOOR_FIRST_MODEL to DOOR_END_MODEL - 1): animation frames of
// DAT_1020b128 that start a motion of the door's node tree.
#define DOOR_FIRST_MODEL 0x5c
#define DOOR_END_MODEL 0x7c
#define DOOR_FRAME_START 1
#define DOOR_FRAME_SETTLE 0x19
#define DOOR_FRAME_OPENED 0x1f
#define DOOR_FRAME_OPEN_LOOP 0x37
#define DOOR_FRAME_CLOSING 0x3d
#define DOOR_FRAME_CLOSE_SETTLE 0x55
#define DOOR_FRAME_CLOSED 0x5b
#define DOOR_FRAME_END 0x73
#define DOOR_SWING_EFFECT 0xfbe
// Message fade steps.
#define MESSAGE_FADE_MAX 0xf
#define MESSAGE_SLIDE_FULL 0x1e
// Dig (DAT_104bfd60): frames of a short dig; a long dig (DAT_104bfd64) lasts DIG_LONG_FRAMES per level.
#define DIG_LONG_FRAMES 600
#define DIG_START_FRAME 9
#define DIG_END_FRAME 0xc
#define DIG_DONE_FRAME 0x14
#define DIG_ANGLE_OFFSET 1.57075f
#define DIG_HEADING (-DAT_10db6ee8[DAT_104bf8b4].targetHeading - DIG_ANGLE_OFFSET)
#define DIG_REACH 40.0
#define DIG_RADIUS_SHORT 0x78
#define DIG_RADIUS_LONG 0x32
#define DIG_SHADE_OFFSET 30.0
#define DIG_SHADE_SIZE 150.0
// Event markers: projected to the 640x480 screen; they live EVENT_MARKER_FRAMES frames.
#define EVENT_MARKER_COUNT 0x80
#define EVENT_MARKER_FRAMES 0x1f
#define MARKER_HEAD_LIFT 25.0f
#define MARKER_OFFSCREEN_X -50000.0f
#define SCREEN_HALF_WIDTH 320.0f
#define SCREEN_HALF_HEIGHT 240.0f
#define SCREEN_HEIGHT 480.0f
// Game over (DAT_104bfe78 counts its frames).
#define GAME_OVER_FADE_FRAME 0x32
#define GAME_OVER_SOUND_FRAME 0x3c
#define GAME_OVER_END_FRAME 0x83
#define GAME_OVER_END_FRAME2 0x78
#define GAME_OVER_SKIP_FRAME 0x3a
#define GAME_OVER_RESTART_FRAME 0x8c
// Walking, locking on and the other input actions (stage 4).
#define INPUT_HISTORY_LAST 29        // DAT_1042dda8 keeps the last 30 input codes
#define RIDING_HEIGHT_MARGIN 80.0f   // the player is high above the riding floor when higher than this
#define PROBE_FAR 30.0f              // floor probes ahead of the player (walk start and stop)
#define PROBE_NEAR 20.0f
#define SOUND_WALK_START 0x35c
#define SEAT_PUSH_FRAMES 0x78        // frames before the end of a seat animation in which a push counts
#define SEAT_PUSH_KICK (-28.0f)
#define LOCK_REACH 150.0f            // camera positions within this distance of the player (or of the point ahead) count
#define LOCK_RANGE 350.0f
#define LOCK_HEIGHT 200.0
#define LOCK_TAG_INDEX_BASE 100      // nearTag = index + 100: the near position of kind 1 or 2 at that index
#define LOCK_TAG_KIND_BASE 200       // nearTag = kind + 200: the near position has a kind above 2
#define LOCK_SPARKLE_EFFECT 0x26ff
#define SETTLE_MOTION_SPEED 35.0f
#define LOCK_NEAREST_START 100000.0f // distance a lock candidate must beat
#define LOCK_AHEAD 60.0f             // the lock search point lies this far ahead of the player
#define LOCK_KEEP_HEIGHT 0x1e0       // the locked target stays while it is within this height of that point
#define LOCK_BASE_REACH 0x15e        // reach added to the target's own value (DAT_102192c0) and the player's collider reach
#define LOCK_REACH_HEIGHT 480.0
#define ITEM_USE_TIME_LIMIT 10000    // DAT_104c0228 at or above this: items cannot be used
#define HELD_LIGHT_RAISE 60.0f       // the player's matrix is raised this much for the held item's effects
#define HELMET_KIND_RED 0x1a         // ItemInfo::helmetKind values with their own light colour
#define HELMET_KIND_BLUE 0x1c
#define HELMET_KIND_WHITE 0x1d
#define FocusPoint(index) ((Vec4 *)DAT_1032d5b8[index])   // the camera position as a Vec4 (the records are 16 bytes)
// The greeting and talk block: the attack records' effects started while a swing plays (pseudo 5073+).
#define DRILL_SCALE_1 1.5f           // effect scale of drill levels 1..3 (level 0 = 1.0)
#define DRILL_SCALE_2 2.0f
#define DRILL_SCALE_3 3.0f
#define SWING_EARLY_FRAMES 0x18      // an attack swing's first frames start the held-item flash
#define SWING_FLASH_INTERVAL 7
#define ATTACK_AREA_SWORD_0 0x1c     // attack kinds whose effect is thrown along the nearest enemies
#define ATTACK_AREA_SWORD_1 0x1d
#define ATTACK_AREA_SWORD_2 0x1f
#define ATTACK_AREA_SWORD_3 0x20
#define ATTACK_AREA_BEAM 0x1a        // the attack record that DAT_108bb4b8 belongs to
#define ITEM_KIND_SWORD_LONG 3       // ItemText::m_0074 values that hit every enemy in reach
#define ITEM_KIND_SWORD 2
#define SWORD_HIT_RANGE 2400.0f
#define SWORD_EFFECT_RAISE 50.0f
#define SWORD_EFFECT_MAX_HITS 4
#define SWORD_EXTRA_RADIUS_MIN 200
#define SWORD_EXTRA_RADIUS_SPREAD 400
#define SWORD_EXTRA_RAISE 60.0f
#define SWORD_EFFECT_OWNER 9999
#define EFFECT_STRENGTH_DIVISOR_LARGE 1000.0f
#define EFFECT_STRENGTH_DIVISOR 100.0f
#define EFFECT_STRENGTH_THROWN 1.5f  // multiplier of the thrown (DAT_104c0173) strength
#define LOADOUT_COSTUME_A 0xc        // costumes and helmet bits that scale the effect (same as the claws)
#define LOADOUT_COSTUME_B 0xb
#define LOADOUT_COSTUME_C 8
#define LOADOUT_HELMET_STRONG 0x400
#define LOADOUT_HELMET_MID 0x300
#define LOADOUT_HELMET_HIGH 0x200
#define LOADOUT_HELMET_LOW 0x100
#define LOADOUT_SCALE_A 5.0f
#define LOADOUT_SCALE_B 3.0f
#define LOADOUT_SCALE_C 2.5f
#define LOADOUT_SCALE_MID 1.7f
#define LOADOUT_SCALE_HIGH 1.5f
#define LOADOUT_SCALE_LOW 1.2f
#define EFFECT_SETTING_AREA 5        // EffectOptions::settings[5]
#define EFFECT_SETTING_FLAGS 4       // EffectOptions::settings[4]
#define HELMET_THIRD_A 0x2c         // worn helmets that boost the third effect
#define HELMET_THIRD_B 0x3d
#define SWING_TURN_FRAMES 8          // the swing turns towards its target in the first frames
#define TURN_NEAREST_START 4000000.0f // squared distance a turn target must beat
#define SCOPE_EFFECT 35000
#define BURST_EFFECT 34000
#define BURST_EFFECT_POPON 0x84d1
#define SOUND_STRONG_SWING 0x365
#define SOUND_SWING_MODE 0x366
#define SWING_PROBE_NEAR_Y (-38.0f)
#define SWING_PROBE_NEAR_Z 20.0f
#define SWING_PROBE_FAR_Y (-30.0f)
#define SWING_PROBE_FAR_Z 80.0f
#define SWING_CAMERA_FRAMES 0x12
#define SWING_DROP_DIVISOR 4.0f
#define SWING_FADE_STEP 0.1f
#define SWING_TARGET_MIN_X (-50000.0f)
#define SWING_AIM_LOWER 10.0f
#define SWING_ARRIVE_DISTANCE 8.0f
#define SWING_SHIFT_LIMIT 40.0f
#define SWING_SAME_HEIGHT 30.0
#define SWING_SAME_DISTANCE 30.0f
#define ATTACK_EFFECT_OWNER 999      // owner of the attack record effects

#define POINTER_EFFECT_DEFAULT 0x8534   // light effect of a helmet pointer node by ItemInfo::helmetKind
#define POINTER_EFFECT_RED 0x8598
#define POINTER_EFFECT_BLUE 0x85fc
#define POINTER_EFFECT_WHITE 0x8660
#define POINTER_SIX_DEFAULT 0x8536      // the same for the three-pointer costume
#define POINTER_SIX_RED 0x859a
#define ACTION_EVENT_PAST 0x78          // attack events older than this many ticks are dropped
#define ACTION_EVENT_AHEAD 0x1e0        // and those further ahead than this
#define CHAIN_GREETINGS_MAX 0x11        // a chain state is kept while the swing has at most this many frames left
#define ATTACK_STATE_LIMIT 10000        // DAT_104c0228: at or above it no attack is chosen
#define ATTACK_STATE_RESUME 0x271a      // the value at which the player stands up again
#define ATTACK_GREETINGS_MAX 0x14       // a held press only counts while the swing has fewer frames left
#define FADE_HOLD_LIMIT 0x44            // DAT_104c0108 frames after which the colour fade is cleared
#define LOW_HEALTH_INTERVAL 0x1e        // frames between the low health warnings

// The light effect of the three-pointer costume's first pointer, by the helmet kind.
static __forceinline int PointerEffectForCostumeSix(void)
{
    int helmetKind = DAT_1088e6b0[DAT_10daabb8].helmetKind;
    if (helmetKind == HELMET_KIND_RED)
        return POINTER_SIX_RED;
    if (helmetKind == HELMET_KIND_BLUE)
        return POINTER_EFFECT_BLUE;
    if (helmetKind == HELMET_KIND_WHITE)
        return POINTER_EFFECT_WHITE;
    return POINTER_SIX_DEFAULT;
}

// The light effect of the other two pointer nodes of the three-pointer costume (the same sequence as the first).
static __forceinline void StartExtraPointerEffect(Node *pointer, Vec4 &pointAt)
{
    pointAt.Copy(pointer->worldMatrix.GetTranslation());
    DAT_10836c74.Copy(DAT_10867e08);
    DAT_10867e08.Copy(pointAt);
    EffectOptions options;
    options.flags = 1;
    options.owner = ATTACK_EFFECT_OWNER;
    DAT_10daaa80.Start(PointerEffectForCostumeSix(), pointAt, -1, 0, 0, 0, 0, 0, &options, 0);
}

#define FLASH_LEVELS 6                  // DAT_10467798: levels of the flash tint (it starts at this value)
#define FLASH_EFFECT 0x3347
#define SPOT_COUNT 4                    // the stair spots of map SPOT_MAP the charged release can snap to
#define SPOT_MAP 0x136
#define SPOT_REACH 120.0f               // how close the player must be to a spot
#define SPOT_TURN 0.8                   // and how closely it must face its direction
#define CHARGE_MIN_FRAMES 10            // DAT_104c0108: frames from which a charge counts
#define CHARGE_LONG_FRAMES 0x46         // the long charge (second effect, release attack 0x30)
#define CHARGE_MAX_FRAMES 0x8c
#define CHARGE_SOUND_A 0x44
#define CHARGE_SOUND_B 0x8a
#define SOUND_CHARGE_STEP 0x116
#define SOUND_CHARGE_PEAK 0x367
#define CHARGE_EFFECT_A 0x8237          // the charge effect by stage of the counter, and its ring effect
#define CHARGE_EFFECT_B 0x8235
#define CHARGE_EFFECT_C 0x8236
#define CHARGE_RING_A 0x80f2
#define CHARGE_RING_B 33000
#define CHARGE_RING_C 0x80fc
#define GLOW_EFFECT_0 0x650             // the drill glow by drill level, and the one every level adds
#define GLOW_EFFECT_1 0x8ef8
#define GLOW_EFFECT_2 0x8ef9
#define GLOW_EFFECT_3 0x8efa
#define GLOW_EFFECT_BASE 36000
#define CARRY_PULL_FRAMES 6             // DAT_104bfdc8 counts up to this while the carried object is pulled in
#define TALK_TARGET_KEEP 10000          // DAT_104bf888 at or above: slot + this is a kept target
#define TALK_FIRST_OBJECT 100           // DAT_104bf888 from here: object record + this

// The player is placed on a stair spot (position and heading) after a charge, and the arrival starts.
static __forceinline void ArriveAtSpot(const Vec4 &spot, int mapSpot)
{
    DAT_10db9210.SetTranslation(spot);
    DAT_10db6ee8[0].heading = spot.w;
    DAT_10db6ee8[0].targetHeading = spot.w;
    DAT_104c0108 = 0;
    DAT_104c0190 = 0;
    DAT_1049ef8c = 0;
    FUN_1005ce80(0x13, 0, 0, 0);
    FUN_1015c9e0(0xf);
    DAT_104c02e4 = mapSpot;
    FUN_1016b230("565656565656565656565656565656565656565656");
}

// Scales an attack effect by the worn costume and helmet (the same chain follows every attack effect).
static __forceinline void ApplyLoadoutScale(float &value)
{
    if (DAT_10c655f0 == LOADOUT_COSTUME_A)
        value = value * LOADOUT_SCALE_A;
    else if (DAT_10c655f0 == LOADOUT_COSTUME_B)
        value = value * LOADOUT_SCALE_B;
    else if (DAT_10c655f0 == LOADOUT_COSTUME_C)
        value = value * LOADOUT_SCALE_C;
    else if ((DAT_104c02c8 & LOADOUT_HELMET_STRONG) == 0)
    {
        if ((DAT_104c02c8 & LOADOUT_HELMET_MID) == LOADOUT_HELMET_MID)
            value = value * LOADOUT_SCALE_MID;
        else if ((DAT_104c02c8 & LOADOUT_HELMET_HIGH) == 0)
        {
            if ((DAT_104c02c8 & LOADOUT_HELMET_LOW) != 0)
                value = value * LOADOUT_SCALE_LOW;
        }
        else
            value = value * LOADOUT_SCALE_HIGH;
    }
    else
        value = value + value;
}

// The door models that play the swing effect when their animation starts.
#define DoorSwingsEffect(model) ((model) == 0xe7 || (model) == 0xe8 || (model) == 0xcd || (model) == 0xce)

// Starts the door swing effect at the origin of the door's own matrix.
static __forceinline void StartDoorEffect(AnimModel *door)
{
    Vec4 origin;
    origin.Fill(0.0f);
    Matrix4 placement = door->matrix;
    DAT_10daaa80.Start(DOOR_SWING_EFFECT, origin, -1, 0, 0, 0, 0, &placement, 0, 0);
}

// Stage 6: the player's trail (ring of recent positions DAT_10802ad8, write index DAT_104c022c, phase DAT_104c0228).
// Phase 0..TRAIL_PHASE_LIMIT counts ticks of safe walking; from TRAIL_REPLAY_BASE on the player is put back on the trail.
#define TRAIL_MASK 0xf                  // ring index mask (PLAYER_TRAIL_LENGTH - 1)
#define TRAIL_REPLAY_BASE 10000         // phase of the glide back after a drop; the phase is 10000 + ticks
#define TRAIL_REPLAY_ARRIVE 10010       // the glide ends (turn to the side, the last motion)
#define TRAIL_REPLAY_SWEEP_END 10015    // from here the glide only eases out
#define TRAIL_RESUME_PHASE 0x14         // walking phase after the landing
#define TRAIL_BLEND_STEPS 5             // blend weights run 0..5 in fifths
#define TRAIL_FADE_START 20             // step where the first ease weight starts
#define TRAIL_SWEEP_STEPS 15            // step where the second ease weight starts
#define TRAIL_GLIDE_STEPS 15            // blends done at once when sliding back up to the trail
#define TRAIL_GLIDE_NEAR 33.0f          // blended point closer than this to the focus keeps the older point
#define TRAIL_FOCUS_DROP 110.0f         // the focus point is this far below the trail point
#define TRAIL_SPOT_NEAR -10.0f          // collision radius (w) of the near spot probe
#define TRAIL_SPOT_FAR -20.0f           // and of the far one
#define TRAIL_MOTION_START 0x18         // motions of the glide
#define TRAIL_MOTION_LAND 0x1a
#define TRAIL_MOTION_END 7
#define TRAIL_MOTION_TURN 99
#define TRAIL_SIDE_OFFSET 1.57075f      // quarter turn from the heading to the probe direction
#define TRAIL_SIDE_ANGLE 1.0471667f     // the side probes are turned by this angle (60 degrees)
#define TRAIL_PROBE_REACH 20.0f         // wall probes sit this far ahead
#define TRAIL_PROBE_LIFT 9.0f           // above the probe height
#define TRAIL_PROBE_LOW 20.0f           // probe height on level ground
#define TRAIL_PROBE_HIGH 50.0f          // and when well below the highest ground seen
#define TRAIL_PROBE_MARGIN 8.0f
#define TRAIL_LANDING_LIFT 5.0f         // height of the landing probe
#define TRAIL_PHASE_LIMIT 500           // the walking phase stops counting here
#define TRAIL_SLIDE_TICKS 8             // sliding back up is only tried this soon after a refill
#define TRAIL_SLIDE_HOLD 4              // ticks the slide state lasts
#define TRAIL_DROP_MIN 25.0f            // standing this far below the trail point starts a slide
#define TRAIL_DROP_REACH 50.0f          // and horizontally closer than this
#define TRAIL_DROP_FALL 90.0f           // dropping this far below the trail point starts the glide back

// The ring entry written `back` steps before the write index `newest`.
static __forceinline Vec4 &TrailBack(unsigned int newest, int back)
{
    return DAT_10802ad8[(newest - back) & TRAIL_MASK];
}

// Puts the player at a position: the model's own placement call and the matrix translation.
static __forceinline void PlacePlayer(float x, float y, float z)
{
    DAT_10db6ee8[0].SetPosition(x, y, z);
    DAT_10db6ee8[0].matrix._41 = x;
    DAT_10db6ee8[0].matrix._42 = y;
    DAT_10db6ee8[0].matrix._43 = z;
}

// out = base * (1 - weight / 5) + toward * (weight / 5)
static __forceinline void BlendTrailPoint(Vec4 &out, const Vec4 &base, const Vec4 &toward, int weight)
{
    out.Copy(base * ((float)(TRAIL_BLEND_STEPS - weight) / 5.0f) + toward * ((float)weight / 5.0f));
}

// True when the player standing at the spot the probes describe would be stuck: the near probe is inside the map
// and clear, but the player-sized probes (w = radius) are not.
static __forceinline bool TrailSpotBlocked(Vec4 &probeA, Vec4 &probeB)
{
    if (!(DAT_10db6ee8[0].IsClearOfMapLimited(&probeA) && DAT_10db6ee8[0].IsClear(&probeA)))
        return false;
    probeA.w = TRAIL_SPOT_NEAR;
    probeB.w = TRAIL_SPOT_FAR;
    return !(DAT_10db6ee8[0].IsClearOfMapLimited(&probeA) && DAT_10db6ee8[0].IsClearFlagged(&probeA) &&
             DAT_10db6ee8[0].IsClearOfMapLimited(&probeB) && DAT_10db6ee8[0].IsClearFlagged(&probeB));
}

// Probes for a wall ahead of the player in direction `angle` (a high and a low point). When the way is clear
// but the map blocks the player-sized probes, refills the whole trail with the spot ahead and returns true.
static __forceinline bool TrailBlockedAhead(Vec4 &probe, Vec4 &probeLow, double angle, float height)
{
    float lift = height + TRAIL_PROBE_LIFT;
    probe.x = (float)cos(angle) * TRAIL_PROBE_REACH;
    probe.y = (float)sin(angle) * TRAIL_PROBE_REACH;
    probe.z = lift;
    probeLow.Copy(probe);
    if (!(DAT_10db6ee8[0].IsClearOfMapLimited(&probe) && DAT_10db6ee8[0].IsClear(&probe)))
        return false;
    probe.z = lift;
    probeLow.Copy(probe);
    probe.z = height - 1.0f;
    probeLow.z = height - 11.0f;
    if (DAT_10db6ee8[0].IsClearOfMap(&probe) && DAT_10db6ee8[0].IsClearFlagged(&probe) &&
        DAT_10db6ee8[0].IsClearOfMap(&probeLow) && DAT_10db6ee8[0].IsClearFlagged(&probeLow))
        return false;
    probe.z = lift;
    for (int step = PLAYER_TRAIL_LENGTH; step != 0; step--)
    {
        DAT_10802ad8[DAT_104c022c].Copy(DAT_10db6ee8[0].matrix.GetTranslation() + probe);
        DAT_104c022c = DAT_104c022c + 1 & TRAIL_MASK;
    }
    return true;
}

// Stage 6: regeneration, forced walk, the pit area.
#define REGEN_HEALTH_FAST_TICKS 0x78    // helmet regeneration periods (frames)
#define REGEN_HEALTH_SLOW_TICKS 0x96
#define REGEN_STAMINA_FAST_TICKS 10
#define REGEN_STAMINA_SLOW_TICKS 0xf
#define HELMET_REGEN_HEALTH_FAST 0x8000 // DAT_104c02c8 bits of the worn helmet
#define HELMET_REGEN_HEALTH_SLOW 0x4000
#define HELMET_REGEN_STAMINA_FAST 0x80000
#define HELMET_REGEN_STAMINA_SLOW 0x40000
#define HEALTH_FLASH_TICKS 8            // DAT_104c011d: how long the health gauge flashes after a gain
#define HELD_ITEM_TIMER_LIMIT 1000      // DAT_104bfdf0 values from here are not countdowns
#define STICK_FLICK 5.0f                // stick deflection that counts as a push
#define STICK_FLICK_TICKS 20            // how long a push is remembered
#define FORCED_RUN_SPEED 7.0            // forced stick values above this make the player run
#define FORCED_WALK_SPEED_SCALE 2.5f    // motion speed scale of a forced walk
#define PIT_ICE_BELOW 120.0f            // the frozen pit surface counts from this far below its floor height
#define PIT_ICE_ABOVE 10.0f             // to this far above
#define ACTION_GAUGE_MAX 0x1e           // DAT_104687b4 stops counting here
#define PIT_FLOOR_MARGIN 60.0f          // player lower than this under the floor is in the pit
#define PIT_STEP 25                     // and the margin grows by this per DAT_104bfe50
#define ARM_SPEED_MAX 30.0f             // the arm node's speed scale range
#define ARM_SPEED_STEP 2.0f

// Stage 6: item pad, overlay shift, swimming, the pit water.
#define ITEM_PAD_MAX 0xf                // DAT_104c0099: item pad counter range
#define OVERLAY_SHIFT_MAX 8             // DAT_104bfe18: frames the overlay mode shifts the screen
#define OVERLAY_SHIFT_DIVISOR 32        // shift in pixels = back-buffer size * frames / 32
#define YOTSU_HIDE_MAX 0x1e             // DAT_104c0301 counts up to this while swimming
#define YOTSU_HIDE_MIN 9                // and the "@yotsu" node is hidden from here
#define YOTSU_NODE_FLAG 4               // Node::flags bit that shows it
#define MOTION_SURFACE 0x81
#define MOTION_PIT_SWIM 0x1b
#define PIT_EXIT_SPEED 0.4f             // m_237c after leaving the pit
#define PIT_SPLASH_MIN_SPEED 2.0f       // entering slower than this makes no splash
#define PIT_SINK_TICKS 0x10             // ticks the player is pushed down after entering
#define PIT_MODEL_COUNT 0x7c            // models 0..0x7b get pit effects
#define PIT_MODEL_GONE_X (-50000.0f)    // models parked below this x are skipped
#define STAT_PIT_SKIP 14                // CharSlot::stats entry: non-zero = no pit effects for this slot
#define PIT_SURFACE_MARGIN 10.0f        // a foot this close to the floor height touches the water
#define PIT_RIPPLE_HEIGHT 13.0f         // height of the ripples above the floor height
#define PIT_CONTACT_HEIGHT 20.0f        // the head or neck splashes below this
#define PIT_WAKE_RANGE 110.0f           // the body must be within this of the floor height for wake and ripples
#define SPLASH_DROP 10.0f               // the neck splash starts this much lower
#define SPLASH_REACH 25.0f              // and this far ahead of the player
#define WAKE_REACH 50.0f                // the wake starts this far ahead
#define PIT_COSTUME_SAFE 0xc            // costume that is never hurt by the water
#define PIT_PROTECT_FRAMES 0x3c         // DAT_10253e98: protection frames after a hit
#define PIT_HEAL_FLASH 8                // DAT_104c011d: HUD flash value when the water heals
#define EFFECT_SWAN_WATER 0xc7f
#define EFFECT_PIT_SPLASH 0x9b
#define EFFECT_PIT_RIPPLE 0x3d
#define EFFECT_PIT_WAKE 0xad
#define SOUND_PIT_WALK 0x36f
#define SOUND_PIT_SWIM 0x36e
#define SOUND_PIT_SWIM_PERIOD 12        // swimming strokes sound every this many frame pairs
#define SWING_ROCK_REACH 100.0f         // a rock this close to the player bursts
#define SWING_ROCK_DAMAGE 10            // and hurts the player by this much
#define EFFECT_ROCK_BURST 0x5eb
#define EFFECT_SWING_LANDING 0x695
#define SWING_DROP_ACCEL 3.0f           // DAT_104c0214: the camera drop speeds up by this per frame
#define SWING_PLAYER_LIFT 1.2f          // the player rides this multiple of the drop above the frame
#define SWING_PLAYER_HEIGHT 80.0f       // plus this
#define SWING_TRUCK_LIFT 0.9f           // the truck node rides this multiple of the drop
#define SWING_TURN_ROUND 3.1415f        // the player faces the other way on the forward swing
#define SWING_BURST_LEAD 2              // the landing bursts sit this many frames ahead of (or behind) the current one
#define SWING_BURST_SIDE 40.0f          // to the right (and left) of the frame
#define SWING_BURST_HEIGHT 10.0f        // above it
#define PIT_SINK_SPEED_SCALE 2.4f
#define PIT_SPLASH_LIFT 20.0f
#define PIT_SPLASH_REACH 50.0f
#define PIT_SPLASH_EFFECT 0xab
#define PIT_SPLASH_SOUND 0x370

// Stage 6: gesture counters, carried-object pickup, timed event.
#define GESTURE_LONG_TICKS 0xf          // DAT_104c0330 resets after this many ticks
#define GESTURE_SHORT_TICKS 7           // DAT_104c0331 resets after this many
#define PICKUP_FACING_DOT 0.3           // facing the carried object closer than this counts as looking at it
#define PICKUP_FACING_TICKS 0xf         // ticks of looking before the pickup starts
#define MOTION_PICKUP 0xf
#define TIMED_EVENT_BASE 500

// Movement with nothing held in the hands: run speed, lean (spinAngle) and the carried-object push.
#define SOUND_CARRY_PUSH 0x379
#define MOTION_SWIM_START 0x1d
#define MOTION_SLIDE 0x85
#define MOTION_STAND 0x10
#define MOTION_CARRY_LEFT 0x11
#define MOTION_CARRY_RIGHT 0x12
#define SLIDE_HURRY_TICK 9              // DAT_104c0301 counts the slide's ticks; earlier ticks blend 5 frames, later ones 2
#define MOVE_STEP_LENGTH 8.0f           // the steer direction is scaled to this many units per tick
#define SLIDE_SCALE 0.6f
#define WALK_SCALE 0.7f
#define RUN_SPEED_FALLOFF 0.2f
#define RUN_SPEED_BUILDUP 0.1f
#define RUN_SPEED_MAX 2.5f
#define LEAN_EASE_FAST 0.1f
#define LEAN_EASE_SLOW 0.05f
#define LEAN_TURN_STEP 0.03f
#define LEAN_TURN_MARGIN 0.04f
#define LEAN_TURN_LIMIT 0.3f
#define LEAN_TURN_SPAN 0.6666667f       // turn angle (radians) that gives the full lean limit
#define TURN_DEAD_ZONE 0.09817187488079071   // turns smaller than this (pi / 32) do not lean
#define TURN_SHARP_LIMIT 2.0943334102630615  // turns of more than this (about 120 degrees) only ease the lean back
#define CARRY_MOTION_BLEND 2
#define CARRY_SHOVE_FIRST 0x10          // DAT_104bfdcc counts the carry action down from 0x1e to 0
#define CARRY_PUSH_FRAME 0x1e
#define CARRY_SNAP_FRAME 0xe
#define CARRY_RESET_FRAME 100
#define CARRY_DRAG_FRAME 10
#define CARRY_SETTLE_FRAME 8
#define CARRY_GRID 75.0f                // carried objects sit on a grid of this size; a push moves one cell
#define CARRY_SIDE_PUSH 2.0f
#define CARRY_SHOVE_SCALE 1.34f
#define CARRY_DRAG_SCALE 0.2f
#define CARRY_SNAP_SCALE 0.5f
#define CARRY_LIFT 30.0f

// The run speed multiplier falls by 0.2 per tick down to 1.0.
static __forceinline void RunSpeedFalloff(void)
{
    DAT_102027ec = DAT_102027ec - RUN_SPEED_FALLOFF;
    if (DAT_102027ec < 1.0f)
        DAT_102027ec = 1.0f;
}

// The lean of the player's model (spinAngle) returns to zero by `rate` per tick. Returns true when it is within
// `rate` of zero, where the caller sets it to exactly zero.
static __forceinline bool LeanEasesBack(float rate)
{
    if (DAT_10db6ee8[0].spinAngle > 0.0f)
        DAT_10db6ee8[0].spinAngle = DAT_10db6ee8[0].spinAngle - rate;
    if (DAT_10db6ee8[0].spinAngle < 0.0f)
        DAT_10db6ee8[0].spinAngle = DAT_10db6ee8[0].spinAngle + rate;
    return fabs(DAT_10db6ee8[0].spinAngle) < rate;
}

#define EFFECT_SWING_DUST 0x2da
#define SOUND_LEAP_LAND 0x377
#define SOUND_GRUNT_BASE 0x85
#define LEAP_STAGE_LOOK 5               // DAT_104bfe40: the leap's stage counter (0 = not leaping)
#define LEAP_STAGE_CLIMB_END 0x32
#define LEAP_STAGE_RISE_END 0x46
#define LEAP_STAGE_FALL_END 0x5a
#define LEAP_STAGE_DONE 100
#define LEAP_CURVE_LENGTH 28
#define LEAP_HEIGHT_SCALE 10.0f
#define LEAP_REACH_LIMIT 130.0f         // beyond this distance from the obstacle the player vaults over it
#define LEAP_VAULT_STEER 5.0f
#define LEAP_PUSH_X 0.3f
#define LEAP_PUSH_Y 0.4f
#define LEAP_SNAP_STEP 37.5f
#define LEAP_SNAP_OFFSET 10.0f
#define LEAP_FACE_TURN 3.1415f
#define LEAP_DRIFT_FAST 1.1f
#define LEAP_DRIFT_LIFT 0.54f
#define SWIM_MIN_FRAME 0x10
#define SWIM_DEPTH_OFFSET 60.0f
#define SWIM_RISE_LIMIT 12.0f
#define SWIM_PUSH_SCALE 1.4f
#define RUN_SPEED_MIN 1.0f
#define CARRY_NEAR_HEIGHT 50.0f
#define CARRY_NEAR_DISTANCE 155.0f
#define SWING_STEP_SCALE 1.4f
#define SWING_PROBE_LIFT 5.0f
#define SWING_MAX_BLOCKED_STEPS 4
#define SMOOTH_OLD 0.7f
#define SMOOTH_NEW 0.3f
#define SPEED_SCALE_SLOW 0.5f
#define MOTION_SPEED_SLOW 0.6f
#define MOTION_SPEED_TOP 2.5f           // run speed that plays the motion at its normal speed
#define AIR_STEP_DECAY 6                // DAT_104bf8c4 falls by this much per tick

#define MOTION_BOUNCE 1
#define BOUNCE_LEAN_LIMIT 0.7f          // a gust bounce leans the model toward its side up to this far
#define BOUNCE_LEAN_STEP 0.15f
#define CARRY_DROP_DISTANCE 150.0f      // a carried object further away than this is let go
#define SWIM_SHALLOW_LIMIT 9.0f

// Run speed rises by 0.1 per tick up to 2.5 while the player steers.
static __forceinline void RunSpeedBuildup(void)
{
    if (RUN_SPEED_MIN <= DAT_102027ec)
    {
        if (DAT_102027ec < RUN_SPEED_MAX)
            DAT_102027ec = DAT_102027ec + RUN_SPEED_BUILDUP;
    }
    else
    {
        DAT_102027ec = RUN_SPEED_MIN;
        DAT_102027ec = DAT_102027ec + RUN_SPEED_BUILDUP;
    }
    if (DAT_102027ec > RUN_SPEED_MAX)
        DAT_102027ec = RUN_SPEED_MAX;
}

// Steering with a held direction: the model leans against the change of the walk direction (spinAngle follows the turn,
// kept inside +-limit) and the run speed builds up; sharp turns and no turn ease the lean back instead.
static __forceinline void SteerLeanAndSpeed(float moveX, float moveY)
{
    float previousAngle = (float)atan2(DAT_10866de8.x, DAT_10866de8.y);
    float newAngle = (float)atan2(moveX, moveY);
    float turn = FUN_101ac810(previousAngle, newAngle);
    if (fabs(turn) >= TURN_DEAD_ZONE)
    {
        if (fabs(turn) < TURN_SHARP_LIMIT)
        {
            float limit = (float)fabs(turn) / LEAN_TURN_SPAN * LEAN_TURN_LIMIT;
            float lowLimit;
            if (limit > LEAN_TURN_LIMIT)
                limit = LEAN_TURN_LIMIT;
            if (turn <= 0.0f)
            {
                if (turn < 0.0f)
                    DAT_10db6ee8[0].spinAngle = DAT_10db6ee8[0].spinAngle + LEAN_TURN_STEP;
            }
            else
                DAT_10db6ee8[0].spinAngle = DAT_10db6ee8[0].spinAngle - LEAN_TURN_STEP;
            if (limit + LEAN_TURN_MARGIN < DAT_10db6ee8[0].spinAngle)
                DAT_10db6ee8[0].spinAngle = DAT_10db6ee8[0].spinAngle - LEAN_TURN_STEP;
            if (limit < DAT_10db6ee8[0].spinAngle)
                DAT_10db6ee8[0].spinAngle = DAT_10db6ee8[0].spinAngle - LEAN_TURN_STEP;
            lowLimit = -limit;
            if (DAT_10db6ee8[0].spinAngle < lowLimit - LEAN_TURN_MARGIN)
                DAT_10db6ee8[0].spinAngle = DAT_10db6ee8[0].spinAngle + LEAN_TURN_STEP;
            if (DAT_10db6ee8[0].spinAngle < lowLimit)
                DAT_10db6ee8[0].spinAngle = DAT_10db6ee8[0].spinAngle + LEAN_TURN_STEP;
            RunSpeedBuildup();
        }
        else
        {
            if (LeanEasesBack(LEAN_EASE_SLOW))
                DAT_10db6ee8[0].spinAngle = 0.0f;
            RunSpeedFalloff();
        }
    }
    else
    {
        if (LeanEasesBack(LEAN_EASE_SLOW))
            DAT_10db6ee8[0].spinAngle = 0.0f;
        RunSpeedBuildup();
    }
}

// The 15 pieces a defeated enemy bursts into at `at`, thrown along `direction` (three places in the enemy code).
static __forceinline void BurstIntoPieces(const Vec4 &at, const Vec4 *direction, int slot)
{
    FUN_1000df10(at.x + 5.0f, at.y, at.z + 50.0f, direction, 100, slot);
    FUN_1000df10(at.x, at.y + 5.0f, at.z + 55.0f, direction, 100, slot);
    FUN_1000df10(at.x - 4.0f, at.y, at.z + 50.0f, direction, 100, slot);
    FUN_1000df10(at.x + 8.0f, at.y + 5.0f, at.z + 55.0f, direction, 100, slot);
    FUN_1000df10(at.x - 5.0f, at.y, at.z + 50.0f, direction, 100, slot);
    FUN_1000df10(at.x, at.y + 7.0f, at.z + 55.0f, direction, 100, slot);
    FUN_1000df10(at.x - 5.0f, at.y - 5.0f, at.z + 50.0f, direction, 100, slot);
    FUN_1000df10(at.x + 5.0f, at.y, at.z + 45.0f, direction, 100, slot);
    FUN_1000df10(at.x - 4.0f, at.y + 7.0f, at.z + 55.0f, direction, 100, slot);
    FUN_1000df10(at.x + 5.0f, at.y - 5.0f, at.z + 60.0f, direction, 100, slot);
    FUN_1000df10(at.x - 5.0f, at.y + 10.0f, at.z + 55.0f, direction, 100, slot);
    FUN_1000df10(at.x + 10.0f, at.y - 5.0f, at.z + 55.0f, direction, 100, slot);
    FUN_1000df10(at.x - 5.0f, at.y - 10.0f, at.z + 40.0f, direction, 100, slot);
    FUN_1000df10(at.x + 4.0f, at.y + 7.0f, at.z + 55.0f, direction, 100, slot);
    FUN_1000df10(at.x + 10.0f, at.y - 10.0f, at.z + 65.0f, direction, 100, slot);
}

// The player's frame tail: face cues, fall and landing motions, the levers and chests in reach, the view sort.
#define FACE_CUE_COUNT 0x80
#define FACE_CUES_END (-3000)           // frame value that ends the face cue table
#define FACE_BLINK_PERIOD 200           // the idle blink starts 1 time in 100 (random value below 2 of this)
#define FACE_BLINK 2
#define SCENE_STATE_LIMIT 10000         // DAT_104c0228 from here on: the scene states, no landing motions
#define LANDING_SOFT 1                  // DAT_104c009c: the landing kind that was started
#define LANDING_FALL 2
#define LANDING_HARD 3
#define LANDING_IDLE_FRAMES 0xb         // frames in the air that give the longer landing motion
#define FALL_FAST_SPEED 14.99f          // falling speed (m_237c) that starts the fall motion
#define FALL_IDLE_SPEED 3.5f            // falling speed that gives a soft landing when standing
#define FALL_WALK_SPEED 8.5f            // the same when moving
#define STUMBLE_AIR_FRAMES 10u
#define STUMBLE_SPEED_MIN 5.0f
#define STUMBLE_MOVE_MAX 3.0f
#define GIMMICK_COUNT 64
#define GIMMICK_TYPE_CHEST 10           // GimmickSlot::type
#define GIMMICK_TYPE_LEVER 0xb
#define GIMMICK_TYPE_MARKER 0xe
#define GIMMICK_PICK_REACH 100.0f       // within this the gimmick is the one the player works
#define GIMMICK_LOOK_REACH 160.0f       // within this the head looks at the nearest gimmick
#define GIMMICK_PICK_MOTION 0x52
#define GIMMICK_MARKER_BASE 100         // DAT_104bf888 = this + slot for a picked marker
#define IDLE_COUNT_MAX 0x3c
#define IDLE_COUNT_RESET 0xf
#define WALK_COUNT_RESET 8
#define LEDGE_FEET_DROP 25.0f           // ground this far below the feet is not a ledge
#define LEDGE_AHEAD 30.0                // the ground height is read this far ahead of the player
#define VIEW_FAR_SQUARED 20250000.0f    // squared distance beyond which a character is out of view
#define VIEW_NEAR_PLAYER_SQUARED 40000.0f
#define VIEW_NEAR_SQUARED 62500.0f
#define SCENERY_FIRST_MODEL 0x5c
#define SCENERY_NEAR_SQUARED 4000000.0f

// 0x1011d810 (the in-game frame). Mode 1 of the frame step. The first call resets the mode's globals (DAT_104bf8bc
// was 0 until then). Each call computes how many engine ticks have passed since the last one (at most
// MAX_CATCH_UP_TICKS, otherwise one); every tick polls the input (and records or replays it), runs the main menu and
// the queued events and then the whole game logic for the player and the characters. After the last tick the model
// bank and the lens flare are prepared and the scene is drawn. Returns 0 when the frame was cut short, 1 otherwise.
extern "C" int FUN_1011d810(void)
{
    int ticks;
    int tick;
    int inputActive;
    char mouseLeft;
    char mouseRight;
    char mouseMiddle;
    char mouseExtra;
    int mouseWheel;
    int mousePos[2];
    int inputEvents;
    float steerX;
    float steerY;
    int moveInput;
    int controlsEnabled;
    int viewWidth;
    int viewHeight;
    float aimX;
    float aimY;
    char gameOverButton;
    char gameOverPadA;
    char gameOverPadB;
    int gameOverPos[2];
    int stepForced;
    int engineTick;
    int windowBusy;
    int modelCount;
    int modelPos;
    int modelIdx;
    float closestEnemy;
    int enemyClose;
    int gimmickFound;
    int i;
    unsigned char flag0155;
    unsigned int greetings;
    char stepDelay;
    bool riding;
    int ridingFlag;
    int modelOrder[125];

    if (DAT_104bfd84 != 0)
        return 0;
    if (DAT_104bf8bc == 0)
    {
        DAT_102024a8 = 1;
        DAT_104bf8bc = 1;
        DAT_10db9380 = 1;
        DAT_11018734 = 1;
        DAT_1101de30 = 1;
        DAT_104687b0 = DAT_10db9274;
        memset(DAT_102550e0, 0, sizeof(DAT_102550e0));
        memset(DAT_10253e98, 0, sizeof(DAT_10253e98));
        memset(DAT_104686b0, 0, sizeof(DAT_104686b0));
        memset(DAT_10210d30, 0, sizeof(DAT_10210d30));
        memset(DAT_1030c278, 0, sizeof(DAT_1030c278));
        DAT_104bf8b4 = 0;
        DAT_104c00b2 = 0;
        DAT_104c00b0 = 0;
        DAT_102024bc = 0xffff;
        DAT_104c0092 = 0;
        DAT_104c0093 = 0;
        DAT_104c00a0 = 0;
        DAT_102024b8 = -1;
        DAT_104c0097 = 0;
        DAT_104c0094 = 0;
        DAT_1030c478[0] = 0;
        DAT_1049f1f8[0] = 0;
        DAT_1030c479 = 0;
        DAT_1049f1f9 = 0;
        DAT_1030c47a = 0;
        DAT_1049f1fa = 0;
        DAT_1030c47b = 0;
        DAT_1049f1fb = 0;
        DAT_104c00b7 = 0;
        DAT_104c0128 = 0;
        DAT_10f6553c = Hud_DrawFrame;
        DAT_104c010c = 0;
        DAT_10db6ee8[0].m_0075 = 0;
        DAT_104c0068 = 0;
        DAT_104bff00 = 0;
        DAT_104c009e = 0;
        DAT_104c009d = 0;
        DAT_102024b0 = -1;
        DAT_104c0030 = 0;
        DAT_104bfeb9 = 0;
        DAT_1030b868 = 0;
        DAT_104bfd74 = 4;
        DAT_104bfeb0 = 0;
        DAT_104bfe70 = 0;
        DAT_104bfe74 = 0;
        DAT_104c0028 = 0;
        DAT_104c002c = 0;
        DAT_104c00bd = 0;
        DAT_104bfe30 = 0;
        DAT_104bfdd0 = 0;
        DAT_104bfdcc = 0;
        DAT_104bfddc = 0;
        DAT_104bfdc8 = 0;
        DAT_1020d430 = 0;
        DAT_10468128 = 0;
        DAT_104bfe20 = 0;
        DAT_104c009c = 0;
        DAT_104bfd78 = 0;
        DAT_104bfd80 = 0;
        DAT_104bfe3c = 0;
        DAT_104bfe7c = 0;
        DAT_104bfe4c = 0;
        DAT_104bfe50 = 0;
        for (i = 1; i < CHAR_SLOT_COUNT; i++)
            DAT_10daaae8[i].stats[STAT_HEALTH] = DAT_10daaae8[i].stats[STAT_MAX_HEALTH];
        if (DAT_104b7210 > -1)
            FUN_10174c70(DAT_104b7210, 0);
        DAT_104b7210 = -1;
        for (i = 0; i < HUD_TEXTURE_SLOT_COUNT; i++)
        {
            if (DAT_10202808[i] > -1)
                FUN_10174c70(DAT_10202808[i], 0);
            DAT_10202808[i] = -1;
        }
        FUN_1013fa10(0, 0, 1.0f, 1.0f, 1.0f, 1.0f);
        FUN_1013fa10(1, 0, 0.0f, 0.0f, 0.0f, 1.0f);
        FUN_1000ec70(0, 1.0f, 1.0f, 1.0f, 1.0f);
        DAT_1101ddf4 = 0;
        DAT_104c0138 = GetTickCount();
    }
    if (DAT_1065cb90 != 0 && DAT_104bf8b8 == DAT_10f65504)
        return 0;
    DWORD now = GetTickCount();
    if (now - DAT_104c0138 >= 1000)
    {
        unsigned int seconds = (now - DAT_104c0138) / 1000;
        DAT_104c00dc = DAT_104c00dc + seconds;
        DAT_104c0138 = DAT_104c0138 + seconds * 1000;
    }
    if (DAT_10db6ee8[NEAR_MODEL].root == 0)
        DAT_104c02f0 = 0;
    else
    {
        float modelDistance = DAT_10db6ee8[NEAR_MODEL].matrix.GetTranslation().Distance(DAT_10db6ee8[0].matrix.GetTranslation());
        DAT_104c02f0 = 1;
        if (modelDistance >= NEAR_MODEL_DISTANCE)
            DAT_104c02f0 = 0;
    }
    windowBusy = DAT_104b7200.IsAnyWindowBusy();
    engineTick = DAT_10f65504;
    stepForced = DAT_104c0134;
    if (windowBusy != 0 || (DAT_104c0135 = 0, DAT_104c03dc != 0))
        DAT_104c0135 = 1;
    DAT_10db6ee8[DAT_104bf8b4].m_02a0 = 0;
    if (DAT_104bf8b8 == engineTick && stepForced == 0)
        goto prepareFrame;
    if (DAT_104bf87c > 0 && DAT_104bf87c < 10)
        DAT_104bf87c = DAT_104bf87c + 1;
    ticks = engineTick - DAT_104bf8b8;
    DAT_104bf8b8 = engineTick;
    if (DAT_1065cb90 != 0)
        DAT_104c0184 = DAT_104c0184 + 0xf - ticks * 5;
    if (ticks > MAX_CATCH_UP_TICKS || DAT_104bf8a0 != 0 || DAT_104bf880 != 0 || ticks == 0 || stepForced != 0
        || DAT_1065cb90 != 0)
        ticks = 1;
    DAT_104c020c = 1;
    for (tick = 0; tick < ticks; tick++)
    {
        if (DAT_1032d69c != 0 && DAT_104b7200.IsQueuedEventReady() != 0)
        {
            if (DAT_104bfe1c != 0)
            {
                DAT_1030c54c = 0;
                DAT_104bfd78 = 0;
                DAT_104b7200.CloseWindow(WINDOW_ID_SAVE_MENU);
                DAT_104b7200.CloseWindow(WINDOW_ID_CHOICE_MENU);
                DAT_104b7200.CloseWindow(WINDOW_ID_YES_NO);
                DAT_1020e0f8[0] = 0;
                FUN_1000cb30("");
                FUN_1000cb90("");
                DAT_104bf8d4 = 1;
                if (DAT_104c03c8 != 0)
                {
                    DAT_104c03dc = 0;
                    DAT_104c03c8->Release();
                    DAT_11b1ba40(DAT_104c03c8);
                    DAT_104c03c8 = 0;
                    DAT_104c03cc = 0;
                    DAT_104c03d0 = 0;
                    DAT_104c03d4 = 0;
                    DAT_104c03d8 = 0;
                    DAT_104c03e0 = 0;
                    DAT_104c03e4 = 0;
                    for (i = 0; i < OVERLAY_TEXTURE_SLOT_COUNT; i++)
                    {
                        if (DAT_1030b870[i] > -1)
                        {
                            FUN_10174c70(DAT_1030b870[i], 1);
                            DAT_1030b870[i] = -1;
                        }
                    }
                }
                DAT_104b7200.CloseWindow(WINDOW_ID_CHOICE_MENU);
                DAT_104b7200.CloseWindow(WINDOW_ID_YES_NO);
                DAT_1020e0f8[0] = 0;
                DAT_104bfe1c = 0;
                memset(DAT_1030c500, 0, sizeof(DAT_1030c500));
            }
            DAT_104bf8b8 = DAT_104bf8b8 + (tick - ticks);
            break;
        }
        FUN_1000e870();
        DAT_10210b2c = 0;
        DAT_1030c990 = 0;
        DAT_1020d9e0.M_10001110(0, 0, 0, 0, 0, 0);
        DAT_104c0134 = 0;
        inputActive = 0;
        DAT_10f08008.Update();
        DAT_10210f30 = DAT_10f08008.ReadControls(0);
        DAT_1020d9e0.M_10001550();
        DAT_104bf890 = DAT_104bf890 + 1;
        if (DAT_10202358 < BGM_TRACK_LIMIT && DAT_108db358[DAT_10202358].m_002c != 0)
            DAT_1032d6a0 = ((DAT_10254088 - DAT_108db358[DAT_10202358].start) * BGM_BEAT_STEPS)
                           / (BGM_BEAT_SAMPLES / DAT_108db358[DAT_10202358].m_002c);
        DAT_1065cb28.SetButtons(DAT_10210f30);
        DAT_1065cb28.M_1015caa0(DAT_10f08008.ReadControls(1), 0);
        DAT_1065cb28.M_1015caa0(DAT_10f08008.ReadControls(2), 1);
        DAT_1065cb28.M_1015caa0(DAT_1032d6a0, 2);
        DAT_1065cb28.M_1015caa0(DAT_10f65558, 3);
        DAT_1065cb28.M_1015caa0(DAT_104bf890, 4);
        DAT_1065cb28.M_1015cac0(DAT_10db6ee8[0].matrix._41, 0);
        DAT_1065cb28.M_1015cac0(DAT_10db6ee8[0].matrix._42, 1);
        DAT_1065cb28.M_1015cac0(DAT_10db6ee8[0].matrix._43, 2);
        DAT_1065cb28.M_1015cac0(DAT_10db6ee8[SECOND_MODEL].matrix._41, 3);
        DAT_1065cb28.M_1015cac0(DAT_10db6ee8[SECOND_MODEL].matrix._42, 4);
        DAT_1065cb28.M_1015cac0(DAT_10db6ee8[SECOND_MODEL].matrix._43, 5);
        DAT_1065cb28.M_1015cac0(DAT_118e6cb8.x, 6);
        DAT_1065cb28.M_1015cac0(DAT_118e6cb8.y, 7);
        DAT_1065cb28.M_1015cac0(DAT_118e6cb8.z, 8);
        if (DAT_1065cb94 != 0 && DAT_1065cb90 != 0)
            DAT_1065cb28.RecordFrame();
        if (DAT_1065cb98 != 0 && DAT_1065cb90 != 0)
        {
            DAT_1065cb28.PlayFrame();
            DAT_1032d6a0 = DAT_1065cb28.values_now[2];
            if (DAT_104bf890 != DAT_1065cb28.values_now[4] || DAT_10f65558 != DAT_1065cb28.values_now[3]
                || DAT_10db6ee8[0].matrix._41 != DAT_1065cb28.floats_now[0]
                || DAT_10db6ee8[0].matrix._42 != DAT_1065cb28.floats_now[1]
                || DAT_10db6ee8[0].matrix._43 != DAT_1065cb28.floats_now[2]
                || DAT_10db6ee8[SECOND_MODEL].matrix._41 != DAT_1065cb28.floats_now[3]
                || DAT_10db6ee8[SECOND_MODEL].matrix._42 != DAT_1065cb28.floats_now[4]
                || DAT_10db6ee8[SECOND_MODEL].matrix._43 != DAT_1065cb28.floats_now[5]
                || DAT_118e6cb8.x != DAT_1065cb28.floats_now[6] || DAT_118e6cb8.y != DAT_1065cb28.floats_now[7]
                || DAT_118e6cb8.z != DAT_1065cb28.floats_now[8])
            {
                DAT_1065cb28.playFrame = DAT_1065cb28.frameCount;
                Bgm_Play(0, 0);
                DAT_104c012c = 0;
                FUN_1000da40(0, BGM_VOLUME_FULL, DEMO_DESYNC_FADE_FRAMES);
            }
            if (DAT_1065cb90 != 0 && DAT_1065cb28.playFrame == DEMO_END_FRAME)
            {
                Bgm_Play(0, 0);
                DAT_104c012c = 0;
                FUN_1000da40(0, BGM_VOLUME_FULL, DEMO_END_FADE_FRAMES);
            }
        }
        if (DAT_104bfe7c != 0)
            DAT_104bfe7c = DAT_104bfe7c - 1;
        if (FUN_10112140() != 0)
            return 0;
        UpdateDrillLevel();
        if ((DAT_104bf890 & 1) != 0 && DAT_10255f08 != 0)
            DAT_10255f08->M_10194ee0();
        if (DAT_104bfe1c == 0 && DAT_104bfe6c == 1)
        {
            DAT_104bf8a4 = DAT_10d4ee30;
            FUN_10189080();
        }
        Camera_Refresh();

        // The alpha-pass nodes of the listed parts are hidden while the camera sees the player through their mesh.
        for (i = 0; i < DAT_1032d868; i++)
        {
            if (DAT_1049ef90[i]->SegmentHitsMesh(&DAT_118e69e8, &DAT_118e6cb8) == 0)
            {
                DAT_102550c0[i]->ClearFlags(NODE_FLAG_HIDDEN);
                DAT_1020aefc[i]->ClearFlags(NODE_FLAG_HIDDEN);
            }
            else
            {
                DAT_102550c0[i]->SetFlags(NODE_FLAG_HIDDEN);
                DAT_1020aefc[i]->SetFlags(NODE_FLAG_HIDDEN);
            }
        }
        DAT_10f65548 = 0;
        DAT_10f6554c = 0;

        // The two light shafts slide across the screen and fade in and out.
        for (i = 0; i < SHAFT_COUNT; i++)
        {
            Vec4 &shaft = DAT_10f65848[i];
            if (shaft.x > SHAFT_LIMIT || shaft.y > SHAFT_LIMIT)
            {
                shaft.w = shaft.w - SHAFT_FADE;
                if (shaft.w <= 0.0f)
                {
                    shaft.w = 0.0f;
                    shaft.x = (float)(-FUN_1013f880() % 1000 - SHAFT_RESTART);
                    shaft.y = (float)(-FUN_1013f880() % 1000 - SHAFT_RESTART);
                }
            }
            else
            {
                shaft.x = shaft.x + SHAFT_SPEED;
                shaft.y = shaft.y + SHAFT_SPEED;
                shaft.w = shaft.w + SHAFT_FADE;
                if (shaft.w >= 1.0f)
                    shaft.w = 1.0f;
            }
        }
        DAT_10f65368 = (float)sin((double)((float)DAT_104bf890 * 0.1f)) * 0.3f;
        if (DAT_104c0232 != 0)
            DAT_104c0232 = DAT_104c0232 - 1;
        if (DAT_104c0233 != 0)
            DAT_104c0233 = DAT_104c0233 - 1;

        // The effects the map keeps requesting start when their wait is over.
        if (DAT_104bfe1c == 0 && DAT_104c0135 == 0 && DAT_1020b22c > 0)
        {
            for (i = 0; i < DAT_1020b22c; i++)
            {
                EffectHold &hold = DAT_1020dcf8[i];
                hold.age = hold.age + 1;
                if (hold.m_0010 < hold.age)
                {
                    EffectOptions options;
                    options.m_0004 = hold.value;
                    options.settings[2] = (char)hold.m_001c;
                    if (options.m_0004 == HOLD_VALUE_FIRE_A || options.m_0004 == HOLD_VALUE_FIRE_B)
                        options.m_000e = 6;
                    options.m_0000 = 0;
                    hold.age = 0;
                    Vec4 where;
                    where.x = (float)hold.position[0];
                    where.y = (float)hold.position[1];
                    where.z = (float)hold.position[2];
                    where.w = 0.0f;
                    DAT_10daaa80.Start(hold.effect, where, -1, -1, 0, 0, 0, 0, &options, 0);
                }
            }
        }
        if (DAT_104c0128 > 0)
        {
            DAT_104c0128 = DAT_104c0128 - 1;
            if (DAT_104c0128 == 0 || DAT_104bfe6c != 0)
            {
                DAT_104c0128 = 0;
                Bgm_Play(DAT_104c012c, 0);
            }
        }

        // The HUD counters count down.
        if (DAT_104c0119 != 0)
            DAT_104c0119 = DAT_104c0119 - 1;
        if (DAT_104c011e != 0)
            DAT_104c011e = DAT_104c011e - 1;
        if (DAT_104c011f != 0)
            DAT_104c011f = DAT_104c011f - 1;
        if (DAT_104c011d != 0)
            DAT_104c011d = DAT_104c011d - 1;
        if (DAT_104c011a != 0)
            DAT_104c011a = DAT_104c011a - 1;
        if (DAT_104c011c != 0)
            DAT_104c011c = DAT_104c011c - 1;
        if (DAT_104c011b != 0)
        {
            DAT_104c011b = DAT_104c011b - 1;
            if (DAT_104c011b == 0)
                DAT_104c0114 = 0;
        }
        if (DAT_104c0108 > 10 || DAT_104c00e0 != 0 || DAT_102024c4 != 0 || DAT_102024ac > 0)
            DAT_104c0124 = HUD_HOLD_FRAMES;
        if (DAT_104bfe6c == 0)
        {
            if (DAT_104c0124 > 0)
                DAT_104c0124 = DAT_104c0124 - 1;
        }
        else
            DAT_104c0124 = 0;
        DAT_10daaae8[0].stats[STAT_EFFECT_PARAM] = 10;
        DAT_10daaae8[0].stats[STAT_VALUE_4] = 5;

        // Rain falls inside the weather box of the map: effects in mode 2, falling drops otherwise.
        if (DAT_104c0224 != 0 && DAT_1083631c.x < DAT_10db6ee8[0].matrix._41 && DAT_1083631c.y < DAT_10db6ee8[0].matrix._42
            && DAT_10db6ee8[0].matrix._41 < DAT_1083631c.z && DAT_10db6ee8[0].matrix._42 < DAT_1083631c.w)
        {
            if (DAT_104c0224 == 2)
            {
                if ((DAT_104bf890 & 1) == 0)
                {
                    Vec4 spawn = DAT_118e6cb8;
                    spawn.x = (float)(FUN_1013f880() % 2000 - 1000) + spawn.x;
                    spawn.y = (float)(FUN_1013f880() % 2000 - 1000) + spawn.y;
                    spawn.z = spawn.z - 200.0f;
                    EffectOptions options;
                    options.flags = 1;
                    DAT_10daaa80.Start(RAIN_EFFECT, spawn, -1, 0, 0, 0, 0, 0, &options, 0);
                }
            }
            else
            {
                i = 2;
                do
                {
                    Vec4 drop = DAT_118e6cb8;
                    drop.x = (float)(FUN_1013f880() % 0xa28 - 0x514) + drop.x;
                    drop.y = (float)(FUN_1013f880() % 0xa28 - 0x514) + drop.y;
                    drop.z = (float)(400 - FUN_1013f880() % 0x32) + drop.z;
                    FUN_1000c5c0(drop);
                    i = i - 1;
                } while (i != 0);
            }
        }
        FUN_1000c390();

        // The levers count down their pulse and press their parts when it ends.
        for (i = 0; i < KSK_COUNT; i++)
        {
            if (DAT_10219970[i] != 0)
                DAT_10219970[i] = DAT_10219970[i] - 1;
            if (DAT_10467718[i] != 0)
            {
                DAT_10467718[i] = DAT_10467718[i] - 1;
                if (DAT_10467718[i] == 0)
                {
                    char kskName[0x40];
                    sprintf(kskName, "@ksk%02d+", i);
                    for (modelPos = 0; modelPos < KSK_PARTS; modelPos++)
                    {
                        Node *kskPart = (Node *)DAT_10255f08->M_1019ed10(kskName, modelPos);
                        if (kskPart == 0)
                            break;
                        if (kskPart->name[strlen(kskPart->name) - 1] != '0')
                            kskPart->M_10191d90();
                    }
                }
            }
        }

        // The fish swim: they keep their distance from the player and turn away from walls.
        if (DAT_10210f38[0] != 0)
        {
            int fish;
            int dirTry;
            for (i = FISH_MAX; i < FISH_NODE_COUNT; i++)
            {
                Vec4 away;
                away.Set(FISH_HIDDEN_X, 0.0f, 0.0f, 0.0f);
                DAT_10210f38[i]->matrix.SetTranslation(away);
            }
            for (fish = 0; fish < FISH_MAX; fish++)
            {
                Vec4 swim;
                swim.Fill(0.0f);
                int swimming = 0;
                int fleeing = 0;
                DAT_1042d958[fish] = DAT_1042d958[fish] % FISH_WAVE_FRAMES;
                Vec4 pos = DAT_10210f38[fish]->matrix.GetTranslation();
                if (DAT_1021a1c0[fish] == 0)
                {
                    DAT_1021a1c0[fish] = 1;
                    pos.Set((float)(FUN_1013f880() % 0x5dc), 0.0f, (float)(-FISH_DEPTH_MIN - FUN_1013f880() % 0x3c), 0.0f);
                    pos.RotateZ((float)FUN_1013f880() / 100.0f);
                    pos.y = pos.y + FISH_START_Y;
                    DAT_10210f38[fish]->frameSource = &DAT_1042d958[fish];
                    DAT_10210f38[fish]->matrix.SetTranslation(pos);
                    DAT_10868618[fish].Set(0.0f, 1.0f, 0.0f, 0.0f);
                }
                Vec4 playerPos = DAT_10db6ee8[0].matrix.GetTranslation();
                if ((playerPos - pos).Length() < FISH_FLEE_START)
                    DAT_1021a1c0[fish] = 3;
                if (DAT_1021a1c0[fish] == 1)
                {
                    if (FUN_1013f880() % 1000 < 10)
                    {
                        DAT_1021a1c0[fish] = 2;
                        DAT_10daa880[fish].Set((float)(FUN_1013f880() % 200), 0.0f, 0.0f, 0.0f);
                        DAT_10daa880[fish].RotateZ((float)FUN_1013f880() / 100.0f);
                        DAT_10daa880[fish].RotateX((float)FUN_1013f880() / 100.0f);
                        DAT_10daa880[fish].Copy(pos + DAT_10daa880[fish]);
                        if (DAT_10daa880[fish].z > -FISH_DEPTH_MIN)
                            DAT_10daa880[fish].z = -FISH_DEPTH_MIN;
                        if (DAT_10daa880[fish].z < -FISH_DEPTH_MAX)
                            DAT_10daa880[fish].z = -FISH_DEPTH_MAX;
                    }
                    swim.x = (float)sin((double)((float)(fish * DAT_104bf890) / 100.0f));
                    swim.y = (float)sin((double)((float)(fish * DAT_104bf890 + 0x96) / 100.0f));
                    swim.z = (float)cos((double)((float)(fish * DAT_104bf890 + 0x122) / 100.0f));
                    swimming = 0;
                }
                if (DAT_1021a1c0[fish] == 2)
                {
                    swim.Copy(DAT_10daa880[fish] - pos);
                    if (swim.Length() < FISH_ARRIVE_DISTANCE)
                        DAT_1021a1c0[fish] = 1;
                    swim.Normalized3();
                    swim.Copy(swim * FISH_SWIM_SPEED);
                    DAT_1042d958[fish] = DAT_1042d958[fish] + 1;
                    if (DAT_1042d958[fish] > FISH_WAVE_FRAMES - 1)
                        DAT_1042d958[fish] = DAT_1042d958[fish] - FISH_WAVE_FRAMES;
                    swimming = 1;
                }
                if (DAT_1021a1c0[fish] == 3)
                {
                    if ((playerPos - pos).Length() > FISH_FLEE_END)
                        DAT_1021a1c0[fish] = 1;
                    swim.Copy(pos - playerPos);
                    swim.z = swim.z * 0.1f;
                    swim.Normalized3();
                    swim.Copy(swim * FISH_FLEE_SPEED);
                    DAT_1042d958[fish] = DAT_1042d958[fish] + 1;
                    if (DAT_1042d958[fish] > FISH_WAVE_FRAMES - 1)
                        DAT_1042d958[fish] = DAT_1042d958[fish] - FISH_WAVE_FRAMES;
                    fleeing = 1;
                    swimming = 1;
                }
                DAT_1042d958[fish] = DAT_1042d958[fish] + 1;
                if (DAT_1042d958[fish] > FISH_WAVE_FRAMES - 1)
                    DAT_1042d958[fish] = DAT_1042d958[fish] - FISH_WAVE_FRAMES;
                Matrix4 swimSpace;
                swimSpace.SetIdentity();
                Vec4 turn;
                Vec4 hitNormal;
                Vec4 newPos;
                float hitDepth;

                // The swim direction is tried straight, then turned by 45 and 90 degrees about each axis, until the
                // fish has room (inside the depth range and clear of the map).
                for (dirTry = 0; dirTry <= FISH_TURN_TRIES; dirTry++)
                {
                    turn.Copy(swim);
                    if (dirTry == 1)
                        turn.RotateZ(FISH_TURN_SMALL);
                    else if (dirTry == 2)
                        turn.RotateZ(-FISH_TURN_SMALL);
                    else if (dirTry == 3)
                    {
                        turn.RotateX(FISH_TURN_SMALL);
                        turn.RotateY(FISH_TURN_SMALL);
                    }
                    else if (dirTry == 4)
                    {
                        turn.RotateX(-FISH_TURN_SMALL);
                        turn.RotateY(-FISH_TURN_SMALL);
                    }
                    else if (dirTry == 5)
                        turn.RotateZ(FISH_TURN_LARGE);
                    else if (dirTry == 6)
                        turn.RotateZ(-FISH_TURN_LARGE);
                    else if (dirTry == 7)
                        turn.RotateX(FISH_TURN_LARGE);
                    else if (dirTry == 8)
                        turn.RotateX(-FISH_TURN_LARGE);
                    else if (dirTry == 9)
                        turn.RotateY(FISH_TURN_LARGE);
                    else if (dirTry == 10)
                        turn.RotateY(-FISH_TURN_LARGE);
                    newPos.Copy(pos + turn);
                    if (newPos.z > -FISH_DEPTH_MIN || newPos.z < -FISH_DEPTH_MAX
                        || DAT_10255f08->Query(&swimSpace, &newPos, FISH_RADIUS, &hitNormal, &hitDepth, 0, 0) != 0)
                        continue;
                    if (swimming != 0)
                    {
                        turn.Normalized3();
                        DAT_10868618[fish].Copy(DAT_10868618[fish] * FISH_COURSE_KEEP + turn * FISH_COURSE_STEER);
                        DAT_10868618[fish].Normalized3();
                        turn.Copy(DAT_10868618[fish]);
                        float yaw = (float)(atan2(-turn.x, -turn.y) + FISH_PI);
                        turn.RotateZ(yaw);
                        float pitch = (float)(atan2(-turn.z, -turn.y) + FISH_PI);
                        DAT_10210f38[fish]->matrix = RotationX(pitch) * RotationZ(yaw + FISH_PI);
                    }
                    DAT_10210f38[fish]->matrix.SetTranslation(newPos);
                    break;
                }
                if (swimming == 0)
                {
                    turn.Normalized3();
                    DAT_10868618[fish].z = DAT_10868618[fish].z * 0.8f;
                    DAT_10868618[fish].Normalized3();
                    turn.Copy(DAT_10868618[fish]);
                    float yaw = (float)(atan2(-turn.x, -turn.y) + FISH_PI);
                    turn.RotateZ(yaw);
                    float pitch = (float)(atan2(-turn.z, -turn.y) + FISH_PI);
                    newPos.Copy(DAT_10210f38[fish]->matrix.GetTranslation());
                    DAT_10210f38[fish]->matrix = RotationX(pitch) * RotationZ(yaw + FISH_PI);
                    DAT_10210f38[fish]->matrix.SetTranslation(newPos);
                }
                if (dirTry != 0 && DAT_1021a1c0[fish] == 2)
                    DAT_1021a1c0[fish] = 1;
                DAT_1042d958[fish] = DAT_1042d958[fish] + fleeing * FISH_WAVE_FRAMES;
            }
        }

        // The swing ("@BurankoRoot", seats "@b1" and "@b2"): the player standing within reach of a seat pumps it. The
        // power DAT_104c024c is level * 10 + a sub-step; the swing plays the motion "00d".."80d" of its level, and a
        // push that fits the 0x60 frame rhythm (DAT_104c0254 = start frame) raises it, a late or early one lets it fade.
        if (DAT_104c0240 != 0)
        {
            int swingFading = 0;
            if (DAT_10db9274 != 0 && DAT_10db9248 > SWING_MIN_HEIGHT)
            {
                Vec4 playerPos = DAT_10db6ee8[0].matrix.GetTranslation();
                Vec4 seatA = DAT_104c0244->worldMatrix.GetTranslation();
                Vec4 seatB = DAT_104c0248->worldMatrix.GetTranslation();
                if ((seatA - playerPos).Length2D() < SWING_REACH && DAT_104c024c == 0)
                {
                    DAT_104c0258 = DAT_104bf8a4;
                    DAT_104c0254 = DAT_104bf8a4;
                    DAT_104c0240->RestartColourPlugs("02d", DAT_104bf8a4);
                    DAT_104c024c = DAT_104c024c + SWING_START_POWER;
                    DAT_104c0250 = DAT_104c024c;
                }
                if ((seatB - playerPos).Length2D() < SWING_REACH && DAT_104c024c == 0)
                {
                    DAT_104c0258 = DAT_104bf8a4;
                    DAT_104c0254 = DAT_104bf8a4 - SWING_HALF_BEAT;
                    DAT_104c0240->RestartColourPlugs("02d", DAT_104c0254);
                    DAT_104c024c = DAT_104c024c + SWING_START_POWER;
                    DAT_104c0250 = DAT_104c024c;
                }
                if (((seatA - playerPos).Length2D() < SWING_REACH && DAT_104c024c != 0
                     && (DAT_104bf8a4 - DAT_104c0254) % SWING_BEAT >= SWING_HALF_BEAT)
                    || ((seatB - playerPos).Length2D() < SWING_REACH && DAT_104c024c != 0
                        && (DAT_104bf8a4 - DAT_104c0254) % SWING_BEAT < SWING_HALF_BEAT))
                {
                    swingFading = 1;
                    if (DAT_104c024c % 10 > 1 && DAT_104c024c % 10 != 9)
                        DAT_104c024c = DAT_104c024c - 1;
                }
                else if (((seatA - playerPos).Length2D() < SWING_REACH && DAT_104c024c != 0
                          && (DAT_104bf8a4 - DAT_104c0254) % SWING_BEAT < SWING_HALF_BEAT)
                         || ((seatB - playerPos).Length2D() < SWING_REACH && DAT_104c024c != 0
                             && (DAT_104bf8a4 - DAT_104c0254) % SWING_BEAT >= SWING_HALF_BEAT))
                {
                    if (DAT_104c024c % 10 < 9)
                        DAT_104c024c = DAT_104c024c + 1;
                }
            }
            if (DAT_104c024c != 0)
            {
                if (DAT_104bf890 % 5 == 0 && DAT_104c024c % 10 > 1)
                    DAT_104c024c = DAT_104c024c - 1;
                if (DAT_104c024c < SWING_LEVEL_STEP)
                {
                    DAT_104c0240->RestartColourPlugs("00d", DAT_104c0254);
                    DAT_104c024c = 0;
                }
                unsigned int swingBeat = (DAT_104bf8a4 - DAT_104c0258) % SWING_HALF_BEAT;
                if (swingBeat == SWING_SAVE_POWER_BEAT)
                    DAT_104c0250 = DAT_104c024c;
                int powerStep = DAT_104c0250 % 10;
                if ((powerStep == 8 || powerStep == 9 || powerStep == 0 || powerStep == 1) && swingBeat == 0)
                {
                    if (swingFading == 0)
                    {
                        if (powerStep == 8)
                            DAT_104c024c = DAT_104c0250 + 4;
                        if (powerStep == 9)
                            DAT_104c024c = DAT_104c0250 + 3;
                    }
                    else if (swingFading == 1)
                    {
                        if (powerStep == 8)
                            DAT_104c024c = DAT_104c0250;
                        if (powerStep == 9)
                            DAT_104c024c = DAT_104c0250;
                    }
                    if (powerStep == 0)
                    {
                        DAT_104c024c = DAT_104c0250 - 3;
                        if (DAT_104c024c < 0)
                            DAT_104c024c = 0;
                    }
                    if (powerStep == 1)
                    {
                        DAT_104c024c = DAT_104c0250 - 4;
                        if (DAT_104c024c < 0)
                            DAT_104c024c = 0;
                    }
                    if (DAT_104c024c / 10 == 0)
                        DAT_104c0240->RestartColourPlugs("00d", DAT_104c0254);
                    if (DAT_104c024c / 10 == 1)
                        DAT_104c0240->RestartColourPlugs("02d", DAT_104c0254);
                    if (DAT_104c024c / 10 == 2)
                        DAT_104c0240->RestartColourPlugs("04d", DAT_104c0254);
                    if (DAT_104c024c / 10 == 3)
                        DAT_104c0240->RestartColourPlugs("06d", DAT_104c0254);
                    if (DAT_104c024c / 10 == 4)
                        DAT_104c0240->RestartColourPlugs("10d", DAT_104c0254);
                    if (DAT_104c024c / 10 == 5)
                        DAT_104c0240->RestartColourPlugs("15d", DAT_104c0254);
                    if (DAT_104c024c / 10 == 6)
                        DAT_104c0240->RestartColourPlugs("20d", DAT_104c0254);
                    if (DAT_104c024c / 10 == 7)
                        DAT_104c0240->RestartColourPlugs("30d", DAT_104c0254);
                    if (DAT_104c024c / 10 == 8)
                        DAT_104c0240->RestartColourPlugs("40d", DAT_104c0254);
                    if (DAT_104c024c / 10 == 9)
                        DAT_104c0240->RestartColourPlugs("50d", DAT_104c0254);
                    if (DAT_104c024c / 10 == 10)
                        DAT_104c0240->RestartColourPlugs("60d", DAT_104c0254);
                    if (DAT_104c024c / 10 == 11)
                        DAT_104c0240->RestartColourPlugs("70d", DAT_104c0254);
                    if (DAT_104c024c / 10 > 11)
                        DAT_104c0240->RestartColourPlugs("80d", DAT_104c0254);
                }
            }
        }
        // The player's scene state DAT_104c00d0 (non-zero: the player cannot be pushed). 1 = ends when the counter
        // DAT_104c00d4 is 0; the others count down through timelines (below SCENE_TIMELINE_LIMIT, 1000..1999,
        // 2000..4999, 5000..10000) or run until they finish (SCENE_SHOVE_WAIT.., SCENE_SWAN_WAIT..).
        if (DAT_104c01a0 != 0)
            DAT_104c01a0 = DAT_104c01a0 - 1;
        if (DAT_104c00d4 == 0 && DAT_104c00d0 == 1)
            DAT_104c00d0 = 0;
        else if (DAT_104c00d0 > 1)
        {
            if (DAT_104c00d0 < SCENE_SWAN_WAIT)
            {
                if (DAT_104c00d0 < SCENE_SHOVE_WAIT)
                {
                    if (DAT_104c00d0 < SCENE_DAMAGE_LIMIT)
                    {
                        if (DAT_104c00d0 < SCENE_HAND_LIMIT)
                        {
                            if (DAT_104c00d0 < SCENE_SPARK_LIMIT)
                            {
                                if (DAT_104c00d0 < SCENE_BLANK_LIMIT)
                                {
                                    if (DAT_104c00d0 < SCENE_TIMELINE_LIMIT)
                                    {
                                        // The timeline counts down from SCENE_TIMELINE_START to 1.
                                        if (DAT_104c00d0 == SCENE_TIMELINE_START)
                                        {
                                            CloseEventOverlay();
                                            DAT_10db6ee8[0].m_0240 = 0;
                                            FUN_1003ad20(0x35e, 0, 1.0f);
                                            Node *seat = (Node *)DAT_104c00d8;
                                            if (DAT_104c00d8 != 0)
                                                seat->RestartColourPlugs("ON", DAT_104bf8a4);
                                            Vec4 effectAt;
                                            effectAt.Fill(0.0f);
                                            DAT_10daaa80.Start(0xfaa, effectAt, -1, 0, 0, 0, 0, &seat->worldMatrix, 0, 0);
                                            DAT_10daaa80.Start(0x5015, effectAt, -1, 0, 0, 0, 10, &seat->worldMatrix, 0, 0);
                                            DAT_10edbdd8.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
                                            DAT_10db6ee8[0].m_0294 = 0;
                                            DAT_10db6ee8[0].motionFrame = 0;
                                            DAT_10db6ee8[0].UpdateFrame(&DAT_108392d8, 1, 0, 0, 0);
                                            DAT_10db6ee8[0].PlayMotion(2, 0, 1, 1.0f);
                                            if (DAT_10c655f0 != COSTUME_THREE_POINTERS)
                                            {
                                                DAT_10db6ee8[SCENE_DOUBLE_MODEL].LoadFile(FUN_1013f8a0("char02bn.it3", 0));
                                                DAT_10db6ee8[SCENE_DOUBLE_MODEL].m_0294 = 0;
                                                DAT_10db6ee8[SCENE_DOUBLE_MODEL].motionFrame = 0;
                                                DAT_10db6ee8[SCENE_DOUBLE_MODEL].UpdateFrame(&DAT_108392d8, 1, 0, 0, 0);
                                                DAT_10db6ee8[SCENE_DOUBLE_MODEL].matrix = DAT_10db6ee8[0].matrix;
                                                DAT_10db6ee8[SCENE_DOUBLE_MODEL].heading = DAT_10db6ee8[0].heading;
                                                DAT_10db6ee8[SCENE_DOUBLE_MODEL].targetHeading = DAT_10db6ee8[0].targetHeading;
                                                DAT_10db6ee8[SCENE_DOUBLE_MODEL].m_17e8 = 1;
                                                DAT_10804bf8.AddLitModel(&DAT_10db6ee8[SCENE_DOUBLE_MODEL]);
                                                DAT_10db6ee8[0].root->M_10193cb0(0x21, 0x31);
                                                DAT_10db6ee8[0].root->M_10193cb0(0x1f, 0x32);
                                                DAT_10db6ee8[SCENE_DOUBLE_MODEL].root->M_10193cb0(0x21, 0x2f);
                                                DAT_10db6ee8[SCENE_DOUBLE_MODEL].root->M_10193cb0(0x1f, 0x30);
                                                FUN_1013faf0();
                                            }
                                            DAT_10edbdd8.z = DAT_10edbdd8.z + 40.0f;
                                            DAT_10f65598 = DAT_10edbdd8.z - 60.0f;
                                            DAT_10836c70.ResetFrameTimer();
                                        }
                                        if (DAT_104c00d0 == SCENE_TIMELINE_EFFECT)
                                        {
                                            FUN_1000f360(0, 3);
                                            Node *seat = (Node *)DAT_104c00d8;
                                            Vec4 effectAt;
                                            effectAt.Fill(0.0f);
                                            DAT_10daaa80.Start(0x5019, effectAt, -1, 0, 0, 0, 0, &seat->worldMatrix, 0, 0);
                                        }
                                        if (DAT_104c00d0 == SCENE_TIMELINE_VOICE)
                                        {
                                            Vec4 voiceAt = DAT_10db6ee8[0].matrix.GetTranslation();
                                            int voice = FUN_1013f880();
                                            if (DAT_104c0332 == 0)
                                                FUN_1003ad20(voice % 3 + 0x8f, &voiceAt, 1.0f);
                                            Node *seat = (Node *)DAT_104c00d8;
                                            Vec4 effectAt;
                                            effectAt.Fill(0.0f);
                                            DAT_10daaa80.Start(0x5016, effectAt, -1, 0, 0, 0, 0, &seat->worldMatrix, 0, 0);
                                            DAT_10daaae8[0].stats[STAT_HEALTH] = DAT_10daaae8[0].stats[STAT_MAX_HEALTH];
                                            FUN_1000e750();
                                        }
                                        if (DAT_104c00d0 == SCENE_TIMELINE_SHADE)
                                        {
                                            DAT_10f65598 = DAT_10edbdd8.z - 60.0f;
                                            DAT_10db6ee8[0].root->M_10193cb0(0x31, 0x2f);
                                            DAT_10db6ee8[0].root->M_10193cb0(0x32, 0x30);
                                            if (DAT_10c655f0 != COSTUME_THREE_POINTERS)
                                            {
                                                DAT_10db6ee8[SCENE_DOUBLE_MODEL].root->M_10193cb0(0x2f, 0x31);
                                                DAT_10db6ee8[SCENE_DOUBLE_MODEL].root->M_10193cb0(0x30, 0x32);
                                            }
                                        }
                                        DAT_10f65598 = DAT_10f65598 + 3.0f;
                                        if (DAT_104c00d0 == SCENE_TIMELINE_RELEASE)
                                        {
                                            DAT_104c0074 = 0;
                                            DAT_104bfdc8 = 0;
                                            DAT_104bfdcc = 0;
                                            DAT_104bfdd0 = 0;
                                            DAT_104c0090 = 1;
                                            FUN_10016500(0, "mt_refre.it3", 0, 200, 1);
                                            if (DAT_10c655f0 != COSTUME_THREE_POINTERS)
                                                FUN_10016500(SCENE_DOUBLE_MODEL, "mt_refre.it3", 0, 200, 1);
                                            DAT_10836c70.ResetFrameTimer();
                                        }
                                        Vec4 playerAt = DAT_10db6ee8[0].matrix.GetTranslation();
                                        playerAt.Copy(playerAt + (DAT_10edbdd8 - playerAt) * 0.1f);
                                        DAT_10db6ee8[0].matrix.SetTranslation(playerAt);
                                        if (DAT_10c655f0 != COSTUME_THREE_POINTERS)
                                        {
                                            DAT_10db6ee8[SCENE_DOUBLE_MODEL].matrix = DAT_10db6ee8[0].matrix;
                                            DAT_10db6ee8[0].heading = DAT_10db6ee8[0].targetHeading;
                                            DAT_10db6ee8[SCENE_DOUBLE_MODEL].heading = DAT_10db6ee8[0].targetHeading;
                                            DAT_10db6ee8[SCENE_DOUBLE_MODEL].targetHeading = DAT_10db6ee8[0].targetHeading;
                                            DAT_10db6ee8[SCENE_DOUBLE_MODEL].m_0267 = 1;
                                        }
                                        DAT_104c00d0 = DAT_104c00d0 - 1;
                                        if (DAT_104c00d0 == 1)
                                        {
                                            DAT_10db6ee8[0].m_0240 = 1;
                                            DAT_10db6ee8[DAT_104c0030].motionFrame = 0;
                                            DAT_10db6ee8[DAT_104c0030].M_10161b60();
                                            FUN_100121e0(0);
                                            DAT_10db6ee8[DAT_104c0030].m_0294 = 0;
                                            DAT_10db6ee8[0].PlayMotion(2, 0, 5, 1.0f);
                                            if (DAT_10c655f0 != COSTUME_THREE_POINTERS)
                                            {
                                                DAT_10db6ee8[0].root->M_10193cb0(0x2f, 0x21);
                                                DAT_10db6ee8[0].root->M_10193cb0(0x30, 0x1f);
                                                DAT_10db6ee8[SCENE_DOUBLE_MODEL].SetPosition(FISH_HIDDEN_X, 0.0f, 0.0f);
                                                DAT_10db6ee8[SCENE_DOUBLE_MODEL].root->M_10193cb0(0x31, 0x21);
                                                DAT_10db6ee8[SCENE_DOUBLE_MODEL].root->M_10193cb0(0x32, 0x1f);
                                            }
                                            FUN_1013faf0();
                                        }
                                    }
                                }
                                else
                                {
                                    // 1000..1999: a spark effect at the player while the counter is in 1005..1049.
                                    CloseEventOverlay();
                                    if (DAT_104c00d0 < SCENE_SPARK_END && DAT_104c00d0 > SCENE_SPARK_START)
                                    {
                                        EffectOptions options;
                                        int strength = DAT_10daaae8[0].stats[STAT_EFFECT_PARAM];
                                        if ((DAT_104c02c8 & HELMET_FACE_400) != 0)
                                            options.m_0004 = (int)((float)strength * 2.0f);
                                        else if ((DAT_104c02c8 & HELMET_FACE_300) == HELMET_FACE_300)
                                            options.m_0004 = (int)((float)strength * 1.7f);
                                        else if ((DAT_104c02c8 & HELMET_FACE_200) != 0)
                                            options.m_0004 = (int)((float)strength * 1.5f);
                                        else if ((DAT_104c02c8 & HELMET_FACE_100) != 0)
                                            options.m_0004 = (int)((float)strength * 1.2f);
                                        else
                                            options.m_0004 = strength;
                                        options.m_0008 = 1.0f;
                                        options.m_001c = 1;
                                        options.m_0000 = 1;
                                        options.settings[4] = 0;
                                        options.m_000e = 5;
                                        options.settings[5] = 1;
                                        options.settings[0] = 0x19;
                                        options.settings[1] = 0xc;
                                        Vec4 effectAt;
                                        effectAt.Fill(0.0f);
                                        Matrix4 playerMatrix;
                                        playerMatrix = DAT_10db6ee8[0].matrix;
                                        DAT_10daaa80.Start(0x8ec8, effectAt, -1, 0, 0, 0, 0, &playerMatrix, &options, 0);
                                    }
                                    DAT_104c00d0 = DAT_104c00d0 - 1;
                                    if (DAT_104c00d0 == SCENE_SPARK_FINISH)
                                        DAT_104c00d0 = 0;
                                }
                            }
                            else
                            {
                                // 2000..4999: the left hand glows while the counter is in 2020..2053.
                                DAT_104c00d0 = DAT_104c00d0 - 1;
                                CloseEventOverlay();
                                if (DAT_104c00d0 < SCENE_HAND_END && DAT_104c00d0 > SCENE_HAND_START)
                                {
                                    DAT_1101de30 = 1;
                                    DAT_1020344c = 0.6f;
                                    Node *hand = DAT_10db6ee8[0].root->Find("Bip01 L Hand", 0);
                                    Vec4 effectAt;
                                    effectAt.Fill(0.0f);
                                    Matrix4 handMatrix;
                                    handMatrix = DAT_10db6ee8[0].matrix;
                                    if (hand != 0)
                                        handMatrix.SetTranslation(hand->worldMatrix.GetTranslation());
                                    DAT_10daaa80.Start(0x8f02, effectAt, -1, 0, 0, 0, 0, &handMatrix, 0, 0);
                                }
                                if (DAT_104c00d0 == SCENE_HAND_FINISH)
                                {
                                    DAT_1020344c = 1.0f;
                                    if (DAT_104c01b0 == 0)
                                        FUN_10001b70((10 - DAT_102027d8) * 500);
                                    DAT_104687b0 = 1;
                                    DAT_104c00d0 = 0;
                                }
                            }
                        }
                        else
                        {
                            // 5000..10000: a damage state (the player takes damage every time the counter reaches
                            // SCENE_DAMAGE_TICK) and its recovery.
                            CloseEventOverlay();
                            DAT_104c00d0 = DAT_104c00d0 - 1;
                            if (DAT_104c00d0 == SCENE_DAMAGE_TICK)
                            {
                                DAT_104c00d0 = 0;
                                unsigned int damage = 5;
                                if (DAT_10c635f8 == 1)
                                    damage = 2;
                                else if (DAT_10c635f8 == 2)
                                {
                                    if (DAT_10c66cc0 == 1)
                                        damage = 0x14;
                                    else if (DAT_10c66cc0 == 0)
                                        damage = 10;
                                    else
                                        damage = 0x32;
                                }
                                if ((DAT_104c02c8 & HELMET_NO_DAMAGE) == 0)
                                {
                                    if ((DAT_104c02c8 & HELMET_HALF_DAMAGE) != 0)
                                        damage = damage / 2;
                                }
                                else
                                {
                                    FUN_10011aa0(0, 1);
                                    damage = 0;
                                }
                                FUN_1005dbf0(damage);
                                DAT_104c00d0 = SCENE_DAMAGE_TICK;
                                if (DAT_10daaae8[0].stats[STAT_HEALTH] > 0)
                                    FUN_1000da40(0, 0, 0x1e);
                            }
                            else
                            {
                                if (DAT_104c00d0 == SCENE_DAMAGE_RECOVER)
                                {
                                    DAT_104c0190 = 0;
                                    DAT_104c0108 = 0;
                                    DAT_104bfe70 = 0;
                                    FUN_1000ec70(5, 1.0f, 1.0f, 1.0f, 1.0f);
                                    ResetPlayerTrail();
                                    DAT_104687b0 = 1;
                                    DAT_10db9274 = 1;
                                    DAT_10db6ee8[0].Teleport(DAT_10daa400[DAT_1030c274].x, DAT_10daa400[DAT_1030c274].y,
                                                             DAT_10daa400[DAT_1030c274].z);
                                    DAT_10db6ee8[0].PlayMotion(2, 1, 5, 1.0f);
                                    FUN_1016a8e0();
                                    DAT_11018734 = 1;
                                    DAT_1101de30 = 1;
                                }
                                if (DAT_104c00d0 == SCENE_DAMAGE_END)
                                    DAT_104c00d0 = 0;
                            }
                        }
                    }
                }
                else
                {
                    // 20000..29999: the player is shoved aside: first wait until he stands still, then push him
                    // along the first of four directions that is clear, with growing strength.
                    if (DAT_104c00d0 == SCENE_SHOVE_WAIT)
                    {
                        if (DAT_10db6ee8[0].velocity.Length2D() >= 1.0f || DAT_10db9274 == 0)
                            DAT_104c00d0 = 0;
                        else
                        {
                            DAT_104c00d0 = SCENE_SHOVE;
                            DAT_10314eb8 = 0.2f;
                            DAT_10868828.Fill(0.0f);
                            ResetPlayerTrail();
                        }
                    }
                    if (DAT_104c00d0 == SCENE_SHOVE)
                    {
                        Vec4 shoveDirs[4];
                        Vec4 shoveFrom;
                        shoveDirs[0].Set(SHOVE_SIDE, 0.0f, SHOVE_DOWN, 0.0f);
                        shoveDirs[1].Set(-SHOVE_SIDE, 0.0f, SHOVE_DOWN, 0.0f);
                        shoveDirs[2].Set(0.0f, SHOVE_SIDE, SHOVE_DOWN, 0.0f);
                        shoveDirs[3].Set(0.0f, -SHOVE_SIDE, SHOVE_DOWN, 0.0f);
                        shoveFrom.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
                        for (i = 0; i < 4; i++)
                        {
                            Vec4 shoveDir;
                            shoveDir.Copy(shoveDirs[i]);
                            shoveDirs[i].Copy(shoveDirs[i] * DAT_10314eb8);
                            if (DAT_10db6ee8[0].IsClearOfMapLimited(&shoveDirs[i]) != 0)
                            {
                                DAT_10db6ee8[0].important = 1;
                                DAT_10db6ee8[0].PlayMotion(0x67, 1, 8, 1.0f);
                                DAT_10314eb8 = DAT_10314eb8 + 0.05;
                                if (DAT_10314eb8 >= 4.0f)
                                    DAT_10314eb8 = 4.0f;
                                DAT_10868828.Copy(shoveDir);
                                DAT_10db6ee8[0].Push(shoveDirs[i].x, shoveDirs[i].y, 0.0f);
                                shoveFrom.Copy(shoveFrom + shoveDirs[i]);
                                DAT_10db6ee8[0].FaceAway(shoveFrom);
                                if (DAT_104bfe1c != 0)
                                {
                                    DAT_1030c54c = 0;
                                    DAT_104bfd78 = 0;
                                    DAT_104b7200.CloseWindow(WINDOW_ID_SAVE_MENU);
                                    DAT_104b7200.CloseWindow(WINDOW_ID_CHOICE_MENU);
                                    DAT_104b7200.CloseWindow(WINDOW_ID_YES_NO);
                                    DAT_1020e0f8[0] = 0;
                                    FUN_1000cb30("");
                                    FUN_1000cb90("");
                                    DAT_104bf8d4 = 1;
                                    if (DAT_104c03c8 != 0)
                                    {
                                        DAT_104c03dc = 0;
                                        DAT_104c03c8->Release();
                                        DAT_11b1ba40(DAT_104c03c8);
                                        DAT_104c03c8 = 0;
                                        DAT_104c03cc = 0;
                                        DAT_104c03d0 = 0;
                                        DAT_104c03d4 = 0;
                                        DAT_104c03d8 = 0;
                                        DAT_104c03e0 = 0;
                                        DAT_104c03e4 = 0;
                                        for (i = 0; i < OVERLAY_TEXTURE_SLOT_COUNT; i++)
                                        {
                                            if (DAT_1030b870[i] > -1)
                                            {
                                                FUN_10174c70(DAT_1030b870[i], 1);
                                                DAT_1030b870[i] = -1;
                                            }
                                        }
                                    }
                                    DAT_104b7200.CloseWindow(WINDOW_ID_CHOICE_MENU);
                                    DAT_104b7200.CloseWindow(WINDOW_ID_YES_NO);
                                    DAT_1020e0f8[0] = 0;
                                    DAT_104bfe1c = 0;
                                    memset(DAT_1030c500, 0, sizeof(DAT_1030c500));
                                }
                                goto sceneStateDone;
                            }
                        }
                        DAT_10314eb8 = DAT_10314eb8 - 0.2f;
                        Vec4 shove;
                        shove.Copy(DAT_10868828 * DAT_10314eb8);
                        DAT_10db6ee8[0].Push(shove.x, shove.y, 0.0f);
                        if (DAT_10314eb8 < 1.05)
                        {
                            DAT_104c00d0 = 0;
                            DAT_10db6ee8[0].PlayMotion(2, 1, 8, 1.0f);
                        }
                    }
                }
            }
            else
            {
                // From 30000: the swan ride. The player follows the swan node ("swanat") and turns with the swan.
                CloseEventOverlay();
                if (DAT_104c00d0 == SCENE_SWAN_WAIT)
                {
                    ResetPlayerTrail();
                    DAT_10db6ee8[0].PlayMotion(0x68, 1, 8, 1.0f);
                    DAT_104c00d0 = SCENE_SWAN;
                    DAT_104c0260 = 0;
                    if (DAT_104c025c == 1)
                    {
                        DAT_104c0268->RestartColourPlugs("A", DAT_104bf8a4);
                        DAT_104c025c = 0;
                    }
                    else
                    {
                        DAT_104c0268->RestartColourPlugs("B", DAT_104bf8a4);
                        DAT_104c025c = 1;
                    }
                }
                DAT_10db6ee8[0].SetPosition(DAT_104c0264->worldMatrix._41, DAT_104c0264->worldMatrix._42,
                                            DAT_104c0264->worldMatrix._43);
                Matrix4 swanMatrix;
                swanMatrix = DAT_104c0268->worldMatrix;
                swanMatrix._41 = 0.0f;
                swanMatrix._42 = 0.0f;
                swanMatrix._43 = 0.0f;
                float swanHeading = swanMatrix.M_101b0a80();
                DAT_104c0260 = DAT_104c0260 + 1;
                DAT_10db6ee8[0].targetHeading = swanHeading - SWAN_HEADING_OFFSET;
                DAT_10db6ee8[0].heading = DAT_10db6ee8[0].targetHeading;
                if (DAT_104c0260 > SWAN_RIDE_FRAMES)
                {
                    DAT_104c025d = 0x1e;
                    DAT_104c00d0 = 0;
                    DAT_10db6ee8[0].PlayMotion(2, 1, 8, 1.0f);
                }
                if (DAT_104c0260 % SWAN_SOUND_PERIOD == 0)
                    FUN_10065b10(0, 0x3b5, 0, 0);
            }
        }
sceneStateDone:
        // Caption scroll timer: counts up every fifth frame and wraps after the scroll range.
        if (DAT_104bf890 % 5 == 0)
        {
            DAT_104c02cc++;
            if (DAT_104c02cd + 10 < DAT_104c02cc)
                DAT_104c02cc = 0;
        }
        DAT_104c00d5 = DAT_104c00d4;
        if (DAT_104c00d4 != 0)
            DAT_104c00d4--;

        // Doors (switch models of kind 0x13/0x14): the animation frame in DAT_1020b128 triggers the motions.
        AnimModel *door = &DAT_10db6ee8[DOOR_FIRST_MODEL];
        for (int doorSlot = DOOR_FIRST_MODEL; door < &DAT_10db6ee8[DOOR_END_MODEL]; doorSlot++, door++)
        {
            if (DAT_1021f3c0[doorSlot] != 0x13 && DAT_1021f3c0[doorSlot] != 0x14)
                continue;
            if (DAT_1020b128[doorSlot] == DOOR_FRAME_START)
            {
                Vec4 facing;
                Vec4 toPlayer;
                facing.Set(0.0f, 1.0f, 0.0f, 0.0f);
                facing.RotateZ(3.1415f - door->targetHeading);
                toPlayer.Copy(DAT_10db6ee8[0].matrix.GetTranslation() - door->matrix.GetTranslation());
                toPlayer.Normalized3();
                float side = Vec3_Dot(&facing, &toPlayer);
                if (DoorSwingsEffect(door->m_0284))
                    StartDoorEffect(door);
                if (side <= 0.0f)
                {
                    DAT_1021b3c8[doorSlot] = 1;
                    door->root->RestartColourPlugs("OPEN1", DAT_104bf8a4);
                }
                else
                {
                    DAT_1021b3c8[doorSlot] = 0;
                    door->root->RestartColourPlugs("OPEN0", DAT_104bf8a4);
                }
            }
            if (DAT_1020b128[doorSlot] == DOOR_FRAME_SETTLE)
                DAT_1020b128[doorSlot] = 0;
            if (DAT_1020b128[doorSlot] == DOOR_FRAME_OPENED)
            {
                if (DoorSwingsEffect(door->m_0284))
                    StartDoorEffect(door);
                door->root->RestartColourPlugs("OPEN0", DAT_104bf8a4);
            }
            if (DAT_1020b128[doorSlot] == DOOR_FRAME_OPEN_LOOP)
            {
                DAT_1020b128[doorSlot] = 0;
                door->root->RestartColourPlugs("OPEN", DAT_104bf8a4);
            }
            if (DAT_1020b128[doorSlot] == DOOR_FRAME_CLOSING)
            {
                door->root->RestartColourPlugs(DAT_1021b3c8[doorSlot] == 0 ? "CLOSE0" : "CLOSE1", DAT_104bf8a4);
                if (DoorSwingsEffect(door->m_0284))
                    StartDoorEffect(door);
            }
            if (DAT_1020b128[doorSlot] == DOOR_FRAME_CLOSE_SETTLE)
                DAT_1020b128[doorSlot] = 0;
            if (DAT_1020b128[doorSlot] == DOOR_FRAME_CLOSED)
            {
                door->root->RestartColourPlugs("CLOSE0", DAT_104bf8a4);
                if (door->m_0284 == 0xcd || door->m_0284 == 0xce)
                    StartDoorEffect(door);
            }
            if (DAT_1020b128[doorSlot] == DOOR_FRAME_END)
            {
                DAT_1020b128[doorSlot] = 0;
                door->root->RestartColourPlugs("CLOSE", DAT_104bf8a4);
            }
            if (DAT_1020b128[doorSlot] != 0)
                DAT_1020b128[doorSlot]++;
        }

        // Per-tick updates of the effects, windows, messages, fades and the other systems.
        DAT_10daaa80.Update((DAT_104bfe1c == 0 && DAT_104c0135 == 0) ? 0 : 1,
                            (tick == ticks - 1 || DAT_1065cb90 != 0) ? 1 : 0, ~DAT_104bf890 & 1);
        if (DAT_104bf8d4 != 0)
            FUN_1000cbe0();
        FUN_1000ed20();
        FUN_1003a6c0();
        FUN_1000f320();
        FUN_100f5a80();
        TutorialBook_Update();
        FUN_10058500();
        if (DAT_104bfe1c == 0)
        {
            FUN_100564d0();
            FUN_1000d670();
            FUN_1015f4e0();
        }
        FUN_1000f490();
        FUN_1003a9d0();
        if (DAT_104bfd28 != 0)
            FUN_1000ec10();
        FUN_1000cdf0();
        if (DAT_104bf8c8 != 0)
            FUN_1000ee60();
        if (DAT_104bff00 == 0)
        {
            if (DAT_104bff0c > 0)
                DAT_104bff0c--;
        }
        else if (DAT_104bff0c < MESSAGE_FADE_MAX)
            DAT_104bff0c++;
        FUN_1003a940();
        FUN_1005c680();
        if (DAT_104bfd78 != 0)
        {
            if (DAT_104bfd7c == 1 && (DAT_104bfd78 += 2, DAT_104bfd78 > MESSAGE_SLIDE_FULL - 1))
            {
                DAT_104bfd78 = MESSAGE_SLIDE_FULL;
                DAT_104bfd7c = 0;
            }
            else if (DAT_104bfd78 == 0 || DAT_104bfd7c != 2 || (DAT_104bfd78 -= 2, DAT_104bfd78 > 0))
                ;
            else
            {
                DAT_104bfd78 = 0;
                DAT_104bfd7c = 0;
            }
        }
        DAT_104bf88c = DAT_104bf88c + DAT_10202348;
        if (DAT_104bf88c == 10 || DAT_104bf88c == 0)
            DAT_10202348 = -DAT_10202348;
        steerX = 0.0f;
        steerY = 0.0f;
        DAT_1020234c = 0.5f;
        moveInput = 0;
        if (DAT_104c00bd != 0 && (DAT_104c00bd++, DAT_104c00bd > 0x3b))
            DAT_104c00bd = 0;

        // Dig: DAT_104bfd60 counts the frames of the dig (long when DAT_104bfd64 is set); it digs the ground in
        // front of the player once and then waits for the end.
        if (DAT_104bfd60 != 0)
        {
            int digStart = DAT_104bfd64 * DIG_LONG_FRAMES + DIG_START_FRAME;
            if (DAT_104bfd60 < digStart && DAT_104bfd64 != 0 && DAT_10db9274 != 0)
            {
                DAT_10db6ee8[0].verticalSpeed = DAT_104bfd68;
                DAT_104bfd60 = digStart;
            }
            if (DAT_10db6ee8[0].verticalSpeed < DAT_104bfd68)
                DAT_104bfd68 = DAT_10db6ee8[0].verticalSpeed;
            if (DAT_104bfd60 == digStart)
            {
                DAT_104bfd60++;
                Vec4 digger = DAT_10db6ee8[0].matrix.GetTranslation();
                DAT_104bfdb0 = 0;
                FUN_1003ad20(0x35f, &DAT_10db6ee8[0].matrix.GetTranslation(), 1.0f);
                int digRadius = DAT_104bfd64 == 0 ? DIG_RADIUS_SHORT : DIG_RADIUS_LONG;
                FUN_1003bc60((int)(cos(DIG_HEADING) * DIG_REACH + (digger.x - DAT_10edbd38.x * 2.0f)),
                             (int)(sin(DIG_HEADING) * DIG_REACH + (digger.y - DAT_10edbd38.y * 2.0f)), 1, digRadius);
                if (DAT_104bfd64 != 0)
                    DAT_104bfd60 = DAT_104bfd64 * DIG_LONG_FRAMES + DIG_END_FRAME;
            }
            if (DAT_104bfd60 == DAT_104bfd64 * DIG_LONG_FRAMES + DIG_END_FRAME)
            {
                DAT_104c0091 = 8;
                Vec4 digger = DAT_10db6ee8[0].matrix.GetTranslation();
                IntRect area;
                area.left = (int)(cos(DIG_HEADING) * DIG_SHADE_OFFSET + (digger.x - DAT_10edbd38.x * 2.0f) - DIG_SHADE_SIZE);
                area.top = (int)(sin(DIG_HEADING) * DIG_SHADE_OFFSET + (digger.y - DAT_10edbd38.y * 2.0f) - DIG_SHADE_SIZE);
                area.right = (int)(cos(DIG_HEADING) * DIG_SHADE_OFFSET + (digger.x - DAT_10edbd38.x * 2.0f) + DIG_SHADE_SIZE);
                area.bottom = (int)(sin(DIG_HEADING) * DIG_SHADE_OFFSET + (digger.y - DAT_10edbd38.y * 2.0f) + DIG_SHADE_SIZE);
                FUN_10016ec0(&area, -1, 1);
            }
            if (DAT_104bfd60 == DAT_104bfd64 * DIG_LONG_FRAMES + DIG_DONE_FRAME)
                DAT_104bfd60 = 0;
            else if (DAT_104bfd60 != 0)
                DAT_104bfd60++;
        }

        // Event markers (DAT_1020d660): age them and project their world position to the 640x480 screen.
        for (int marker = 0; marker < EVENT_MARKER_COUNT; marker++)
        {
            if (DAT_1020d660[marker] == 0)
                continue;
            if (DAT_104463f8[marker] < EVENT_MARKER_FRAMES)
                DAT_104463f8[marker]++;
            else
                DAT_1020d660[marker] = 0;
            Vec4 markerAt;
            int markerY;
            if (DAT_1021f640[marker] == 3)
            {
                if (DAT_104463f8[marker] < 2)
                {
                    markerAt.Copy(DAT_10db6ee8[0].headNode->worldMatrix.GetTranslation());
                    markerAt.z += MARKER_HEAD_LIFT;
                    markerAt.Copy(Project(markerAt, DAT_108392d8));
                    DAT_10253c58[marker] = (int)((markerAt.x + 1.0f) * SCREEN_HALF_WIDTH);
                    DAT_10256218[marker] = (int)(SCREEN_HEIGHT - (markerAt.y + 1.0f) * SCREEN_HALF_HEIGHT);
                }
                markerY = DAT_10256218[marker] - 2;
            }
            else
            {
                int target = DAT_10447fd8[marker];
                if (DAT_1021f3c0[target] == 3)
                {
                    markerAt.Copy(DAT_1020e338[target]->GetTranslation());
                    if (markerAt.x <= MARKER_OFFSCREEN_X)
                        continue;
                }
                else if (DAT_10db6ee8[target].root == 0)
                    markerAt.Copy(DAT_10db6ee8[target].matrix.GetTranslation());
                else
                    markerAt.Copy(DAT_10db6ee8[target].root->worldMatrix.GetTranslation());
                markerAt.Copy(Project(markerAt, DAT_108392d8));
                DAT_10253c58[marker] = (int)((markerAt.x + 1.0f) * SCREEN_HALF_WIDTH);
                markerY = (int)(SCREEN_HEIGHT - (markerAt.y + 1.0f) * SCREEN_HALF_HEIGHT);
            }
            DAT_10256218[marker] = markerY;
        }

        // Movies: DAT_104bfe6c 3 = DirectShow movie, 2 = movie library frames; DAT_104bfe70 = the game over movie.
        if (DAT_104bfe6c == 3)
        {
            if (DAT_10836e98.M_10148930() == 0)
            {
                DAT_10836e98.M_10148960();
                DAT_10836e98.M_1015c6a0(&DAT_118d0ba8[DAT_10202478]);
            }
            else
            {
                DAT_10836e98.M_10148920();
                DAT_10836e98.Close();
                DAT_104bfe6c = 0;
            }
        }
        if (DAT_104bfe6c == 2 && (DAT_104bf890 & 1) != 0)
        {
            if (tick == ticks - 1 || tick == ticks - 2)
            {
                if (FUN_101b25a0(0) == 0)
                    DAT_104bfe6c = 0;
                else
                    DAT_118d0ba8[0].Write16(0, 0, DAT_11b1bb58, DAT_11b1bb64, DAT_11b1bb58 * 2, DAT_11b1bb54);
            }
            else if (FUN_101b25a0(2) == 0)
                DAT_104bfe6c = 0;
        }
        if (DAT_104bfe70 != 0)
        {
            if ((DAT_104bfe70 & 1) == 0)
                DAT_10f08ab0.M_10161050();
            DAT_10f08ab0.DrawToTexture(&DAT_118d0ba8[DAT_10202478]);
            DAT_104bfe70++;
            if (DAT_10f08ab0.m_004b == 0)
                DAT_104bfe70 = 0;
        }

        // Game over: the player fell out of the world (or the game over movie is running).
        if (DAT_104bfe6c == 0)
        {
            if (DAT_1032d69c != 1 || -3450.0f <= DAT_10db9248 || DAT_104bfe78 != 0)
            {
                if (DAT_104bfe78 < GAME_OVER_END_FRAME)
                {
                    if (DAT_104bfe78 != 0)
                        goto gameOverStep;
                }
                else
                    DAT_104bfe78 = 0;
                goto waterThaw;
            }
            DAT_10f08ab0.Open(FUN_1013f8a0("gameover.mmv", 0));
            DAT_10f08ab0.DrawToTexture(&DAT_118d0ba8[DAT_10202478]);
            FUN_1003ad20(0x335, &DAT_10db6ee8[0].matrix.GetTranslation(), 1.0f);
            DAT_10f08ab0.state = MMV_PLAYING;
            DAT_10f08ab0.loop = 0;
            FUN_1000da40(0, 100, 0x28);
            DAT_10db6ee8[0].PlayMotion(0x84, 1, 5, 1.0f);
            DAT_104bfe78 = 1;
            if (DAT_104bfe6c == 0)
            {
gameOverStep:
                DAT_104bfe70 = 0;
                gameOverButton = 0;
                gameOverPadA = 0;
                gameOverPadB = 0;
                if (DAT_104bfe78 == 2)
                {
                    FUN_1013fa10(0, 10, 1.0f, 1.0f, 1.0f, 1.0f);
                    FUN_1013fa10(1, 10, 0.0f, 0.0f, 0.0f, 1.0f);
                    FUN_1000ec70(10, 1.0f, 1.0f, 1.0f, 1.0f);
                }
                if (DAT_104bfe78 == GAME_OVER_FADE_FRAME)
                {
                    if (DAT_104bf8e8 < 100.0f)
                        FUN_1000da40(0, 100, 0x14);
                    goto gameOverCount;
                }
                if (DAT_104bfe78 == GAME_OVER_SOUND_FRAME)
                {
                    if (DAT_104c0332 != 0)
                        goto gameOverCount;
                    FUN_1003ad20((DAT_104c033a + 10) * 0xf, 0, 0.8f);
                }
                if (DAT_104bfe78 < GAME_OVER_END_FRAME2)
                {
gameOverCount:
                    DAT_104bfe78++;
                }
                if ((DAT_104bfe78 & 1) == 0)
                    DAT_10f08ab0.M_10161050();
                DAT_10f08ab0.DrawToTexture(&DAT_118d0ba8[DAT_10202478]);
                DAT_1020d9e0.M_10001450();
                DAT_1020d9e0.M_10001110(gameOverPos, &gameOverButton, &gameOverPadA, &gameOverPadB, 0, 0);
                if (DAT_104bfe78 > GAME_OVER_SKIP_FRAME && (DAT_1065cb30 != 0 || gameOverButton == 2))
                {
                    // Skipped: back to the map's start point with fresh health.
                    int gameOvers;
                    FUN_1000da40(0, 0, 0x1e);
                    if (DAT_104c01e0 != 0)
                    {
                        DAT_11018734 = 1;
                        DAT_1101de30 = 1;
                    }
                    DAT_104c01e0 = 0;
                    DAT_104c0190 = 0;
                    DAT_104c0108 = 0;
                    FUN_1000ec70(5, 1.0f, 1.0f, 1.0f, 1.0f);
                    ResetPlayerTrail();
                    DAT_10c62ed0 = DAT_1030b660;
                    DAT_10db6ee8[0].Teleport(DAT_108029b8.x, DAT_108029b8.y, DAT_108029b8.z);
                    DAT_10db9364 = DAT_108029b8.w;
                    DAT_10db9368 = (int)DAT_108029b8.w;
                    DAT_104bfe4c = 0;
                    DAT_104bfe50 = 0;
                    DAT_104c00b0 = 0;
                    DAT_102024bc = -1;
                    DAT_104bfe30 = 0;
                    DAT_10db6ee8[0].PlayMotion(5, 1, 0, 1.0f);
                    DAT_10db6ee8[0].PlayMotion(2, 1, 0, 1.0f);
                    DAT_104686b0[PLAYER_SLOT] = 0;
                    DAT_10db7c34 = 1.0f;
                    DAT_10db9380 = 1;
                    DAT_104bfeb0 = 0;
                    DAT_104bfeb4 = 0;
                    FUN_1013fb10(0, 0, 0);
                    gameOvers = DAT_10c64000;
                    DAT_104c0128 = 0;
                    DAT_10253e98[PLAYER_SLOT] = 0;
                    if (DAT_10c635f8 == 0)
                        DAT_10daaaec = DAT_10daaaf0 / 2;
                    else if (DAT_10c635f8 == 1)
                        DAT_10daaaec = DAT_10daaaf0;
                    else
                    {
                        int divisor;
                        if (DAT_10c635f8 != 2)
                            goto gameOverResetDone;
                        if (DAT_10c66cc0 == 1)
                            divisor = DAT_10c64000 + 3;
                        else
                        {
                            divisor = DAT_10c64000 * 2 + 4;
                            if (DAT_10c66cc0 == 0)
                                divisor = DAT_10c64000 + 2;
                        }
                        DAT_10daaaec = DAT_10daaaf0 / divisor;
                        if (DAT_10daaaec < 1)
                            DAT_10daaaec = 1;
                    }
                    FUN_1000e750();
gameOverResetDone:
                    DAT_1021cff8 = 1;
                    DAT_103086a8 = 0;
                    DAT_1030c900 = 0;
                    DAT_1042de20 = DAT_1030b660;
                    DAT_104bfe78 = GAME_OVER_RESTART_FRAME;
                    DAT_10c62ed0 = DAT_1030b660;
                    DAT_10c635ec = 1;
                    DAT_10c64000 = gameOvers + 1;
                    DAT_10db6ee8[0].m_0075 = 0;
                    DAT_11018734 = 1;
                    DAT_1101de30 = 1;
                    return 0;
                }
                goto gameOverAfter;
            }
        }
        else
        {
gameOverAfter:
            if (DAT_104bfe78 == 0)
            {
waterThaw:
                // The frozen water thaws when its timer runs out.
                if (DAT_104c02dc != 0 && DAT_104c02e7 != 0 && (DAT_104c02e7--, DAT_104c02e7 < 2))
                {
                    FUN_1000dab0(0, 100, 0, 0xff5050);
                    FUN_10011fb0(0);
                    FUN_1000dab0(0, 0, 0xf, 0xff5050);
                    DAT_104c02dc = 0;
                    DAT_104c02e7 = 0x1e;
                }
            }
        }
        if (DAT_104bfe20 == 0)
            DAT_104c00b2 = 0;
        else if (DAT_104c00b2 < 0x32)
            DAT_104c00b2++;

        // Input of this tick: mouse state, the Enter key and the caption timers.
        mouseLeft = 0;
        mouseRight = 0;
        mouseMiddle = 0;
        mouseExtra = 0;
        mouseWheel = 0;
        modelIdx = DAT_10202464[0];
        DAT_1020d9e0.M_10001450();
        mousePos[0] = 0;
        mousePos[1] = 0;
        if (DAT_1065cb98 == 0)
            DAT_1020d9e0.M_10001110(mousePos, &mouseLeft, &mouseRight, &mouseMiddle, &mouseWheel, &mouseExtra);
        if (DAT_104c030a != 0 && ((GetKeyState(VK_RETURN) & 0xf000) != 0 || mouseLeft == 2) && DAT_10f6811c > 5)
            DAT_104c030c = DAT_104bf890;
        if (DAT_104c0148 != 0)
            DAT_104c0148--;
        DAT_104c022e = 0;
        inputEvents = 0;
        if ((DAT_104bfe6c == 2 || DAT_104bfe6c == 3) && (DAT_104bfeb4 != 0 || (DAT_104c0230 & 3) != 0)
            && (mouseLeft == 2 || DAT_1065cb30 != 0))
            DAT_104bfe6c = 0;
        if (DAT_104c0334 != 0)
            DAT_104c0334--;

        if (DAT_104c00d0 < 2 && FUN_1015d510() == 0 && DAT_104c0334 == 0 && DAT_104bf890 > 4 && DAT_104c0135 == 0
            && DAT_104bfe78 == 0 && DAT_104bfe6c == 0 && DAT_104bfd08 == 0 && DAT_104bfd60 == 0
            && DAT_10db6ee8[DAT_104bf8b4].M_1017b160() == 0)
        {
            inputActive = 1;

            if (DAT_1032d69c != 0)
            {
                if (DAT_1065cb90 == 0)
                {
                    if (DAT_1020d9e0.M_10001440() != 0)
                    {
                        if (DAT_104c0204 != 0)
                        {
                            mouseLeft = 0;
                            mouseMiddle = 0;
                            mouseRight = 0;
                            mouseWheel = 0;
                        }

                        // The item list (while the overlay mode is open): wheel scrolling, hover and the page tabs.
                        if (DAT_104bfdf0 == 0 && DAT_104c0028 == 0)
                        {
                            int row;
                            if (DAT_104c0068 == 1 || DAT_104bfe1c == 0 || mousePos[0] < ITEM_MENU_LEFT || mousePos[1] < ITEM_MENU_TOP
                                || mousePos[0] > ITEM_MENU_RIGHT || mousePos[1] > ITEM_MENU_BOTTOM)
                            {
                                DAT_104c0050 = 0;
                                if (DAT_104bfe1c != 0)
                                    mouseExtra = 0;
                            }
                            else
                            {
                                if (mouseLeft == 2)
                                {
                                    if (DAT_104c02bc == 1)
                                    {
                                        if (mousePos[0] > 0x1e1 && mousePos[1] > 0x35 && mousePos[0] < 0x221 && mousePos[1] < ITEM_MENU_ROW_TOP)
                                        {
                                            DAT_104c02bc = 0;
                                            DAT_104c0048 = 0;
                                        }
                                    }
                                    else if (DAT_104c02bc == 0 && mousePos[0] > 0x21f && mousePos[1] > 0x35 && mousePos[0] < 0x26b
                                             && mousePos[1] < ITEM_MENU_ROW_TOP)
                                    {
                                        DAT_104c02bc = 1;
                                        DAT_104c0048 = 0;
                                    }
                                }
                                row = (mousePos[1] - ITEM_MENU_ROW_TOP) / ITEM_MENU_ROW_HEIGHT;
                                if (row > -1 && row < ITEM_MENU_ROWS)
                                    DAT_10202464[0] = DAT_104485e8[row];
                                if (mouseWheel < 1 || DAT_104c0048 < 1)
                                {
                                    if (mouseWheel < 0 && DAT_104c0044 - DAT_104c0048 > ITEM_MENU_ROWS && mousePos[1] > -0x3d
                                        && mousePos[1] < 0x19f)
                                        DAT_104c0048++;
                                    else
                                    {
                                        if (row < 0 && DAT_104c0048 > 0)
                                        {
                                            if (mouseWheel == 0)
                                                DAT_104bfe90 = 0;
                                            else
                                                DAT_104bfe90--;
                                            if (DAT_104c004c < 1)
                                            {
                                                DAT_104c0048--;
menuScrollRepeat:
                                                DAT_104c004c = 5 - DAT_104c0050;
                                                if (DAT_104c0050 < 5)
                                                    DAT_104c0050++;
                                                if (DAT_104c004c == 0)
                                                    goto menuInputDone;
                                            }
                                        }
                                        else
                                        {
                                            if (row < 0x10 || DAT_104c0044 - DAT_104c0048 < 0x10 || mousePos[1] < -0x3c
                                                || mousePos[1] > 0x1d6)
                                            {
                                                DAT_104c0050 = 0;
                                                if (row < ITEM_MENU_ROWS && row > -1 && DAT_104485e8[row] != 0)
                                                    DAT_104bfe90 = row;
                                                goto menuInputDone;
                                            }
                                            if (mouseWheel == 0)
                                                DAT_104bfe90 = ITEM_MENU_ROWS - 1;
                                            else
                                                DAT_104bfe90--;
                                            if (DAT_104c004c < 1)
                                            {
                                                DAT_104c0048++;
                                                goto menuScrollRepeat;
                                            }
                                        }
                                        DAT_104c004c--;
                                    }
                                }
                                else
                                    DAT_104c0048--;
                            }
                        }
menuInputDone:
                        if (mouseLeft == 0)
                            DAT_104c0097 = 0;

                        // Mouse wheel: turns the camera about the target.
                        viewWidth = (DAT_10f654b4 * 0x280) / DAT_10202b2c;
                        viewHeight = (DAT_10f654b8 * 0x1e0) / DAT_10202b30;
                        if (DAT_10468128 == 0 && DAT_10db6ee8[0].m_229c < 5 && DAT_104c010f != 0 && mouseWheel != 0
                            && (DAT_104bfe1c == 0 || (mousePos[0] < viewWidth + 0x280 && mousePos[1] < viewHeight + 0x1e0))
                            && FUN_1016c300() == 0)
                        {
                            DAT_1101de30 = 2;
                            DAT_11af3e28.Copy(DAT_11af3e28 - DAT_118e6c78);
                            Matrix4 turn = RotationZ(mouseWheel * CAMERA_WHEEL_TURN);
                            DAT_11af3e28.Copy(Project(DAT_11af3e28, turn) + DAT_118e6c78);
                        }

                        if (DAT_104bfdf0 == 0)
                        {
                            // Middle button closes the item list.
                            if (DAT_104bfe1c != 0 && DAT_104c0068 != 1 && DAT_104c0028 == 0 && mouseMiddle != 0)
                            {
                                FUN_1003ad20(SOUND_ITEM_LIST_CLOSE, 0, 1.0f);
                                mouseMiddle = 0;
                                DAT_104bfe90 = 0;
                                if (DAT_104bfe1c != 0)
                                {
                                    CloseBookModel();
                                    DAT_104b7200.CloseWindow(WINDOW_ID_CHOICE_MENU);
                                    DAT_104b7200.CloseWindow(WINDOW_ID_YES_NO);
                                    memset(DAT_1030c500, 0, sizeof(DAT_1030c500));
                                    DAT_1020e0f8[0] = 0;
                                    DAT_104bfe1c = 0;
                                }
                                DAT_104bfd74 = 1;
                                if (DAT_104bfdf0 != 0)
                                    goto itemMouseDone;
                            }
                            // Left click on the corner opens or closes the item list.
                            if (DAT_104c01e0 == 0 && DAT_104c0068 != 1 && DAT_104c0028 == 0 && mouseLeft != 0
                                && mousePos[0] + mousePos[1] > ITEM_LIST_CORNER)
                            {
                                if (mouseLeft == 2)
                                {
                                    DAT_104c0232 = 4;
                                    if (DAT_104bfe1c == 0)
                                    {
                                        FUN_1003ad20(SOUND_ITEM_LIST_OPEN, 0, 1.0f);
                                        DAT_104bfe1c = 1;
                                    }
                                    else
                                    {
                                        FUN_1003ad20(SOUND_ITEM_LIST_CLOSE, 0, 1.0f);
                                        DAT_104bfe90 = 0;
                                        if (DAT_104bfe1c != 0)
                                        {
                                            CloseBookModel();
                                            FUN_100106a0();
                                            DAT_104bfe1c = 0;
                                            memset(DAT_1030c500, 0, sizeof(DAT_1030c500));
                                        }
                                    }
                                    DAT_104bfe90 = DAT_1030c479;
                                    DAT_104c0048 = DAT_1049f1f9;
                                    DAT_104bfd74 = 1;
                                    DAT_102024b0 = 1;
                                    DAT_1020249c = 1;
                                    DAT_1030b868 = 2;
                                    DAT_104bfeb9 = 2;
                                    modelIdx = DAT_10202464[0];
                                }
                                goto afterMouseControls;
                            }
                        }
itemMouseDone:
                        // Mouse look: the cursor's offset from the player steers the character.
                        if (DAT_104c0099 < 0xf || DAT_104c0135 != 0)
                        {
                            if (DAT_104bfdf0 == 0)
                            {
aimWithMouse:
                                if (DAT_104bfd78 == 0 && DAT_104c0068 != 1
                                    && (DAT_104bfe1c == 0 || (mousePos[0] < viewWidth + 0x280 && mousePos[1] < viewHeight + 0x1e0)))
                                {
                                    int playerScreen[2];
                                    int motion;
                                    DAT_10db6ee8[0].ScreenPosition(&playerScreen[0], &playerScreen[1], DAT_108392d8);
                                    playerScreen[0] = (int)((float)(playerScreen[0] + 0x140) / 2.0f);
                                    playerScreen[1] = (int)((float)(playerScreen[1] + 0xf0) / 2.0f);
                                    aimX = (float)(mousePos[0] - playerScreen[0]) / AIM_X_DIVISOR;
                                    if (aimX >= AIM_LIMIT)
                                        aimX = AIM_LIMIT;
                                    else if (aimX <= -AIM_LIMIT)
                                        aimX = -AIM_LIMIT;
                                    aimY = -(float)(mousePos[1] - playerScreen[1]) / AIM_Y_DIVISOR;
                                    if (aimY >= AIM_LIMIT)
                                        aimY = AIM_LIMIT;
                                    else if (aimY <= -AIM_LIMIT)
                                        aimY = -AIM_LIMIT;
                                    DAT_102027e8 = aimX;
                                    DAT_104c0234 = aimY;

                                    // A fresh click behind the player turns the character around.
                                    if (DAT_104c0228 >= TURN_STATE_BASE && DAT_104bfe1c == 0 && mouseLeft != 0
                                        && (mouseLeft == 1 || mousePos[1] + mousePos[0] < 0x430) && DAT_104c0097 == 0
                                        && (aimX != 0.0f || aimY != 0.0f) && DAT_104c0228 == TURN_PENDING)
                                    {
                                        Vec4 facing;
                                        Vec4 wish;
                                        float behind;
                                        facing.x = cos(-DAT_10db6ee8[DAT_104bf8b4].targetHeading - HALF_PI);
                                        facing.y = sin(-DAT_10db6ee8[DAT_104bf8b4].targetHeading - HALF_PI);
                                        facing.z = 0;
                                        FUN_1016b340(&wish.x, &wish.y, aimX, aimY);
                                        wish.z = 0;
                                        wish.Normalized3();
                                        behind = Vec3_Dot(&facing, &wish);
                                        if (behind < TURN_AROUND_DOT)
                                        {
                                            DAT_10db6ee8[0].PlayMotion(2, 0, 5, 1.0f);
                                            DAT_104c0228 = 0;
                                            for (i = PLAYER_TRAIL_LENGTH; i != 0; i--)
                                            {
                                                DAT_10802ad8[DAT_104c022c].Copy(DAT_10db6ee8[0].matrix.GetTranslation());
                                                DAT_104c022c = DAT_104c022c + 1 & 0xf;
                                            }
                                        }
                                        else
                                            DAT_104c0228 = TURN_REFUSED;
                                    }
                                    if (mouseExtra != 0 && DAT_104c010e != 0 && DAT_10c652cc != 0 && DAT_104bfe1c == 0
                                        && DAT_104bfe30 == 0 && DAT_104bfdc8 == 0 && DAT_104c0300 == 0 && DAT_104bfe4c == 0
                                        && DAT_104c00e0 < 4)
                                        DAT_104c022e = 1;
                                    if (DAT_104c0228 < TURN_STATE_BASE && mouseLeft != 0
                                        && (mouseLeft == 1 || mousePos[1] + mousePos[0] < 0x430) && DAT_104c0097 == 0)
                                    {
                                        Vec4 unused;   // constructed by the original but never read
                                        if ((unsigned char)DAT_10db6ee8[0].m_2288 < 2)
                                        {
                                            steerY = aimY;
                                            steerX = aimX;
                                        }
                                        if (DAT_104c009d == 0)
                                        {
                                            if (abs((int)steerX) > 4 || abs((int)steerY) > 4)
                                                moveInput = 1;
                                        }
                                        else if (abs((int)steerX) > 2 || abs((int)steerY) > 2)
                                            moveInput = 1;
                                    }
                                    else if (DAT_104bfdc8 == 0 && DAT_1065cb48 == 0 && DAT_1065cb4c == 0 && DAT_104c0228 < TURN_STATE_BASE)
                                    {
                                        if (DAT_104c0300 == 0)
                                        {
                                            if (DAT_104bfe4c == 0)
                                            {
                                                if (DAT_104c00e0 == 0 && DAT_104c009e == 0)
                                                {
                                                    motion = 2;
                                                    goto playIdleMotion;
                                                }
                                            }
                                            else if (DAT_10468128 == 0 && DAT_10db6ee8[0].m_229c == 0)
                                            {
                                                motion = 0x1c;
playIdleMotion:
                                                DAT_10db6ee8[0].PlayMotion(motion, 0, 5, 1.0f);
                                            }
                                        }
                                        else if (DAT_10468128 == 0 && DAT_10db6ee8[0].m_229c == 0)
                                        {
                                            motion = 0x83;
                                            goto playIdleMotion;
                                        }
                                    }
                                }
                            }
                        }
                        else if (DAT_104bfdf0 == 0)
                        {
                            // The item list's entries are chosen with the mouse.
                            if (mouseLeft == 0 || DAT_104c0068 == 1 || DAT_104bfe1c == 0 || mousePos[0] < ITEM_MENU_LEFT
                                || mousePos[1] < ITEM_MENU_ROW_TOP || mousePos[0] > 0x26e || mousePos[1] > 0x194)
                                goto aimWithMouse;
                            if (DAT_104c0028 == 0)
                            {
                                if (mouseLeft == 2)
                                {
                                    int entry = DAT_104485e8[(mousePos[1] - ITEM_MENU_ROW_TOP) / ITEM_MENU_ROW_HEIGHT];
                                    if (entry != 0)
                                    {
                                        DAT_104c0148 = 2;
                                        if (entry == 0xa0)
                                            DAT_1030c564 = 1;
                                        else if (entry == 0xa1)
                                            DAT_1030c540 = 1;
                                        else if (entry == 0xa2)
                                            DAT_1030c558 = 1;
                                        else if (entry == 0xa3)
                                            DAT_1030c55c = 1;
                                        else if (entry == 0xa4)
                                            DAT_1030c510 = 1;
                                        else if (entry == 0xa5)
                                            DAT_1030c508 = 1;
                                        else if (entry == 0xa9)
                                        {
                                            CloseBookModel();
                                            FUN_100106a0();
                                            DAT_104bfe1c = 0;
                                            memset(DAT_1030c500, 0, sizeof(DAT_1030c500));
                                            FUN_1000da20(0x531);
                                        }
                                        else
                                        {
                                            inputEvents = 1;
                                            DAT_10202464[0] = entry;
                                            modelIdx = entry;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
afterMouseControls:
                if (DAT_1032d69c != 0)
                {
                    // Keys and pad: camera turn, camera reset and the lock-on target.
                    if ((DAT_1065cb44 != 0 || DAT_1065cb42 != 0) && FUN_1016c300() == 0)
                    {
                        float cameraTurn = -CAMERA_KEY_TURN;
                        if (DAT_1065cb44 == 0)
                            cameraTurn = CAMERA_KEY_TURN;
                        DAT_1101de30 = 2;
                        DAT_11af3e28.Copy(DAT_11af3e28 - DAT_118e6c78);
                        Matrix4 turn = RotationZ(cameraTurn * 0.01f);
                        DAT_11af3e28.Copy(Project(DAT_11af3e28, turn) + DAT_118e6c78);
                    }
                    if (DAT_1065cb32 == 0)
                    {
                        if (DAT_1065cb40 != 0 && FUN_1016c300() == 0)
                            DAT_1101de20 = 1;
                    }
                    else if (FUN_1016c300() == 0)
                    {
                        DAT_1101de30 = 2;
                        FUN_1016bdf0(-DAT_10db6ee8[0].targetHeading);
                    }
                    if (DAT_1065cb41 != 0 && FUN_1016c300() == 0)
                    {
                        Vec4 toTarget;
                        float angle;
                        toTarget.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
                        if (DAT_1065cb33 == 0)
                        {
                            if (DAT_10db6ee8[DAT_102027f8].matrix.m[MATRIX_TRANSLATION_X] >= TARGET_GONE_X)
                            {
                                if (DAT_102027f8 > 0)
                                {
                                    toTarget.Copy(toTarget - DAT_10db6ee8[DAT_102027f8].matrix.GetTranslation());
                                    DAT_1101de30 = 2;
                                    angle = atan2(toTarget.x, toTarget.y);
                                    goto turnCameraToTarget;
                                }
                            }
                            else
                                DAT_102027f8 = -1;
                        }
                        else
                        {
                            if (DAT_102024ac < 1)
                            {
                                int nearest = -1;
                                float nearestDistance = TARGET_SEARCH_START;
                                for (i = LOCK_FIRST_MODEL; i < DOOR_FIRST_MODEL; i++)
                                {
                                    if (DAT_104bfe30 == 0 && (DAT_1021f3c0[i] == 'c' || DAT_1021f3c0[i] == 3)
                                        && DAT_10daaae8[i].stats[STAT_BUSY] == 0 && DAT_10daaae8[i].stats[STAT_HEALTH] > 0
                                        && DAT_10daaae8[i].m_0141 == 0)
                                    {
                                        float distance = DAT_1020e338[i]->GetTranslation().DistanceSquared(toTarget);
                                        if (distance < nearestDistance)
                                        {
                                            nearest = i;
                                            nearestDistance = distance;
                                        }
                                    }
                                }
                                if (nearest < 1)
                                {
                                    nearest = -1;
                                    for (i = LOCK_FIRST_MODEL; i < DOOR_FIRST_MODEL; i++)
                                    {
                                        if (DAT_104bfe30 == 0 && (DAT_1021f3c0[i] == 'c' || DAT_1021f3c0[i] == 3)
                                            && DAT_10daaae8[i].stats[STAT_HEALTH] > 0 && DAT_10daaae8[i].m_0141 == 0)
                                        {
                                            float distance = DAT_1020e338[i]->GetTranslation().DistanceSquared(toTarget);
                                            if (distance < nearestDistance)
                                            {
                                                nearest = i;
                                                nearestDistance = distance;
                                            }
                                        }
                                    }
                                    if (nearest > 0)
                                    {
                                        toTarget.Copy(toTarget - DAT_10db6ee8[nearest].matrix.GetTranslation());
                                        DAT_1101de30 = 2;
                                        angle = atan2(toTarget.x, toTarget.y);
                                        FUN_1016bdf0(-angle);
                                    }
                                    DAT_102027f8 = nearest;
                                    goto lockOnDone;
                                }
                                DAT_102027f8 = nearest;
                                toTarget.Copy(toTarget - DAT_10db6ee8[nearest].matrix.GetTranslation());
                                DAT_1101de30 = 2;
                                angle = atan2(toTarget.x, toTarget.y);
                            }
                            else
                            {
                                toTarget.Copy(toTarget - DAT_10db6ee8[DAT_102024ac].matrix.GetTranslation());
                                DAT_102027f8 = DAT_102024ac;
                                DAT_1101de30 = 2;
                                angle = atan2(toTarget.x, toTarget.y);
                            }
turnCameraToTarget:
                            FUN_1016bdf0(-angle);
                        }
                    }
lockOnDone:
                    // Middle and Q keys: item list shortcuts; the menu key opens and closes the item list.
                    if (DAT_104bfdf0 == 0)
                    {
                        if (DAT_104c0028 == 0)
                        {
                            if (DAT_1065cb3e == 1 && DAT_104c0068 != 1 && DAT_104bfe1c == 0)
                                mouseMiddle = 1;
                            if (DAT_1065cb30 == 1 && DAT_104c0068 != 1 && DAT_104bfe1c == 0)
                                mouseMiddle = 2;
                        }
                        if (DAT_1065cb39 != 0 && DAT_104c0068 != 1 && DAT_104c0028 == 0 && DAT_104c01e0 == 0)
                        {
                            if (DAT_104bfe1c == 0)
                            {
                                DAT_104bfd74 = 1;
                                modelIdx = DAT_10202464[0];
                                DAT_104bfe90 = 0;
                                FUN_1003ad20(SOUND_ITEM_LIST_OPEN, 0, 1.0f);
                                DAT_104bfe90 = 0;
                                DAT_104c0048 = 0;
                                DAT_104bfe1c = 1;
                            }
                            else
                            {
                                FUN_1003ad20(SOUND_ITEM_LIST_CLOSE, 0, 1.0f);
                                if (DAT_104bfe1c != 0)
                                {
                                    CloseItemList();
                                }
                                if (DAT_102024b0 > -1 && DAT_104bfdf0 == 0)
                                {
                                    DAT_104bfd74 = DAT_102024b0;
                                    DAT_1020249c = DAT_102024b0;
                                }
                                DAT_102024b0 = -1;
                            }
                        }
                    }
                    // The pad's stick: flick gestures (jump), turning around, steering and moving through the item list.
                    if (DAT_1032d69c != 0 && DAT_104bfdf0 == 0)
                    {
                        int padX = DAT_1065cb48;
                        int padY = DAT_1065cb4c;
                        Vec4 padDir;
                        float padLength;
                        padDir.Set((float)padX, (float)padY, 0, 0);
                        padDir.Normalized3();
                        padLength = (float)sqrt((double)(padY * padY + padX * padX));

                        // History of the stick: the last PAD_HISTORY_LENGTH magnitudes and directions, the newest last.
                        for (i = 0; i < PAD_HISTORY_LENGTH - 1; i++)
                        {
                            DAT_104507e8[i] = DAT_104507e8[i + 1];
                            DAT_10836dd8[i].Copy(DAT_10836dd8[i + 1]);
                        }
                        DAT_104507e8[PAD_HISTORY_LENGTH - 1] = (short)padLength;
                        DAT_10836dd8[PAD_HISTORY_LENGTH - 1].Copy(padDir);

                        // A flick (the stick goes out, back in and out again in the same direction) is a jump.
                        if (padLength > 0.0f)
                        {
                            int phase = 0;
                            float peak = padLength * PAD_FLICK_PEAK;
                            for (i = PAD_HISTORY_LENGTH - 1; i >= 0; i--)
                            {
                                if (peak <= (float)DAT_104507e8[i] && Vec3_Dot(&DAT_10836dd8[i], &padDir) < PAD_FLICK_ALIGN)
                                    goto padGestureDone;
                                if (phase == 0)
                                {
                                    if ((float)DAT_104507e8[i] < padLength / PAD_FLICK_LOW_DIVISOR)
                                        phase = 1;
                                }
                                else if (phase == 1)
                                {
                                    if (peak <= (float)DAT_104507e8[i])
                                        phase = 2;
                                }
                                else if (phase == 2)
                                {
                                    if ((float)DAT_104507e8[i] < padLength / PAD_FLICK_LOW_DIVISOR)
                                        phase = 3;
                                }
                            }
                            if (phase == 3 && DAT_104c010d == 0 && DAT_10c652cc != 0 && DAT_104bfe1c == 0 && DAT_104bfe30 == 0
                                && DAT_104bfdc8 == 0 && DAT_104c0300 == 0 && DAT_104bfe4c == 0 && DAT_104c00e0 < 4)
                                DAT_104c022e = 1;
                        }
padGestureDone:
                        if (DAT_10c652cc != 0 && DAT_104c010d != 0 && DAT_1065cb35 != 0 && DAT_104bfe1c == 0 && DAT_104bfe30 == 0
                            && DAT_104bfdc8 == 0 && DAT_104c0300 == 0 && DAT_104bfe4c == 0 && DAT_104c00e0 < 4)
                            DAT_104c022e = 1;

                        // A stick push behind the player turns the character around.
                        if (DAT_104c0228 >= TURN_STATE_BASE && DAT_104bfe1c == 0 && (padX != 0 || padY != 0)
                            && DAT_104c0228 == TURN_PENDING)
                        {
                            Vec4 facing;
                            Vec4 wish;
                            float behind;
                            facing.x = cos(-DAT_10db6ee8[DAT_104bf8b4].targetHeading - HALF_PI);
                            facing.y = sin(-DAT_10db6ee8[DAT_104bf8b4].targetHeading - HALF_PI);
                            facing.z = 0;
                            FUN_1016b340(&wish.x, &wish.y, (float)padX, (float)-padY);
                            wish.z = 0;
                            wish.Normalized3();
                            behind = Vec3_Dot(&facing, &wish);
                            if (behind < TURN_AROUND_DOT)
                            {
                                DAT_10db6ee8[0].PlayMotion(2, 0, 5, 1.0f);
                                DAT_104c0228 = 0;
                                for (i = PLAYER_TRAIL_LENGTH; i != 0; i--)
                                {
                                    DAT_10802ad8[DAT_104c022c].Copy(DAT_10db6ee8[0].matrix.GetTranslation());
                                    DAT_104c022c = DAT_104c022c + 1 & 0xf;
                                }
                            }
                            else
                                DAT_104c0228 = TURN_REFUSED;
                        }
                        if (DAT_104c0068 == 1 || DAT_104c0228 >= TURN_STATE_BASE)
                        {
                            if (DAT_104bfe1c != 0)
                                goto padItemList;
                        }
                        else if (DAT_104bfe1c == 0)
                        {
                            if ((unsigned char)DAT_10db6ee8[0].m_2288 < 2)
                            {
                                if (padX < 0)
                                {
                                    steerX = (float)padX / PAD_STEER_DIVISOR;
                                    DAT_1020d9e0.M_100014a0();
                                }
                                if (padX > 0)
                                {
                                    steerX = (float)padX / PAD_STEER_DIVISOR;
                                    DAT_1020d9e0.M_100014a0();
                                }
                                if (padY < 0)
                                {
                                    steerY = (float)-padY / PAD_STEER_DIVISOR;
                                    DAT_1020d9e0.M_100014a0();
                                }
                                if (padY > 0)
                                {
                                    steerY = (float)-padY / PAD_STEER_DIVISOR;
                                    DAT_1020d9e0.M_100014a0();
                                }
                                if (DAT_104c009d == 0)
                                {
                                    if (abs((int)steerX) > 8 || abs((int)steerY) > 8)
                                        moveInput = 1;
                                }
                                else if (abs((int)steerX) > 5 || abs((int)steerY) > 5)
                                    moveInput = 1;
                            }
                        }
                        else
                        {
padItemList:
                            // The stick moves through the item list.
                            if (DAT_104c0135 == 0 && DAT_104c0148 == 0 && DAT_104c0028 == 0)
                            {
                                if (padY == 0 && padX == 0)
                                {
                                    if (DAT_104bfe98 < 0x19)
                                        DAT_104bfe98 += 5;
                                    DAT_104bfe9c = 0;
                                }
                                else if (DAT_104bfe9c < 9)
                                    DAT_104bfe9c++;
                                if (DAT_104bfe9c + DAT_104bfe98 > 5)
                                {
                                    if (fabs((double)padY) < PAD_PAGE_LIMIT && padX > 0x32 && DAT_104c02bc == 0)
                                    {
                                        DAT_104c02bc = 1;
                                        DAT_104c0048 = 0;
                                    }
                                    else if (fabs((double)padY) < PAD_PAGE_LIMIT && padX < -0x32 && DAT_104c02bc == 1)
                                    {
                                        DAT_104c02bc = 0;
                                        DAT_104c0048 = 0;
                                    }
                                    if (padY < -0x32)
                                    {
                                        DAT_104bfe98 = 0;
                                        if (DAT_104bfe94 != 0 && DAT_104bfe90 + DAT_104c0048 != 0)
                                            FUN_1003ad20(SOUND_ITEM_LIST_MOVE, 0, 1.0f);
                                        if (DAT_104bfe90 < 1)
                                        {
                                            if (DAT_104c0048 > 0)
                                                DAT_104c0048--;
                                        }
                                        else
                                            DAT_104bfe90--;
                                        DAT_1020d9e0.M_100014a0();
                                    }
                                    if (padY > 0x32)
                                    {
                                        DAT_104bfe98 = 0;
                                        if (DAT_104bfe94 != 0 && DAT_104bfe90 + DAT_104c0048 < DAT_104c0044 - 1)
                                            FUN_1003ad20(SOUND_ITEM_LIST_MOVE, 0, 1.0f);
                                        if (DAT_104bfe90 == ITEM_MENU_ROWS - 1 && DAT_104c0044 - DAT_104c0048 > ITEM_MENU_ROWS)
                                            DAT_104c0048++;
                                        else if (DAT_104c0048 + DAT_104bfe90 < DAT_104c0044 - 1)
                                            DAT_104bfe90++;
                                        DAT_1020d9e0.M_100014a0();
                                    }
                                }
                            }
                        }

                        // Without a stick push the character stands (or keeps the idle pose); with one it starts to walk.
                        if (DAT_104bfdc8 == 0 && DAT_104c0068 != 1 && DAT_104bfe30 == 0)
                        {
                            if (steerX == 0.0f && steerY == 0.0f && DAT_104bf8c4 == 0)
                            {
                                int motion;
                                if (DAT_104c0300 == 0)
                                {
                                    if (DAT_104bfe4c == 0)
                                    {
                                        if (DAT_104c00e0 == 0 && DAT_104c009e == 0)
                                        {
                                            motion = 2;
                                            goto playPadIdleMotion;
                                        }
                                    }
                                    else if (DAT_10468128 == 0 && DAT_10db6ee8[0].m_229c == 0)
                                    {
                                        motion = 0x1c;
playPadIdleMotion:
                                        DAT_10db6ee8[0].PlayMotion(motion, 0, 5, 1.0f);
                                    }
                                }
                                else if (DAT_10468128 == 0 && DAT_10db6ee8[0].m_229c == 0)
                                {
                                    motion = 0x83;
                                    goto playPadIdleMotion;
                                }
                            }
                            else if (DAT_104c0300 == 0 && DAT_104bfe4c == 0 && DAT_104c01e0 == 0)
                            {
                                float steerLength;
                                DAT_1020234c = 1.0f;
                                if (moveInput == 0)
                                {
                                    if (DAT_104c009e == 0 && DAT_104c00e0 == 0)
                                        DAT_10db6ee8[0].PlayMotion(0, 0, 5, 1.0f);
                                }
                                else
                                {
                                    if (DAT_104c009e == 0 && DAT_104c00e0 == 0)
                                        DAT_10db6ee8[0].PlayMotion(1, 0, 5, 1.0f);
                                    moveInput = 1;
                                }
                                steerLength = (float)sqrt((double)(steerX * steerX + steerY * steerY));
                                steerX = steerX / steerLength * STEER_FULL;
                                steerY = steerY / steerLength * STEER_FULL;
                            }
                        }
                    }
                }
            }
            // While the item list is open: the pad's buttons close it or pick the highlighted entry.
            controlsEnabled = 1;
            if (DAT_104c0099 > 0xe)
            {
                if (DAT_104c0148 == 0)
                {
                    if (DAT_104c0135 == 0)
                    {
                        if (DAT_104bfe1c != 0)
                        {
                            if (DAT_1032d69c != 0)
                            {
                                if (DAT_1065cb30 == 0)
                                {
                                    if (DAT_1065cb31 != 0)
                                    {
                                        DAT_1065cb31 = 0;
                                        FUN_1003ad20(SOUND_ITEM_LIST_CLOSE, 0, 1.0f);
                                        if (DAT_104bfe1c != 0)
                                            CloseItemList();
                                    }
                                }
                                else
                                {
                                    int entry = DAT_104485e8[DAT_104bfe90];
                                    if (entry != 0)
                                    {
                                        if (entry == 0xa0)
                                        {
                                            DAT_1030c564 = 1;
                                            controlsEnabled = 0;
                                        }
                                        else if (entry == 0xa1)
                                        {
                                            DAT_1030c540 = 1;
                                            controlsEnabled = 0;
                                        }
                                        else if (entry == 0xa2)
                                        {
                                            DAT_1030c558 = 1;
                                            controlsEnabled = 0;
                                        }
                                        else if (entry == 0xa3)
                                        {
                                            DAT_1030c55c = 1;
                                            controlsEnabled = 0;
                                        }
                                        else if (entry == 0xa4)
                                        {
                                            DAT_1030c510 = 1;
                                            controlsEnabled = 0;
                                        }
                                        else if (entry == 0xa5)
                                        {
                                            DAT_1030c508 = 1;
                                            controlsEnabled = 0;
                                        }
                                        else if (entry == 0xa9)
                                        {
                                            CloseBookModel();
                                            FUN_100106a0();
                                            memset(DAT_1030c500, 0, sizeof(DAT_1030c500));
                                            DAT_1020247c[DAT_104bfe88] = 0x531;
                                            DAT_104bfe88 = DAT_104bfe88 + 1 & 7;
                                            DAT_104bfe1c = 0;
                                            controlsEnabled = 0;
                                        }
                                        else
                                        {
                                            inputEvents = 1;
                                            controlsEnabled = 0;
                                            DAT_10202464[0] = entry;
                                            modelIdx = entry;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            // The walking state: counters, the input history, the riding speed and the start / stop of walking.
            flag0155 = DAT_104c0155;
            greetings = DAT_104c00e0;
            DAT_104c009d = (char)moveInput;
            if (0 < DAT_1020d430)
                DAT_1020d430 = DAT_1020d430 - 1;
            if (0 < DAT_1032d6a4)
                DAT_1032d6a4 = DAT_1032d6a4 - 1;
            if (DAT_104c010c != 0)
                DAT_104c010c = DAT_104c010c - 1;
            stepDelay = DAT_104c010c;
            if (DAT_10db9274 == 0)
            {
                DAT_1042dda8[INPUT_HISTORY_LAST] = (unsigned int)DAT_104c0155 + (DAT_10468128 * 100 + DAT_104c00e0) * 10;
                for (i = 0; i < INPUT_HISTORY_LAST; i++)
                    DAT_1042dda8[i] = DAT_1042dda8[i + 1];
            }
            else
            {
                flag0155 = 0;
                DAT_104c0155 = 0;
            }
            riding = false;
            ridingFlag = 0;
            if (DAT_104bfe4c == 0)
                DAT_10db6ee8[0].speedDivisor = 0;
            else
            {
                riding = true;
                ridingFlag = 1;
                if (DAT_10db9248 <= DAT_104bfe24 - RIDING_HEIGHT_MARGIN)
                    DAT_10db6ee8[0].speedDivisor = 3;
                else
                    DAT_10db6ee8[0].speedDivisor = 1;
            }
            if (DAT_104c01e0 == 0)
            {
                if (DAT_104bfdc8 != 0 || DAT_104c0300 != 0 || DAT_1032d69c == 0
                    || (DAT_1065cb31 == 0 && mouseRight != 2 && stepDelay == 0) || mouseMiddle == 2 || DAT_1065cb30 != 0)
                    goto walkTailCheck;
                if (DAT_104c01e8 != 0 || DAT_104c0068 == 1 || DAT_104bfdf0 != 0)
                {
walkCancel:
                    if (3 < greetings)
                        DAT_104c010c = 8;
                    goto walkTailCheck;
                }
                if (DAT_104c00e8 == 0x2b)
                {
                    if (greetings == 0 || DAT_1065cb3e != 0 || mouseMiddle != 0)
                        goto walkNeedsGreeting;
                }
                else if (DAT_104c00e8 != 0x24 || 7 < greetings)
                {
walkNeedsGreeting:
                    if (3 < greetings && flag0155 == 0)
                        goto walkCancel;
                }
                if (DAT_104bfe1c != 0 || 7 < DAT_104c009a)
                    goto walkCancel;
                if (controlsEnabled == 0)
                    goto walkTailCheck;
                DAT_104c010c = 0;
                DAT_104c0154 = 1;
                if (9999 < DAT_104c0228)
                {
                    if (DAT_104c0228 == 0x271a)
                        DAT_104c0228 = 0x271b;
                    goto walkTailCheck;
                }
                if ((riding && DAT_10db6ee8[0].m_229c != 0) || DAT_104bfe30 != 0 || DAT_104c02e4 != 0 || DAT_104bfe3c != 0)
                    goto walkTailCheck;
                if (!riding && flag0155 == 0 && DAT_10db6ee8[DAT_104bf8b4].m_238c == 0)
                {
                    // Only start walking when there is floor ahead of the player.
                    Vec4 groundProbe;
                    groundProbe.Set(0.0f, 0.0f, -PROBE_FAR, 0.0f);
                    int blockedAhead = DAT_10db6ee8[DAT_104bf8b4].IsClearOfMap(&groundProbe) == 0;
                    if (!blockedAhead)
                    {
                        groundProbe.Set(0.0f, 0.0f, -PROBE_NEAR, 0.0f);
                        blockedAhead = DAT_10db6ee8[DAT_104bf8b4].IsClear(&groundProbe) == 0;
                        if (!blockedAhead && DAT_104c0155 == 0)
                        {
                            greetings = DAT_104c00e0;
                            goto walkCancel;
                        }
                    }
                }
                DAT_104c009c = 0;
                DAT_1032d6a4 = 9;
                DAT_1020d430 = 0xf;
                DAT_1020d4b8 = 0;
                DAT_104bfdb0 = -0x1e;
                FUN_1003ad20(SOUND_WALK_START, &DAT_10db6ee8[0].matrix.GetTranslation(), 1.0f);
                if (DAT_104bfdc8 != 0)
                {
                    DAT_10db6ee8[DAT_104bfdd8].m_0075 = 0;
                    DAT_10db6ee8[0].m_0075 = 0;
                    DAT_104bfdc8 = 0;
                    DAT_104bfdcc = 0;
                    DAT_104c0090 = 1;
                }
                DAT_104bfdd0 = 0;
                DAT_104c009e = 0x14;
                if (DAT_104c00e8 == 0x2b && DAT_104c00e0 != 0)
                    FUN_100114a0();
                if (DAT_10db6ee8[DAT_104bf8b4].m_238c != 0 || riding)
                {
                    if (DAT_104c0155 != 0)
                        goto walkResetBlend;
                }
                else
                {
                    if (DAT_104c0155 == 0)
                    {
                        DAT_10db6ee8[DAT_104bf8b4].m_237c = 0.4f;
                        DAT_10db6ee8[DAT_104bf8b4].m_238c = 1;
                        goto walkBlendDone;
                    }
walkResetBlend:
                    DAT_10db6ee8[DAT_104bf8b4].m_237c = 0.4f;
walkBlendDone:;
                }
                if (DAT_10db6ee8[DAT_104bf8b4].m_238c != 0)
                    DAT_104687b0 = 1;
                DAT_1021a360 = 0;
                DAT_104c009c = 0;
                if (moveInput == 0)
                {
                    if (riding)
                        DAT_10db6ee8[0].PlayMotion(0x1e, 0, 5, 1.0f);
                    else
                    {
                        DAT_10db6ee8[0].PlayMotion(9, 0, 2, 1.0f);
                    }
                    DAT_10468128 = 8;
                }
                else
                {
                    DAT_10db6ee8[0].PlayMotion(0x1e, 0, riding ? 5 : 2, 1.0f);
                    DAT_10468128 = 8;
                }
walkTail:
                if (DAT_10db6ee8[DAT_104bf8b4].m_238c == 0 && !riding && DAT_104c0155 == 0)
                {
                    Vec4 floorProbe;
                    floorProbe.Set(0.0f, 0.0f, -PROBE_FAR, 0.0f);
                    greetings = DAT_10db6ee8[DAT_104bf8b4].IsClearOfMap(&floorProbe) == 0;
                    if (greetings == 0)
                    {
                        floorProbe.Set(0.0f, 0.0f, -PROBE_NEAR, 0.0f);
                        greetings = DAT_10db6ee8[DAT_104bf8b4].IsClear(&floorProbe) == 0;
                    }
                    DAT_10db6ee8[DAT_104bf8b4].m_238c = greetings;
                }
                DAT_10468128 = DAT_10468128 + 1;
            }
            else
            {
                if (((0 < DAT_104c01e0 && DAT_104c01e4 < DAT_104c01dc->frameMatrixCount - SEAT_PUSH_FRAMES)
                     || (DAT_104c01e0 < 0 && SEAT_PUSH_FRAMES < DAT_104c01e4))
                    && (DAT_1065cb31 != 0 || mouseRight == 2) && DAT_104c0210 == 0)
                    DAT_104c0214 = SEAT_PUSH_KICK;
walkTailCheck:
                if (DAT_10468128 != 0)
                    goto walkTail;
            }
            if (DAT_104c009e != 0)
                DAT_104c009e = DAT_104c009e - 1;
            if (DAT_104c0300 != 0 || DAT_104bfe4c != 0 || DAT_104bfe50 != 0)
                DAT_104c010c = 0;
            // Walking on level ground: the camera positions ahead of the player that the sparkle marks.
            if (DAT_104bfe3c == 0)
            {
                if (DAT_104bfdc8 == 0 && DAT_104c0228 < 10000
                    && ((DAT_10db6ee8[DAT_104bf8b4].m_238c != 0 || (riding && DAT_104bfe24 - RIDING_HEIGHT_MARGIN < DAT_10db9248))
                        && DAT_1049f0f0 != 0 && DAT_10c635f8 == 1))
                {
                    Vec4 playerAt = DAT_10db6ee8[0].matrix.GetTranslation();
                    Vec4 reach = playerAt;
                    float facingCos = (float)cos((double)(-DAT_10db6ee8[DAT_104bf8b4].heading - HALF_PI));
                    reach.x = facingCos * LOCK_REACH + reach.x;
                    float facingSin = (float)sin((double)(-DAT_10db6ee8[DAT_104bf8b4].heading - HALF_PI));
                    reach.y = facingSin * LOCK_REACH + reach.y;
                    int nearTag = 0;
                    int blockedIdx = -1;
                    for (i = 0; i < DAT_1049f0f0; i++)
                    {
                        if (FocusPoint(i)->Distance(playerAt) < LOCK_REACH)
                        {
                            unsigned char kind = DAT_1021bbc0[i];
                            if (kind < 3)
                                nearTag = (kind != 0) ? i + LOCK_TAG_INDEX_BASE : 1;
                            else
                            {
                                nearTag = kind + LOCK_TAG_KIND_BASE;
                                blockedIdx = i;
                            }
                        }
                    }
                    int bestIdx = -1;
                    float bestDistance = LOCK_RANGE;
                    for (i = 0; i < DAT_1049f0f0; i++)
                    {
                        float distance = FocusPoint(i)->Distance(reach);
                        if (distance < bestDistance && distance < FocusPoint(i)->Distance(playerAt))
                        {
                            unsigned char kind = DAT_1021bbc0[i];
                            if (fabs((double)(DAT_1032d5b8[i]->z - playerAt.z)) < LOCK_HEIGHT
                                && (kind == 0 || nearTag == 1
                                    || (kind > 2 && blockedIdx != i && nearTag - LOCK_TAG_KIND_BASE == kind)
                                    || (kind == 2 && nearTag != 0 && nearTag - LOCK_TAG_INDEX_BASE != i)))
                            {
                                bestDistance = distance;
                                bestIdx = i;
                            }
                        }
                    }
                    DAT_104c01ac = DAT_104c01ac + 1;
                    if (bestIdx != DAT_104c01a8)
                    {
                        DAT_104c01ac = 0;
                        DAT_104c01a8 = bestIdx;
                    }
                    if (bestIdx >= 0 && (DAT_104c01ac & 7) == 0)
                        DAT_10daaa80.Start(LOCK_SPARKLE_EFFECT, *FocusPoint(bestIdx), -1, 0, 0, 0, 0, 0, 0, 0);
                }
            }
            // After ten idle steps: pull the player towards the camera position it faces and settle.
            if (9 < DAT_10468128)
            {
                Vec4 *pullTarget;
                int blockedAhead = 0;
                if (ridingFlag == 0)
                {
                    if (DAT_104c0155 == 0)
                    {
                        if (DAT_10db6ee8[DAT_104bf8b4].m_238c == 0)
                        {
                            Vec4 settleProbe;
                            settleProbe.Set(0.0f, 0.0f, -PROBE_FAR, 0.0f);
                            blockedAhead = DAT_10db6ee8[DAT_104bf8b4].IsClearOfMap(&settleProbe) == 0;
                            if (blockedAhead == 0)
                            {
                                settleProbe.Set(0.0f, 0.0f, -PROBE_NEAR, 0.0f);
                                blockedAhead = DAT_10db6ee8[DAT_104bf8b4].IsClear(&settleProbe) == 0;
                                if (blockedAhead == 0)
                                {
                                    if (DAT_104c0155 == 0)
                                        goto settleTargetDone;
                                    goto noPullTarget;
                                }
                            }
                        }
                        else
                            blockedAhead = 1;
                        goto pickPullTarget;
                    }
                    goto noPullTarget;
                }
                else
                {
                    if (DAT_10db9248 <= DAT_104bfe24 - RIDING_HEIGHT_MARGIN)
                        goto noPullTarget;
pickPullTarget:
                    if (DAT_1049f0f0 == 0 || DAT_10c635f8 != 1)
                        goto noPullTarget;
                    {
                        Vec4 playerAt = DAT_10db6ee8[0].matrix.GetTranslation();
                        Vec4 reach = playerAt;
                        float facingCos = (float)cos((double)(-DAT_10db6ee8[DAT_104bf8b4].heading - HALF_PI));
                        reach.x = facingCos * LOCK_REACH + reach.x;
                        float facingSin = (float)sin((double)(-DAT_10db6ee8[DAT_104bf8b4].heading - HALF_PI));
                        reach.y = facingSin * LOCK_REACH + reach.y;
                        int nearTag = 0;
                        int blockedIdx = -1;
                        for (i = 0; i < DAT_1049f0f0; i++)
                        {
                            if (FocusPoint(i)->Distance(playerAt) < LOCK_REACH)
                            {
                                unsigned char kind = DAT_1021bbc0[i];
                                if (kind < 3)
                                    nearTag = (kind != 0) ? i + LOCK_TAG_INDEX_BASE : 1;
                                else
                                {
                                    nearTag = kind + LOCK_TAG_KIND_BASE;
                                    blockedIdx = i;
                                }
                            }
                        }
                        int bestIdx = -1;
                        float bestDistance = LOCK_RANGE;
                        if (DAT_1049f0f0 < 1)
                            goto noPullTarget;
                        for (i = 0; i < DAT_1049f0f0; i++)
                        {
                            float distance = FocusPoint(i)->Distance(reach);
                            if (distance < bestDistance && distance < FocusPoint(i)->Distance(playerAt))
                            {
                                unsigned char kind = DAT_1021bbc0[i];
                                if (fabs((double)(DAT_1032d5b8[i]->z - playerAt.z)) < LOCK_HEIGHT
                                    && (kind == 0 || nearTag == 1
                                        || (kind > 2 && blockedIdx != i && nearTag - LOCK_TAG_KIND_BASE == kind)
                                        || (kind == 2 && nearTag != 0 && nearTag - LOCK_TAG_INDEX_BASE != i)))
                                {
                                    bestDistance = distance;
                                    bestIdx = i;
                                }
                            }
                        }
                        if (bestIdx < 0)
                            goto noPullTarget;
                        pullTarget = FocusPoint(bestIdx);
                    }
                    goto pullTargetChosen;
                }
noPullTarget:
                pullTarget = 0;
pullTargetChosen:
                DAT_10db6ee8[DAT_104bf8b4].SetPullTarget(pullTarget);
                DAT_10468128 = 0;
                if (ridingFlag != 0 || blockedAhead != 0 || DAT_104c0155 != 0)
                {
                    DAT_10db6ee8[DAT_104bf8b4].m_229c = 0;
                    DAT_10db6ee8[DAT_104bf8b4].m_237c = 1.0f;
                    DAT_10db6ee8[DAT_104bf8b4].verticalSpeed = 0.0f;
                    DAT_10db6ee8[DAT_104bf8b4].AddMotionSpeed(SETTLE_MOTION_SPEED);
                }
                DAT_10db6ee8[DAT_104bf8b4].m_008c = 0;
                if (ridingFlag != 0)
                    DAT_10db6ee8[DAT_104bf8b4].mode = 1;
                DAT_104c0155 = 0;
                DAT_10db6ee8[0].matrix.GetTranslation();
            }
settleTargetDone:
            {
                // Lock on the character slot nearest to the point LOCK_AHEAD units ahead of the player (the slots
                // LOCK_FIRST_MODEL..DOOR_FIRST_MODEL - 1 that can be lunged at).
                int lockTarget = -1;
                float nearestLock = LOCK_NEAREST_START;
                Vec4 ahead = DAT_10db6ee8[0].matrix.GetTranslation();
                float aheadCos = (float)cos((double)(-DAT_10db6ee8[DAT_104bf8b4].targetHeading - HALF_PI));
                ahead.x = aheadCos * LOCK_AHEAD + ahead.x;
                float aheadSin = (float)sin((double)(-DAT_10db6ee8[DAT_104bf8b4].targetHeading - HALF_PI));
                ahead.y = aheadSin * LOCK_AHEAD + ahead.y;
                if (DAT_104c02b8 > 0 && DAT_102239d0[DAT_104c02b8] != 0
                    && abs((int)(DAT_10db6ee8[DAT_104c02b8].matrix.translation.z - ahead.z)) < LOCK_KEEP_HEIGHT)
                    DAT_102024ac = DAT_104c02b8;
                else
                {
                    for (i = LOCK_FIRST_MODEL; i < DOOR_FIRST_MODEL; i++)
                    {
                        float distance;
                        if (i == DAT_104c02b8)
                        {
                            if (DAT_104bfe30 == 0 && (DAT_1021f3c0[i] == 'c' || DAT_1021f3c0[i] == 3)
                                && DAT_10daaae8[i].stats[STAT_BUSY] == 0
                                && (DAT_102239d0[i] != 0 || DAT_10daaae8[i].stats[STAT_HEALTH] > 0)
                                && DAT_10daaae8[i].m_0141 == 0)
                            {
                                distance = (DAT_1020e338[i]->GetTranslation() - ahead).Length2D();
                                if (distance < nearestLock)
                                {
                                    nearestLock = distance;
                                    goto lockCandidateFound;
                                }
                            }
                        }
                        else if (DAT_104bfe30 == 0 && (DAT_1021f3c0[i] == 'c' || DAT_1021f3c0[i] == 3)
                                 && DAT_10daaae8[i].stats[STAT_BUSY] == 0
                                 && (DAT_102239d0[i] != 0 || DAT_10daaae8[i].stats[STAT_HEALTH] > 0)
                                 && DAT_10daaae8[i].m_0141 == 0)
                        {
                            distance = DAT_1020e338[i]->GetTranslation().Distance(ahead);
                            if (distance < nearestLock)
                            {
                                nearestLock = distance;
lockCandidateFound:
                                lockTarget = i;
                            }
                        }
                    }
                    DAT_104bfeba = 0;
                    if (DAT_104bfdf0 == 0)
                    {
                        if (lockTarget >= 0)
                        {
                            Vec4 lockAt = DAT_1020e338[lockTarget]->GetTranslation();
                            if (DAT_104bfe30 == 0 && DAT_104c00ac == 0
                                && (DAT_1021f3c0[lockTarget] == 'c' || DAT_1021f3c0[lockTarget] == 3)
                                && (DAT_102239d0[lockTarget] != 0 || DAT_10daaae8[lockTarget].stats[STAT_HEALTH] > 0))
                            {
                                float lockDistance = (ahead - lockAt).Length2D();
                                float lockLimit = (float)(DAT_102192c0[lockTarget] + LOCK_BASE_REACH);
                                if (lockDistance <= lockLimit + DAT_10db6ee8[DAT_104bf8b4].colliderReach
                                    && fabs((double)(lockAt.z - ahead.z)) < LOCK_REACH_HEIGHT)
                                {
                                    DAT_104bfe20 = 1;
                                    DAT_102024ac = lockTarget;
                                    goto lockTargetStored;
                                }
                            }
                        }
                        DAT_104bfe20 = 0;
                        DAT_102024ac = -1;
                    }
                }
lockTargetStored:
                DAT_104c02b8 = DAT_102024ac;
                // The held item: refused while it cannot be used here, otherwise taken in hand.
                if (DAT_104bfdf0 == 0 && inputEvents != 0)
                {
                    DAT_10db6ee8[0].matrix.GetTranslation();
                    if (DAT_104c0228 >= ITEM_USE_TIME_LIMIT)
                        FUN_1000edf0(TEXT_CANNOT_USE_NOW, 0, ALTERNATE_STYLE_FRAMES);
                    else if (DAT_104bfe30 != 0)
                        FUN_1000edf0(TEXT_CANNOT_USE_LADDER, 0, ALTERNATE_STYLE_FRAMES);
                    else if (DAT_104bfe4c != 0)
                        FUN_1000edf0(TEXT_CANNOT_USE_UNDERWATER, 0, ALTERNATE_STYLE_FRAMES);
                    else if (DAT_104c0300 != 0)
                        FUN_1000edf0(TEXT_HEAD_BUMP, 0, ALTERNATE_STYLE_FRAMES);
                    else
                    {
                        const ItemInfo &info = DAT_1088e6b0[modelIdx];
                        const ItemText &text = DAT_108f667c[modelIdx];
                        if (info.type == 1 && DAT_10c655f0 == COSTUME_TOO_SMALL)
                            FUN_1000edf0(TEXT_TOO_SMALL, 0, ALTERNATE_STYLE_FRAMES);
                        else if (info.type == 1 && (DAT_10c655f0 == COSTUME_UNSUITED_A || DAT_10c655f0 == COSTUME_UNSUITED_B
                                                    || DAT_10c655f0 == COSTUME_UNSUITED_C))
                            FUN_1000edf0(TEXT_UNSUITED, 0, ALTERNATE_STYLE_FRAMES);
                        else if (modelIdx > 0)
                        {
                            if (info.healsHealth != 0 && DAT_10daaae8[0].stats[STAT_HEALTH] == DAT_10daaae8[0].stats[STAT_MAX_HEALTH])
                                FUN_1000edf0(TEXT_HEALTH_FULL, 0, ALTERNATE_STYLE_FRAMES);
                            else if (info.drillFullFlag != 0 && DAT_10874848.TestScriptFlag(info.drillFullFlag) != 0)
                                FUN_1000edf0(TEXT_DRILL_FULL, 0, ALTERNATE_STYLE_FRAMES);
                            else
                            {
                                DAT_102024ac = -1;
                                if (info.type != 2 && info.type != 3 && info.type != 4)
                                {
                                    DAT_10db6ee8[0].m_0294 = 1;
                                    DAT_10db6ee8[0].motionFrame = 0;
                                    DAT_10db6ee8[0].LoadModel(FUN_1013f8a0(text.modelPath, 0));
                                    DAT_104bfdf0 = text.heldState;
                                    DAT_104c0319 = 0;
                                }
                                if (text.keepOnUse == 0 && info.consumable != 0)
                                {
                                    int amount = 1;
                                    int index = modelIdx;
                                    FUN_100120a0(&index, &amount);
                                    if (DAT_103150c0[modelIdx] < 1)
                                        DAT_10202464[0] = 0;
                                }
                                if (DAT_10c5e1c4[text.useScript] != 0)
                                {
                                    DAT_1020247c[DAT_104bfe88] = text.useScript;
                                    DAT_104bfe88 = DAT_104bfe88 + 1 & 7;
                                    FUN_1003ad20(SOUND_ITEM_USED, 0, VOLUME_FULL);
                                    return 0;
                                }
                            }
                        }
                    }
                }
                // The player's matrix raised for the effects, and the light colour of the worn helmet kind.
                DAT_10836c98 = DAT_10db6ee8[0].matrix;
                DAT_10836c98.m[MATRIX_TRANSLATION_Z] = DAT_10db9248 + HELD_LIGHT_RAISE;
                DAT_1030c990 = steerY;
                DAT_10210b2c = steerX;
                DAT_10db6ee8[0].m_02a1 = 0;
                {
                    float lightR, lightG, lightB;
                    int helmetKind = DAT_1088e6b0[DAT_10daabb8].helmetKind;
                    if (helmetKind == HELMET_KIND_RED)
                    {
                        lightR = 1.0f;
                        lightG = 0.3f;
                        lightB = 0.1f;
                    }
                    else if (helmetKind == HELMET_KIND_BLUE)
                    {
                        lightR = 0.4f;
                        lightG = 0.3f;
                        lightB = 1.0f;
                    }
                    else if (helmetKind == HELMET_KIND_WHITE)
                    {
                        lightR = 1.0f;
                        lightG = 1.0f;
                        lightB = 1.0f;
                    }
                    else
                    {
                        lightR = 0.9f;
                        lightG = 0.8f;
                        lightB = 0.5f;
                    }
                    DAT_10daaa80.colour.Set(lightR, lightG, lightB, 1.0f);
                }
                // The talk block: while an attack record plays, its effects and sounds start at their frames.
                if (DAT_104bfdf0 == 0 && DAT_104c00e0 != 0)
                {
                    DAT_10db6ee8[0].m_02a1 = 1;
                    int strongSwing = DAT_108a6a58[DAT_104c00e8].m_0040;
                    float drillScale = 1.0f;
                    if (DAT_10c64ad4 == 1)
                        drillScale = DRILL_SCALE_1;
                    if (DAT_10c64ad4 == 2)
                        drillScale = DRILL_SCALE_2;
                    if (DAT_10c64ad4 == 3)
                        drillScale = DRILL_SCALE_3;
                    // The first frames of a swing: a flash effect of the helmet in some areas.
                    if (DAT_104c00f4 < SWING_EARLY_FRAMES)
                    {
                        Vec4 flashAt;
                        flashAt.Fill(0.0f);
                        if ((DAT_104c00e8 == 0x28 || DAT_104c00e8 == 0x14 || DAT_104c00e8 == 0x10 || DAT_104c00e8 == 0x19
                             || DAT_104c00e8 == 0x16 || DAT_104c00e8 == 0x17)
                            && DAT_104c00f4 % SWING_FLASH_INTERVAL == 0)
                        {
                            EffectOptions options;
                            options.flags = 1;
                            if (DAT_1088e6b0[DAT_10daabb8].helmetKind == HELMET_KIND_RED)
                                DAT_10daaa80.Start(0x1f4, flashAt, -1, 1, &DAT_10836c98, 0, 0, &DAT_10db6ee8[0].matrix, &options,
                                                   0);
                            if (DAT_1088e6b0[DAT_10daabb8].helmetKind == HELMET_KIND_BLUE)
                                DAT_10daaa80.Start(0x7da, flashAt, -1, 1, &DAT_10836c98, 0, 0, &DAT_10db6ee8[0].matrix, &options,
                                                   0);
                            if (DAT_1088e6b0[DAT_10daabb8].helmetKind == HELMET_KIND_WHITE)
                                DAT_10daaa80.Start(0x14b4, flashAt, -1, 1, &DAT_10836c98, 0, 0, &DAT_10db6ee8[0].matrix, &options,
                                                   0);
                        }
                    }
                    // The attack's own effect, started at its frame.
                    if (DAT_108a6a58[DAT_104c00e8].effect > 0)
                    {
                        int effectFrame = DAT_108a6a58[DAT_104c00e8].effectFrame;
                        if (effectFrame >= 0 || -DAT_104c00f4 == effectFrame || -effectFrame < DAT_104c00f4)
                        {
                            if ((int)((float)effectFrame * DAT_104c00f0) != DAT_104c00f4)
                                goto secondEffect;
                        }
                        EffectOptions options;
                        if (DAT_104c00e8 == ATTACK_AREA_SWORD_0 || DAT_104c00e8 == ATTACK_AREA_SWORD_1
                            || DAT_104c00e8 == ATTACK_AREA_SWORD_2 || DAT_104c00e8 == ATTACK_AREA_SWORD_3)
                        {
                            // Swords: the effect strikes every enemy within reach, then more around the player.
                            int weaponKind = DAT_108f667c[DAT_10daabb8].m_0074;
                            Vec4 strikeAt;
                            Vec4 playerAt = DAT_10db6ee8[0].matrix.GetTranslation();
                            int hits = 0;
                            for (int slot = 1; slot <= 0x7b; slot++)
                            {
                                if (((DAT_1021f3c0[slot] == 'c' || DAT_1021f3c0[slot] == 3)
                                     && DAT_10daaae8[slot].stats[STAT_HEALTH] > 0 && DAT_10daaae8[slot].m_0141 == 0)
                                    || (DAT_1021f3c0[slot] == 'M' && weaponKind == ITEM_KIND_SWORD_LONG))
                                {
                                    if (DAT_10db6ee8[slot].matrix.GetTranslation().Distance(playerAt) < SWORD_HIT_RANGE)
                                    {
                                        strikeAt.Copy(DAT_10db6ee8[slot].matrix.GetTranslation());
                                        strikeAt.z = strikeAt.z + SWORD_EFFECT_RAISE;
                                        EffectOptions hit;
                                        hit.m_0004 = (int)((float)-DAT_10daaae8[0].stats[STAT_EFFECT_PARAM] * DAT_10467508);
                                        hit.m_0008 = (float)(DAT_108a6a58[DAT_104c00e8].strength * (DAT_102024c8 + 10) * 4)
                                                     / EFFECT_STRENGTH_DIVISOR_LARGE;
                                        ApplyLoadoutScale(hit.m_0008);
                                        if (DAT_10c6566c != 0)
                                        {
                                            hit.m_0004 = 1;
                                            hit.m_0008 = 1.0f;
                                        }
                                        hit.owner = SWORD_EFFECT_OWNER;
                                        hit.m_0000 = 1;
                                        hit.flags = 1;
                                        hit.m_000e = (char)weaponKind;
                                        hit.settings[EFFECT_SETTING_AREA] = 2;
                                        DAT_10daaa80.Start(DAT_108a6a58[DAT_104c00e8].effect, strikeAt, -1, 0,
                                                           &DAT_10db6ee8[slot].matrix, 0, 0, 0, &hit, 0);
                                        if ((weaponKind == ITEM_KIND_SWORD_LONG || weaponKind == ITEM_KIND_SWORD)
                                            && DAT_1021f3c0[slot] == 3 && DAT_10daaae8[slot].m_0141 == 0)
                                        {
                                            char stun = 0;
                                            if (weaponKind == ITEM_KIND_SWORD)
                                                stun = 0x28;
                                            DAT_10315350[slot] = stun + 0x14;
                                            if (DAT_10daaae8[slot].m_00a8 != 0)
                                                DAT_10315350[DAT_10daaae8[slot].m_00a4] = stun + 0x14;
                                            hits++;
                                        }
                                    }
                                }
                            }
                            if (hits < SWORD_EFFECT_MAX_HITS)
                            {
                                for (int extra = 0; extra < SWORD_EFFECT_MAX_HITS - hits; extra++)
                                {
                                    float angle = (float)FUN_1013f880() / 100.0f;
                                    int radius = FUN_1013f880();
                                    DAT_10daa600[extra] = DAT_10db6ee8[0].matrix;
                                    float reach = (float)(radius % SWORD_EXTRA_RADIUS_SPREAD + SWORD_EXTRA_RADIUS_MIN);
                                    DAT_10daa600[extra].m[MATRIX_TRANSLATION_X] =
                                        (float)cos(angle) * reach + DAT_10daa600[extra].m[MATRIX_TRANSLATION_X];
                                    DAT_10daa600[extra].m[MATRIX_TRANSLATION_Y] =
                                        (float)sin(angle) * reach + DAT_10daa600[extra].m[MATRIX_TRANSLATION_Y];
                                    DAT_10daa600[extra].m[MATRIX_TRANSLATION_Z] =
                                        DAT_10daa600[extra].m[MATRIX_TRANSLATION_Z] + SWORD_EXTRA_RAISE;
                                    strikeAt.Copy(DAT_10daa600[extra].GetTranslation());
                                    strikeAt.z = strikeAt.z + SWORD_EFFECT_RAISE;
                                    EffectOptions extraHit;
                                    extraHit.m_0004 = (int)((float)-DAT_10daaae8[0].stats[STAT_EFFECT_PARAM] * DAT_10467508);
                                    extraHit.m_0008 = (float)(DAT_108a6a58[DAT_104c00e8].strength * (DAT_102024c8 + 10) * 4)
                                                      / EFFECT_STRENGTH_DIVISOR_LARGE;
                                    extraHit.flags = 1;
                                    ApplyLoadoutScale(extraHit.m_0008);
                                    if (DAT_10c6566c != 0)
                                    {
                                        extraHit.m_0004 = 1;
                                        extraHit.m_0008 = 1.0f;
                                    }
                                    extraHit.m_000e = (char)weaponKind;
                                    extraHit.owner = SWORD_EFFECT_OWNER;
                                    extraHit.m_0000 = 1;
                                    extraHit.settings[EFFECT_SETTING_AREA] = 2;
                                    DAT_10daaa80.Start(DAT_108a6a58[DAT_104c00e8].effect, strikeAt, -1, 0, &DAT_10daa600[extra],
                                                       0, 0, 0, &extraHit, 0);
                                }
                            }
                        }
                        else if (DAT_104c00e8 == ATTACK_AREA_BEAM)
                        {
                            options.m_0004 = (int)((float)-DAT_10daaae8[0].stats[STAT_EFFECT_PARAM] * DAT_10467508);
                            options.m_0008 = (float)((DAT_102024c8 + 10) * DAT_108a6a58[ATTACK_AREA_BEAM].strength * 4)
                                             / EFFECT_STRENGTH_DIVISOR_LARGE;
                            ApplyLoadoutScale(options.m_0008);
                            options.m_000e = DAT_108f667c[DAT_10daabb8].m_0074;
                            options.owner = SWORD_EFFECT_OWNER;
                            options.settings[EFFECT_SETTING_AREA] = 2;
                            options.flags = 1;
                            if (DAT_10c6566c != 0)
                            {
                                options.m_0004 = 1;
                                options.m_0008 = 1.0f;
                            }
                            options.settings[0] = DAT_108a6a58[ATTACK_AREA_BEAM].settingA;
                            options.settings[1] = DAT_108a6a58[ATTACK_AREA_BEAM].settingB;
                            Vec4 beamAt;
                            beamAt.Fill(0.0f);
                            beamAt.z = SWORD_EFFECT_RAISE;
                            DAT_10daaa80.Start(DAT_108a6a58[DAT_104c00e8].effect, beamAt, -1, 0, 0, 0, 0,
                                               &DAT_10db6ee8[0].matrix, &options, 0);
                        }
                        else
                        {
                            if (DAT_104c0173 == 0)
                            {
                                options.m_0004 = DAT_10daaae8[0].stats[STAT_EFFECT_PARAM];
                                options.m_0008 = (float)DAT_108a6a58[DAT_104c00e8].strength;
                            }
                            else
                            {
                                options.m_0004 = -DAT_10daaae8[0].stats[STAT_EFFECT_PARAM];
                                options.m_0008 = (float)DAT_108a6a58[DAT_104c00e8].strength * EFFECT_STRENGTH_THROWN;
                            }
                            options.m_0008 = options.m_0008 / EFFECT_STRENGTH_DIVISOR;
                            ApplyLoadoutScale(options.m_0008);
                            options.m_0008 = options.m_0008 * drillScale;
                            options.m_001c = 1;
                            options.m_0000 = 1;
                            unsigned char modeFlags = (DAT_10447dd0 != 0) << 1 | DAT_104c0118;
                            if (DAT_104c00e8 == 0x25 || DAT_104c00e8 == 0x28)
                                modeFlags = modeFlags | 4;
                            if (DAT_104c00e8 == 0x13 || DAT_104c00e8 == 0x30)
                            {
                                modeFlags = modeFlags | 0x10;
                                if (DAT_104c00e8 == 0x30)
                                    modeFlags = modeFlags | 0x30;
                            }
                            options.settings[EFFECT_SETTING_FLAGS] = modeFlags | 8;
                            int helmetKind = DAT_1088e6b0[DAT_10daabb8].helmetKind;
                            options.m_000e = 5;
                            if (helmetKind == HELMET_KIND_RED)
                                options.m_000e = 1;
                            if (helmetKind == HELMET_KIND_BLUE)
                                options.m_000e = 3;
                            if (helmetKind == HELMET_KIND_WHITE)
                                options.m_000e = 2;
                            options.settings[EFFECT_SETTING_AREA] = 1;
                            options.flags = 1;
                            if (DAT_10c6566c != 0)
                            {
                                options.m_0004 = 1;
                                options.m_0008 = 1.0f;
                            }
                            options.settings[0] = DAT_108a6a58[DAT_104c00e8].settingA;
                            options.settings[1] = DAT_108a6a58[DAT_104c00e8].settingB;
                            options.owner = ATTACK_EFFECT_OWNER;
                            Node *attachNode = DAT_10db6ee8[0].root->Find(DAT_108a6a58[DAT_104c00e8].name, 0);
                            if (_strnicmp(DAT_108a6a58[DAT_104c00e8].name, "scop", 4) == 0)
                                DAT_104c02ee = 1;
                            Vec4 effectAt;
                            effectAt.Fill(0.0f);
                            if (attachNode == 0)
                                DAT_10daaa80.Start(DAT_108a6a58[DAT_104c00e8].effect, effectAt, -1, 0, 0, 0, 0,
                                                   &DAT_10db6ee8[0].matrix, &options, 0);
                            else
                                DAT_10daaa80.Start(DAT_108a6a58[DAT_104c00e8].effect, effectAt, -1, 0,
                                                   &attachNode->worldMatrix, 0, 0, &DAT_10db6ee8[0].matrix, &options, 0);
                        }
                    }
secondEffect:
                    // The second effect (while DAT_104c0170 is set): started once, then the flag clears.
                    if (DAT_104c0170 != 0)
                    {
                        if (DAT_108a6a58[DAT_104c00e8].effect2 > 0)
                        {
                            int effectFrame = DAT_108a6a58[DAT_104c00e8].effect2Frame;
                            if (effectFrame != -1 || DAT_10db6ee8[0].m_238c == 0)
                            {
                                if ((int)((float)effectFrame * DAT_104c00f0) != DAT_104c00f4)
                                    goto thirdEffect;
                            }
                            EffectOptions options;
                            DAT_104c0170 = 0;
                            if (DAT_104c0173 == 0)
                            {
                                options.m_0004 = DAT_10daaae8[0].stats[STAT_EFFECT_PARAM];
                                options.m_0008 = (float)DAT_108a6a58[DAT_104c00e8].strength;
                            }
                            else
                            {
                                options.m_0004 = -DAT_10daaae8[0].stats[STAT_EFFECT_PARAM];
                                options.m_0008 = (float)DAT_108a6a58[DAT_104c00e8].strength * EFFECT_STRENGTH_THROWN;
                            }
                            options.m_0008 = options.m_0008 / EFFECT_STRENGTH_DIVISOR;
                            ApplyLoadoutScale(options.m_0008);
                            options.m_0008 = options.m_0008 * drillScale;
                            options.m_001c = 1;
                            options.m_0000 = 1;
                            options.owner = ATTACK_EFFECT_OWNER;
                            options.flags = 1;
                            options.settings[EFFECT_SETTING_FLAGS] = (DAT_10447dd0 != 0) << 1 | DAT_104c0118;
                            int helmetKind = DAT_1088e6b0[DAT_10daabb8].helmetKind;
                            options.m_000e = 5;
                            if (helmetKind == HELMET_KIND_RED)
                                options.m_000e = 1;
                            if (helmetKind == HELMET_KIND_BLUE)
                                options.m_000e = 3;
                            if (helmetKind == HELMET_KIND_WHITE)
                                options.m_000e = 2;
                            options.settings[0] = DAT_108a6a58[DAT_104c00e8].settingA;
                            options.settings[1] = DAT_108a6a58[DAT_104c00e8].settingB;
                            options.settings[EFFECT_SETTING_AREA] = 0;
                            Vec4 effectAt;
                            effectAt.Fill(0.0f);
                            DAT_10daaa80.Start(DAT_108a6a58[DAT_104c00e8].effect2, effectAt, -1, 0, 0, 0, 0,
                                               &DAT_10db6ee8[0].matrix, &options, 0);
                        }
                    }
thirdEffect:
                    // The third effect, at its frame.
                    if (DAT_108a6a58[DAT_104c00e8].effect3 > 0)
                    {
                        int effectFrame = DAT_108a6a58[DAT_104c00e8].effect3Frame;
                        if (effectFrame >= 0 || -DAT_104c00f4 == effectFrame || -effectFrame < DAT_104c00f4)
                        {
                            if ((int)((float)effectFrame * DAT_104c00f0) != DAT_104c00f4)
                                goto attackSounds;
                        }
                        EffectOptions options;
                        if (DAT_104c0173 == 0)
                        {
                            options.m_0004 = DAT_10daaae8[0].stats[STAT_EFFECT_PARAM];
                            options.m_0008 = (float)DAT_108a6a58[DAT_104c00e8].strength;
                        }
                        else
                        {
                            options.m_0004 = -DAT_10daaae8[0].stats[STAT_EFFECT_PARAM];
                            options.m_0008 = (float)DAT_108a6a58[DAT_104c00e8].strength * EFFECT_STRENGTH_THROWN;
                        }
                        options.m_0008 = options.m_0008 / EFFECT_STRENGTH_DIVISOR;
                        if (DAT_10daabb4 == HELMET_THIRD_A || DAT_10daabb4 == HELMET_THIRD_B)
                            options.m_0008 = options.m_0008 * EFFECT_STRENGTH_THROWN;
                        options.m_0008 = options.m_0008 * drillScale;
                        options.m_001c = 1;
                        options.m_0000 = 1;
                        options.flags = 1;
                        options.settings[EFFECT_SETTING_FLAGS] = (DAT_10447dd0 != 0) << 1 | DAT_104c0118;
                        int helmetKind = DAT_1088e6b0[DAT_10daabb8].helmetKind;
                        options.m_000e = 5;
                        if (helmetKind == HELMET_KIND_RED)
                            options.m_000e = 1;
                        if (helmetKind == HELMET_KIND_BLUE)
                            options.m_000e = 3;
                        if (helmetKind == HELMET_KIND_WHITE)
                            options.m_000e = 2;
                        options.settings[0] = DAT_108a6a58[DAT_104c00e8].settingA;
                        options.settings[1] = DAT_108a6a58[DAT_104c00e8].settingB;
                        options.settings[EFFECT_SETTING_AREA] = 0;
                        options.owner = ATTACK_EFFECT_OWNER;
                        Vec4 effectAt;
                        effectAt.Fill(0.0f);
                        DAT_10daaa80.Start(DAT_108a6a58[DAT_104c00e8].effect3, effectAt, -1, 0, 0, 0, 0,
                                           &DAT_10db6ee8[0].matrix, &options, 0);
                    }
attackSounds:
                    // The attack's two sounds, at their frames.
                    if (DAT_108a6a58[DAT_104c00e8].sound > 0)
                    {
                        if ((int)((float)DAT_108a6a58[DAT_104c00e8].soundFrame * DAT_104c00f0) == DAT_104c00f4)
                        {
                            Vec4 soundAt = DAT_10db9210.GetTranslation();
                            FUN_1003ad20(DAT_108a6a58[DAT_104c00e8].sound, &soundAt, 1.0f);
                        }
                    }
                    if (DAT_108a6a58[DAT_104c00e8].sound2 > 0)
                    {
                        if ((int)((float)DAT_108a6a58[DAT_104c00e8].sound2Frame * DAT_104c00f0) == DAT_104c00f4)
                        {
                            Vec4 soundAt = DAT_10db9210.GetTranslation();
                            FUN_1003ad20(DAT_108a6a58[DAT_104c00e8].sound2, &soundAt, 1.0f);
                        }
                    }
                    // Early in a swing in some areas: turn away from the target (the nearest one of the character slots
                    // LOCK_FIRST_MODEL.. when the locked one is gone).
                    if (DAT_104c00f4 < SWING_TURN_FRAMES && steerX == 0.0f && steerY == 0.0f
                        && (DAT_104c00e8 == 0x16 || DAT_104c00e8 == 0x17 || DAT_104c00e8 == 0x19))
                    {
                        int turnTarget = DAT_102024ac;
                        if (turnTarget < 1)
                        {
                            float nearestDistance = TURN_NEAREST_START;
                            int nearest = -1;
                            for (int slot = LOCK_FIRST_MODEL; slot < DOOR_FIRST_MODEL; slot++)
                            {
                                if (DAT_104bfe30 == 0 && (DAT_1021f3c0[slot] == 'c' || DAT_1021f3c0[slot] == 3)
                                    && DAT_10daaae8[slot].stats[STAT_BUSY] == 0 && DAT_10daaae8[slot].stats[STAT_HEALTH] > 0
                                    && DAT_10daaae8[slot].m_0141 == 0)
                                {
                                    float distance = DAT_1020e338[slot]->GetTranslation().DistanceSquared(ahead);
                                    if (distance < nearestDistance)
                                    {
                                        nearest = slot;
                                        nearestDistance = distance;
                                    }
                                }
                            }
                            if (nearest < 1)
                            {
                                nearest = -1;
                                for (int slot = LOCK_FIRST_MODEL; slot < DOOR_FIRST_MODEL; slot++)
                                {
                                    if (DAT_104bfe30 == 0 && (DAT_1021f3c0[slot] == 'c' || DAT_1021f3c0[slot] == 3)
                                        && DAT_10daaae8[slot].stats[STAT_HEALTH] > 0 && DAT_10daaae8[slot].m_0141 == 0)
                                    {
                                        float distance = DAT_1020e338[slot]->GetTranslation().DistanceSquared(ahead);
                                        if (distance < nearestDistance)
                                        {
                                            nearest = slot;
                                            nearestDistance = distance;
                                        }
                                    }
                                }
                            }
                            if (nearest >= 1)
                                turnTarget = nearest;
                        }
                        if (DAT_104c00e8 != 0x2b && turnTarget > 0 && DAT_10daaae8[turnTarget].stats[STAT_HEALTH] > 0)
                            DAT_10db6ee8[0].FaceAwayFrom(&DAT_10db6ee8[turnTarget]);
                    }
                    // Facing the point the camera looks at (not in the two areas that have their own).
                    if (DAT_104c00e8 != 0x30 && DAT_104c00e8 != 0x2b && DAT_104bfe20 != 0 && steerX == 0.0f && steerY == 0.0f
                        && DAT_102024c0 > 0 && DAT_10daaae8[DAT_102024c0].stats[STAT_HEALTH] > 0)
                    {
                        DAT_10db6ee8[0].FaceAway(DAT_108029a8);
                        if (DAT_10447dd0 != 0)
                            DAT_10db6ee8[0].heading = DAT_10db6ee8[0].targetHeading;
                    }
                    if (DAT_104c00e8 != 0x18 && DAT_104c00f4 < 4)
                        DAT_104c018c = DAT_10db6ee8[0].targetHeading;
                    DAT_104c00f4 = DAT_104c00f4 + 1;
                    // The swing's camera kick and body pose follow the record's key bank (frame * attack speed).
                    {
                        float kickStart;
                        float kickEnd;
                        float ignored;
                        DAT_108a6a58[DAT_104c00e8].sample.Sample((int)((float)DAT_104c00f4 * DAT_104c00ec - 1.0f), &kickStart,
                                                                 &ignored, &ignored, &ignored, 0);
                        DAT_108a6a58[DAT_104c00e8].sample.Sample((int)((float)DAT_104c00f4 * DAT_104c00ec), &kickEnd, &ignored,
                                                                 &ignored, &ignored, 0);
                        if (DAT_104c00f4 == 1)
                        {
                            // The first frame: the scope's light effect, the idle flag and the settled pose.
                            if (DAT_104c01e0 == 0 && DAT_104c00e8 != 0x2b
                                && (DAT_104c02ee != 0 || _strnicmp(DAT_108a6a58[DAT_104c00e8].name, "scop", 4) == 0))
                            {
                                Node *scopePoint = DAT_10db6ee8[0].root->Find("scop-pt", 0);
                                Node *charBase = DAT_10db6ee8[0].root->Find("charbase0", 0);
                                Vec4 scopeAt;
                                if (scopePoint != 0 && charBase != 0)
                                {
                                    DAT_10f66948.Copy(scopePoint->worldMatrix.GetTranslation());
                                    EffectOptions options;
                                    options.flags = 1;
                                    options.owner = ATTACK_EFFECT_OWNER;
                                    DAT_10daaa80.Start(SCOPE_EFFECT, scopeAt, -1, 0, &scopePoint->worldMatrix,
                                                       (int)&charBase->worldMatrix, 0, 0, &options, 0);
                                }
                            }
                            if (DAT_10db6ee8[0].m_238c == 0)
                                DAT_104c0132 = 0;
                            DAT_1046812c = DAT_10db6ee8[0].targetHeading;
                            if (DAT_10db6ee8[0].mode != 1)
                            {
                                DAT_10db6ee8[0].mode = 1;
                                DAT_10db6ee8[0].verticalSpeed = 0.0f;
                                DAT_10db6ee8[0].m_237c = 1.0f;
                            }
                        }
                        // The way the swing moves the player: shift (in the facing direction) and the two probe offsets.
                        Vec4 playerPos = DAT_10db9210.GetTranslation();
                        DAT_102199b0 = DAT_10db6ee8[0].targetHeading;
                        Vec4 shift;
                        shift.Fill(0.0f);
                        shift.y = kickStart - kickEnd;
                        shift.RotateZ(-DAT_10db6ee8[0].heading);
                        shift.Copy(playerPos + shift);
                        if ((shift.Distance(playerPos) < 1.0f && DAT_104c00e8 != 0x13) || DAT_104c00e8 == 0x31)
                        {
                            steerX = 0.0f;
                            steerY = 0.0f;
                        }
                        Vec4 probeNear;
                        probeNear.Fill(0.0f);
                        probeNear.y = SWING_PROBE_NEAR_Y;
                        probeNear.z = SWING_PROBE_NEAR_Z;
                        probeNear.RotateZ(-DAT_10db6ee8[0].heading);
                        Vec4 probeFar;
                        probeFar.Fill(0.0f);
                        probeFar.y = SWING_PROBE_FAR_Y;
                        probeFar.z = SWING_PROBE_FAR_Z;
                        probeFar.RotateZ(-DAT_10db6ee8[0].heading);
                        if (DAT_104c00f4 == 1)
                        {
                            if (DAT_102024c0 < 1)
                                DAT_104463f0 = 0.0f;
                            else
                                DAT_104463f0 = (DAT_108029a8.z - DAT_10db9210.GetTranslation().z) / SWING_DROP_DIVISOR;
                        }
                        shift.z = 0.0f;
                        if (DAT_10447dd0 == 1 && DAT_104c00f4 < SWING_CAMERA_FRAMES)
                            DAT_104c01a4 = 1;
                        // A strong swing kicks the camera and starts the burst effect on its first frames.
                        if (strongSwing == 1 && DAT_104c00f4 < SWING_CAMERA_FRAMES)
                        {
                            DAT_104c01a4 = 1;
                            DAT_104c0171 = 7;
                            DAT_104c0156 = 6;
                            DAT_10db7c34 = 0.7f;
                            Vec4 burstAt;
                            burstAt.Fill(0.0f);
                            Matrix4 burstMatrix = DAT_10db9210;
                            burstMatrix.m[MATRIX_TRANSLATION_X] = DAT_10db6ee8[0].bipNode->worldMatrix.m[MATRIX_TRANSLATION_X];
                            burstMatrix.m[MATRIX_TRANSLATION_Y] = DAT_10db6ee8[0].bipNode->worldMatrix.m[MATRIX_TRANSLATION_Y];
                            burstMatrix.m[MATRIX_TRANSLATION_Z] = DAT_10db6ee8[0].bipNode->worldMatrix.m[MATRIX_TRANSLATION_Z];
                            EffectOptions options;
                            options.flags = 1;
                            if (DAT_10c655f0 == 6)
                                DAT_10daaa80.Start(BURST_EFFECT_POPON, burstAt, -1, 0, 0, 0, 0, &burstMatrix, &options, 0);
                            else
                                DAT_10daaa80.Start(BURST_EFFECT, burstAt, -1, 0, 0, 0, 0, &burstMatrix, &options, 0);
                        }
                        else if (DAT_10db7c34 < 1.0f)
                            DAT_10db7c34 = DAT_10db7c34 + SWING_FADE_STEP;
                        if (strongSwing == 1 && DAT_104c00f4 == 1)
                        {
                            Vec4 soundAt = DAT_10db9210.GetTranslation();
                            FUN_1003ad20(SOUND_STRONG_SWING, &soundAt, 1.0f);
                        }
                        if (DAT_10447dd0 != 0)
                        {
                            if (DAT_104c00f4 == 1)
                            {
                                Vec4 soundAt = DAT_10db9210.GetTranslation();
                                FUN_1003ad20(SOUND_SWING_MODE, &soundAt, 1.0f);
                            }
                            if (DAT_10447dd0 != 0)
                            {
                                if (DAT_10447dd0 != 2 && DAT_102024c0 > 0)
                                {
                                    DAT_10db6ee8[0].mode = 0;
                                    DAT_10db6ee8[0].verticalSpeed = 0.0f;
                                    DAT_10db6ee8[0].m_237c = 1.0f;
                                }
                                if (DAT_104c00f4 < SWING_CAMERA_FRAMES)
                                {
                                    Vec4 targetAt = DAT_1020e338[DAT_102024c0]->GetTranslation();
                                    if (SWING_TARGET_MIN_X < targetAt.x
                                        && (DAT_102239d0[DAT_102024c0] != 0 || DAT_10daaae8[DAT_102024c0].stats[STAT_HEALTH] > 0))
                                        DAT_108029a8.Copy(targetAt);
                                    float aimHeight = DAT_108029a8.z - SWING_AIM_LOWER;
                                    shift.z = aimHeight - DAT_10db9210.GetTranslation().z;
                                    shift.x = DAT_108029a8.x;
                                    shift.y = DAT_108029a8.y;
                                }
                                if (DAT_10468128 == 0)
                                {
                                    if (DAT_10447dd0 != 2 && DAT_104c00f4 > 3
                                        && DAT_10db9210.GetTranslation().Distance(DAT_108392c8) < SWING_ARRIVE_DISTANCE)
                                        goto swingArrived;
                                }
                                else
                                {
swingArrived:
                                    DAT_10db6ee8[0].mode = 1;
                                    DAT_10db6ee8[0].verticalSpeed = 0.0f;
                                    DAT_10db6ee8[0].m_237c = 1.0f;
                                    DAT_10447dd0 = 2;
                                    DAT_104c00e0 = 1;
                                }
                                DAT_108392c8.Copy(DAT_10db9210.GetTranslation());
                                probeNear.z = 0.0f;
                            }
                        }
                        if (shift.z > SWING_SHIFT_LIMIT)
                            shift.z = SWING_SHIFT_LIMIT;
                        else if (shift.z < -SWING_SHIFT_LIMIT)
                            shift.z = -SWING_SHIFT_LIMIT;
                        if (strongSwing != 2 && DAT_10db6ee8[0].IsClearAny(&probeNear) != 0)
                        {
                            if (DAT_102024c0 > 0 && shift.z == 0.0f && fabs(DAT_108029a8.z - DAT_10db9248) < SWING_SAME_HEIGHT
                                && (DAT_10db9210.GetTranslation() - DAT_108029a8).Length2D() < SWING_SAME_DISTANCE)
                                goto swingStepDone;
                            if (DAT_10447dd0 != 2)
                            {
                                if (DAT_10db6ee8[0].m_229c == 0)
                                    DAT_10db6ee8[0].MoveTowards(shift.x, shift.y, *(int *)&shift.z);
                                FUN_1016b260(DAT_10db6ee8[0].m_23d8.x, DAT_10db6ee8[0].m_23d8.y);
                            }
                        }
                    }
swingStepDone:
                    DAT_104c00e0 = DAT_104c00e0 - 1;
                    if (DAT_104c00e0 == 0)
                    {
                        DAT_10db6ee8[0].targetHeading = DAT_102199b0;
                        DAT_10db6ee8[0].mode = 1;
                        if (DAT_10db6ee8[0].m_229c == 0)
                            DAT_10db6ee8[0].verticalSpeed = 0.0f;
                        DAT_10db6ee8[0].m_237c = 1.0f;
                        DAT_10db7c34 = 1.0f;
                    }
                }
            }
            // The scope: while an attack record plays, a light effect follows each pointer node of the helmet.
            if (DAT_104c01e0 == 0 && DAT_104c00f8 != 0)
            {
                Vec4 pointAt;
                pointAt.Copy(DAT_104c00f8->worldMatrix.GetTranslation());
                DAT_10836c74.Copy(DAT_10867e08);
                DAT_10867e08.Copy(pointAt);
                EffectOptions options;
                options.flags = 1;
                options.owner = ATTACK_EFFECT_OWNER;
                if (DAT_10c655f0 == COSTUME_THREE_POINTERS)
                {
                    DAT_10daaa80.Start(PointerEffectForCostumeSix(), pointAt, -1, 0, 0, 0, 0, 0, &options, 0);
                    if (DAT_104c0100 != 0)
                        StartExtraPointerEffect(DAT_104c0100, pointAt);
                    if (DAT_104c0104 != 0)
                        StartExtraPointerEffect(DAT_104c0104, pointAt);
                }
                else
                {
                    int helmetKind = DAT_1088e6b0[DAT_10daabb8].helmetKind;
                    int effect = POINTER_EFFECT_DEFAULT;
                    if (helmetKind == HELMET_KIND_RED)
                        effect = POINTER_EFFECT_RED;
                    else if (helmetKind == HELMET_KIND_BLUE)
                        effect = POINTER_EFFECT_BLUE;
                    else if (helmetKind == HELMET_KIND_WHITE)
                        effect = POINTER_EFFECT_WHITE;
                    DAT_10daaa80.Start(effect, pointAt, -1, 0, 0, 0, 0, 0, &options, 0);
                }
                DAT_10f66948.Copy(DAT_104c00f8->worldMatrix.GetTranslation());
            }
            greetings = DAT_104c00e0;
            if (DAT_10db6ee8[0].m_238c != 0 && DAT_104c00e0 < 2)
                DAT_104c0132 = 1;
            if (DAT_104c00e0 == 0 || (DAT_1101d898 = 1, DAT_102024ac < 1))
                DAT_1101d898 = 0;
            if (DAT_104c00e0 == 0)
            {
                if (DAT_102024c4 != 0 && (DAT_102024c4 = DAT_102024c4 - 1, DAT_102024c4 == 0))
                    DAT_10447dd0 = 0;
                DAT_104c015c = 0;
            }
            if (DAT_104c0171 != 0)
                DAT_104c0171 = DAT_104c0171 - 1;
            if (DAT_104c0156 > -10)
                DAT_104c0156 = DAT_104c0156 - 1;
            // Attack events that are too old or too far ahead are dropped from the queue.
            for (int event = 0; event < ACTION_EVENT_COUNT; event++)
            {
                if (DAT_10254110[event].key <= DAT_1032d6a0 - ACTION_EVENT_PAST || DAT_1032d6a0 + ACTION_EVENT_AHEAD <= DAT_10254110[event].key)
                    DAT_10254110[event].key = -1;
            }
            // The attack chain state (0 none, 1 first attack of a chain, 2 the attack that follows a locked one).
            if (DAT_104c01ec == 0 || greetings > CHAIN_GREETINGS_MAX)
            {
                if (greetings == 0)
                    DAT_104c01f8 = 0;
            }
            else if (greetings != 0)
            {
                if (DAT_104c0164 == DAT_104c0160)
                {
                    if (DAT_104c01ec == 3)
                        DAT_104c01f8 = 2;
                    else
                        DAT_104c01f8 = 1;
                }
                else if (DAT_104c01ec == 2 && DAT_104c0164 - 1 == DAT_104c0160)
                    DAT_104c01f8 = 2;
            }
            else
                DAT_104c01f8 = 0;
            // The attack state counter reaches its resume value: the player stands up again and the position trail is refilled.
            if (DAT_104c0228 > ATTACK_STATE_LIMIT - 1 && DAT_104bfe1c == 0 && mouseMiddle == 2 && DAT_104c0228 == ATTACK_STATE_RESUME)
            {
                DAT_10db6ee8[0].PlayMotion(2, 0, 5, 1.0f);
                DAT_104c0228 = 0;
                for (int step = PLAYER_TRAIL_LENGTH; step != 0; step--)
                {
                    DAT_10802ad8[DAT_104c022c].Copy(DAT_10db6ee8[0].matrix.GetTranslation());
                    DAT_104c022c = DAT_104c022c + 1 & 0xf;
                }
                greetings = DAT_104c00e0;
            }
            if (DAT_104bfe30 != 0 && mouseMiddle != 0)
            {
                DAT_104bfe30 = 0;
                DAT_10db6ee8[DAT_104bf8b4].mode = 1;
                DAT_10db6ee8[0].PlayMotion(2, 1, 5, 1.0f);
                greetings = DAT_104c00e0;
            }
            if (DAT_104c0133 != 0)
                DAT_104c0133 = DAT_104c0133 - 1;
            // The attack buttons: a held press (the charge) is tracked in DAT_104c0168/016c/0194.
            bool attackAllowed = DAT_104c0228 < ATTACK_STATE_LIMIT && DAT_1032d69c != 0 && greetings != 0
                                 && DAT_104c0133 == 0 && greetings < ATTACK_GREETINGS_MAX && DAT_104bfdc8 == 0
                                 && controlsEnabled != 0 && DAT_104c0068 != 1 && DAT_104bfdf0 == 0
                                 && DAT_10db6ee8[0].m_2288 < 3 && DAT_104bfe1c == 0 && DAT_104bfd74 == 2
                                 && DAT_104bfe30 == 0 && DAT_104c0300 == 0 && DAT_104bfe4c == 0 && DAT_104bfd60 == 0
                                 && DAT_104c00f8 != 0 && mouseMiddle == 2;
            if (attackAllowed)
            {
                DAT_104c0168 = 1;
                DAT_104c016c = DAT_1065cb3f != 0 || mouseRight != 0;
                DAT_104c016c |= mouseMiddle != 0 && (DAT_1065cb3f != 0 || mouseRight != 0);
            }
            else if (DAT_104c0168 != 0)
                DAT_104c016c |= mouseMiddle != 0 && (DAT_1065cb3f != 0 || mouseRight != 0);
            bool chargeRunning;
            if (DAT_104c0228 < ATTACK_STATE_LIMIT && DAT_1032d69c != 0)
            {
                chargeRunning = true;
                if (DAT_104c0194 == 0)
                {
                    if (DAT_104c0133 != 0 || DAT_104c00e0 > 3 || DAT_104bfdc8 != 0 || controlsEnabled == 0 || DAT_104c0068 == 1
                        || DAT_104bfdf0 != 0 || DAT_10db6ee8[0].m_2288 > 2 || DAT_104bfe1c != 0 || DAT_104bfd74 != 2
                        || DAT_104bfe30 != 0 || DAT_104c0300 != 0 || DAT_104bfe4c != 0 || DAT_104bfd60 != 0
                        || DAT_104c00f8 == 0
                        || (DAT_104c0168 == 0 && mouseMiddle != 2 && (mouseMiddle == 0 || (DAT_1065cb31 == 0 && mouseRight != 2))))
                        chargeRunning = false;
                    else
                    {
                        DAT_104c0194 = 1;
                        if (DAT_10468128 > 8)
                            DAT_10468128 = 7;
                    }
                }
            }
            else
                chargeRunning = DAT_104c0194 != 0;
            if (chargeRunning)
            {
                DAT_104c0194 = DAT_104c0194 + 1;
                DAT_104c0168 = 1;
                DAT_104c016c |= mouseMiddle != 0 && (DAT_1065cb3f != 0 || mouseRight != 0);
            }
            if (DAT_104c023e != 0 && DAT_104c023e < 10)
                DAT_104c023e = DAT_104c023e + 1;
            // The button is released (or the press ended): choose the attack and start it.
            if (DAT_104c0228 < ATTACK_STATE_LIMIT
                && (DAT_104c022e != 0 || DAT_104c0194 > 3 || (DAT_104c023e != 0 && DAT_104c023e < 3)) && DAT_104c01e8 == 0)
            {
                DAT_104c0194 = 0;
                int attackTarget = 0;
                int charged = mouseMiddle != 0 && (DAT_1065cb3f != 0 || mouseRight != 0);
                if (DAT_104c0168 != 0)
                    charged = DAT_104c016c;
                int chargedAtRelease = charged;
                DAT_104c016c = 0;
                int heldInput = (DAT_1065cb3f != 0 && DAT_1065cb31 == 0) || mouseRight == 1;
                int attack = FUN_10011520(0, &charged, &attackTarget, (int)DAT_10210b2c, (int)DAT_1030c990, heldInput,
                                          DAT_104c022e);
                if ((DAT_10db6ee8[DAT_104bf8b4].m_2288 != 5 || DAT_104bfdf0 != 0 || DAT_104bfe1c != 0) && attack >= 0)
                    FUN_1005ce80(attack, chargedAtRelease, attackTarget, 0);
                if (DAT_104c00e0 != 0 || DAT_104c0300 != 0 || DAT_104bfe4c != 0 || DAT_104bfe30 != 0 || DAT_104bfe1c != 0
                    || DAT_104c0068 != 0 || DAT_104c01e0 != 0)
                {
                    DAT_104c0190 = 0;
                    if (DAT_104c0108 > FADE_HOLD_LIMIT)
                        FUN_1000ec70(5, 1.0f, 1.0f, 1.0f, 1.0f);
                    DAT_104c0108 = 0;
                }
            }
            if (DAT_104c02b4 != 0)
                DAT_104c02b4 = DAT_104c02b4 - 1;
            if (DAT_10daaaec < DAT_10daaaf0 / 4 && DAT_104bf890 % LOW_HEALTH_INTERVAL == 0)
                FUN_1000d620(0, 0xb, 0x1e);
            if (DAT_104c0190 != 0
                && (DAT_104c00e0 != 0 || DAT_104c0300 != 0 || DAT_104bfe4c != 0 || DAT_104bfe30 != 0 || DAT_104bfe1c != 0))
            {
                DAT_104c0190 = 0;
                if (DAT_104c0108 > FADE_HOLD_LIMIT)
                    FUN_1000ec70(0xf, 1.0f, 1.0f, 1.0f, 1.0f);
                DAT_104c0108 = 0;
            }
            // The helmet's special moves (the stair spots, the charged flash): free controls needed.
            if (DAT_104c0228 < ATTACK_STATE_LIMIT && DAT_1032d69c != 0 && controlsEnabled != 0 && DAT_104bfe1c == 0
                && DAT_104bfdf0 == 0 && DAT_104bfdc8 == 0 && DAT_104c0300 == 0 && DAT_104bfe4c == 0 && DAT_104c00f8 != 0
                && DAT_104c0068 == 0 && DAT_104c01e0 == 0)
            {
                // The flash of a finished charge: a ring effect, then the screen tint fades back to white.
                if (DAT_10467798 != 0)
                {
                    if (DAT_10467798 == FLASH_LEVELS)
                    {
                        EffectOptions options;
                        options.m_0004 = -DAT_10daaae8[0].stats[STAT_EFFECT_PARAM];
                        options.m_0008 = (float)DAT_108a6a58[DAT_104c00e8].strength * 1.5f / 100.0f;
                        ApplyLoadoutScale(options.m_0008);
                        options.m_001c = 1;
                        options.m_0000 = 1;
                        options.settings[EFFECT_SETTING_FLAGS] = 1;
                        options.m_000e = 5;
                        options.settings[EFFECT_SETTING_AREA] = 0;
                        options.owner = ATTACK_EFFECT_OWNER;
                        DAT_10daaa80.Start(FLASH_EFFECT, DAT_10864ce8, -1, 0, 0, 0, 0, 0, &options, 0);
                        DAT_10467798 = 0;
                    }
                    Vec4 tint;
                    Vec4 white;
                    tint.Set(0.3f, 0.4f, 0.4f, 1.0f);
                    white.Set(1.0f, 1.0f, 1.0f, 1.0f);
                    int level = DAT_10467798;
                    if (level > FLASH_LEVELS - 1)
                        level = FLASH_LEVELS;
                    tint.Copy(white - (white - tint) * ((float)level / (float)FLASH_LEVELS));
                    FUN_1000ec70(5, tint.x, tint.y, tint.z, 1.0f);
                    if (DAT_1021a368 == 0 || (DAT_1021a368 = DAT_1021a368 - 1, DAT_1021a368 == 0))
                        DAT_10467798 = DAT_10467798 - 1;
                    if (DAT_10467798 == 0)
                        FUN_1000ec70(10, 1.0f, 1.0f, 1.0f, 1.0f);
                }
                if (mouseMiddle == 0 && DAT_1065cb3e == 0)
                {
                    // No button held: the charge counter DAT_104c0108 is released. Near one of the four spots the
                    // player is placed on it, otherwise the release attack starts.
                    DAT_104c0190 = 0;
                    Vec4 spots[SPOT_COUNT + 1];
                    spots[SPOT_COUNT].Copy(DAT_10db9210.GetTranslation());
                    spots[0].Set(1773.0f, 720.0f, 38.0f, 3.85f);
                    spots[1].Set(1763.0f, -720.0f, 38.0f, 5.479f);
                    spots[2].Set(327.0f, -720.0f, 38.0f, 0.76f);
                    spots[3].Set(327.0f, 720.0f, 38.0f, 2.36f);
                    int pathFree = DAT_10db9274;
                    if (DAT_10db9274 == 0)
                    {
                        Vec4 probe;
                        probe.Set(0.0f, 0.0f, -30.0f, 0.0f);
                        pathFree = DAT_10db6ee8[DAT_104bf8b4].IsClearOfMap(&probe) == 0;
                        if (pathFree == 0)
                        {
                            probe.Set(0.0f, 0.0f, -20.0f, 0.0f);
                            pathFree = DAT_10db6ee8[DAT_104bf8b4].IsClear(&probe) == 0;
                        }
                    }
                    if (DAT_104c0108 < CHARGE_MIN_FRAMES)
                        goto chargeRetreat;
                    if (pathFree == 0)
                    {
                        FUN_1000ec70(0xf, 1.0f, 1.0f, 1.0f, 1.0f);
                        DAT_104c0108 = CHARGE_MIN_FRAMES - 1;
                        DAT_104c0190 = 0;
                    }
                    else
                    {
                        if (DAT_10c62ed0 == SPOT_MAP && spots[0].Distance(spots[SPOT_COUNT]) < SPOT_REACH
                            && FUN_101ac810(spots[0].w, DAT_10db6ee8[0].heading) < SPOT_TURN)
                            ArriveAtSpot(spots[0], 2);
                        else if (DAT_10c62ed0 == SPOT_MAP && spots[1].Distance(spots[SPOT_COUNT]) < SPOT_REACH
                                 && FUN_101ac810(spots[1].w, DAT_10db6ee8[0].heading) < SPOT_TURN)
                            ArriveAtSpot(spots[1], 0x66);
                        else if (DAT_10c62ed0 == SPOT_MAP && spots[2].Distance(spots[SPOT_COUNT]) < SPOT_REACH
                                 && FUN_101ac810(spots[2].w, DAT_10db6ee8[0].heading) < SPOT_TURN)
                            ArriveAtSpot(spots[2], 0xca);
                        else if (DAT_10c62ed0 == SPOT_MAP && spots[3].Distance(spots[SPOT_COUNT]) < SPOT_REACH
                                 && FUN_101ac810(spots[3].w, DAT_10db6ee8[0].heading) < SPOT_TURN)
                            ArriveAtSpot(spots[3], 0x12e);
                        else
                        {
chargeRetreat:
                            if (DAT_104c0108 < CHARGE_LONG_FRAMES)
                            {
                                if (DAT_104c0108 < CHARGE_MIN_FRAMES)
                                    DAT_104c0108 = DAT_104c0108 - 4;
                                else
                                {
                                    DAT_104c0108 = 0;
                                    DAT_104c0190 = 0;
                                    DAT_1049ef8c = 0;
                                    FUN_1005ce80(0x13, 0, 0, 0);
                                }
                            }
                            else
                            {
                                DAT_104c0108 = 0;
                                DAT_104c0190 = 0;
                                DAT_1049ef8c = 0;
                                FUN_1005ce80(0x30, 0, 0, 0);
                            }
                        }
                        if (DAT_104c0108 > CHARGE_MAX_FRAMES)
                            DAT_104c0108 = CHARGE_MAX_FRAMES;
                        else if (DAT_104c0108 < 0)
                            DAT_104c0108 = 0;
                    }
                }
                else
                {
                    // A button is held: the counter grows (the charge), with sounds at two frames.
                    if (DAT_104c0108 >= CHARGE_MIN_FRAMES && DAT_104bf890 % CHARGE_MIN_FRAMES == 0)
                    {
                        Vec4 stepAt = DAT_10db9210.GetTranslation();
                        FUN_1003ad20(SOUND_CHARGE_STEP, &stepAt, 1.0f);
                    }
                    if (DAT_104c0108 == CHARGE_SOUND_A)
                        FUN_1003ad20(SOUND_CHARGE_PEAK, 0, 1.0f);
                    if (DAT_104c0108 == CHARGE_SOUND_B)
                        FUN_1003ad20(SOUND_CHARGE_PEAK, 0, 1.0f);
                    if (DAT_104c00e0 == 0)
                    {
                        if (DAT_104c0108 < CHARGE_LONG_FRAMES || (DAT_104bf890 & 1) != 0)
                            DAT_104c0108 = DAT_104c0108 + 1;
                        if (DAT_104c0108 > CHARGE_LONG_FRAMES)
                            DAT_104c0108 = CHARGE_LONG_FRAMES;
                        if (DAT_104c0108 >= CHARGE_MIN_FRAMES && DAT_104c0190 == 0)
                            DAT_104c0190 = 1;
                    }
                    else
                    {
                        DAT_104c0190 = 0;
                        if (DAT_104c0108 > FADE_HOLD_LIMIT)
                            FUN_1000ec70(0xf, 1.0f, 1.0f, 1.0f, 1.0f);
                        DAT_104c0108 = 0;
                    }
                    if (DAT_104c0108 > CHARGE_MAX_FRAMES)
                        DAT_104c0108 = CHARGE_MAX_FRAMES;
                    else if (DAT_104c0108 < 0)
                        DAT_104c0108 = 0;
                }
            }
            // The charge effects follow the counter while it runs.
            if (DAT_104c0108 > CHARGE_MIN_FRAMES)
            {
                Vec4 chargeAt;
                chargeAt.Copy(DAT_10db9210.GetTranslation());
                if (DAT_104c00f8 != 0)
                {
                    EffectOptions options;
                    options.owner = ATTACK_EFFECT_OWNER;
                    options.flags = 1;
                    int ringEffect;
                    if (DAT_104c0108 < CHARGE_LONG_FRAMES)
                    {
                        DAT_10daaa80.Start(CHARGE_EFFECT_A, chargeAt, -1, 0, &DAT_104c00f8->worldMatrix,
                                           (int)&DAT_10db9210, 0, 0, &options, 0);
                        ringEffect = CHARGE_RING_A;
                    }
                    else if (DAT_104c0108 < CHARGE_MAX_FRAMES)
                    {
                        DAT_10daaa80.Start(CHARGE_EFFECT_B, chargeAt, -1, 0, &DAT_104c00f8->worldMatrix,
                                           (int)&DAT_10db9210, 0, 0, &options, 0);
                        ringEffect = CHARGE_RING_B;
                    }
                    else
                    {
                        DAT_10daaa80.Start(CHARGE_EFFECT_C, chargeAt, -1, 0, &DAT_104c00f8->worldMatrix,
                                           (int)&DAT_10db9210, 0, 0, &options, 0);
                        ringEffect = CHARGE_RING_C;
                    }
                    DAT_10daaa80.Start(ringEffect, chargeAt, -1, 0, &DAT_104c00f8->worldMatrix, (int)&DAT_10db9210, 0, 0,
                                       &options, 0);
                }
            }
            // While a swing plays, the helmet's drill glow follows the "charbase0" node.
            if (DAT_104c00e0 != 0 && DAT_104c00fc != 0 && DAT_104c02ee != 0)
            {
                Vec4 glowAt;
                glowAt.Fill(0.0f);
                EffectOptions options;
                options.flags = 1;
                options.owner = ATTACK_EFFECT_OWNER;
                if (DAT_104c00e8 != 0x2b && DAT_10c6566c == 0 && DAT_10c655f0 != COSTUME_THREE_POINTERS)
                {
                    int drillEffect = 0;
                    if (DAT_10c64ad4 == 0)
                        drillEffect = GLOW_EFFECT_0;
                    else if (DAT_10c64ad4 == 1)
                        drillEffect = GLOW_EFFECT_1;
                    else if (DAT_10c64ad4 == 2)
                        drillEffect = GLOW_EFFECT_2;
                    else if (DAT_10c64ad4 == 3)
                        drillEffect = GLOW_EFFECT_3;
                    if (drillEffect != 0)
                        DAT_10daaa80.Start(drillEffect, glowAt, -1, 0, 0, 0, 0, &DAT_104c00fc->worldMatrix, &options, 0);
                }
                DAT_10daaa80.Start(GLOW_EFFECT_BASE, glowAt, -1, 0, 0, 0, 0, &DAT_104c00fc->worldMatrix, &options, 0);
            }
            // The carried object is pulled to the player for a few frames, then the player turns to face it.
            if (DAT_104bfdc8 > 1)
            {
                Vec4 playerAt;
                Vec4 carriedAt;
                Vec4 pull;
                playerAt.Copy(DAT_10db9210.GetTranslation());
                carriedAt.Copy(DAT_10db6ee8[DAT_104bfdd8].matrix.GetTranslation());
                pull.Copy((carriedAt - playerAt) * 0.06f);
                DAT_10db6ee8[0].Push(pull.x, pull.y, pull.z);
                DAT_104bfdc8 = DAT_104bfdc8 + 1;
                if (DAT_104bfdc8 == CARRY_PULL_FRAMES)
                {
                    DAT_104bfdc8 = 1;
                    carriedAt.Copy(carriedAt - playerAt);
                    carriedAt.Copy(Vec3_Normalized(&carriedAt));
                    carriedAt.Copy(carriedAt * 6.0f);
                    float faceAngle = (float)atan2(carriedAt.x, carriedAt.y);
                    DAT_104bfddc = 0;
                    DAT_10db6ee8[0].targetHeading = faceAngle + 3.1415f;
                }
            }
            if (DAT_104bfdc8 == 0)
            {
releaseHold:
                if (DAT_1032d69c == 0)
                    goto afterPlayerInput;
            }
            else if (DAT_1065cb3f != 0 || mouseRight != 0 || mouseMiddle != 0 || DAT_1065cb3e != 0 || DAT_1032d69c == 0)
            {
                if (DAT_104bfdcc == 0)
                {
                    DAT_104c0090 = 1;
                    DAT_104bfdc8 = 0;
                }
                else
                    DAT_104c0074 = 1;
                goto releaseHold;
            }
            // The talk key: starts the dialogue of the object in front of the player.
            if (DAT_104c00e0 == 0 && DAT_1020d430 == 0 && DAT_10468128 == 0 && DAT_104bfd74 == 4
                && ((DAT_104c00ac != 0 && DAT_104c00b7 == 0) || DAT_104bf888 != 0 || DAT_104c00bc != 0 || DAT_104bfdd0 != 0)
                && mouseMiddle == 2)
            {
                if (DAT_104bf888 < TALK_TARGET_KEEP)
                {
                    if (DAT_104bf888 < TALK_FIRST_OBJECT)
                    {
                        if (DAT_104bf888 == 0)
                        {
                            if (DAT_104c00bc != 0 && (DAT_104bfd64 = 0, DAT_104bfd60 == 0) && DAT_104bfe30 == 0
                                && DAT_10db9274 != 0)
                            {
                                DAT_104bfd68 = 0.0f;
                                DAT_10db9210.GetTranslation();
                                DAT_10db6ee8[DAT_104bf8b4].PlayMotion(0xc, 0, 5, 1.0f);
                                DAT_104bfd60 = 1;
                            }
                        }
                        else
                            FUN_1005ecb0(0);
                    }
                    else
                    {
                        DAT_10db6ee8[DAT_104bf8b4].PlayMotion(2, 0, 5, 1.0f);
                        DAT_10874848.RegisterEvent(DAT_10860df0[DAT_104bf888 - TALK_FIRST_OBJECT].id);
                    }
                }
                else
                {
                    DAT_10db6ee8[DAT_104bf8b4].PlayMotion(2, 0, 5, 1.0f);
                    DAT_10874848.RaiseTalkEvents(DAT_104bf888 - TALK_TARGET_KEEP);
                    rand();
                }
            }
        }
afterPlayerInput:

        DrillClaws_Update();
        if (DAT_104bfe6c == 0)
        {
            // Two offsets built and never read again (their constructors and Set calls stay in the original)
            Vec4 backFar;
            Vec4 backNear;
            backFar.Set(0.0f, 0.0f, -30.0f, 0.0f);
            backNear.Set(0.0f, 0.0f, -20.0f, 0.0f);
            if (DAT_104c0068 == 0)
            {
                // Phase >= TRAIL_REPLAY_BASE: after a drop the player glides back along the ring of recent positions
                if (DAT_104c0228 >= TRAIL_REPLAY_BASE)
                {
                    int step = DAT_104c0228 - TRAIL_REPLAY_BASE;
                    unsigned int newest = DAT_104c022c;
                    Vec4 *from = &TrailBack(newest, 4);
                    Vec4 focus = *from;
                    focus.z = focus.z - TRAIL_FOCUS_DROP;
                    Vec4 blendA;
                    Vec4 blendB;
                    bool blocked = false;
                    if (DAT_104c0228 == TRAIL_REPLAY_BASE)
                        TrailBack(newest, 3).Copy(TrailBack(newest, 1));
                    if (DAT_104c0228 < TRAIL_REPLAY_SWEEP_END)
                    {
                        int weightB = TRAIL_BLEND_STEPS;
                        if (step < TRAIL_BLEND_STEPS + 1)
                            weightB = step;
                        int weightA = step;
                        if (step > TRAIL_BLEND_STEPS - 1)
                            weightA = TRAIL_BLEND_STEPS - 1;
                        BlendTrailPoint(blendA, TrailBack(newest, 1), focus, weightA);
                        BlendTrailPoint(blendB, TrailBack(newest, 1), focus, weightB);
                        if ((focus - blendA).Length2D() < TRAIL_GLIDE_NEAR)
                            blendA.Copy(TrailBack(newest, 3));
                        TrailBack(newest, 3).Copy(blendA);
                        TrailBack(newest, 5).Copy(blendB);
                        if (DAT_104c0228 != TRAIL_REPLAY_ARRIVE)
                            DAT_10db6ee8[0].FaceAway(*from);
                    }
                    else
                    {
                        int weightA = step - TRAIL_FADE_START;
                        if (weightA > 0 && weightA > TRAIL_BLEND_STEPS)
                            weightA = TRAIL_BLEND_STEPS;
                        else
                            weightA = weightA & ((weightA <= 0) - 1);
                        int weightB = TRAIL_BLEND_STEPS;
                        if (step - TRAIL_SWEEP_STEPS < TRAIL_BLEND_STEPS + 1)
                            weightB = step - TRAIL_SWEEP_STEPS;
                        BlendTrailPoint(blendA, TrailBack(newest, 3), *from, weightA);
                        BlendTrailPoint(blendB, TrailBack(newest, 5), *from, weightB);
                    }
                    Vec4 probeA;
                    Vec4 probeB;
                    probeA.Fill(0.0f);
                    probeB.Fill(0.0f);
                    Vec4 savedPos = DAT_10db6ee8[0].matrix.GetTranslation();
                    DAT_10db6ee8[0].matrix.SetTranslation(*from);
                    if (TrailSpotBlocked(probeA, probeB))
                        blocked = true;
                    DAT_10db6ee8[0].matrix.SetTranslation(savedPos);
                    PlacePlayer(blendA.x, blendA.y, blendB.z);
                    if (DAT_104c0228 == TRAIL_REPLAY_BASE)
                        DAT_10db6ee8[DAT_104bf8b4].PlayMotion(TRAIL_MOTION_START, 2, 5, 1.0f);
                    if (DAT_104c0228 == TRAIL_REPLAY_BASE + 11)
                        DAT_10db6ee8[DAT_104bf8b4].PlayMotion(TRAIL_MOTION_LAND, 2, 2, 1.0f);
                    if (DAT_104c0228 == TRAIL_REPLAY_BASE + 22)
                        DAT_10db6ee8[DAT_104bf8b4].PlayMotion(TRAIL_MOTION_END, 1, 5, 1.0f);
                    if (DAT_104c0228 == TRAIL_REPLAY_ARRIVE)
                    {
                        if (DAT_104c023d != 0)
                        {
                            DAT_10db6ee8[0].PlayMotion(TRAIL_MOTION_TURN, 0, 5, 1.0f);
                            DAT_10db6ee8[0].SnapHeadingToSide();
                        }
                        Vec4 probeC;
                        probeC.Fill(0.0f);
                        if (blocked && DAT_10db6ee8[0].IsClearOfMapLimited(&probeC) && DAT_10db6ee8[0].IsClear(&probeC))
                            blocked = true;
                        else
                            blocked = false;
                    }
                    else
                        DAT_104c0228 = DAT_104c0228 + 1;
                    DAT_10db6ee8[0].m_238c = 1;
                    if (DAT_104c0228 == TRAIL_REPLAY_BASE + 24)
                    {
                        blendB.Copy(*from);
                        PlacePlayer(blendB.x, blendB.y, blendB.z);
                        DAT_10db6ee8[0].heading = DAT_10db6ee8[0].targetHeading;
                        DAT_104687b0 = 1;
                    }
                    if (DAT_104c0228 == TRAIL_REPLAY_BASE + 25)
                    {
                        blendB.Copy(*from);
                        PlacePlayer(blendB.x, blendB.y, blendB.z);
                        DAT_10db6ee8[0].heading = DAT_10db6ee8[0].targetHeading;
                        DAT_104c009c = 0;
                        DAT_10468128 = 0;
                        Vec4 ahead;
                        ahead.x = (float)cos(-DAT_10db6ee8[DAT_104bf8b4].heading - TRAIL_SIDE_OFFSET) * TRAIL_PROBE_REACH;
                        ahead.y = (float)sin(-DAT_10db6ee8[DAT_104bf8b4].heading - TRAIL_SIDE_OFFSET) * TRAIL_PROBE_REACH;
                        ahead.z = TRAIL_LANDING_LIFT;
                        if (DAT_10db6ee8[0].IsClearOfMapLimited(&ahead) && DAT_10db6ee8[0].IsClear(&ahead))
                        {
                            blendB.Copy(*from + ahead);
                            PlacePlayer(blendB.x, blendB.y, blendB.z);
                        }
                        DAT_104687b0 = 1;
                        DAT_104c0228 = TRAIL_RESUME_PHASE;
                    }
                    if (!blocked)
                    {
                        DAT_10db6ee8[0].PlayMotion(2, 0, 5, 1.0f);
                        ResetPlayerTrail();
                    }
                    goto trailDone;
                }
            }
            else
            {
                DAT_104c022d = 0;
                DAT_10db6ee8[0].m_006c = 0;
                DAT_104c0228 = 0;
            }
            if (DAT_10db6ee8[0].m_238c == 0 || DAT_104687b0 == 0)
            {
                // On solid ground: probe for walls, keep the trail, fall back to it after a drop
                if (DAT_104c00d0 == 0 && DAT_104bfe30 == 0 && DAT_104c0300 == 0 && DAT_104bfe50 == 0 && DAT_104bfe4c == 0 &&
                    DAT_1032d69c != 0 && DAT_104c022d == 0)
                {
                    float height = TRAIL_PROBE_LOW;
                    if ((DAT_1020d430 == 0 && DAT_10db6ee8[0].m_229c == 0) ||
                        DAT_10db6ee8[0].matrix._43 < DAT_10802bd8.z - TRAIL_PROBE_MARGIN)
                        height = TRAIL_PROBE_HIGH;
                    Vec4 probe;
                    Vec4 probeLow;
                    bool refilled = false;
                    if (TrailBlockedAhead(probe, probeLow, -DAT_10db6ee8[DAT_104bf8b4].heading - TRAIL_SIDE_OFFSET, height))
                        refilled = true;
                    else if (DAT_104c0228 != 0 &&
                             TrailBlockedAhead(probe, probeLow,
                                               TRAIL_SIDE_ANGLE - (DAT_10db6ee8[DAT_104bf8b4].heading + TRAIL_SIDE_OFFSET), height))
                        refilled = true;
                    else if (DAT_104c0228 != 0 &&
                             TrailBlockedAhead(probe, probeLow,
                                               (-DAT_10db6ee8[DAT_104bf8b4].heading - TRAIL_SIDE_OFFSET) - TRAIL_SIDE_ANGLE, height))
                        refilled = true;
                    if (refilled)
                        DAT_104c0228 = 0;
                    if (refilled || DAT_104c0228 < TRAIL_PHASE_LIMIT)
                        DAT_104c0228 = DAT_104c0228 + 1;
                    if (DAT_104c022d == 0)
                    {
                        // Standing clearly below the trail: slide back up to it
                        if (DAT_104c0228 < TRAIL_SLIDE_TICKS)
                        {
                            if (DAT_10db6ee8[0].matrix._43 < TrailBack(DAT_104c022c, 2).z - TRAIL_DROP_MIN)
                            {
                                if ((TrailBack(DAT_104c022c, 2) - DAT_10db6ee8[0].matrix.GetTranslation()).Length2D() < TRAIL_DROP_REACH)
                                {
                                    unsigned int newest = DAT_104c022c;
                                    Vec4 probeA;
                                    Vec4 probeB;
                                    probeA.Fill(0.0f);
                                    probeB.Fill(0.0f);
                                    Vec4 savedPos = DAT_10db6ee8[0].matrix.GetTranslation();
                                    Vec4 *from = &TrailBack(newest, 4);
                                    DAT_10db6ee8[0].matrix.SetTranslation(*from);
                                    if (TrailSpotBlocked(probeA, probeB))
                                    {
                                        Vec4 focus = *from;
                                        focus.z = focus.z - TRAIL_FOCUS_DROP;
                                        Vec4 blendA;
                                        Vec4 blendB;
                                        TrailBack(newest, 3).Copy(savedPos);
                                        for (int glide = 0; glide < TRAIL_GLIDE_STEPS; glide++)
                                        {
                                            int weightB = glide < TRAIL_BLEND_STEPS + 1 ? glide : TRAIL_BLEND_STEPS;
                                            int weightA = glide < TRAIL_BLEND_STEPS ? glide : TRAIL_BLEND_STEPS - 1;
                                            BlendTrailPoint(blendA, savedPos, focus, weightA);
                                            BlendTrailPoint(blendB, savedPos, focus, weightB);
                                            if ((focus - blendA).Length2D() < TRAIL_GLIDE_NEAR)
                                                blendA.Copy(TrailBack(newest, 3));
                                            TrailBack(newest, 3).Copy(blendA);
                                            TrailBack(newest, 5).Copy(blendB);
                                        }
                                        DAT_10db6ee8[0].matrix._41 = blendA.x;
                                        DAT_10db6ee8[0].matrix._43 = blendB.z;
                                        DAT_10db6ee8[0].matrix._42 = blendA.y;
                                        probeA.w = 0.0f;
                                        if (DAT_10db6ee8[0].IsClearOfMapLimited(&probeA) && DAT_10db6ee8[0].IsClear(&probeA))
                                        {
                                            DAT_10db6ee8[0].matrix.SetTranslation(savedPos);
                                            DAT_104c022d = TRAIL_SLIDE_HOLD;
                                            DAT_10db6ee8[0].m_006c = 1;
                                        }
                                    }
                                    DAT_10db6ee8[0].matrix.SetTranslation(savedPos);
                                }
                            }
                        }
                        else
                            DAT_10db6ee8[0].m_006c = 0;
                    }
                    else
                    {
                        DAT_10db6ee8[0].m_006c = 1;
                        if ((TrailBack(DAT_104c022c, 2) - DAT_10db6ee8[0].matrix.GetTranslation()).Length2D() > TRAIL_DROP_REACH ||
                            DAT_104bfe50 != 0 || DAT_104c0300 != 0 || DAT_104bfe4c != 0 || DAT_104bfe30 != 0 || DAT_104c00d0 != 0)
                        {
                            DAT_104c022d = 0;
                            DAT_10db6ee8[0].m_006c = 0;
                            DAT_104c0228 = 0;
                        }
                        else if (DAT_10db6ee8[0].matrix._43 < TrailBack(DAT_104c022c, 2).z - TRAIL_DROP_FALL)
                        {
                            TrailBack(DAT_104c022c, 1).Copy(DAT_10db6ee8[0].matrix.GetTranslation());
                            DAT_104c0228 = TRAIL_REPLAY_BASE;
                            FUN_100114a0();
                            DAT_104c022d = 0;
                            if (DAT_104bfdc8 != 0)
                            {
                                DAT_10db6ee8[DAT_104bfdd8].m_0075 = 0;
                                DAT_10db6ee8[0].m_0075 = 0;
                                DAT_104bfdc8 = 0;
                                DAT_104bfdcc = 0;
                                DAT_104c0090 = 1;
                            }
                        }
                    }
                }
            }
            else
            {
                // Airborne or riding: the trail follows the player, one position per tick
                if (DAT_104c0228 != 0)
                {
                    for (int step = PLAYER_TRAIL_LENGTH; step != 0; step--)
                    {
                        DAT_10802ad8[DAT_104c022c].Copy(DAT_10db6ee8[0].matrix.GetTranslation());
                        DAT_104c022c = DAT_104c022c + 1 & TRAIL_MASK;
                    }
                }
                DAT_104c0228 = 0;
                DAT_10802ad8[DAT_104c022c].Copy(DAT_10db6ee8[0].matrix.GetTranslation());
                DAT_104c022c = DAT_104c022c + 1 & TRAIL_MASK;
                DAT_10db6ee8[0].m_006c = 0;
                if (DAT_104c022d != 0)
                    DAT_104c022d = DAT_104c022d - 1;
            }
        }
trailDone:
        // Helmet regeneration: health and stamina come back by themselves while nothing else is going on
        if (DAT_1032d69c == 0 || DAT_104bfe1c != 0 || DAT_104c00d0 > 1 || FUN_1015d510() != 0 || DAT_104c0135 != 0 ||
            DAT_104bfe78 != 0 || DAT_104bfe6c != 0 || DAT_104bfd08 != 0 || DAT_104bfd60 != 0 ||
            DAT_10db6ee8[DAT_104bf8b4].M_1017b160() != 0)
        {
            DAT_1101d87c = 1;
        }
        else
        {
            unsigned int helmetSettings = DAT_104c02c8;
            unsigned int frame = DAT_104bf890;
            DAT_1101d87c = 0;
            if (frame % REGEN_HEALTH_FAST_TICKS == 0 && (helmetSettings & HELMET_REGEN_HEALTH_FAST) != 0)
            {
                int health = DAT_10daaae8[0].stats[STAT_HEALTH] + 1;
                if (DAT_10daaae8[0].stats[STAT_MAX_HEALTH] < health)
                    health = DAT_10daaae8[0].stats[STAT_MAX_HEALTH];
                DAT_104c011d = HEALTH_FLASH_TICKS;
                DAT_10daaae8[0].stats[STAT_HEALTH] = health;
                FUN_1000e750();
            }
            else if (steerX == 0.0f && steerY == 0.0f && frame % REGEN_HEALTH_SLOW_TICKS == 0 &&
                     (helmetSettings & HELMET_REGEN_HEALTH_SLOW) != 0)
            {
                int health = DAT_10daaae8[0].stats[STAT_HEALTH] + 1;
                if (DAT_10daaae8[0].stats[STAT_MAX_HEALTH] < health)
                    health = DAT_10daaae8[0].stats[STAT_MAX_HEALTH];
                if (DAT_10daaae8[0].stats[STAT_HEALTH] != health)
                    DAT_104c011d = HEALTH_FLASH_TICKS;
                DAT_10daaae8[0].stats[STAT_HEALTH] = health;
                FUN_1000e750();
            }
            if ((frame % REGEN_STAMINA_FAST_TICKS == 0 && (helmetSettings & HELMET_REGEN_STAMINA_FAST) != 0) ||
                (steerX == 0.0f && steerY == 0.0f && frame % REGEN_STAMINA_SLOW_TICKS == 0 &&
                 (helmetSettings & HELMET_REGEN_STAMINA_SLOW) != 0))
            {
                DAT_10c64ad8 = DAT_10c64ad8 + 1;
                FUN_1000e750();
            }
        }
        DAT_10802bd8.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
        // Held-item timer: when it runs out the player goes back to the idle pose
        if (DAT_104bfdf0 != 0 && DAT_104bfdf0 < HELD_ITEM_TIMER_LIMIT)
        {
            DAT_10db6ee8[0].motionFrame = DAT_10db6ee8[0].motionFrame + 1;
            DAT_104bfdf0 = DAT_104bfdf0 - 1;
            if (DAT_104bfdf0 == 0 && DAT_104c0319 == 0)
            {
                DAT_104c0319 = 0;
                FUN_100121e0(0);
                DAT_10db6ee8[0].m_0294 = 0;
                DAT_10db6ee8[0].motionFrame = 0;
                DAT_10db6ee8[0].PlayMotion(2, 0, 5, 1.0f);
                FUN_1013faf0();
            }
        }
        // How long ago the stick last pushed each way (counts down from 20)
        if (steerX < -STICK_FLICK)
            DAT_104c0158[0] = STICK_FLICK_TICKS;
        if (STICK_FLICK < steerX)
            DAT_104c0158[1] = STICK_FLICK_TICKS;
        if (steerY < -STICK_FLICK)
            DAT_104c0158[2] = STICK_FLICK_TICKS;
        if (STICK_FLICK < steerY)
            DAT_104c0158[3] = STICK_FLICK_TICKS;
        if (DAT_104c0158[0] != 0)
            DAT_104c0158[0] = DAT_104c0158[0] - 1;
        if (DAT_104c0158[1] != 0)
            DAT_104c0158[1] = DAT_104c0158[1] - 1;
        if (DAT_104c0158[2] != 0)
            DAT_104c0158[2] = DAT_104c0158[2] - 1;
        if (DAT_104c0158[3] != 0)
            DAT_104c0158[3] = DAT_104c0158[3] - 1;
        DAT_10db6ee8[0].m_0d44 = 0;
        // Forced walk (a script moves the player with DAT_102559f0 / DAT_1042d9e4)
        if (DAT_104bfd0c != 0)
        {
            DAT_10db6ee8[0].motion->m_0008 = 1.0f;
            if (DAT_104bfe6c == 0)
            {
                steerX = (float)DAT_102559f0;
                steerY = (float)DAT_1042d9e4;
                if (fabs((double)DAT_102559f0) > FORCED_RUN_SPEED || fabs((double)steerY) > FORCED_RUN_SPEED)
                {
                    DAT_10db6ee8[DAT_104bf8b4].PlayMotion(1, 0, 5, 1.0f);
                    moveInput = 1;
                }
                else
                    DAT_10db6ee8[DAT_104bf8b4].PlayMotion(0, 0, 5, 1.0f);
                DAT_104bfd0c = DAT_104bfd0c - 1;
                if (DAT_104bfd0c == 0)
                {
                    DAT_10db6ee8[DAT_104bf8b4].PlayMotion(2, 0, 5, 1.0f);
                    DAT_104bfd10 = 0;
                    steerX = 0.0f;
                    steerY = 0.0f;
                }
            }
            else
                DAT_104bfd0c = 0;
            DAT_102027ec = FORCED_WALK_SPEED_SCALE;
            if (DAT_104c0024 > 0)
            {
                DAT_102027ec = FORCED_WALK_SPEED_SCALE;
                DAT_10db6ee8[DAT_104bf8b4].Push(steerX, steerY, 0.0f);
                steerX = 0.0f;
                steerY = 0.0f;
                moveInput = 0;
                DAT_10db6ee8[0].motion->m_0008 = DAT_102027ec / FORCED_WALK_SPEED_SCALE;
            }
        }
        DAT_10203430 = DAT_104bfe1c == 0;
        // Frozen water over the pit: stepping onto the ice inside the pit area arms DAT_104c01d8
        if (DAT_104c02dc != 0 && DAT_104bfe24 - PIT_ICE_BELOW < DAT_10db6ee8[0].matrix._43 &&
            DAT_10db6ee8[0].matrix._43 < DAT_104bfe24 + PIT_ICE_ABOVE)
        {
            int y = (int)DAT_10db6ee8[0].matrix._42;
            int x = (int)DAT_10db6ee8[0].matrix._41;
            if (x >= (int)DAT_104d29e8.x && y >= (int)DAT_104d29e8.y && x <= (int)DAT_104d29f8.x && y <= (int)DAT_104d29f8.y)
                DAT_104c01d8 = 3;
        }
        if (DAT_104687b4 < ACTION_GAUGE_MAX)
            DAT_104687b4 = DAT_104687b4 + 1;
        // Standing in the pit area (the pit's floor rises with DAT_104bfe50)
        if (DAT_104bfe28 == 0)
            DAT_104bfe50 = 0;
        else
        {
            Vec4 playerPos;
            playerPos.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
            if (DAT_104bfe24 - PIT_FLOOR_MARGIN + (float)(DAT_104bfe50 * PIT_STEP) <= playerPos.z || DAT_104c0228 >= TRAIL_REPLAY_BASE)
                DAT_104bfe50 = 0;
            else
            {
                int y = (int)playerPos.y;
                int x = (int)playerPos.x;
                if (x >= (int)DAT_104d29e8.x && y >= (int)DAT_104d29e8.y && x <= (int)DAT_104d29f8.x && y <= (int)DAT_104d29f8.y)
                    DAT_104bfe50 = 1;
                else
                    DAT_104bfe50 = 0;
            }
        }
        // The swimming arm's speed scale follows the pit state
        if (DAT_10db6ee8[0].root != 0)
        {
            if ((DAT_104bfe50 != 0 || DAT_104c0300 != 0) && DAT_10db6ee8[0].root->children[1]->m_0298.y < ARM_SPEED_MAX)
            {
                DAT_10db6ee8[0].root->children[1]->m_0298.y = DAT_10db6ee8[0].root->children[1]->m_0298.y + ARM_SPEED_STEP;
                if (ARM_SPEED_MAX < DAT_10db6ee8[0].root->children[1]->m_0298.y)
                    DAT_10db6ee8[0].root->children[1]->m_0298.y = ARM_SPEED_MAX;
            }
            if (DAT_10db6ee8[0].root != 0 && DAT_104bfe50 == 0 && DAT_104c0300 == 0 &&
                0.0f < DAT_10db6ee8[0].root->children[1]->m_0298.y)
            {
                DAT_10db6ee8[0].root->children[1]->m_0298.y = DAT_10db6ee8[0].root->children[1]->m_0298.y - ARM_SPEED_STEP;
                if (DAT_10db6ee8[0].root->children[1]->m_0298.y < 0.0f)
                    DAT_10db6ee8[0].root->children[1]->m_0298.y = 0.0f;
            }
        }
        // Pit effects, every other frame: ripples and splashes where the characters touch the surface
        if (DAT_104bfe28 != 0 && (DAT_104bf890 & 1) == 0)
        {
            if (DAT_104c0268 != 0)
            {
                EffectOptions options;
                options.list = 1;
                options.flags = 1;
                DAT_10daaa80.Start(EFFECT_SWAN_WATER, DAT_104c0264->worldMatrix.GetTranslation(), -1, 0, 0, 0, 0, 0, &options, 0);
            }
            for (int pitModel = 0; pitModel < PIT_MODEL_COUNT; pitModel++)
            {
                if ((pitModel == 0 || DAT_1021f3c0[pitModel] == 3) && DAT_10daaae8[pitModel].stats[STAT_PIT_SKIP] == 0
                    && DAT_10db6ee8[pitModel].root != 0
                    && DAT_10db6ee8[pitModel].matrix.m[MATRIX_TRANSLATION_X] > PIT_MODEL_GONE_X)
                {
                    // The point where the model's foot (alternating toe nodes) touches the water
                    Vec4 surfaceAt;
                    Node *toe = DAT_10db6ee8[pitModel].toeNodes[(DAT_104bf890 >> 1) & 1];
                    surfaceAt.Copy(toe != 0 ? toe->worldMatrix.GetTranslation()
                                            : DAT_10db6ee8[pitModel].matrix.GetTranslation());
                    if (surfaceAt.z < DAT_104bfe24 + PIT_SURFACE_MARGIN)
                    {
                        int surfaceY = (int)surfaceAt.y;
                        int surfaceX = (int)surfaceAt.x;
                        if (surfaceX >= (int)DAT_104d29e8.x && surfaceY >= (int)DAT_104d29e8.y
                            && surfaceX <= (int)DAT_104d29f8.x && surfaceY <= (int)DAT_104d29f8.y)
                        {
                            surfaceAt.Copy(surfaceAt);
                            surfaceAt.z = DAT_104bfe24 + PIT_RIPPLE_HEIGHT;
                            Vec4 bodyAt;
                            bodyAt.Copy(DAT_10db6ee8[pitModel].matrix.GetTranslation());
                            if (pitModel == 0)
                            {
                                // The player: the splash at the feet (or the head when swimming) and the wake
                                DAT_104687b4 = 0;
                                if (DAT_104bfe4c != 0)
                                {
                                    Node *head = DAT_10db6ee8[0].headNode;
                                    surfaceAt.Copy(head != 0 ? head->worldMatrix.GetTranslation()
                                                             : DAT_10db6ee8[0].matrix.GetTranslation());
                                    if ((DAT_104bf890 & 6) == 0 && surfaceAt.z < DAT_104bfe24 + PIT_CONTACT_HEIGHT)
                                    {
                                        Vec4 splashAt;
                                        Node *neck = DAT_10db6ee8[0].headNode->parent;
                                        splashAt.Copy(neck != 0 ? neck->worldMatrix.GetTranslation()
                                                                : DAT_10db6ee8[0].matrix.GetTranslation());
                                        splashAt.z = splashAt.z - SPLASH_DROP;
                                        splashAt.x = (float)cos(-DAT_10db6ee8[DAT_104bf8b4].targetHeading - HALF_PI)
                                                         * SPLASH_REACH
                                                     + splashAt.x;
                                        splashAt.y = (float)sin(-DAT_10db6ee8[DAT_104bf8b4].targetHeading - HALF_PI)
                                                         * SPLASH_REACH
                                                     + splashAt.y;
                                        EffectOptions options;
                                        options.list = 1;
                                        options.flags = 1;
                                        DAT_10daaa80.Start(EFFECT_PIT_SPLASH, splashAt, -1, 0, 0, 0, ((DAT_104bf890 >> 1) % 3) * -2,
                                                           0, &options, 0);
                                        // Standing in the water this deep hurts (or heals) the player
                                        if (surfaceAt.z < DAT_104bfe24 + PIT_CONTACT_HEIGHT - SPLASH_DROP && DAT_104686b0[0] == 0
                                            && DAT_10253e98[0] == 0 && DAT_104bfe1c == 0)
                                        {
                                            if (DAT_10c655f0 == PIT_COSTUME_SAFE
                                                || ((DAT_104c02c8 & 3) != 3 && (DAT_104c02c8 & 2) != 0))
                                            {
                                                FUN_10011aa0(0, 1);
                                                DAT_10253e98[0] = PIT_PROTECT_FRAMES;
                                            }
                                            else if ((DAT_104c02c8 & 3) == 3)
                                            {
                                                FUN_10011aa0(0, 1);
                                                DAT_10253e98[0] = PIT_PROTECT_FRAMES;
                                                int health = DAT_10daaae8[0].stats[STAT_HEALTH] + 1;
                                                if (DAT_10daaae8[0].stats[STAT_MAX_HEALTH] < health)
                                                    health = DAT_10daaae8[0].stats[STAT_MAX_HEALTH];
                                                if (DAT_10daaae8[0].stats[STAT_HEALTH] != health)
                                                    DAT_104c011d = PIT_HEAL_FLASH;
                                                DAT_10daaae8[0].stats[STAT_HEALTH] = health;
                                                FUN_1000e750();
                                            }
                                            else
                                                FUN_1005dbf0((DAT_104c02c8 & 1) == 0 ? 4 : 2);
                                        }
                                    }
                                    surfaceAt.z = DAT_104bfe24 + PIT_RIPPLE_HEIGHT;
                                }
                                if (DAT_104bfe24 - PIT_WAKE_RANGE < bodyAt.z)
                                {
                                    if (DAT_104c02dc == 0)
                                    {
                                        EffectOptions options;
                                        options.flags = 1;
                                        options.angle = DAT_10db6ee8[DAT_104bf8b4].targetHeading + HALF_PI;
                                        DAT_10daaa80.Start(EFFECT_PIT_RIPPLE, surfaceAt, -1, 0, 0, 0,
                                                           ((DAT_104bf890 >> 1) % 3) * -2, 0, &options, 0);
                                    }
                                    if (steerX != 0.0f || steerY != 0.0f)
                                    {
                                        if ((DAT_104bf890 & 6) == 0)
                                        {
                                            Vec4 wakeAt;
                                            wakeAt.Copy(surfaceAt);
                                            wakeAt.z = DAT_104bfe24 + PIT_CONTACT_HEIGHT;
                                            wakeAt.x = (float)cos(-DAT_10db6ee8[DAT_104bf8b4].targetHeading - HALF_PI)
                                                           * WAKE_REACH
                                                       + wakeAt.x;
                                            wakeAt.y = (float)sin(-DAT_10db6ee8[DAT_104bf8b4].targetHeading - HALF_PI)
                                                           * WAKE_REACH
                                                       + wakeAt.y;
                                            EffectOptions options;
                                            options.flags = 1;
                                            DAT_10daaa80.Start(EFFECT_PIT_WAKE, wakeAt, -1, -1, 0, 0, 0, 0, &options, 0);
                                        }
                                        if (DAT_104c01d8 == 0)
                                        {
                                            if (DAT_104bfe4c == 0)
                                            {
                                                if ((DAT_104bf890 & 6) == 0)
                                                    FUN_1003ad20(SOUND_PIT_WALK, &DAT_10db6ee8[0].matrix.GetTranslation(), 1.0f);
                                            }
                                            else if ((DAT_104bf890 >> 1) % SOUND_PIT_SWIM_PERIOD == 0)
                                                FUN_1003ad20(SOUND_PIT_SWIM, &DAT_10db6ee8[0].matrix.GetTranslation(), 1.0f);
                                        }
                                    }
                                }
                            }
                            else if (DAT_104bfe24 - PIT_WAKE_RANGE < bodyAt.z)
                            {
                                // Other models: a ripple at the surface
                                EffectOptions options;
                                options.flags = 1;
                                DAT_10daaa80.Start(EFFECT_PIT_RIPPLE, surfaceAt, -1, 0, 0, 0, ((DAT_104bf890 >> 1) % 3) * -2, 0,
                                                   &options, 0);
                            }
                        }
                    }
                }
            }
        }
        // Item-menu pad state and the screen shift of the overlay mode
        if (DAT_104c009a == 0)
        {
            if (DAT_104c0099 != 0)
                DAT_104c0099 = DAT_104c0099 - 1;
        }
        else if (DAT_104c0099 < ITEM_PAD_MAX)
            DAT_104c0099 = DAT_104c0099 + 1;
        if (DAT_104bfe1c == 0)
        {
            if (0 < DAT_104bfe18)
                DAT_104bfe18 = DAT_104bfe18 - 1;
            DAT_10f654b4 = -(DAT_10202b24 * DAT_104bfe18 / OVERLAY_SHIFT_DIVISOR);
            DAT_10f654b8 = -(DAT_10202b28 * DAT_104bfe18 / OVERLAY_SHIFT_DIVISOR);
            if (DAT_104bfd74 == 4)
                DAT_104c009a = 1;
            else
            {
                DAT_104c009a = 0;
                DAT_104c0099 = 0;
            }
        }
        else
        {
            if (DAT_104bfe18 < OVERLAY_SHIFT_MAX)
                DAT_104bfe18 = DAT_104bfe18 + 1;
            DAT_10f654b4 = -(DAT_10202b24 * DAT_104bfe18 / OVERLAY_SHIFT_DIVISOR);
            DAT_10f654b8 = -(DAT_10202b28 * DAT_104bfe18 / OVERLAY_SHIFT_DIVISOR);
            FUN_1000d620(0, 0, 10);
            DAT_104c009a = 1;
        }
        // Swimming in the water of the pit: the "@yotsu" node is hidden for a moment each time the player surfaces
        if (DAT_104c0300 != 0 && DAT_104c0228 < TRAIL_REPLAY_BASE)
        {
            if (DAT_104c0301 < YOTSU_HIDE_MAX)
                DAT_104c0301 = DAT_104c0301 + 1;
            if (DAT_104c0301 > YOTSU_HIDE_MIN && DAT_104c0304 != 0)
                DAT_104c0304->flags = DAT_104c0304->flags & ~YOTSU_NODE_FLAG;
            if (DAT_104c0300 != 0 && (DAT_104c0300 = DAT_104c0300 - 1, DAT_104c0300 == 0))
            {
                DAT_10db6ee8[0].PlayMotion(MOTION_SURFACE, 1, 5, 1.0f);
                DAT_104c0301 = 0;
                if (DAT_104c0304 != 0)
                    DAT_104c0304->flags = DAT_104c0304->flags | YOTSU_NODE_FLAG;
            }
        }
        if (DAT_104bfe50 == 0 || DAT_104bfe78 != 0 || DAT_104c0228 > TRAIL_REPLAY_BASE - 1)
        {
            // Out of the pit
            if (DAT_104c00e0 == 0 && DAT_10db6ee8[DAT_104bf8b4].mode == 2)
                DAT_10db6ee8[DAT_104bf8b4].mode = 1;
            if (DAT_104bfe4c != 0)
            {
                DAT_1021a360 = 0;
                DAT_104687b0 = 1;
                DAT_10db6ee8[0].m_237c = PIT_EXIT_SPEED;
                DAT_104bfe4c = 0;
                FUN_1000f360(0, 1);
            }
        }
        else
        {
            // In the pit: the first tick splashes, then the player is pushed down by the fall speed
            if (DAT_104bfe4c == 0)
            {
                if (DAT_10db6ee8[0].m_237c <= PIT_SPLASH_MIN_SPEED)
                    DAT_104bfe54 = PIT_SINK_TICKS;
                else
                {
                    Vec4 splash;
                    splash.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
                    splash.z = DAT_104bfe24 + PIT_SPLASH_LIFT;
                    splash.x = (float)cos(-DAT_10db6ee8[DAT_104bf8b4].targetHeading - TRAIL_SIDE_OFFSET) * PIT_SPLASH_REACH + splash.x;
                    splash.y = (float)sin(-DAT_10db6ee8[DAT_104bf8b4].targetHeading - TRAIL_SIDE_OFFSET) * PIT_SPLASH_REACH + splash.y;
                    EffectOptions options;
                    options.flags = 1;
                    DAT_10daaa80.Start(PIT_SPLASH_EFFECT, splash, -1, -1, 0, 0, 0, 0, &options, 0);
                    Vec4 soundAt = DAT_10db6ee8[0].matrix.GetTranslation();
                    FUN_1003ad20(PIT_SPLASH_SOUND, &soundAt, 1.0f);
                    DAT_104bfe58 = DAT_10db6ee8[0].m_237c * PIT_SINK_SPEED_SCALE;
                    DAT_104bfe54 = 0;
                }
                if (DAT_104c00e0 != 0)
                {
                    DAT_104c00e0 = 0;
                    DAT_10db6ee8[0].mode = 1;
                }
                DAT_10db6ee8[0].alpha = 1.0f;
                FUN_1000f360(0, 1);
                DAT_10db6ee8[0].PlayMotion(MOTION_PIT_SWIM, 0, 5, 1.0f);
                DAT_104bfe4c = 1;
            }
            if (DAT_104bfe54 < PIT_SINK_TICKS)
            {
                DAT_104bfe54 = DAT_104bfe54 + 1;
                DAT_10db6ee8[0].m_238c = 1;
                DAT_10db6ee8[DAT_104bf8b4].Push(0.0f, 0.0f, -DAT_104bfe58);
                DAT_10db6ee8[0].m_238c = 0;
                if (DAT_104bfe58 < -1.0f)
                    DAT_104bfe58 = DAT_104bfe58 + 1.0f;
            }
            if (DAT_10db6ee8[DAT_104bf8b4].m_229c == 0 && DAT_10db6ee8[DAT_104bf8b4].mode == 1)
                DAT_10db6ee8[DAT_104bf8b4].mode = 2;
            DAT_1021a360 = 0;
        }
        // The camera swing: the camera node's animation frames (DAT_104c01dc) play forwards (DAT_104c01e0 > 0) or
        // backwards while the player model, and the truck node, ride its matrices
        if (DAT_104c01e0 != 0)
        {
            CloseEventOverlay();
            // A rolling rock (the "IWA" matrices) that reaches the player hurts and bursts
            for (int rock = 0; rock < DAT_104c0218; rock++)
            {
                if (DAT_10467208[rock] != 0
                    && DAT_10467208[rock]->GetTranslation().Distance(DAT_10db6ee8[0].matrix.GetTranslation())
                           < SWING_ROCK_REACH)
                {
                    DAT_10467208[rock] = 0;
                    DAT_10daaa80.Start(EFFECT_ROCK_BURST, DAT_10db6ee8[0].matrix.GetTranslation(), -1, -1, 0, 0, 0, 0, 0, 0);
                    FUN_1005dbf0(SWING_ROCK_DAMAGE);
                    break;
                }
            }
            DAT_104c01dc->frameSource = &DAT_104c01e4;
            DAT_10db6ee8[0].SetPosition(0, 0, 0);
            Matrix4 unusedMatrix;
            Matrix4 swingFrame = DAT_104c01dc->frameMatrices[DAT_104c01e4];
            DAT_104c0214 = DAT_104c0214 + SWING_DROP_ACCEL;
            DAT_104c0210 = DAT_104c0210 - DAT_104c0214;
            if (DAT_104c0210 <= 0.0f)
            {
                DAT_104c0214 = 0.0f;
                DAT_104c0210 = 0.0f;
            }
            if (DAT_104c01e0 > 0)
            {
                // Forwards: the player, turned round, and the truck follow the frame, lowered by the drop
                DAT_10db6ee8[0].matrix =
                    (TranslationMatrix(0, 0, DAT_104c0210 * SWING_PLAYER_LIFT + SWING_PLAYER_HEIGHT) * RotationZ(SWING_TURN_ROUND))
                    * swingFrame;
                if (DAT_104c021c != 0)
                    DAT_104c021c->matrix = (TranslationMatrix(0, 0, DAT_104c0210 * SWING_TRUCK_LIFT) * RotationZ(0)) * swingFrame;
                if (DAT_104bfe1c == 0 && DAT_1032d69c != 0 && DAT_104c0135 == 0)
                    DAT_104c01e4 = DAT_104c01e4 + 1;
                if (DAT_104c01e4 >= DAT_104c01dc->frameMatrixCount)
                {
                    DAT_104c01e0 = 0;
                    DAT_10db6ee8[0].m_0076 = 0;
                    DAT_11018734 = 1;
                    DAT_1101de30 = 1;
                }
                if (DAT_104c0210 == 0.0f)
                {
                    // Landed: the two dust bursts, a little ahead of the frame two steps on
                    Matrix4 burst;
                    int burstFrame = DAT_104c01e4 + SWING_BURST_LEAD < DAT_104c01dc->frameMatrixCount
                                         ? DAT_104c01e4 + SWING_BURST_LEAD
                                         : DAT_104c01dc->frameMatrixCount;
                    burst = DAT_104c01dc->frameMatrices[burstFrame];
                    burst = (TranslationMatrix(SWING_BURST_SIDE, 0, DAT_104c0210 + SWING_BURST_HEIGHT) * RotationZ(0)) * burst;
                    Vec4 burstAt;
                    burstAt.Fill(0.0f);
                    DAT_10daaa80.Start(EFFECT_SWING_LANDING, burstAt, -1, -1, 0, 0, 0, &burst, 0, 0);
                    burstFrame = DAT_104c01e4 + SWING_BURST_LEAD < DAT_104c01dc->frameMatrixCount
                                     ? DAT_104c01e4 + SWING_BURST_LEAD
                                     : DAT_104c01dc->frameMatrixCount;
                    burst = DAT_104c01dc->frameMatrices[burstFrame];
                    burst = (TranslationMatrix(-SWING_BURST_SIDE, 0, DAT_104c0210 + SWING_BURST_HEIGHT) * RotationZ(0)) * burst;
                    DAT_10daaa80.Start(EFFECT_SWING_LANDING, burstAt, -1, -1, 0, 0, 0, &burst, 0, 0);
                }
            }
            else
            {
                // Backwards: the same, facing the other way and stepping the frame down
                DAT_10db6ee8[0].matrix =
                    (TranslationMatrix(0, 0, DAT_104c0210 * SWING_PLAYER_LIFT + SWING_PLAYER_HEIGHT) * RotationZ(0)) * swingFrame;
                if (DAT_104c021c != 0)
                    DAT_104c021c->matrix = (TranslationMatrix(0, 0, DAT_104c0210 * SWING_TRUCK_LIFT) * RotationZ(0)) * swingFrame;
                if (DAT_104bfe1c == 0 && DAT_1032d69c != 0 && DAT_104c0135 == 0)
                    DAT_104c01e4 = DAT_104c01e4 - 1;
                if (DAT_104c01e4 < 1)
                {
                    DAT_104c01e0 = 0;
                    DAT_10db6ee8[0].m_0076 = 0;
                    DAT_11018734 = 1;
                    DAT_1101de30 = 1;
                }
                if (DAT_104c0210 == 0.0f)
                {
                    Matrix4 burst;
                    burst = DAT_104c01dc->frameMatrices[DAT_104c01e4 - SWING_BURST_LEAD > 0 ? DAT_104c01e4 - SWING_BURST_LEAD : 0];
                    burst = (TranslationMatrix(SWING_BURST_SIDE, 0, DAT_104c0210 + SWING_BURST_HEIGHT) * RotationZ(0)) * burst;
                    Vec4 burstAt;
                    burstAt.Fill(0.0f);
                    DAT_10daaa80.Start(EFFECT_SWING_LANDING, burstAt, -1, -1, 0, 0, 0, &burst, 0, 0);
                    burst = DAT_104c01dc->frameMatrices[DAT_104c01e4 - SWING_BURST_LEAD > 0 ? DAT_104c01e4 - SWING_BURST_LEAD : 0];
                    burst = (TranslationMatrix(-SWING_BURST_SIDE, 0, DAT_104c0210 + SWING_BURST_HEIGHT) * RotationZ(0)) * burst;
                    DAT_10daaa80.Start(EFFECT_SWING_LANDING, burstAt, -1, -1, 0, 0, 0, &burst, 0, 0);
                }
            }
        }
        // Hit-stop counters of the walking gesture (DAT_104c0330 / DAT_104c0331 run out and reset)
        if (DAT_104c00e0 == 0 &&
            ((DAT_104c0330 > 2 && (DAT_104c0330 = DAT_104c0330 + 1, DAT_104c0330 > GESTURE_LONG_TICKS)) ||
             ((DAT_104c0330 == 1 || DAT_104c0330 == 2) && (DAT_104c0331 = DAT_104c0331 + 1, DAT_104c0331 > GESTURE_SHORT_TICKS))))
        {
            DAT_104c0330 = 0;
            DAT_104c0331 = 0;
        }
        if (DAT_104bfd0c == 0 && (steerX != 0.0f || steerY != 0.0f))
        {
            if (DAT_1101de30 != 2)
                DAT_1101de30 = 0;
            DAT_104c0090 = 0;
        }
        if (DAT_1101de30 == 2)
            DAT_1101de30 = 1;
        // Walking toward a carried-object spot: after enough ticks of facing it the player picks it up
        if (DAT_104bfdf0 == 0)
        {
            if (DAT_104c00e0 == 0 && DAT_1032d69c != 0 && DAT_104bfdd0 != 0 && DAT_104bfdc8 == 0 && steerX == 0.0f &&
                steerY == 0.0f && DAT_10db6ee8[0].m_238c != 0 && DAT_104c0228 < TRAIL_REPLAY_BASE)
            {
                Vec4 toCarried = DAT_10db6ee8[DAT_104bfdd8].matrix.GetTranslation() - DAT_10db6ee8[0].matrix.GetTranslation();
                toCarried.Normalized3();
                Vec4 facing;
                facing.x = 0.0f;
                facing.y = -1.0f;
                facing.z = 0.0f;
                facing.RotateZ(-DAT_10db6ee8[0].targetHeading);
                if (Vec3_Dot(&facing, &toCarried) > PICKUP_FACING_DOT)
                    DAT_104c019c = DAT_104c019c + 1;
                if (DAT_104c0090 == 0 && DAT_104c019c > PICKUP_FACING_TICKS && DAT_104c00e0 == 0)
                {
                    DAT_104bfea9 = 0;
                    DAT_104bfdc8 = 2;
                    DAT_104bfdcc = 10;
                    DAT_10db6ee8[DAT_104bf8b4].PlayMotion(MOTION_PICKUP, 0, 5, 1.0f);
                }
            }
            else
                DAT_104c019c = 0;
        }
        // Timed event: DAT_104c02e4 counts down in ticks; a multiple of 100 raises event (value / 100 + 500)
        if (DAT_104c02e4 != 0 && DAT_104c0135 == 0 && DAT_104bfe1c == 0)
        {
            DAT_104c02e4 = DAT_104c02e4 - 1;
            if ((int)DAT_104c02e4 % 100 == 0)
            {
                DAT_10874848.RegisterEvent((int)DAT_104c02e4 / 100 + TIMED_EVENT_BASE);
                DAT_104c02e4 = 0;
            }
        }
        // Movement with nothing held in the hands: the carried object's push, the run, leap and swim states, and the
        // easing of the lean and the run speed.
        if (DAT_104bfdf0 == 0)
        {
            float moveX;
            float moveY;
            float inverseLength;
            if (DAT_104bfdcc == 0 && steerX == 0.0f && steerY == 0.0f && DAT_104bf8c4 == 0 && DAT_104bfe30 == 0)
                goto movementDone;
            if (DAT_104bfdcc != 0 || steerX != 0.0f || steerY != 0.0f || DAT_104bf8c4 != 0)
            {
                if (DAT_104bfe1c != 0)
                {
                    FUN_1003ad20(SOUND_ITEM_LIST_CLOSE, 0, 1.0f);
                    if (DAT_104bfe1c != 0)
                        CloseItemList();
                }
                if (DAT_102024b0 > -1 && DAT_104bfdf0 == 0)
                {
                    DAT_104bfd74 = DAT_102024b0;
                    DAT_1020249c = DAT_102024b0;
                }
                DAT_102024b0 = -1;
            }
            if (DAT_104bfe4c != 0 && DAT_10468128 == 0 && DAT_10db6ee8[0].m_229c == 0)
                DAT_10db6ee8[0].PlayMotion(MOTION_SWIM_START, 0, 5, 1.0f);
            if (DAT_104c0300 != 0 && DAT_10468128 == 0 && DAT_10db6ee8[0].m_229c == 0)
                DAT_10db6ee8[0].PlayMotion(MOTION_SLIDE, 0, DAT_104c0301 < SLIDE_HURRY_TICK ? 5 : 2, 1.0f);
            if (DAT_104bfd0c == 0)
                FUN_1016b400(&moveX, &moveY, steerX, steerY);
            else
            {
                moveX = steerX;
                moveY = steerY;
            }
            inverseLength = (float)(1.0 / sqrt((double)(moveX * moveX + moveY * moveY)));
            moveX = inverseLength * moveX * MOVE_STEP_LENGTH;
            moveY = inverseLength * moveY * MOVE_STEP_LENGTH;
            if (steerX == 0.0f && steerY == 0.0f)
            {
                // No steering: the carry action, the slide and the standing pose; every path ends at settleCheck.
                if (DAT_104bfe30 != 0)
                    goto leapStart;
                if (DAT_104bfdc8 == 0)
                {
                    if (DAT_104bfe4c != 0)
                        goto swimOrSlide;
                    if (DAT_104c0300 == 0)
                    {
                        DAT_10db6ee8[DAT_104bf8b4].PlayMotion(MOTION_STAND, 0, 5, 1.0f);
                        goto leapCheck;
                    }
                slideNoSteer:
                    DAT_10db6ee8[DAT_104bf8b4].Push(moveX * SLIDE_SCALE, moveY * SLIDE_SCALE, 0.0f);
                    RunSpeedFalloff();
                    if (LeanEasesBack(LEAN_EASE_FAST))
                    {
                        DAT_10866de8.Fill(0.0f);
                        DAT_10db6ee8[0].spinAngle = 0.0f;
                    }
                }
                else
                {
                carryAction:
                    if (DAT_104bfdcc != 0)
                    {
                        if (DAT_104bfddc == 0 && (DAT_104bfdd0 == 1 || DAT_104bfdd0 == 2))
                        {
                            // Choose the side the object is pushed to from the walk direction.
                            Vec4 playerToCarried;
                            Vec4 stepDirection;
                            playerToCarried.Copy(DAT_10db9210.GetTranslation() -
                                                 DAT_10db6ee8[DAT_104bfdd8].matrix.GetTranslation());
                            stepDirection.Set(moveX, moveY, 0.0f, 0.0f);
                            playerToCarried.Normalized3();
                            stepDirection.Normalized3();
                            if (DAT_104bfdcc > CARRY_DRAG_FRAME)
                            {
                                float side = Vec3_Dot(&playerToCarried, &stepDirection);
                                DAT_104bfddc = 1;
                                if (0.0f <= side)
                                {
                                    DAT_104bfdd4 = 0;
                                    DAT_10db6ee8[DAT_104bf8b4].PlayMotion(MOTION_CARRY_RIGHT, 0, CARRY_MOTION_BLEND, 1.0f);
                                }
                                else
                                {
                                    DAT_104bfdd4 = 1;
                                    DAT_10db6ee8[DAT_104bf8b4].PlayMotion(MOTION_CARRY_LEFT, 0, CARRY_MOTION_BLEND, 1.0f);
                                }
                            }
                            if (DAT_104bfdd0 == 1)
                            {
                                DAT_1020d648 = 0.0f;
                                DAT_1020aee8 = moveX <= 0.0f ? -CARRY_SIDE_PUSH : CARRY_SIDE_PUSH;
                            }
                            else
                            {
                                DAT_1020aee8 = 0.0f;
                                DAT_1020d648 = moveY <= 0.0f ? -CARRY_SIDE_PUSH : CARRY_SIDE_PUSH;
                            }
                        }
                        DAT_10db6ee8[DAT_104bfdd8].m_02a0 = 1;
                        DAT_10db6ee8[DAT_104bf8b4].m_02a0 = 1;
                        // Frames 0x1d..0x10: both models slide along the push direction.
                        if (DAT_104bfdcc >= CARRY_SHOVE_FIRST && DAT_104bfdcc < CARRY_PUSH_FRAME)
                        {
                            DAT_10db6ee8[DAT_104bfdd8].m_0075 = 1;
                            DAT_10db6ee8[0].m_0075 = 1;
                            if (DAT_104bfdd4 == 0)
                            {
                                DAT_10db6ee8[DAT_104bfdd8].SetMoveStep(
                                    DAT_1020aee8 * DAT_10202360[DAT_104bfdcc] * CARRY_SHOVE_SCALE,
                                    DAT_1020d648 * DAT_10202360[DAT_104bfdcc] * CARRY_SHOVE_SCALE, 0.0f);
                                DAT_10db6ee8[DAT_104bf8b4].SetMoveStep(
                                    DAT_1020aee8 * DAT_10202360[DAT_104bfdcc - 2] * CARRY_SHOVE_SCALE,
                                    DAT_1020d648 * DAT_10202360[DAT_104bfdcc - 2] * CARRY_SHOVE_SCALE, 0.0f);
                            }
                            else
                            {
                                DAT_10db6ee8[DAT_104bfdd8].SetMoveStep(
                                    DAT_1020aee8 * DAT_102023e0[DAT_104bfdcc - 2] * CARRY_SHOVE_SCALE,
                                    DAT_1020d648 * DAT_102023e0[DAT_104bfdcc - 2] * CARRY_SHOVE_SCALE, 0.0f);
                                DAT_10db6ee8[DAT_104bf8b4].SetMoveStep(
                                    DAT_1020aee8 * DAT_102023e0[DAT_104bfdcc] * CARRY_SHOVE_SCALE,
                                    DAT_1020d648 * DAT_102023e0[DAT_104bfdcc] * CARRY_SHOVE_SCALE, 0.0f);
                            }
                            DAT_10db6ee8[DAT_104bfdd8].velocity.Fill(0.0f);
                            DAT_10db6ee8[DAT_104bf8b4].velocity.Fill(0.0f);
                        }
                        // Frame 0x1e: test whether the player and the object both have room for the push, then commit.
                        if (DAT_104bfdcc == CARRY_PUSH_FRAME)
                        {
                            Vec4 pushStep;
                            int pushOrder[2];
                            int clearCount;
                            int mover;
                            if (DAT_1020aee8 > 0.0f)
                                pushStep.Set(CARRY_GRID, 0.0f, 0.0f, 0.0f);
                            if (DAT_1020aee8 < 0.0f)
                                pushStep.Set(-CARRY_GRID, 0.0f, 0.0f, 0.0f);
                            if (DAT_1020d648 > 0.0f)
                                pushStep.Set(0.0f, CARRY_GRID, 0.0f, 0.0f);
                            if (DAT_1020d648 < 0.0f)
                                pushStep.Set(0.0f, -CARRY_GRID, 0.0f, 0.0f);
                            if (DAT_104bfdd4 == 0)
                            {
                                pushOrder[1] = DAT_104bfdd8;
                                pushOrder[0] = 0;
                            }
                            else
                            {
                                pushOrder[0] = DAT_104bfdd8;
                                pushOrder[1] = 0;
                            }
                            Vec4 playerStart = DAT_10db9210.GetTranslation();
                            Vec4 carriedStart = DAT_10db6ee8[DAT_104bfdd8].matrix.GetTranslation();
                            Matrix4 unusedMatrix;
                            unusedMatrix.SetIdentity();
                            clearCount = 0;
                            for (i = 0; i < 2; i++)
                            {
                                Vec4 noOffset;
                                Vec4 liftOffset;
                                noOffset.Fill(0.0f);
                                liftOffset.Set(0.0f, 0.0f, CARRY_LIFT, 0.0f);
                                mover = pushOrder[i];
                                Vec4 moverAt = DAT_10db6ee8[mover].matrix.GetTranslation();
                                if (mover == 0)
                                    DAT_10db6ee8[mover].matrix.SetTranslation(
                                        moverAt + (pushStep * CARRY_GRID).Divided(CARRY_GRID));
                                else
                                {
                                    DAT_10db6ee8[mover].matrix.SetTranslation(moverAt + pushStep);
                                    DAT_10db6ee8[mover].SnapPosition(CARRY_GRID, 0.0f);
                                }
                                if ((i == 1 || DAT_10db6ee8[mover].IsClearAny(&noOffset) != 0 ||
                                     DAT_10db6ee8[mover].IsClearAny(&liftOffset) != 0) &&
                                    (DAT_10db6ee8[mover].IsClearOfMapLimited(&noOffset) != 0 ||
                                     DAT_10db6ee8[mover].IsClearOfMapLimited(&liftOffset) != 0))
                                    clearCount = clearCount + 1;
                                DAT_10db6ee8[mover].root->UpdateWorld(DAT_10db6ee8[mover].matrix,
                                                                      DAT_10db6ee8[mover].motionFrame);
                            }
                            DAT_10db9210.SetTranslation(playerStart);
                            DAT_10db6ee8[DAT_104bfdd8].matrix.SetTranslation(carriedStart);
                            if (clearCount == 2)
                            {
                                Vec4 soundAt = DAT_10db9210.GetTranslation();
                                FUN_1003ad20(SOUND_CARRY_PUSH, &soundAt, 1.0f);
                            }
                            else
                            {
                                DAT_10db6ee8[DAT_104bfdd8].m_0075 = 0;
                                DAT_10db6ee8[0].m_0075 = 0;
                                DAT_104bfddc = 0;
                                DAT_104bfdcc = 0;
                                DAT_10db6ee8[0].PlayMotion(MOTION_STAND, 0, 5, 1.0f);
                                DAT_10db6ee8[DAT_104bfdd8].m_02a0 = 0;
                            }
                            DAT_10db6ee8[pushOrder[0]].root->UpdateWorld(DAT_10db6ee8[pushOrder[0]].matrix,
                                                                         DAT_10db6ee8[pushOrder[0]].motionFrame);
                            DAT_10db6ee8[pushOrder[1]].root->UpdateWorld(DAT_10db6ee8[pushOrder[1]].matrix,
                                                                         DAT_10db6ee8[pushOrder[1]].motionFrame);
                        }
                        DAT_104bfdcc = DAT_104bfdcc - 1;
                        if (DAT_104bfdcc == CARRY_RESET_FRAME)
                            goto carryFinish;
                        if (DAT_104bfdcc < CARRY_DRAG_FRAME + 1)
                        {
                            if (DAT_104bfdcc < CARRY_SETTLE_FRAME)
                                goto carryFinish;
                            // Frames 10..8: the player is drawn toward the object.
                            DAT_10db6ee8[DAT_104bfdd8].m_0075 = 0;
                            DAT_10db6ee8[0].m_0075 = 0;
                            Vec4 playerToObject;
                            playerToObject.Copy(DAT_10db6ee8[DAT_104bfdd8].matrix.GetTranslation() -
                                                DAT_10db9210.GetTranslation());
                            DAT_10db6ee8[DAT_104bf8b4].SetMoveStep(playerToObject.x * CARRY_DRAG_SCALE,
                                                                   playerToObject.y * CARRY_DRAG_SCALE, 0.0f);
                        }
                        else if (DAT_104bfdcc < CARRY_SNAP_FRAME + 1)
                        {
                            // Frames 14..11: the object settles onto its grid cell.
                            Vec4 before = DAT_10db6ee8[DAT_104bfdd8].matrix.GetTranslation();
                            DAT_10db6ee8[DAT_104bfdd8].SnapPosition(CARRY_GRID, 0.0f);
                            Vec4 after = DAT_10db6ee8[DAT_104bfdd8].matrix.GetTranslation();
                            DAT_10db6ee8[DAT_104bfdd8].SetMoveStep((after.x - before.x) * CARRY_SNAP_SCALE,
                                                                   (after.y - before.y) * CARRY_SNAP_SCALE, 0.0f);
                            DAT_10db6ee8[DAT_104bfdd8].matrix.SetTranslation(before);
                        }
                        goto carryEasing;
                    carryFinish:
                        DAT_10db6ee8[DAT_104bfdd8].m_0075 = 0;
                        DAT_10db6ee8[0].m_0075 = 0;
                        DAT_104bfddc = 0;
                        DAT_104bfdcc = 0;
                        {
                            Vec4 noOffset;
                            noOffset.Fill(0.0f);
                            Vec4 objectAt = DAT_10db6ee8[DAT_104bfdd8].matrix.GetTranslation();
                            DAT_10db6ee8[DAT_104bfdd8].SnapPosition(CARRY_GRID, 0.0f);
                            if (DAT_10db6ee8[DAT_104bfdd8].IsClearAny(&noOffset) == 0 ||
                                DAT_10db6ee8[DAT_104bfdd8].IsClearOfMapLimited(&noOffset) == 0)
                                DAT_10db6ee8[DAT_104bfdd8].matrix.SetTranslation(objectAt);
                        }
                        bool putDown = DAT_104c0074 != 0;
                        DAT_10db6ee8[DAT_104bfdd8].m_0075 = 0;
                        DAT_10db6ee8[0].m_0075 = 0;
                        if (putDown)
                        {
                            DAT_10db6ee8[DAT_104bfdd8].SnapPosition(CARRY_GRID, 0.0f);
                            DAT_104c0074 = 0;
                            DAT_104bfdc8 = 0;
                            DAT_104bfdcc = 0;
                            DAT_104bfdd0 = 0;
                            DAT_104c0090 = 1;
                        }
                    }
                carryEasing:
                    RunSpeedFalloff();
                    if (LeanEasesBack(LEAN_EASE_FAST))
                    {
                        DAT_10866de8.Fill(0.0f);
                        DAT_10db6ee8[0].spinAngle = 0.0f;
                    }
                }
                goto settleCheck;
            }
            // Steering: walk, leap over an obstacle, swim or slide.
            if (DAT_104bfdc8 == 0)
            {
                if (DAT_104bfe30 == 0)
                {
                    if ((DAT_104c00e0 == 0 || DAT_104c00e8 != 0x30) && DAT_104c02e4 == 0 && DAT_104c02ed == 0 &&
                        DAT_104c0300 == 0)
                        DAT_10db6ee8[DAT_104bf8b4].M_101629b0(moveX, moveY);
                    if (DAT_104c00e0 != 0 && DAT_104c00f4 > 0 && DAT_104c02e4 == 0)
                        DAT_102199b0 = DAT_10db6ee8[DAT_104bf8b4].targetHeading;
                leapCheck:
                    if (DAT_104bfe30 == 0)
                        goto carryCheck;
                }
            leapStart:
                // The leap over an obstacle (DAT_104bfe44 is its node): the stage counter runs the approach (0-4), the
                // look (5), the climb (6-0x31), the rise and fall along the leap curve and the landing.
                float leapCurve[LEAP_CURVE_LENGTH];
                float leapAngle;
                leapCurve[0] = 0.4f;
                leapCurve[1] = 0.5f;
                leapCurve[2] = 0.3f;
                leapCurve[3] = 0.3f;
                leapCurve[4] = 0.3f;
                leapCurve[5] = 0.6f;
                leapCurve[6] = 1.0f;
                leapCurve[7] = 1.8f;
                leapCurve[8] = 1.8f;
                leapCurve[9] = 1.2f;
                leapCurve[10] = 0.5f;
                leapCurve[11] = 0.0f;
                leapCurve[12] = 0.0f;
                leapCurve[13] = 0.0f;
                leapCurve[14] = 0.6f;
                leapCurve[15] = 0.8f;
                leapCurve[16] = 1.0f;
                leapCurve[17] = 1.0f;
                leapCurve[18] = 1.0f;
                leapCurve[19] = 1.0f;
                leapCurve[20] = 1.0f;
                leapCurve[21] = 1.2f;
                leapCurve[22] = 1.2f;
                leapCurve[23] = 0.8f;
                leapCurve[24] = 0.0f;
                leapCurve[25] = 0.0f;
                leapCurve[26] = 0.0f;
                leapCurve[27] = 0.0f;
                DAT_104bfe38 = 1;
                DAT_104bfe34 = 0;
                if (DAT_104bfe40 < LEAP_STAGE_LOOK)
                {
                    Vec4 toObstacle;
                    Vec4 obstacleAt;
                    DAT_10db6ee8[0].PlayMotion(0x14, 0, 3, 1.0f);
                    DAT_104bfe40 = DAT_104bfe40 + 1;
                    toObstacle.Copy(DAT_10db9210.GetTranslation());
                    obstacleAt.Copy(DAT_104bfe44->children[0]->worldMatrix.GetTranslation());
                    toObstacle.Copy(obstacleAt - toObstacle);
                    DAT_10db6ee8[0].Push(toObstacle.x * LEAP_PUSH_X, toObstacle.y * LEAP_PUSH_Y, 0.0f);
                    leapAngle = (float)atan2(toObstacle.x, toObstacle.y);
                    goto leapFace;
                }
                if (DAT_104bfe40 != LEAP_STAGE_LOOK)
                {
                    if (DAT_104bfe40 < LEAP_STAGE_CLIMB_END)
                    {
                        Vec4 climbAt;
                        Vec4 obstacleAt;
                        DAT_104bfe40 = DAT_104bfe40 + 1;
                        climbAt.Copy(DAT_10db9210.GetTranslation());
                        obstacleAt.Copy(DAT_104bfe44->children[0]->worldMatrix.GetTranslation());
                        if (DAT_104bfe40 == 0x1f)
                            DAT_10836c84.Copy((obstacleAt - climbAt).Divided(20.0f));
                        if (DAT_104bfe40 < 0x2c)
                        {
                            if (DAT_104bfe40 > 0x25)
                            {
                                climbAt.x = DAT_10836c84.x * LEAP_DRIFT_FAST + climbAt.x;
                                climbAt.y = DAT_10836c84.y * LEAP_DRIFT_FAST + climbAt.y;
                            }
                            if (DAT_104bfe40 < 0x25)
                                climbAt.z = DAT_10836c84.z * LEAP_DRIFT_LIFT + climbAt.z;
                            DAT_10db6ee8[0].SetPosition(climbAt.x, climbAt.y, climbAt.z);
                            DAT_10db6ee8[0].targetHeading = (float)atan2(DAT_10836c84.x, DAT_10836c84.y) + LEAP_FACE_TURN;
                            DAT_10db6ee8[0].heading = DAT_10db6ee8[0].targetHeading;
                        }
                        if (DAT_104bfe40 == 0x2d)
                            DAT_10836c84.Copy((obstacleAt - climbAt).Divided(5.0f));
                        if (DAT_104bfe40 < 0x2d)
                        {
                            climbAt.Copy(obstacleAt - climbAt);
                            leapAngle = (float)atan2(climbAt.x, climbAt.y);
                        }
                        else
                        {
                            climbAt.Copy(climbAt + DAT_10836c84);
                            DAT_10db6ee8[0].SetPosition(climbAt.x, climbAt.y, climbAt.z);
                            leapAngle = (float)atan2(DAT_10836c84.x, DAT_10836c84.y);
                        }
                        goto leapFace;
                    }
                    if (DAT_104bfe40 == LEAP_STAGE_CLIMB_END)
                    {
                    leapEnd:
                        DAT_104bfe30 = 0;
                        DAT_10db6ee8[DAT_104bf8b4].mode = 1;
                        DAT_10db6ee8[0].PlayMotion(2, 1, 5, 1.0f);
                        goto leapEasing;
                    }
                    if (DAT_104bfe40 < LEAP_STAGE_RISE_END)
                    {
                        Vec4 playerAt;
                        Vec4 obstacleAt;
                        float rise;
                        DAT_104bfe40 = DAT_104bfe40 + 1;
                        DAT_10db6ee8[0].m_238c = 1;
                        if (DAT_102027f4 == 1.0f)
                            rise = leapCurve[DAT_104bfe40 - 60];
                        else
                            rise = leapCurve[DAT_104bfe40 * 2 - 129] + leapCurve[DAT_104bfe40 * 2 - 130];
                        DAT_10db6ee8[0].Push(0.0f, 0.0f, rise * LEAP_HEIGHT_SCALE);
                        DAT_10db6ee8[0].m_238c = 0;
                        playerAt.Copy(DAT_10db9210.GetTranslation());
                        obstacleAt.Copy(DAT_104bfe44->children[0]->worldMatrix.GetTranslation());
                        playerAt.Copy(obstacleAt - playerAt);
                        leapAngle = (float)atan2(playerAt.x, playerAt.y);
                        goto leapFace;
                    }
                    if (DAT_104bfe40 == LEAP_STAGE_RISE_END)
                        goto leapLand;
                    if (DAT_104bfe40 < LEAP_STAGE_FALL_END)
                    {
                        Vec4 playerAt;
                        Vec4 obstacleAt;
                        float fall;
                        if (DAT_10db6ee8[0].m_0d48 != 0)
                            goto leapEnd;
                        DAT_104bfe40 = DAT_104bfe40 + 1;
                        DAT_10db6ee8[0].m_238c = 1;
                        if (DAT_102027f4 == 1.0f)
                            fall = leapCurve[DAT_104bfe40 - 66] * -LEAP_HEIGHT_SCALE;
                        else
                            fall = leapCurve[DAT_104bfe40 * 2 - 154] * -LEAP_HEIGHT_SCALE -
                                   leapCurve[DAT_104bfe40 * 2 - 153] * LEAP_HEIGHT_SCALE;
                        DAT_10db6ee8[0].Push(0.0f, 0.0f, fall);
                        DAT_10db6ee8[0].m_0d48 = 0;
                        DAT_10db6ee8[0].m_238c = 0;
                        playerAt.Copy(DAT_10db9210.GetTranslation());
                        obstacleAt.Copy(DAT_104bfe44->children[0]->worldMatrix.GetTranslation());
                        playerAt.Copy(obstacleAt - playerAt);
                        leapAngle = (float)atan2(playerAt.x, playerAt.y);
                        goto leapFace;
                    }
                    if (DAT_104bfe40 == LEAP_STAGE_FALL_END)
                    {
                    leapLand:
                        {
                            Vec4 landAt = DAT_10db9210.GetTranslation();
                            FUN_1003ad20(SOUND_LEAP_LAND, &landAt, 1.0f);
                        }
                        DAT_10db6ee8[0].SnapHeight(LEAP_SNAP_STEP, LEAP_SNAP_OFFSET);
                        DAT_104bfe40 = 0x62;
                        goto leapEasing;
                    }
                    if (DAT_104bfe40 < LEAP_STAGE_DONE)
                    {
                        DAT_104bfe40 = DAT_104bfe40 + 1;
                        goto leapEasing;
                    }
                    // Stage 100: wait for the steer to start the next vault, or turn round when too far away.
                    {
                        Vec4 waitAt;
                        Vec4 obstacleNow;
                        waitAt.Copy(DAT_10db9210.GetTranslation());
                        obstacleNow.Copy(DAT_104bfe44->children[0]->worldMatrix.GetTranslation());
                        if (LEAP_REACH_LIMIT <= waitAt.Distance(obstacleNow))
                        {
                            if (steerY > LEAP_VAULT_STEER)
                            {
                                DAT_104bfe48 = DAT_104bfe48 + 1;
                                DAT_102027f4 = 2.0f;
                                DAT_10db6ee8[DAT_104bf8b4].PlayMotion((DAT_104bfe48 & 1) == 0 ? 0x16 : 0x17, 2, 2, 2.0f);
                                DAT_104bfe40 = 0x41;
                                Vec4 resumeAt = DAT_10db9210.GetTranslation();
                                DAT_10db6ee8[0].SetPosition(resumeAt.x, resumeAt.y, resumeAt.z);
                            }
                            else if (steerY < -LEAP_VAULT_STEER)
                            {
                                DAT_104bfe48 = DAT_104bfe48 + 1;
                                DAT_102027f4 = 2.0f;
                                DAT_10db6ee8[DAT_104bf8b4].PlayMotion((DAT_104bfe48 & 1) == 0 ? 0x16 : 0x17, 2, 2, 2.0f);
                                DAT_104bfe40 = 0x55;
                                Vec4 resumeAt = DAT_10db9210.GetTranslation();
                                DAT_10db6ee8[0].SetPosition(resumeAt.x, resumeAt.y, resumeAt.z);
                            }
                        }
                        else
                        {
                            DAT_104bfe40 = 0x1e;
                            DAT_10db6ee8[0].PlayMotion(0x1a, 2, 5, 1.0f);
                            Vec4 resumeAt = DAT_10db9210.GetTranslation();
                            DAT_10db6ee8[0].SetPosition(resumeAt.x, resumeAt.y, resumeAt.z);
                        }
                        {
                            Vec4 playerAt;
                            Vec4 obstacleAt;
                            playerAt.Copy(DAT_10db9210.GetTranslation());
                            obstacleAt.Copy(DAT_104bfe44->children[0]->worldMatrix.GetTranslation());
                            playerAt.Copy(obstacleAt - playerAt);
                            leapAngle = (float)atan2(playerAt.x, playerAt.y);
                        }
                    }
                leapFace:
                    DAT_10db6ee8[0].targetHeading = leapAngle + LEAP_FACE_TURN;
                    DAT_10db6ee8[0].heading = DAT_10db6ee8[0].targetHeading;
                    goto leapEasing;
                }
                // Stage 5: the look ends; take the vault or wait.
                DAT_10db6ee8[0].SnapHeight(LEAP_SNAP_STEP, LEAP_SNAP_OFFSET);
                if (DAT_10db6ee8[0].m_238c == 0)
                    DAT_104bfe40 = LEAP_STAGE_DONE;
                else
                {
                    Vec4 groundAt = DAT_10db9210.GetTranslation();
                    DAT_10db6ee8[0].SetPosition(groundAt.x, groundAt.y, groundAt.z);
                    DAT_104bfe48 = DAT_104bfe48 + 1;
                    DAT_10db6ee8[0].m_0d48 = 0;
                    DAT_10db6ee8[DAT_104bf8b4].PlayMotion((DAT_104bfe48 & 1) == 0 ? 0x16 : 0x17, 2, 5, 1.0f);
                    DAT_104bfe40 = 0x3c;
                }
            leapEasing:
                RunSpeedFalloff();
                if (LeanEasesBack(LEAN_EASE_FAST))
                {
                    DAT_10866de8.Fill(0.0f);
                    DAT_10db6ee8[0].spinAngle = 0.0f;
                }
            }
            else
            {
                if (DAT_104bfe30 != 0)
                    goto leapStart;
                if (DAT_104bfe4c == 0 && DAT_104c0300 == 0 && DAT_104bfdcc == 0)
                {
                    DAT_104bfdcc = CARRY_PUSH_FRAME;
                    if (DAT_104bfea9 == 0)
                    {
                        Vec4 gruntAt = DAT_10db9210.GetTranslation();
                        int gruntPick = FUN_1013f880() % 2;
                        if (DAT_104c0332 == 0)
                            FUN_1003ad20(gruntPick + SOUND_GRUNT_BASE, &gruntAt, 1.0f);
                    }
                    DAT_104bfea9 = 1;
                    goto leapCheck;
                }
            carryCheck:
                if (DAT_104bfdc8 != 0)
                    goto carryAction;
            swimOrSlide:
                if (DAT_104c0300 != 0)
                    goto slideNoSteer;
                if (DAT_104bfe4c != 0)
                {
                    if (DAT_104bfe54 >= SWIM_MIN_FRAME)
                    {
                        float depth;
                        float lift;
                        DAT_10db6ee8[0].m_238c = 1;
                        depth = ((DAT_104bfe24 - SWIM_DEPTH_OFFSET) - DAT_10db6ee8[0].matrix._43) - 1.0f;
                        if (depth > SWIM_RISE_LIMIT || depth > 0.0f)
                            lift = depth <= SWIM_RISE_LIMIT ? depth : SWIM_RISE_LIMIT;
                        else
                            lift = 0.0f;
                        DAT_10db6ee8[DAT_104bf8b4].Push(moveX * SWIM_PUSH_SCALE, moveY * SWIM_PUSH_SCALE, lift);
                        DAT_10db6ee8[0].m_238c = 0;
                    }
                    RunSpeedFalloff();
                    if (LeanEasesBack(LEAN_EASE_FAST))
                    {
                        DAT_10866de8.Fill(0.0f);
                        DAT_10db6ee8[0].spinAngle = 0.0f;
                    }
                    goto settleCheck;
                }
                if (DAT_104c00e0 != 0)
                {
                    // Swinging: the model walks slowly while the swing runs, only if the step is clear.
                    Vec4 swingStep;
                    Vec4 swingProbe;
                    if (moveInput == 1)
                    {
                        SteerLeanAndSpeed(moveX, moveY);
                        swingStep.Set(DAT_102027ec * moveX, DAT_102027ec * moveY, 0.0f, 0.0f);
                    }
                    else
                    {
                        if (LeanEasesBack(LEAN_EASE_SLOW))
                            DAT_10db6ee8[0].spinAngle = 0.0f;
                        swingStep.Set(moveX * WALK_SCALE, moveY * WALK_SCALE, 0.0f, 0.0f);
                        RunSpeedFalloff();
                    }
                    swingProbe.Set(swingStep.x * SWING_STEP_SCALE, swingStep.y * SWING_STEP_SCALE, swingStep.z + SWING_PROBE_LIFT,
                                   swingStep.w);
                    if (DAT_104c00e0 == 0 && DAT_104bfe1c == 0 && DAT_104c00d0 == 0 && DAT_1032d69c == 1 && DAT_104bfe6c == 0)
                        DAT_10db6ee8[0].motion->m_0008 = DAT_102027ec / MOTION_SPEED_TOP;
                    if (DAT_10db6ee8[DAT_104bf8b4].IsClearAny(&swingProbe) != 0 && DAT_104c00e0 < SWING_MAX_BLOCKED_STEPS)
                        DAT_10db6ee8[DAT_104bf8b4].Push(swingStep.x, swingStep.y, 0.0f);
                    goto settleCheck;
                }
                if (moveInput == 1)
                {
                    SteerLeanAndSpeed(moveX, moveY);
                    if (DAT_104c00e0 == 0 && DAT_104bfe1c == 0 && DAT_104c00d0 == 0 && DAT_1032d69c == 1 && DAT_104bfe6c == 0)
                        DAT_10db6ee8[0].motion->m_0008 = DAT_102027ec / MOTION_SPEED_TOP;
                    DAT_10db6ee8[DAT_104bf8b4].Push(DAT_102027ec * moveX, DAT_102027ec * moveY, 0.0f);
                }
                else
                {
                    if (LeanEasesBack(LEAN_EASE_SLOW))
                        DAT_10db6ee8[0].spinAngle = 0.0f;
                    DAT_10db6ee8[DAT_104bf8b4].Push(moveX * WALK_SCALE, moveY * WALK_SCALE, 0.0f);
                    RunSpeedFalloff();
                }
                if (DAT_10db6ee8[0].m_238c == 0 && DAT_10db6ee8[0].m_229c < 6 && DAT_10db6ee8[0].m_0090.x != 0.0f)
                {
                    DAT_1065cb18.Copy(DAT_10db6ee8[0].m_0090);
                    DAT_104c02ed = 3;
                    if (DAT_104c01d0 != 0)
                        goto settleSlow;
                    if (DAT_104c01d8 == 0 && DAT_104c01cc == 0 && DAT_104c01c8 == 0 && DAT_10db6ee8[0].hitSolid == 0)
                    {
                        Vec4 dustAt;
                        dustAt.Copy(DAT_10db9210.GetTranslation());
                        DAT_10daaa80.Start(EFFECT_SWING_DUST, dustAt, -1, 0, 0, 0, 0, 0, 0, 0);
                        goto settleCheck;
                    }
                settleCommon:
                    DAT_10db6ee8[0].speedScale = 1.0f;
                }
                else
                {
                settleCheck:
                    if (DAT_104c01d0 == 0)
                        goto settleCommon;
                settleSlow:
                    if (DAT_104c00e0 == 0 && DAT_104bfe1c == 0 && DAT_104c00d0 == 0 && DAT_1032d69c == 1 && DAT_104bfe6c == 0)
                    {
                        DAT_10db6ee8[0].speedScale = SPEED_SCALE_SLOW;
                        DAT_10db6ee8[0].motion->m_0008 = MOTION_SPEED_SLOW;
                    }
                }
                DAT_10edbd38.x = moveX;
                DAT_10edbd38.y = moveY;
                DAT_10866de8.x = DAT_10866de8.x * SMOOTH_OLD + moveX * SMOOTH_NEW;
                DAT_10866de8.y = DAT_10866de8.y * SMOOTH_OLD + moveY * SMOOTH_NEW;
                if (DAT_104bfdc8 != 0)
                {
                    DAT_10866de8.Fill(0.0f);
                    if (fabs(DAT_10db9210.GetTranslation().z - DAT_10db6ee8[DAT_104bfdd8].matrix.GetTranslation().z) <= CARRY_NEAR_HEIGHT &&
                        DAT_10db9210.GetTranslation().Distance(DAT_10db6ee8[DAT_104bfdd8].matrix.GetTranslation()) <= CARRY_NEAR_DISTANCE)
                        goto movementTail;
                    DAT_104c0074 = 1;
                }
            }
        movementTail:
            DAT_104bf8c4 = DAT_104bf8c4 - AIR_STEP_DECAY;
            if (DAT_104bf8c4 < 1)
                DAT_104bf8c4 = 0;
        }
movementDone:;
        // The player follows a gust (DAT_104c02ed counts its ticks), the carried object is let go when it is out of
        // reach, and the model stays at the water surface while swimming without steering.
        if (DAT_104bfd0c != 0 && LeanEasesBack(LEAN_EASE_SLOW))
            DAT_10db6ee8[0].spinAngle = 0.0f;
        if (DAT_104c02ed != 0)
        {
            if (DAT_10db6ee8[0].m_238c == 0 && DAT_104c00e0 == 0)
            {
                DAT_104c02ed = DAT_104c02ed - 1;
                DAT_10db6ee8[0].PlayMotion(MOTION_BOUNCE, 0, 5, 1.0f);
                DAT_10db6ee8[0].targetHeading = (float)atan2(DAT_10db6ee8[0].m_00a0.x, DAT_10db6ee8[0].m_00a0.y) + LEAP_FACE_TURN;
                if (DAT_1065cb18.x < 0.0f && DAT_10db6ee8[0].spinAngle > -BOUNCE_LEAN_LIMIT)
                    DAT_10db6ee8[0].spinAngle = DAT_10db6ee8[0].spinAngle - BOUNCE_LEAN_STEP;
                if (DAT_1065cb18.x > 0.0f && DAT_10db6ee8[0].spinAngle < BOUNCE_LEAN_LIMIT)
                    DAT_10db6ee8[0].spinAngle = DAT_10db6ee8[0].spinAngle + BOUNCE_LEAN_STEP;
            }
            else
            {
                DAT_10db6ee8[0].hitSolid = 0;
                DAT_104c02ed = 0;
            }
        }
        DAT_10db6ee8[0].m_0013 = DAT_104c02ed;
        if (DAT_104bfdc8 != 0 && DAT_104bfdd8 > 0)
        {
            if (DAT_10db9210.GetTranslation().Distance(DAT_10db6ee8[DAT_104bfdd8].matrix.GetTranslation()) > CARRY_DROP_DISTANCE &&
                DAT_104bfdc8 != 0)
            {
                DAT_10db6ee8[DAT_104bfdd8].m_0075 = 0;
                DAT_10db6ee8[0].m_0075 = 0;
                DAT_104bfdc8 = 0;
                DAT_104bfdcc = 0;
                DAT_104c0090 = 1;
            }
        }
        if (DAT_104c01e0 != 0)
        {
            DAT_10866de8.Fill(0.0f);
            DAT_10db6ee8[0].spinAngle = 0.0f;
        }
        if (steerX == 0.0f && steerY == 0.0f)
        {
            RunSpeedFalloff();
            if (LeanEasesBack(LEAN_EASE_FAST))
            {
                DAT_10866de8.Fill(0.0f);
                DAT_10db6ee8[0].spinAngle = 0.0f;
            }
            if (DAT_104bfe4c != 0 && (DAT_10468128 != 0 || DAT_10db6ee8[0].m_229c > 8) && DAT_104bfe54 >= SWIM_MIN_FRAME)
            {
                float depth;
                float lift;
                DAT_10db6ee8[0].m_238c = 1;
                depth = ((DAT_104bfe24 - SWIM_DEPTH_OFFSET) - DAT_10db6ee8[0].matrix._43) - 1.0f;
                if (depth > SWIM_SHALLOW_LIMIT || depth > 0.0f)
                    lift = depth <= SWIM_SHALLOW_LIMIT ? depth : SWIM_SHALLOW_LIMIT;
                else
                    lift = 0.0f;
                DAT_10db6ee8[DAT_104bf8b4].Push(0.0f, 0.0f, lift);
                DAT_10db6ee8[0].m_238c = 0;
            }
        }

        DAT_104c0098 = 0;
        if (DAT_104bfdc8 == 0)
        {
            DAT_102027e4 = 135.0f;
            DAT_104bfdd8 = 0;
        }
        DAT_104bf888 = 0;
        closestEnemy = ENEMY_DISTANCE_START;
        enemyClose = 0;
        if (DAT_104bfdc8 == 0)
            DAT_104bfdd0 = 0;
        if (DAT_10255f08 != 0)
            DAT_10255f08->UpdateVisibleCells();
        gimmickFound = 0;
        modelCount = 0;
        for (i = 0; i < MODEL_UPDATE_COUNT; i++)
        {
            if (DAT_10db6ee8[i].root != 0)
            {
                modelOrder[modelCount] = i;
                modelCount = modelCount + 1;
            }
        }
        qsort(modelOrder, modelCount, sizeof(int), FUN_1000c2f0);
        if (DAT_104c0135 == 0)
        {
            for (modelPos = 0; modelPos < modelCount; modelPos++)
            {
                modelIdx = modelOrder[modelPos];
                DAT_104c01f0 = 0;
                if (DAT_10db6ee8[modelIdx].root != 0)
                {
                    if (DAT_10253e98[modelIdx] != 0)
                        DAT_10253e98[modelIdx]--;
                    if (DAT_102239d0[modelIdx] != 0)
                    {
                        // fading out: the model sinks to the floor of its slot and its colour dims
                        if (DAT_10db6ee8[modelIdx].m_238c != 0)
                            DAT_102239d0[modelIdx] = 1;
                        float sinkFloor = DAT_108377c8[modelIdx].z + FADE_OUT_FLOOR_HEIGHT;
                        DAT_10db6ee8[modelIdx].matrix.m[MATRIX_TRANSLATION_X] = DAT_108377c8[modelIdx].x;
                        DAT_10db6ee8[modelIdx].matrix.m[MATRIX_TRANSLATION_Y] = DAT_108377c8[modelIdx].y;
                        if (DAT_10db6ee8[modelIdx].matrix.m[MATRIX_TRANSLATION_Z] > sinkFloor)
                            DAT_10db6ee8[modelIdx].matrix.m[MATRIX_TRANSLATION_Z] = sinkFloor;
                        DAT_102239d0[modelIdx]--;
                        DAT_10db6ee8[modelIdx].m_022c.Set(
                            ((float)DAT_10467f28[modelIdx] / FADE_OUT_DIVISOR + FADE_OUT_BASE) * DAT_102239d0[modelIdx] / FADE_OUT_FRAMES,
                            0, 0, 0);
                        DAT_102550e0[modelIdx]++;
                        if (DAT_102239d0[modelIdx] == 0 && DAT_10255a88[modelIdx] != 0)
                        {
                            DAT_10db6ee8[modelIdx].m_022c.Set(0, 0, 0, 0);
                            DAT_102550e0[modelIdx] = 1;
                            DAT_10db6ee8[modelIdx].HoldAtSpine(0, 0);
                            DAT_10210b30[modelIdx] = 1;
                        }
                    }
                    if (DAT_102550e0[modelIdx] != 0 && --DAT_102550e0[modelIdx] == 0 && DAT_10255a88[modelIdx] != 0)
                        DAT_10210b30[modelIdx] = 1;
                    if (modelIdx > 0 && DAT_104bfe6c == 0 && DAT_104bfeb0 == 0)
                    {
                        // an enemy of slot kind 3 acts here unless the frame is an event's (then the general code below runs)
                        if (DAT_1021f3c0[modelIdx] == 3 && modelIdx > 0x39
                            && !((DAT_10210d30[modelIdx] == 0 && DAT_104c0094 != 0) || DAT_104c0095 != 0 || DAT_104c0135 != 0))
                        {
                            if (tick != ticks - 1 && DAT_1065cb90 == 0)
                                goto modelTail;
                            if (DAT_103153b0[modelIdx] != 0)
                            {
                                DAT_103153b0[modelIdx]--;
                                if (DAT_10daaae8[modelIdx].stats[STAT_HEALTH] < 1)
                                    DAT_103153b0[modelIdx] = 0;
                                else if (DAT_104bf890 % ENEMY_SPARK_PERIOD == 0)
                                {
                                    Vec4 sparkAt;
                                    sparkAt.Fill(0.0f);
                                    if (DAT_10db6ee8[modelIdx].spineNode != 0)
                                    {
                                        DAT_10daaa80.Start(EFFECT_ENEMY_SPARK, sparkAt, -1, -1, &DAT_10db6ee8[modelIdx].spineNode->worldMatrix, 0, 0, 0,
                                                           0, 0);
                                        if (DAT_104bf890 % ENEMY_SPARK_BURST_PERIOD == 0)
                                        {
                                            EffectOptions burstOptions;
                                            burstOptions.m_0000 = 1;
                                            burstOptions.m_0004 = 4;
                                            burstOptions.m_0008 = 1.0f;
                                            burstOptions.m_000e = 5;
                                            burstOptions.m_001c = 1;
                                            burstOptions.owner = ENEMY_SPARK_OWNER;
                                            Vec4 burstAt;
                                            burstAt.Fill(0.0f);
                                            DAT_10daaa80.Start(EFFECT_ENEMY_SPARK_BURST, burstAt, -1, 0, 0, 0, 0, &DAT_10db6ee8[modelIdx].matrix,
                                                               &burstOptions, 0);
                                        }
                                    }
                                }
                            }
                            if (DAT_10219770[modelIdx] != 0)
                            {
                                DAT_10219770[modelIdx]--;
                                if (DAT_10daaae8[modelIdx].stats[STAT_HEALTH] < 1)
                                    DAT_10219770[modelIdx] = 0;
                                else if (DAT_104bf890 % ENEMY_SPARK_PERIOD == 0 && DAT_10db6ee8[modelIdx].spineNode != 0)
                                {
                                    Vec4 smokeAt;
                                    smokeAt.Fill(0.0f);
                                    smokeAt.Copy(DAT_10db6ee8[modelIdx].spineNode->worldMatrix.GetTranslation());
                                    DAT_10daaa80.Start(EFFECT_ENEMY_SMOKE, smokeAt, -1, -1, 0, 0, 0, 0, 0, 0);
                                }
                            }
                            if (DAT_1020d4c0[modelIdx] == 0
                                || (DAT_102239d0[modelIdx] == 0 && DAT_10daaae8[modelIdx].stats[STAT_HEALTH] < 1))
                            {
                                // the hit flash: the model's tint pulses while the flash frames run down
                                if (DAT_1021b940[modelIdx] != 0)
                                {
                                    unsigned int flashAge = DAT_104bf890 - DAT_1030c998[modelIdx];
                                    DAT_10f08080[modelIdx].z = DAT_10f08080[modelIdx].z + HIT_FLASH_RISE;
                                    char flashFrames = DAT_1021b940[modelIdx];
                                    double pulse = cos((float)flashAge / HIT_FLASH_PULSE_DIVISOR);
                                    DAT_1021b940[modelIdx] = flashFrames - 1;
                                    DAT_10447bd0[modelIdx].value =
                                        (float)((int)flashFrames / HIT_FLASH_FRAMES * DAT_1021b9c0[modelIdx] * pulse);
                                    if (DAT_1021b940[modelIdx] <= 0)
                                    {
                                        DAT_10447bd0[modelIdx].value = 0;
                                        DAT_1021b940[modelIdx] = 0;
                                    }
                                }
                                else
                                {
                                    DAT_10447bd0[modelIdx].value = DAT_10447bd0[modelIdx].value + HIT_FLASH_STEP;
                                    if (HIT_FLASH_MAX < DAT_10447bd0[modelIdx].value)
                                        DAT_10447bd0[modelIdx].value = HIT_FLASH_MAX;
                                    DAT_1021b9c0[modelIdx] = DAT_10447bd0[modelIdx].value;
                                    DAT_1021b940[modelIdx] = HIT_FLASH_START;
                                    DAT_1030c998[modelIdx] = DAT_104bf890;
                                }
                            }
                            int enemyState = DAT_10210d30[modelIdx];
                            if (enemyState == ENEMY_TOSSED)
                            {
                                // a defeated enemy flies away: it slides along its direction, bounces off walls and bursts
                                for (int tossStep = 0; tossStep < TOSS_STEPS_PER_FRAME; tossStep++)
                                {
                                    Vec4 startAt;
                                    startAt.Copy(DAT_10db6ee8[modelIdx].matrix.GetTranslation());
                                    Vec4 sweep[TOSS_SWEEP_COUNT];
                                    Vec4 probe;
                                    int sweepTry = 0;
                                    if (DAT_10467ba0[modelIdx] != 0)
                                    {
                                        probe.Copy(DAT_10db6ee8[DAT_10467ba0[modelIdx]].matrix.GetTranslation() + DAT_108387c8[modelIdx]);
                                        DAT_10db6ee8[modelIdx].SetPosition(probe.x, probe.y, probe.z);
                                    }
                                    else
                                    {
                                        sweep[1].Copy(DAT_10836470[modelIdx]);
                                        sweep[0].Copy(sweep[1]);
                                        sweep[0].z = sweep[1].z - TOSS_DROP;
                                        Vec4 turned;
                                        sweep[2].Copy(*FUN_101ae1c0(&turned, &sweep[1], -TOSS_TURN));
                                        sweep[3].Copy(*FUN_101ae250(&turned, &sweep[1], TOSS_TURN));
                                        sweep[4].Copy(*FUN_101ae250(&turned, &sweep[1], -TOSS_TURN));
                                        Vec4 noOffset;
                                        noOffset.Fill(0.0f);
                                        do
                                        {
                                            probe.Copy(sweep[sweepTry] * TOSS_REACH + startAt);
                                            DAT_10db6ee8[modelIdx].SetPosition(probe.x, probe.y, probe.z);
                                            if (DAT_10db6ee8[modelIdx].IsClearOfMapLimited(&noOffset) != 0)
                                                break;
                                            sweepTry++;
                                        } while (sweepTry < TOSS_SWEEP_COUNT);
                                        if (sweepTry == 0)
                                        {
                                            DAT_10836470[modelIdx].Copy(sweep[0]);
                                            DAT_10836470[modelIdx].Normalized3();
                                        }
                                        else if (sweepTry == TOSS_SWEEP_COUNT)
                                        {
                                            if (--DAT_1030c480[modelIdx] != 0)
                                            {
                                                // every way is blocked: find the wall and reflect off it
                                                Matrix4 wallSpace;
                                                wallSpace.SetIdentity();
                                                Vec4 wallNormal;
                                                Vec4 wallPoint;
                                                float wallRadius = 0.0f;
                                                DAT_10db6ee8[modelIdx].SetPosition(startAt.x, startAt.y, startAt.z);
                                                DAT_10db6ee8[modelIdx].matrix.SetTranslation(startAt);
                                                wallPoint.Copy(startAt);
                                                int wallTry = 0;
                                                do
                                                {
                                                    float depth[4];
                                                    if (DAT_10db6ee8[0].dropCollider->Query(&wallSpace, &wallPoint, wallRadius + TOSS_WALL_RADIUS,
                                                                                            &wallNormal, depth, 0, 0) != 0)
                                                        break;
                                                    wallTry++;
                                                    wallRadius = wallRadius + TOSS_WALL_RADIUS_STEP;
                                                } while (wallTry < TOSS_SWEEP_COUNT);
                                                if (wallTry == TOSS_SWEEP_COUNT)
                                                {
                                                    DAT_10836470[modelIdx].x = -DAT_10836470[modelIdx].x;
                                                    DAT_10836470[modelIdx].y = -DAT_10836470[modelIdx].y;
                                                    DAT_10836470[modelIdx].z = -DAT_10836470[modelIdx].z;
                                                }
                                                else
                                                {
                                                    DAT_10836470[modelIdx].RotateByDirection(wallNormal);
                                                    DAT_10836470[modelIdx].Normalized3();
                                                }
                                            }
                                        }
                                    }
                                    if ((fabs(DAT_10836470[modelIdx].x) < TOSS_STOP_SPEED && fabs(DAT_10836470[modelIdx].y) < TOSS_STOP_SPEED
                                         && fabs(DAT_10836470[modelIdx].z) < TOSS_STOP_SPEED)
                                        || sweepTry == TOSS_SWEEP_COUNT
                                        || DAT_10db6ee8[modelIdx].matrix.m[MATRIX_TRANSLATION_Z] > TOSS_CEILING
                                        || DAT_10db6ee8[modelIdx].matrix.m[MATRIX_TRANSLATION_X] < TOSS_FLOOR_X
                                        || (DAT_10467ba0[modelIdx] != 0 && DAT_10210d30[DAT_10467ba0[modelIdx]] != ENEMY_TOSSED))
                                    {
                                        // the toss ends: the model bursts into pieces
                                        DAT_10db6ee8[modelIdx].HoldAtSpine(0, 0);
                                        probe.Copy(DAT_10db6ee8[modelIdx].matrix.GetTranslation());
                                        Vec4 pieceDirection;
                                        pieceDirection.Copy(probe);
                                        pieceDirection.z = pieceDirection.z + TOSS_PIECE_LIFT;
                                        pieceDirection.Copy(pieceDirection - DAT_10836fc8[modelIdx]);
                                        BurstIntoPieces(probe, &pieceDirection, modelIdx);
                                        DAT_10210d30[modelIdx] = ENEMY_DYING;
                                        break;
                                    }
                                    DAT_10db6ee8[modelIdx].holdState = 1;
                                    DAT_10db6ee8[modelIdx].holdAngle = DAT_10db6ee8[modelIdx].holdAngle + TOSS_SPIN;
                                    EffectOptions trailOptions;
                                    if (DAT_104c0173 == 0)
                                    {
                                        trailOptions.m_0004 = DAT_10daaae8[0].stats[STAT_EFFECT_PARAM];
                                        trailOptions.m_0008 = 1.5f;
                                    }
                                    else
                                    {
                                        trailOptions.m_0004 = -DAT_10daaae8[0].stats[STAT_EFFECT_PARAM];
                                        trailOptions.m_0008 = 2.25f;
                                    }
                                    trailOptions.m_001c = 1;
                                    trailOptions.m_0000 = 1;
                                    trailOptions.settings[4] = 1;
                                    trailOptions.settings[5] = 1;
                                    trailOptions.settings[7] = (char)modelIdx;
                                    trailOptions.m_000e = 5;
                                    trailOptions.settings[0] = 8;
                                    trailOptions.settings[1] = 8;
                                    FUN_10093cc0(probe.x, probe.y, probe.z, DAT_10db6ee8[modelIdx].colliderReach + 10.0f, &trailOptions);
                                }
                            }
                            else
                            {
                                if (enemyState == ENEMY_DYING)
                                    DAT_10210d30[modelIdx] = ENEMY_FADE_START;
                                if (enemyState == ENEMY_DYING || enemyState > ENEMY_FADE_START)
                                {
                                    // fading out: the colour turns red and the model dims until it is moved away
                                    DAT_10210d30[modelIdx]++;
                                    DAT_10db6ee8[modelIdx].alpha = (float)(ENEMY_FADE_END - DAT_10210d30[modelIdx]) / FADE_ALPHA_DIVISOR;
                                    DAT_10db6ee8[modelIdx].m_022c.Set(1.0f, 0.2f, 0.2f, 0);
                                    if (DAT_10210d30[modelIdx] >= ENEMY_FADE_END)
                                    {
                                        DAT_10db6ee8[modelIdx].m_022c.Fill(0.0f);
                                        DAT_10daaae8[modelIdx].m_00a8 = 0;
                                        DAT_10210d30[modelIdx] = ENEMY_GONE;
                                        DAT_10db6ee8[modelIdx].Teleport(GONE_X - (float)modelIdx * GONE_SPACING, 0, 0);
                                        DAT_1020d4c0[modelIdx] = 0;
                                    }
                                    if (DAT_10315350[modelIdx] == 0)
                                    {
                                        if (DAT_10daaae8[modelIdx].kind >= 100)
                                            FUN_101148b0(modelIdx);
                                        else
                                            FUN_100f9670(modelIdx);
                                    }
                                }
                                else if (DAT_104bfdf0 == 0 && enemyState == 0
                                         && (DAT_102550e0[modelIdx] == 0 || DAT_102239d0[modelIdx] == 0)
                                         && DAT_10daaae8[modelIdx].stats[STAT_HEALTH] < 1)
                                {
                                    // out of health: the death starts on the next step
                                    DAT_10210d30[modelIdx] = ENEMY_DEFEATED;
                                    DAT_10315350[modelIdx] = 0;
                                }
                            }
                            if (DAT_10210d30[modelIdx] != ENEMY_DEFEATED)
                            {
                                if (DAT_104bfe1c == 0 && DAT_10210d30[modelIdx] == 0)
                                {
                                    if (DAT_104686b0[modelIdx] != 0)
                                        DAT_104686b0[modelIdx]--;
                                    if (DAT_1020d4c0[modelIdx] != 0)
                                    {
                                        // the hurt colour steps from dark grey to red as the display state counts down
                                        if (DAT_102239d0[modelIdx] == 0)
                                        {
                                            if (DAT_1020d4c0[modelIdx] == 1)
                                                DAT_10db6ee8[modelIdx].m_022c.Set(0.2f, 0.2f, 0.2f, 0);
                                            if (DAT_1020d4c0[modelIdx] == 2)
                                                DAT_10db6ee8[modelIdx].m_022c.Set(0.5f, 0.5f, 0.5f, 0);
                                            if (DAT_1020d4c0[modelIdx] == 3)
                                                DAT_10db6ee8[modelIdx].m_022c.Set(0.8f, 0.8f, 0.8f, 0);
                                            if (DAT_1020d4c0[modelIdx] == 4)
                                                DAT_10db6ee8[modelIdx].m_022c.Set(1.0f, 1.0f, 1.0f, 0);
                                            if (DAT_1020d4c0[modelIdx] == 5)
                                                DAT_10db6ee8[modelIdx].m_022c.Set(1.0f, 0.2f, 0.2f, 0);
                                            if (DAT_1020d4c0[modelIdx] == 6)
                                                DAT_10db6ee8[modelIdx].m_022c.Set(1.0f, 0, 0, 0);
                                        }
                                        char displayState = DAT_1020d4c0[modelIdx];
                                        DAT_1020d4c0[modelIdx] = displayState - 1;
                                        if (DAT_102239d0[modelIdx] == 0 && (char)(displayState - 1) == 0)
                                            DAT_10db6ee8[modelIdx].m_022c.Fill(0.0f);
                                    }
                                    Vec4 playerAt;
                                    Vec4 enemyAt;
                                    playerAt.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
                                    enemyAt.Copy(DAT_10db6ee8[modelIdx].matrix.GetTranslation());
                                    float distance = (enemyAt - playerAt).Length();
                                    if (DAT_104bfdf0 == 0 && modelIdx == DAT_102024ac && DAT_10db6ee8[modelIdx].headNode != 0)
                                    {
                                        closestEnemy = distance;
                                        DAT_10802ac8.Copy(DAT_10db6ee8[modelIdx].headNode->parent->worldMatrix.GetTranslation());
                                        enemyClose = 1;
                                    }
                                    if (DAT_10315350[modelIdx] == 0
                                        && (DAT_10254310[modelIdx] != 0 || DAT_10255a88[modelIdx] == 0 || DAT_102550e0[modelIdx] == 0)
                                        && (DAT_10219770[modelIdx] == 0 || (DAT_104bf890 & 1) == 0))
                                        goto dispatchByKind;
                                }
                                goto modelTail;
                            }
                            if (DAT_1021f9c0[modelIdx] == 0)
                            {
                                if (DAT_10315350[modelIdx] == 0)
                                    goto dispatchByKind;
                                goto modelTail;
                            }
                            {
                                // the enemy is defeated: it is thrown at the nearest other enemy in front of the player
                                DAT_10874848.MarkKilled(DAT_10daaae8[modelIdx].routeRecord);
                                DAT_10210d30[modelIdx] = ENEMY_TOSSED;
                                DAT_10210b30[modelIdx] = 1;
                                DAT_10db6ee8[modelIdx].root->M_10194730(0, -1);
                                Vec4 here;
                                here.Copy(DAT_10db6ee8[modelIdx].matrix.GetTranslation());
                                Vec4 playerPos;
                                playerPos.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
                                int bestDistance = TOSS_TARGET_RANGE;
                                int bestTarget = 0;
                                Vec4 toTarget;
                                Vec4 targetDirection;
                                Vec4 aim;
                                aim.Copy(DAT_10db6ee8[modelIdx].matrix.GetTranslation() - playerPos);
                                aim.Normalized3();
                                for (int target = FIRST_ENEMY_SLOT; target < TOSS_LAST_TARGET_SLOT; target++)
                                {
                                    if (DAT_104bfe30 == 0 && (DAT_1021f3c0[target] == TOSS_TARGET_KIND_A || DAT_1021f3c0[target] == 3)
                                        && DAT_10daaae8[target].stats[STAT_BUSY] == 0 && DAT_10daaae8[target].stats[STAT_HEALTH] > 0)
                                    {
                                        toTarget.Copy(DAT_1020e338[target]->GetTranslation() - playerPos);
                                        int targetDistance = (int)toTarget.Length();
                                        targetDirection.Copy(toTarget);
                                        targetDirection.Normalized3();
                                        if (DAT_10daaae8[target].m_0141 == 0 && targetDistance < bestDistance
                                            && Vec3_Dot(&aim, &targetDirection) > TOSS_TARGET_ALIGNMENT)
                                        {
                                            bestDistance = targetDistance;
                                            bestTarget = target;
                                        }
                                    }
                                }
                                Vec4 &tossDirection = DAT_10836470[modelIdx];
                                if (bestTarget == 0)
                                {
                                    tossDirection.Copy(here - DAT_10db6ee8[0].matrix.GetTranslation());
                                    tossDirection.Normalized3();
                                    if (tossDirection.x == 0 && tossDirection.y == 0)
                                        tossDirection.x = TOSS_NUDGE;
                                    tossDirection.Normalized3();
                                }
                                else
                                {
                                    tossDirection.Copy(DAT_10db6ee8[bestTarget].matrix.GetTranslation() - here);
                                    tossDirection.Normalized3();
                                }
                                char carrier = DAT_10467ba0[modelIdx];
                                if (carrier == DEFEATED_FROM_ABOVE)
                                {
                                    DAT_10467ba0[modelIdx] = 0;
                                    tossDirection.Set(0, 0, -1.0f, 0);
                                    DAT_10db6ee8[modelIdx].HoldAtSpine(1, TOSS_HOLD_WEIGHT);
                                    DAT_10db6ee8[modelIdx].FaceAwayFrom(&DAT_10db6ee8[0]);
                                    DAT_10467510[modelIdx] = 1;
                                    DAT_1030c480[modelIdx] = 1;
                                }
                                else if (carrier == DEFEATED_BURST)
                                {
                                    DAT_10467ba0[modelIdx] = 0;
                                    DAT_10db6ee8[modelIdx].HoldAtSpine(0, 0);
                                    here.Copy(DAT_10db6ee8[modelIdx].matrix.GetTranslation());
                                    Vec4 pieceDirection;
                                    pieceDirection.Copy(here);
                                    pieceDirection.z = pieceDirection.z + TOSS_PIECE_LIFT;
                                    pieceDirection.Copy(pieceDirection - DAT_10836fc8[modelIdx]);
                                    BurstIntoPieces(here, &pieceDirection, modelIdx);
                                    // two more passes of the last five pieces
                                    for (int again = 0; again < 2; again++)
                                    {
                                        FUN_1000df10(here.x - 5.0f, here.y + 10.0f, here.z + 55.0f, &pieceDirection, 100, modelIdx);
                                        FUN_1000df10(here.x + 10.0f, here.y - 5.0f, here.z + 55.0f, &pieceDirection, 100, modelIdx);
                                        FUN_1000df10(here.x - 5.0f, here.y - 10.0f, here.z + 40.0f, &pieceDirection, 100, modelIdx);
                                        FUN_1000df10(here.x + 4.0f, here.y + 7.0f, here.z + 55.0f, &pieceDirection, 100, modelIdx);
                                        FUN_1000df10(here.x + 10.0f, here.y - 10.0f, here.z + 65.0f, &pieceDirection, 100, modelIdx);
                                    }
                                    DAT_10210d30[modelIdx] = ENEMY_DYING;
                                }
                                else if (carrier < 1 || carrier >= TOSS_CARRIER_LIMIT)
                                {
                                    if (carrier == 0)
                                    {
                                        DAT_10db6ee8[modelIdx].HoldAtSpine(1, TOSS_HOLD_WEIGHT);
                                        DAT_10db6ee8[modelIdx].FaceAwayFrom(&DAT_10db6ee8[0]);
                                        DAT_10467510[modelIdx] = 1;
                                        DAT_1030c480[modelIdx] = 1;
                                    }
                                    else
                                    {
                                        DAT_10467ba0[modelIdx] = 0;
                                        DAT_10db6ee8[modelIdx].HoldAtSpine(0, 0);
                                        here.Copy(DAT_10db6ee8[modelIdx].matrix.GetTranslation());
                                        Vec4 pieceDirection;
                                        pieceDirection.Copy(here);
                                        pieceDirection.z = pieceDirection.z + TOSS_PIECE_LIFT;
                                        pieceDirection.Copy(pieceDirection - DAT_10836fc8[modelIdx]);
                                        BurstIntoPieces(here, &pieceDirection, modelIdx);
                                        DAT_10210d30[modelIdx] = ENEMY_DYING;
                                    }
                                }
                                else
                                {
                                    // a model of slot `carrier` holds it: it rides along and counts a hit for the carrier
                                    DAT_10db6ee8[modelIdx].Hold(1, TOSS_HOLD_WEIGHT, &DAT_10db6ee8[carrier]);
                                    DAT_10db6ee8[modelIdx].FaceAwayFrom(&DAT_10db6ee8[0]);
                                    DAT_10467510[modelIdx] = 1;
                                    DAT_1030c480[carrier]++;
                                    DAT_108387c8[modelIdx].Copy(DAT_10db6ee8[carrier].matrix.GetTranslation()
                                                                - DAT_10db6ee8[modelIdx].matrix.GetTranslation());
                                    DAT_104c0114 = DAT_104c0114 + 1;
                                    if (DAT_104c0114 > COMBO_MAX)
                                        DAT_104c0114 = COMBO_MAX;
                                    DAT_104c011a = COMBO_SHOW_A;
                                    DAT_104c011b = COMBO_SHOW_B;
                                }
                                FUN_1003ad20(SOUND_ENEMY_DEFEATED, &DAT_10836fc8[modelIdx], 1.0f);
                                FUN_100615f0(modelIdx, 1, 0);
                                FUN_100615f0(modelIdx, 1, 0);
                                FUN_100615f0(modelIdx, 1, 0);
                                // the coins it drops: scaled by the bonuses and the game overs so far
                                int coins = DAT_10daaae8[modelIdx].coins;
                                if (DAT_10daaae8[modelIdx].stats[STAT_COIN_BONUS] != 0)
                                    coins = (int)(coins * COIN_BONUS_FACTOR);
                                if ((DAT_104c02c8 & COIN_FLAGS_BOTH) == COIN_FLAGS_BOTH)
                                    coins = (int)(coins * COIN_FACTOR_BOTH);
                                else if ((DAT_104c02c8 & COIN_FLAG_DOUBLE) != 0)
                                    coins = (int)(coins * 2.0f);
                                else if ((DAT_104c02c8 & COIN_FLAG_BONUS) != 0)
                                    coins = (int)(coins * COIN_BONUS_FACTOR);
                                int dropped = ((COIN_GAME_OVER_LIMIT - DAT_10c64000 > 0 ? COIN_GAME_OVER_LIMIT - DAT_10c64000 : 0) * coins) / COIN_GAME_OVER_LIMIT;
                                if (dropped < 1)
                                    dropped = 1;
                                while (dropped != 0)
                                {
                                    if (dropped >= 500)
                                    {
                                        FUN_1000dd90(here.x, here.y, here.z + 50.0f, 0x56);
                                        dropped -= 500;
                                    }
                                    else if (dropped >= 100)
                                    {
                                        FUN_1000dd90(here.x, here.y, here.z + 50.0f, 0x55);
                                        dropped -= 100;
                                    }
                                    else if (dropped >= 0x32)
                                    {
                                        FUN_1000dd90(here.x, here.y, here.z + 50.0f, 0x54);
                                        dropped -= 0x32;
                                    }
                                    else if (dropped >= 10)
                                    {
                                        FUN_1000dd90(here.x, here.y, here.z + 50.0f, 0x53);
                                        dropped -= 10;
                                    }
                                    else if (dropped >= 5)
                                    {
                                        FUN_1000dd90(here.x, here.y, here.z + 50.0f, 0x52);
                                        dropped -= 5;
                                    }
                                    else if (dropped > 0)
                                    {
                                        FUN_1000dd90(here.x, here.y, here.z + 50.0f, 0x51);
                                        dropped--;
                                    }
                                }
                                FUN_1013f880();
                                if (DAT_10315350[modelIdx] != 0)
                                    goto modelTail;
                            }
                        }
                        else
                        {
char slotKind = DAT_1021f3c0[modelIdx];
if (slotKind == 0x16 || slotKind == 0x0f)
    DAT_10db6ee8[modelIdx].root->M_1019b050(0x27);
else if (slotKind == 'E')
{
    // a prop the player can talk to: it steps forward and back (phases 201..400) when the player stands in one of two spots beside it
    if (DAT_104bfdf0 == 0)
    {
        if (DAT_104c0068 == 0 && DAT_10db6ee8[modelIdx].phase == 0)
        {
            Vec4 propAt = DAT_10db6ee8[modelIdx].matrix.GetTranslation();
            float distanceSquared = DAT_10db6ee8[0].matrix.GetTranslation().DistanceSquared(propAt);
            if (DAT_102024b4 != 0 && DAT_1020b430[modelIdx].sawPlayer == 0 && DAT_104bfdc8 == 0
                && distanceSquared < PROP_TALK_DISTANCE_SQUARED)
            {
                DAT_10db6ee8[0].PlayMotion(2, 1, 5, 1.0f);
                FUN_100114a0();
                DAT_1020b430[modelIdx].behaviour = DAT_10db6ee8[modelIdx].phase + 1;
                DAT_104c0068 = 1;
                CloseEventOverlay();
                DAT_104c006c = modelIdx;
                DAT_10daaad4.Copy(DAT_10db6ee8[0].matrix.GetTranslation() - propAt);
                DAT_10db6ee8[modelIdx].phase = 1;
                DAT_10db6ee8[modelIdx].swayParam = 1;
                DAT_10db6ee8[modelIdx].swayFrames = 0;
            }
        }
        if (DAT_10db6ee8[modelIdx].phase >= PROP_BACK_FIRST)
        {
            // phase 301..400: backs away
            DAT_10db6ee8[modelIdx].phase = DAT_10db6ee8[modelIdx].phase + 1;
            Vec4 here = DAT_10db6ee8[modelIdx].matrix.GetTranslation();
            DAT_10db6ee8[modelIdx].SetPosition(here.x, here.y, here.z - PROP_STEP);
            if (DAT_10db6ee8[modelIdx].phase == PROP_BACK_END)
            {
                DAT_10db6ee8[modelIdx].phase = PROP_PHASE_IDLE;
                DAT_104c0068 = 0;
            }
        }
        else if (DAT_10db6ee8[modelIdx].phase >= PROP_FORWARD_FIRST)
        {
            // phase 201..300: steps forward
            DAT_10db6ee8[modelIdx].phase = DAT_10db6ee8[modelIdx].phase + 1;
            Vec4 here = DAT_10db6ee8[modelIdx].matrix.GetTranslation();
            DAT_10db6ee8[modelIdx].SetPosition(here.x, here.y, here.z + PROP_STEP);
            if (DAT_10db6ee8[modelIdx].phase == PROP_FORWARD_END)
            {
                DAT_10db6ee8[modelIdx].phase = PROP_PHASE_IDLE;
                DAT_104c0068 = 0;
            }
        }
        else if (DAT_10db6ee8[modelIdx].phase == PROP_PHASE_IDLE)
        {
            // idle: it leans away from the player (a tilt of its first child); a step starts when the player stands
            // in one of the two spots 500 units to either side
            DAT_10db6ee8[0].m_02a1 = 1;
            Vec4 away;
            Vec4 here = DAT_10db6ee8[modelIdx].matrix.GetTranslation();
            away.Copy(here - DAT_10db6ee8[0].matrix.GetTranslation());
            float awayScale = PROP_LEAN_SCALE;
            away.Copy(away * awayScale);
            DAT_10db6ee8[modelIdx].SetPosition(here.x - away.x, here.y - away.y, here.z);
            Matrix4 lean;
            float heading = (float)atan2(-away.x, -away.y);
            away.z = 0;
            lean = RotationX(away.Length() * PROP_LEAN_TILT);
            lean = RotationZ(-heading) * lean * RotationZ(heading);
            DAT_10db6ee8[modelIdx].root->children[0]->matrix = lean;
            DAT_1021b548[DAT_10db6ee8[modelIdx].attachedNode]->matrix = DAT_10db6ee8[modelIdx].matrix;
            Vec4 spotA;
            Vec4 spotB;
            spotA.Set(0, -PROP_SPOT_OFFSET, 0, 0);
            spotB.Set(0, PROP_SPOT_OFFSET, 0, 0);
            here.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
            here.z = 0;
            float distanceA = spotA.Distance(here);
            float distanceB = spotB.Distance(here);
            if (distanceA >= PROP_SPOT_RADIUS)
            {
                if (distanceB >= PROP_SPOT_RADIUS)
                    DAT_10db6ee8[modelIdx].swayFrames = 0;
                else if (DAT_10db6ee8[modelIdx].swayFrames == 0 && DAT_10db6ee8[modelIdx].swayParam > 0)
                {
                    DAT_10db6ee8[modelIdx].swayFrames = 1;
                    DAT_10db6ee8[modelIdx].phase = PROP_BACK_FIRST;
                    DAT_10db6ee8[modelIdx].swayParam = DAT_10db6ee8[modelIdx].swayParam - 1;
                    StartPropPull(modelIdx, here);
                }
            }
            else if (DAT_10db6ee8[modelIdx].swayFrames == 0 && DAT_10db6ee8[modelIdx].swayParam < 2)
            {
                DAT_10db6ee8[modelIdx].swayFrames = 1;
                DAT_10db6ee8[modelIdx].phase = PROP_FORWARD_FIRST;
                DAT_10db6ee8[modelIdx].swayParam = DAT_10db6ee8[modelIdx].swayParam + 1;
                StartPropPull(modelIdx, here);
            }
        }
        else if (DAT_10db6ee8[modelIdx].phase != 0)
        {
            // phase 1..92: after the player spoke to it it moves aside for a while, then goes idle
            DAT_10db6ee8[modelIdx].phase = DAT_10db6ee8[modelIdx].phase + 1;
            if (DAT_10db6ee8[modelIdx].phase < PROP_TALK_RISE_END)
            {
                Vec4 here = DAT_10db6ee8[modelIdx].matrix.GetTranslation();
                DAT_10db6ee8[modelIdx].SetPosition(here.x - PROP_TALK_STEP_X, here.y, here.z - PROP_TALK_STEP_Z);
            }
            if (DAT_10db6ee8[modelIdx].phase > PROP_TALK_END)
            {
                DAT_104c0068 = 0;
                DAT_10db6ee8[modelIdx].phase = PROP_PHASE_IDLE;
            }
            DAT_1021b548[DAT_10db6ee8[modelIdx].attachedNode]->matrix = DAT_10db6ee8[modelIdx].matrix;
        }
    }
    goto modelTail;
}
else if (slotKind == 'B')
{
    // a swaying prop that, once hit, rises, is dropped far below and settles on the ground
    Vec4 here = DAT_10db6ee8[modelIdx].matrix.GetTranslation();
    Matrix4 sway;
    sway.SetIdentity();
    if (DAT_10db6ee8[modelIdx].swayFrames != 0)
    {
        SwayProp(sway, DAT_10db6ee8[modelIdx].swayFrames, DAT_10db6ee8[modelIdx].swayParam, SWAY_DIVISOR_FLAG,
                         SWAY_FRAME_ANGLE);
        here.z = DAT_104bfe24 + FLAG_LIFT;
        EffectOptions options;
        options.flags = 1;
        if (DAT_10db6ee8[modelIdx].phase == 0 && DAT_10db6ee8[modelIdx].swayFrames > 1)
            DAT_10daaa80.Start(0xc80, here, -1, -1, 0, 0, 0, 0, &options, 0);
    }
    if (DAT_10db6ee8[modelIdx].phase != 0)
    {
        if (DAT_10db6ee8[modelIdx].phase < FLAG_FALL_END)
        {
            here.Copy(DAT_10db6ee8[modelIdx].matrix.GetTranslation());
            here.z = here.z + FLAG_RISE_STEP;
            DAT_10db6ee8[modelIdx].matrix.SetTranslation(here);
            DAT_10db6ee8[modelIdx].phase = DAT_10db6ee8[modelIdx].phase + 1;
        }
        if (DAT_10db6ee8[modelIdx].phase >= FLAG_FALL_END)
        {
            here.Copy(DAT_10db6ee8[modelIdx].matrix.GetTranslation());
            if (DAT_10db6ee8[modelIdx].phase == FLAG_FALL_END)
            {
                here.z = FLAG_DROP_HEIGHT;
                DAT_10db6ee8[modelIdx].matrix.SetTranslation(here);
                DAT_10db6ee8[modelIdx].root->children[0]->matrix = TranslationMatrix(0, 0, 0);
            }
            Vec4 probe;
            probe.Set(0, 0, FLAG_PROBE_HEIGHT, 0);
            if (DAT_10db6ee8[modelIdx].IsClearAny(&probe))
            {
                DAT_10db6ee8[modelIdx].matrix.SetTranslation(here + probe);
                DAT_10db6ee8[modelIdx].phase = DAT_10db6ee8[modelIdx].phase + 1;
            }
            if (DAT_10db6ee8[modelIdx].phase >= FLAG_PHASE_RESET)
                DAT_10db6ee8[modelIdx].phase = 0;
        }
    }
}
else if (slotKind == 'z')
{
    // a pot: it sways when hit; with no cutscene on it smashes into a debris model, scores and is parked away
    Matrix4 sway;
    sway.SetIdentity();
    if (DAT_10db6ee8[modelIdx].swayFrames != 0)
        SwayProp(sway, DAT_10db6ee8[modelIdx].swayFrames, DAT_10db6ee8[modelIdx].swayParam, SWAY_DIVISOR_POT,
                         SWAY_FRAME_ANGLE);
    DAT_10db6ee8[modelIdx].root->children[1]->matrix = sway;
    if (DAT_10db6ee8[modelIdx].swayFrames != 0 && DAT_104c01e0 == 0 && DAT_10c62ed0 != 0x16 && DAT_10c62ed0 != 0x17
        && DAT_10c62ed0 != 0x18 && DAT_10c62ed0 != 0x15 && DAT_10c62ed0 != 0x19 && DAT_10c62ed0 != 0x1a
        && DAT_10c62ed0 != 0x1b && DAT_10c62ed0 != 0x191)
    {
        DAT_1021f3c0[modelIdx] = (char)POT_SMASH_KIND;
        Vec4 smashAt = DAT_10db6ee8[modelIdx].matrix.GetTranslation();
        DAT_10874848.RegisterEvent(POT_SMASH_EVENT);
        Vec4 colour;
        colour.Set(DAT_10db6ee8[modelIdx].scale.x, DAT_10db6ee8[modelIdx].scale.y, DAT_10db6ee8[modelIdx].scale.z, 1.0f);
        DAT_10c64ad8 = DAT_10c64ad8 + 1;
        FUN_1000e750();
                            Vec4 soundAt = DAT_10db6ee8[modelIdx].matrix.GetTranslation();
                            FUN_1003ad20(POT_SMASH_SOUND, &soundAt, 1.0f);
        FUN_1003a880("O_061br.it3", smashAt.x, smashAt.y, smashAt.z, 0, 0, 0,
                     DAT_10db6ee8[modelIdx].targetHeading * 180.0f / 3.1415f, colour);
        DAT_10db6ee8[modelIdx].SetPosition(POT_HIDE_X, 0, 0);
    }
}
else if (slotKind == 'P')
{
    // a hanging prop: it sways when hit
    Matrix4 sway;
    sway.SetIdentity();
    if (DAT_10db6ee8[modelIdx].swayFrames != 0)
        SwayProp(sway, DAT_10db6ee8[modelIdx].swayFrames, DAT_10db6ee8[modelIdx].swayParam, SWAY_DIVISOR_POT,
                         SWAY_FRAME_ANGLE);
    DAT_10db6ee8[modelIdx].root->children[1]->matrix = sway;
}
else if (slotKind == ',')
{
    // a fire spitter next to the player: every 90th frame it starts a flame effect at its side
    if (DAT_104bfe1c == 0 && DAT_104c0094 == 0)
    {
        if (DAT_10db6ee8[modelIdx].m_0288 > 0)
            DAT_10db6ee8[modelIdx].m_0288--;
        Vec4 here = DAT_10db6ee8[modelIdx].matrix.GetTranslation();
        here.x = (float)cos(-DAT_10db6ee8[DAT_104bf8b4].targetHeading - LAMP_HEADING_SHIFT) * LAMP_TURN_OFFSET + here.x;
        here.y = (float)sin(-DAT_10db6ee8[DAT_104bf8b4].targetHeading - LAMP_HEADING_SHIFT) * LAMP_TURN_OFFSET + here.y;
        Matrix4 unusedPose;
        unusedPose.SetIdentity();
        DAT_10db6ee8[modelIdx].m_0294 = 1;
        DAT_10db6ee8[modelIdx].motionFrame = DAT_104bf8a4;
        if (DAT_10db6ee8[modelIdx].m_0284 != 0)
            DAT_10db6ee8[modelIdx].FaceAwayFrom(&DAT_10db6ee8[0]);
        if (DAT_104bf890 % LAMP_FIRE_PERIOD == 0)
        {
            float heading = DAT_10db6ee8[modelIdx].targetHeading;
            Vec4 muzzle;
            Matrix4 spin;
            EffectOptions options;
            spin = RotationZ(heading);
            muzzle.Fill(0.0f);
            muzzle.y = -40.0f;
            muzzle.z = 60.0f;
            muzzle.RotateZ(-heading);
            spin.SetTranslation(DAT_10db6ee8[modelIdx].matrix.GetTranslation());
            spin.m[MATRIX_TRANSLATION_X] += muzzle.x;
            spin.m[MATRIX_TRANSLATION_Y] += muzzle.y;
            spin.m[MATRIX_TRANSLATION_Z] += muzzle.z;
            muzzle.Fill(0.0f);
            options.m_0000 = 0;
            options.m_0004 = 10;
            if (DAT_10c635f8 == 1)
                options.m_0008 = 0.5f;
            if (DAT_10c635f8 == 2)
            {
                options.m_0008 = 2.0f;
                if (DAT_10c66cc0 == 1)
                    options.m_0008 = 3.0f;
                else if (DAT_10c66cc0 != 0)
                    options.m_0008 = 24.0f;
            }
            options.m_000e = 5;
            DAT_10daaa80.Start(0x762, muzzle, -1, -1, 0, 0, 8, &spin, &options, 0);
            DAT_10db6ee8[modelIdx].root->RestartColourPlugs("fire", DAT_104bf8a4);
        }
    }
}
else if (slotKind == 'Q')
{
    // a stall prop in front of the player: it sways when hit
    if (DAT_10db6ee8[modelIdx].m_0288 > 0)
        DAT_10db6ee8[modelIdx].m_0288--;
    Vec4 here = DAT_10db6ee8[modelIdx].matrix.GetTranslation();
    here.x = (float)cos(-DAT_10db6ee8[DAT_104bf8b4].targetHeading - LAMP_HEADING_SHIFT) * LAMP_TURN_OFFSET + here.x;
    here.y = (float)sin(-DAT_10db6ee8[DAT_104bf8b4].targetHeading - LAMP_HEADING_SHIFT) * LAMP_TURN_OFFSET + here.y;
    Matrix4 sway;
    sway.SetIdentity();
    if (DAT_10db6ee8[modelIdx].swayFrames != 0)
        SwayProp(sway, DAT_10db6ee8[modelIdx].swayFrames, DAT_10db6ee8[modelIdx].swayParam, SWAY_DIVISOR_STALL,
                 SWAY_FRAME_ANGLE_SLOW);
    DAT_10db6ee8[modelIdx].root->children[0]->matrix = sway;
    here.Distance(DAT_10db6ee8[modelIdx].matrix.GetTranslation());
}
else if (slotKind == 'M')
{
    // a floor button: it turns "down" when the player steps on it and "up" again when the player leaves; the
    // colour of its box shows whether it can be pressed
    if (DAT_10db6ee8[modelIdx].phase == 0)
    {
        float full = 1.0f;
        DAT_10db6ee8[modelIdx].scale.Fill(full);
    }
    else
        DAT_10db6ee8[modelIdx].scale.Set(BUTTON_DIM, BUTTON_DIM, BUTTON_DIM, 0);
    if (DAT_104c0068 == 0)
    {
        Vec4 buttonAt = DAT_10db6ee8[modelIdx].matrix.GetTranslation();
        float distanceSquared = DAT_10db6ee8[0].matrix.GetTranslation().DistanceSquared(buttonAt);
        if (DAT_10db6ee8[modelIdx].phase == 0 && DAT_102024b4 != 0 && DAT_1020b430[modelIdx].sawPlayer == 0
            && DAT_104bfdc8 == 0 && distanceSquared < BUTTON_TALK_DISTANCE_SQUARED)
        {
            DAT_1020b430[modelIdx].behaviour = DAT_10db6ee8[modelIdx].swayParam + 1;
            DAT_10db6ee8[0].PlayMotion(2, 1, 5, 1.0f);
            FUN_100114a0();
            DAT_104c006c = modelIdx;
            DAT_104c0068 = 1;
            CloseEventOverlay();
            DAT_10daaad4.Copy(DAT_10db6ee8[0].matrix.GetTranslation() - buttonAt);
            DAT_10db6ee8[modelIdx].root->Find("button", 0)->RestartColourPlugs("down", DAT_104bf8a4);
        }
    }
    FUN_100f9670(modelIdx);
    if (DAT_104c0068 == 2 && DAT_104c006c == modelIdx)
    {
        Vec4 buttonAt = DAT_10db6ee8[modelIdx].matrix.GetTranslation();
        float distanceSquared = DAT_10db6ee8[0].matrix.GetTranslation().DistanceSquared(buttonAt);
        if (BUTTON_LEAVE_DISTANCE_SQUARED < distanceSquared)
        {
            DAT_104c0068 = 0;
            DAT_10db6ee8[modelIdx].root->Find("button", 0)->RestartColourPlugs("up", DAT_104bf8a4);
        }
    }
    bool pressable;
    if (DAT_104c0068 == 1)
        pressable = true;
    else if (DAT_104c0068 == 2 && DAT_104c006c == modelIdx)
        pressable = false;
    else
        pressable = DAT_1020b430[modelIdx].behaviour == 0;
    int pressState = DAT_104c0068;
    DAT_10db6ee8[modelIdx].m_0266 = 0;
    const char *boxMotion;
    if (!pressable || DAT_102024b4 == 0 || DAT_1020b430[modelIdx].sawPlayer != 0)
        boxMotion = "stop";
    else if (pressState == 1 && DAT_104c006c == modelIdx)
        boxMotion = "red";
    else
        boxMotion = "blue";
    DAT_10db6ee8[modelIdx].root->Find("Box01", 0)->RestartColourPlugs(boxMotion, DAT_104bf8a4);
    DAT_10db6ee8[modelIdx].motionFrame = DAT_104bf8a4;
    if (DAT_10db6ee8[modelIdx].swayFrames == 0)
    {
        if (DAT_1020b430[modelIdx].behaviour != 0 && DAT_1020b430[modelIdx].heldDown == 0)
        {
            Vec4 soundAt = DAT_10db6ee8[modelIdx].matrix.GetTranslation();
            FUN_1003ad20(BUTTON_SOUND_DOWN, &soundAt, 1.0f);
        }
    }
    if (DAT_10db6ee8[modelIdx].swayFrames != 0)
    {
        if (DAT_1020b430[modelIdx].behaviour == 0 && DAT_1020b430[modelIdx].heldDown == 0)
        {
            Vec4 soundAt = DAT_10db6ee8[modelIdx].matrix.GetTranslation();
            FUN_1003ad20(BUTTON_SOUND_UP, &soundAt, 1.0f);
        }
    }
    if (DAT_1020b430[modelIdx].heldDown == 0)
        DAT_10db6ee8[modelIdx].swayFrames = DAT_1020b430[modelIdx].behaviour;
}
else if (slotKind != '\\')
{
    if (slotKind == 6)
    {
        // a prop that only marks whether the player stands under it
        DAT_10db6ee8[modelIdx].m_0012 = 2;
        if (DAT_104bf890 % PROP_SHADOW_PERIOD == modelIdx % PROP_SHADOW_STAGGER && DAT_102022b8 != 0)
            PlaceShadowOnGroundV2(modelIdx);
        MarkPlayerUnderProp(modelIdx);
    }
    else if (slotKind == '_')
    {
        // a pick-up prop: it is offered to the player, kept on the ground and put back where the player holds it
        float zero = 0;
        DAT_10db6ee8[modelIdx].velocity.Fill(zero);
        DAT_10db6ee8[modelIdx].m_02a0 = 1;
        if (DAT_104bfe4c == 0 && DAT_10db6ee8[0].m_238c != 0)
            OfferPickup(modelIdx);
        DAT_10db6ee8[modelIdx].m_0012 = 2;
        if (DAT_104bf890 % PROP_SHADOW_PERIOD == modelIdx % PROP_SHADOW_STAGGER && DAT_102022b8 != 0)
            PlaceShadowOnGroundV2(modelIdx);
        int carried = IsCarriedByHeld(modelIdx);
        MarkPlayerUnderProp(modelIdx);
        DAT_10db6ee8[modelIdx].root->M_1019b050(0x27);
        if ((DAT_104bfdd8 != modelIdx && carried == 0) || DAT_104bfdcc == 0)
            SnapPropToGround(modelIdx);
    }
    else if (slotKind == 5)
    {
        // a pick-up prop that sways when hit
        float zero = 0;
        DAT_10db6ee8[modelIdx].velocity.Fill(zero);
        DAT_10db6ee8[modelIdx].m_02a0 = 1;
        if (DAT_10db6ee8[modelIdx].m_0288 > 0)
            DAT_10db6ee8[modelIdx].m_0288--;
        Matrix4 sway;
        sway.SetIdentity();
        if (DAT_10db6ee8[modelIdx].m_0288 != 0)
            SwayProp(sway, DAT_10db6ee8[modelIdx].m_0288, DAT_10db6ee8[modelIdx].m_0284, SWAY_DIVISOR_SIGN, SWAY_FRAME_ANGLE);
        DAT_10db6ee8[modelIdx].root->children[0]->matrix = sway;
        if (DAT_104bfe4c == 0 && DAT_10db6ee8[0].m_238c != 0)
            OfferPickup(modelIdx);
        DAT_10db6ee8[modelIdx].m_0012 = 2;
        if (DAT_104bf890 % PROP_SHADOW_PERIOD == modelIdx % PROP_SHADOW_STAGGER && DAT_102022b8 != 0)
            PlaceShadowOnGroundV2(modelIdx);
        MarkPlayerUnderProp(modelIdx);
        int carried = IsCarriedByHeld(modelIdx);
        DAT_10db6ee8[modelIdx].root->M_1019b050(0x27);
        if ((DAT_104bfdd8 != modelIdx && carried == 0) || DAT_104bfdcc == 0)
            SnapPropToGround(modelIdx);
    }
    else
    {
        if (slotKind == 'F')
        {
            Vec4 buttonAt = DAT_10db6ee8[modelIdx].matrix.GetTranslation();
            float distanceSquared = DAT_10db6ee8[0].matrix.GetTranslation().DistanceSquared(buttonAt);
            if (DAT_1020b430[modelIdx].behaviour == 0 && DAT_10db6ee8[modelIdx].phase == 0 && DAT_102024b4 != 0
                && DAT_1020b430[modelIdx].sawPlayer == 0 && DAT_104bfdc8 == 0
                && distanceSquared < BUTTON_TALK_DISTANCE_SQUARED)
            {
                DAT_1020b430[modelIdx].behaviour = DAT_10db6ee8[modelIdx].swayParam + 1;
                if (DAT_10db6ee8[modelIdx].root->Find("button", 0) != 0)
                    DAT_10db6ee8[modelIdx].root->Find("button", 0)->RestartColourPlugs("down", DAT_104bf8a4);
            }
            goto dispatchBaseKind;
        }
        if (slotKind == 'N')
            FUN_100f9670(modelIdx);
        else if (slotKind == 2)
        {
            // a spout in front of the player: the player's target becomes it
            Vec4 ahead = DAT_10db6ee8[0].matrix.GetTranslation();
            ahead.x = (float)cos(-DAT_10db6ee8[DAT_104bf8b4].targetHeading - LAMP_HEADING_SHIFT) * SPOUT_AHEAD + ahead.x;
            ahead.y = (float)sin(-DAT_10db6ee8[DAT_104bf8b4].targetHeading - LAMP_HEADING_SHIFT) * SPOUT_AHEAD + ahead.y;
            if (DAT_10db6ee8[modelIdx].matrix.GetTranslation().Distance(ahead) < SPOUT_REACH)
            {
                DAT_104bf888 = modelIdx + TALK_TARGET_KEEP;
                if (DAT_104bfd80 == 0 && steerX == 0.0f && steerY == 0.0f)
                    DAT_104bfd80 = modelIdx;
                if (DAT_104bfdf0 == 0 && DAT_10db6ee8[modelIdx].headNode != 0)
                {
                    DAT_10802ac8.Copy(DAT_10db6ee8[modelIdx].headNode->parent->worldMatrix.GetTranslation());
                    enemyClose = 1;
                }
            }
            goto dispatchByKind;
        }
    }
}
else
{
    // a marker: while no other marker of the slots 0x5c..0x7b is within 45 units, the gimmick record of its
    // switch is closed (frame 0x1e) when the way is clear
    int savedTarget = DAT_104bf888;
    bool crowded = false;
    Vec4 here = DAT_10db6ee8[modelIdx].matrix.GetTranslation();
    if (DAT_104bfdcc == 0)
    {
        for (int other = MARKER_FIRST_SLOT; other <= MARKER_LAST_SLOT; other++)
        {
            if (other != modelIdx)
            {
                float distanceSquared = DAT_10db6ee8[other].matrix.GetTranslation().DistanceSquared(here);
                if (distanceSquared < MARKER_CLEAR_DISTANCE_SQUARED)
                {
                    DAT_104bf888 = DAT_10db6ee8[modelIdx].switchNode + 1;
                    if (DAT_10860cec[DAT_104bf888].frame == 0)
                        FUN_1005ecb0(0);
                    DAT_104bf888 = 0;
                    crowded = true;
                }
            }
        }
    }
    if (DAT_104bfdcc != 0 || !crowded)
    {
        DAT_104bf888 = DAT_10db6ee8[modelIdx].switchNode + 1;
        if (DAT_10860cec[DAT_104bf888].frame == MARKER_CLOSED_FRAME)
        {
            Vec4 probe;
            probe = here;
            probe.Set(0, 0, 1.0f, 0);
            if (DAT_10db6ee8[modelIdx].IsClearAny(&probe))
            {
                DAT_10860cec[DAT_104bf888].frame = MARKER_CLOSED_FRAME;
                FUN_1005ecb0(0);
            }
        }
    }
    DAT_104bf888 = savedTarget;
}
                            // dispatchBaseKind = LAB_10135df2; the code ends with `goto modelTail`)
                            goto modelTail;
                        }
                    dispatchByKind:
                        // the per-kind update of the slot (kinds from 100 have their own)
                        if (DAT_10daaae8[modelIdx].kind >= 100)
                            FUN_101148b0(modelIdx);
                        else
                        {
                        dispatchBaseKind:
                            FUN_100f9670(modelIdx);
                        }
                    }
                }
            modelTail:
                // the player's own updates (slot 0), or the other characters' last steps
                if (modelIdx == 0)
                {
                    if (DAT_104c0068 == 1)
                    {
                        if (DAT_1020b430[DAT_104c006c].behaviour == 0)
                            DAT_104c0068 = 2;
                        else
                        {
                            // sitting: the player follows the seat it sits on
                            DAT_10db6ee8[0].PlayMotion(2, 0, 5, 1.0f);
                            Vec4 seatAt = DAT_10db6ee8[DAT_104c006c].matrix.GetTranslation();
                            seatAt.Copy(seatAt + DAT_10daaad4);
                            DAT_10db6ee8[0].SetPosition(seatAt.x, seatAt.y, seatAt.z);
                        }
                    }
                    // face changes at the frames of the playing motion, and the idle blink
                    if (DAT_104bfe6c == 0 && DAT_104c00d0 == 0 && DAT_104bfdf0 == 0)
                    {
                        FaceCue *cue = DAT_108dbf58;
                        do
                        {
                            if (cue->frame == FACE_CUES_END)
                                break;
                            if (cue->frame == DAT_10db6ee8[0].motionFrame)
                                FUN_1000f360(0, cue->face);
                            cue++;
                        } while (cue < DAT_108dbf58 + FACE_CUE_COUNT);
                        if (DAT_10b314c0 == 0 && FUN_1013f880() % FACE_BLINK_PERIOD < 2)
                            FUN_1000f360(0, FACE_BLINK);
                    }
                    Vec4 unusedSpot;
                    if (DAT_104686b0[0] != 0)
                        DAT_104686b0[0]--;
                    // the colour of the player: flickers while the invincibility counter runs
                    unsigned int flickerPhase = DAT_104bf890 & 7;
                    if (DAT_10c66d00 == 0 || flickerPhase == 0)
                        DAT_10db6ee8[0].m_022c.Set(0, 0, 0, 0);
                    else if (flickerPhase == 1)
                        DAT_10db6ee8[0].m_022c.Set(0.1f, 0.1f, 0.1f, 0);
                    else if (flickerPhase == 2)
                        DAT_10db6ee8[0].m_022c.Set(0.2f, 0.2f, 0.2f, 0);
                    else if (flickerPhase == 3)
                        DAT_10db6ee8[0].m_022c.Set(0.4f, 0.4f, 0.4f, 0);
                    else if (flickerPhase == 4)
                        DAT_10db6ee8[0].m_022c.Set(0.5f, 0.5f, 0.5f, 0);
                    else if (flickerPhase == 5)
                        DAT_10db6ee8[0].m_022c.Set(0.4f, 0.4f, 0.4f, 0);
                    else if (flickerPhase == 6)
                        DAT_10db6ee8[0].m_022c.Set(0.2f, 0.2f, 0.2f, 0);
                    else if (flickerPhase == 7)
                        DAT_10db6ee8[0].m_022c.Set(0.1f, 0.1f, 0.1f, 0);
                    if (DAT_1020d4c0[0] != 0)
                    {
                        // the hurt colour steps from dark grey to white as the display state counts down
                        if (DAT_102239d0[0] == 0)
                        {
                            if (DAT_1020d4c0[0] == 1)
                                DAT_10db6ee8[0].m_022c.Set(0.2f, 0.2f, 0.2f, 0);
                            if (DAT_1020d4c0[0] == 2)
                                DAT_10db6ee8[0].m_022c.Set(0.5f, 0.5f, 0.5f, 0);
                            if (DAT_1020d4c0[0] == 3)
                                DAT_10db6ee8[0].m_022c.Set(0.8f, 0.8f, 0.8f, 0);
                            if (DAT_1020d4c0[0] == 4)
                                DAT_10db6ee8[0].m_022c.Set(1.0f, 1.0f, 1.0f, 0);
                        }
                        DAT_1020d4c0[0]--;
                        if (DAT_102239d0[0] == 0 && DAT_1020d4c0[0] == 0)
                            DAT_10db6ee8[0].m_022c.Fill(0.0f);
                    }
                    // the fall and landing motions of the player
                    if (DAT_104c0228 < SCENE_STATE_LIMIT && DAT_104bfe30 == 0 && DAT_104bfdc8 == 0 && DAT_104c0300 == 0 && DAT_104c00d0 == 0
                        && DAT_104bfe4c == 0 && DAT_104bfe6c == 0 && DAT_104c00e0 == 0 && DAT_104bfd60 == 0)
                    {
                        if (DAT_104c01e8 == 0 && DAT_10db6ee8[DAT_104bf8b4].M_1017b1c0() == 9 && DAT_10468128 == 0 && DAT_10db6ee8[0].m_229c == 0
                            && DAT_10db6ee8[0].m_238c != 0)
                        {
                            DAT_10db6ee8[0].matrix.GetTranslation();
                            if (DAT_104bfe78 == 0)
                                DAT_10db6ee8[DAT_104bf8b4].PlayMotion(7, 0, 5, 1.0f);
                        }
                        if (DAT_104687b0 != 0)
                            DAT_104c009c = 0;
                        if (DAT_104c01e8 == 0)
                        {
                            if (DAT_104687b0 != 0)
                                goto landingFlagsDone;
                            if (DAT_10db6ee8[0].m_238c != 0)
                            {
                                if (inputActive != 0)
                                {
                                    DAT_10db6ee8[0].matrix.GetTranslation();
                                    if (DAT_104c009c != LANDING_HARD)
                                    {
                                        if (DAT_104c009c == LANDING_FALL)
                                        {
                                            if (DAT_104bfe78 == 0)
                                                DAT_10db6ee8[DAT_104bf8b4].PlayMotion(8, 0, 5, 1.0f);
                                        }
                                        else if (moveInput == 0)
                                        {
                                            if (DAT_104bfe78 == 0)
                                            {
                                                if (DAT_1021a360 < LANDING_IDLE_FRAMES)
                                                    DAT_10db6ee8[DAT_104bf8b4].PlayMotion(0x22, 0, 2, 1.0f);
                                                else
                                                    DAT_10db6ee8[DAT_104bf8b4].PlayMotion(7, 1, 5, 1.0f);
                                            }
                                        }
                                        else if (DAT_104c009c != 0 && DAT_104bfe78 == 0)
                                            DAT_10db6ee8[DAT_104bf8b4].PlayMotion(0x1f, 1, 2, 1.0f);
                                    }
                                    DAT_1021a360 = 0;
                                    DAT_104c009c = 0;
                                    if (DAT_104c01e8 != 0)
                                        goto landingBounce;
                                }
                                goto landingFlagsDone;
                            }
                        airborneStart:
                            if (DAT_10db6ee8[0].m_237c < FALL_FAST_SPEED || inputActive == 0)
                                goto landingWatch;
                            if (DAT_104bfe78 == 0)
                                DAT_10db6ee8[DAT_104bf8b4].PlayMotion(0x20, 0, 5, 1.0f);
                            DAT_104c009c = LANDING_FALL;
                        }
                        else
                        {
                        landingBounce:
                            if (inputActive != 0 && DAT_10db6ee8[0].m_229c == 0)
                            {
                                DAT_104c01e8 = 0;
                                if (DAT_104bfe78 == 0)
                                    DAT_10db6ee8[DAT_104bf8b4].PlayMotion(2, 1, 3, 1.0f);
                            }
                        landingFlagsDone:
                            if (DAT_10db6ee8[0].m_238c == 0)
                                goto airborneStart;
                        landingWatch:
                            if (moveInput == 0)
                            {
                                if (DAT_10db6ee8[0].m_238c == 0 && !(DAT_10db6ee8[0].m_237c < FALL_IDLE_SPEED) && inputActive != 0)
                                {
                                    DAT_104c009c = LANDING_SOFT;
                                    if (DAT_104bfe78 == 0)
                                        DAT_10db6ee8[DAT_104bf8b4].PlayMotion(0x20, 0, 5, 1.0f);
                                }
                            }
                            else if (DAT_10db6ee8[0].m_238c == 0 && !(DAT_10db6ee8[0].m_237c < FALL_WALK_SPEED) && inputActive != 0)
                            {
                                DAT_104c009c = LANDING_SOFT;
                                if (DAT_104bfe78 == 0)
                                    DAT_10db6ee8[DAT_104bf8b4].PlayMotion(0x20, 0, 5, 1.0f);
                            }
                        }
                        // running off a ledge in the air: the stumble motion
                        if (DAT_104c01e8 == 0 && DAT_104687b0 != 0 && DAT_10db6ee8[0].m_238c != 0 && DAT_10db6ee8[0].m_2374 > STUMBLE_AIR_FRAMES
                            && DAT_10db6ee8[0].m_23e8.Length2D() > STUMBLE_SPEED_MIN && DAT_10db6ee8[0].move.Length2D() < STUMBLE_MOVE_MAX
                            && DAT_10db6ee8[DAT_104bf8b4].M_1017b1c0() != 1 && DAT_104bfdc8 == 0 && DAT_10db6ee8[0].m_238c != 0 && DAT_104bfd60 == 0
                            && DAT_104c00e0 == 0 && DAT_104bfdf0 == 0 && DAT_10468128 == 0 && DAT_10db6ee8[0].m_229c == 0
                            && DAT_10db6ee8[DAT_104bf8b4].m_2288 == 0 && DAT_104bfe78 == 0)
                            DAT_10db6ee8[DAT_104bf8b4].PlayMotion(0x22, 0, 2, 1.0f);
                        // a push against a wall (flag 5): the model turns away from it
                        if (DAT_10db6ee8[DAT_104bf8b4].m_2288 == 5 && DAT_104bfdf0 == 0 && DAT_104bfe1c == 0)
                        {
                            Vec4 wallSide;
                            wallSide.x = (float)cos(-DAT_10db6ee8[DAT_104bf8b4].targetHeading - PICKUP_HEADING_SHIFT);
                            wallSide.y = (float)sin(-DAT_10db6ee8[DAT_104bf8b4].targetHeading - PICKUP_HEADING_SHIFT);
                            wallSide.z = 0;
                            Vec4 pushDirection = DAT_10db6ee8[0].m_228c;
                            pushDirection.z = 0;
                            pushDirection.Normalized3();
                            if (DAT_104bfe78 == 0)
                                DAT_10db6ee8[DAT_104bf8b4].PlayMotion(Vec3_Dot(&wallSide, &pushDirection) <= 0.0f ? 8 : 0x5c, 0, 5, 1.0f);
                            DAT_10db6ee8[0].m_2374 = 0;
                        }
                        if (DAT_104687b0 != 0)
                            DAT_1021a360 = 0;
                        DAT_104687b0 = DAT_10db6ee8[0].m_238c;
                        if (DAT_10db6ee8[0].m_238c == 0)
                            DAT_1021a360++;
                    }
                    DAT_10db6ee8[0].m_23c8.Length2D();
                    DAT_104c01f0 = 0;
                    FUN_100157b0(0);
                    DAT_11b04508 = 1;
                    if (-50000.0f < DAT_10db6ee8[0].matrix.m[MATRIX_TRANSLATION_X])
                        DAT_10db6ee8[0].UpdateFrame(&DAT_108392d8, 1, 1, DAT_104bf8a4, &DAT_10836330);
                    DAT_11b04508 = 0;
                    if (DAT_102022b8 != 0)
                    {
                        // the shadow of the player is cast down from the model bank's light
                        Vec4 from;
                        Vec4 ground;
                        Vec4 top = DAT_10db6ee8[0].matrix.GetTranslation();
                        from.Copy(top);
                        top.z += SHADOW_GROUND_LIFT;
                        Vec4 direction = *DAT_10804bf8.LightDirection(&DAT_10db6ee8[0], &DAT_118e69e8, &DAT_118e6cb8);
                        direction.Normalized3();
                        from.Copy(from - direction * SHADOW_GROUND_DEPTH);
                        if (DAT_10255f08->CastSegment2(0, 0x4000000, &from, &top, &ground, 0) != 0)
                        {
                            ground.x = from.x;
                            ground.y = from.y;
                            DAT_10db6ee8[0].shadowMatrix.SetTranslation(ground);
                        }
                        else
                        {
                            ground.x = from.x;
                            ground.y = from.y;
                            ground.z = SHADOW_NO_GROUND;
                            DAT_10db6ee8[0].shadowMatrix.SetTranslation(ground);
                        }
                    }
                }
                else
                {
                    // a character that shares the previous slot's model copies its place and heading
                    if (DAT_10daaae8[modelIdx].m_00a0 == 1 && DAT_10daaae8[modelIdx - 1].m_00a8 != 0 && DAT_10daaae8[modelIdx - 1].m_00a4 == modelIdx)
                    {
                        DAT_10db6ee8[modelIdx].SetPosition(0, 0, 0);
                        DAT_10db6ee8[modelIdx].matrix = DAT_10db6ee8[modelIdx - 1].matrix;
                        DAT_10db6ee8[modelIdx].targetHeading = DAT_10db6ee8[modelIdx - 1].targetHeading;
                        DAT_10db6ee8[modelIdx].heading = DAT_10db6ee8[modelIdx - 1].heading;
                        DAT_10315350[modelIdx] = DAT_10315350[modelIdx - 1];
                    }
                    int savedPlaying = DAT_10db6ee8[modelIdx].trackA.playing;
                    if (DAT_104bfe1c != 0 && DAT_10db6ee8[modelIdx].m_0294 != 0)
                        DAT_10db6ee8[modelIdx].trackA.playing = 0;
                    if (DAT_10db6ee8[modelIdx].m_0267 == 0 && -50000.0f < DAT_10db6ee8[modelIdx].matrix.m[MATRIX_TRANSLATION_X])
                        FUN_100157b0(modelIdx);
                    int drawNow = (tick == ticks - 1 || DAT_1065cb90 != 0) && -50000.0f < DAT_10db6ee8[modelIdx].matrix.m[MATRIX_TRANSLATION_X];
                    DAT_104c01f0 = modelIdx;
                    if ((tick == ticks - 1 || DAT_1065cb90 != 0 || DAT_1021f3c0[modelIdx] != 3) && (DAT_10315350[modelIdx] == 0 || DAT_1021f3c0[modelIdx] != 3)
                        && (DAT_10219770[modelIdx] == 0 || (DAT_104bf890 & 1) == 0 || DAT_1021f3c0[modelIdx] != 3))
                    {
                        if (DAT_1021f3c0[modelIdx] == 3 && modelIdx >= 0x3a)
                        {
                            // an enemy is updated only when nothing held or its holder is this slot
                            if (DAT_104bfe1c == 0
                                && (DAT_104bfdf0 == 0 || DAT_102024b8 != -1 || DAT_102024ac == modelIdx
                                    || (DAT_10daaae8[modelIdx].m_00a0 != 0 && DAT_102024ac == modelIdx - 1)))
                                DAT_10db6ee8[modelIdx].UpdateFrame(&DAT_108392d8, drawNow, drawNow, DAT_104bf8a4, &DAT_10836330);
                        }
                        else
                            DAT_10db6ee8[modelIdx].UpdateFrame(&DAT_108392d8, 1, 1, DAT_104bf8a4, &DAT_10836330);
                    }
                    if (DAT_10315350[modelIdx] != 0)
                        DAT_10315350[modelIdx]--;
                    DAT_10db6ee8[modelIdx].trackA.playing = savedPlaying;
                }
            }
        }

        // the models that are drawn last (the player's double and the three beyond it)
        for (AnimModel *sceneModel = &DAT_10db6ee8[SCENE_DOUBLE_MODEL]; sceneModel < &DAT_10db6ee8[CHAR_SLOT_COUNT]; sceneModel++)
        {
            if (sceneModel > &DAT_10db6ee8[PLAYER_SLOT] && DAT_104bfe1c == 0 && DAT_104bfe6c == 0 && DAT_104bfeb0 == 0)
                sceneModel->UpdateFrame(&DAT_108392d8, 1, tick == ticks - 1 || DAT_1065cb90 != 0, DAT_104bf8a4, 0);
        }
        if (DAT_10255f08 != 0)
            DAT_10255f08->StartProximityPass(DAT_10836ec8.m, &DAT_108392d8, &DAT_10836330, DAT_104bf8a4, 0);
        if (tick == ticks - 1 || DAT_1065cb90 != 0)
        {
            DAT_104c00bc = 0;
            if (DAT_10255f0c != 0)
                DAT_10255f0c->StartProximityPass(DAT_10836ec8.m, &DAT_10836cd8, &DAT_10836330, DAT_104bf8a4, 0);
            DAT_108654f8.StartProximityPass(DAT_10836ec8.m, &DAT_108392d8, &DAT_10836330, 0, 0);
            Vec4 playerAt = DAT_10db6ee8[0].matrix.GetTranslation();
            if (DAT_104bfdd8 != 0 && DAT_104bfdd0 != 0 && DAT_104bfdc8 != 0)
            {
                DAT_10db6ee8[DAT_104bfdd8].root->M_1019b050(0x28);
                DAT_104c0098 = 1;
            }
            // the levers and chests: their frames run to the target, and the nearest one in reach is picked
            for (i = 0; i < GIMMICK_COUNT; i++)
            {
                GimmickSlot &gimmick = DAT_10860cec[i + 1];
                float reach;
                if (gimmick.node != 0 || gimmick.active != 0)
                {
                    if (gimmick.frame < gimmick.targetFrame)
                        gimmick.frame++;
                    if (gimmick.targetFrame < gimmick.frame)
                        gimmick.frame--;
                    if (DAT_1032d69c != 0)
                    {
                        if (DAT_104c00d0 == 0 && gimmick.owner != 0 && gimmick.type != GIMMICK_TYPE_MARKER)
                            gimmick.position.Copy(gimmick.owner->matrix.GetTranslation());
                        if (DAT_1032d69c != 0)
                        {
                            if (DAT_104c00d0 == 0 && DAT_104bfdf0 == 0 && gimmick.type == GIMMICK_TYPE_CHEST && gimmick.node != 0)
                            {
                                reach = playerAt.Distance(gimmick.node->worldMatrix.GetTranslation());
                                if (reach >= GIMMICK_PICK_REACH)
                                {
                                    if (reach < GIMMICK_LOOK_REACH && reach < closestEnemy)
                                    {
                                        closestEnemy = reach;
                                        DAT_10802ac8.Copy(gimmick.node->worldMatrix.GetTranslation());
                                        enemyClose = 1;
                                    }
                                    goto chestDone;
                                }
                                DAT_10802ac8.Copy(gimmick.node->worldMatrix.GetTranslation());
                                enemyClose = 1;
                                if (DAT_104bfd80 == 0 && steerX == 0.0f && steerY == 0.0f)
                                    DAT_104bfd80 = GIMMICK_PICK_MOTION;
                                gimmickFound = 1;
                                gimmick.node->M_1019b050(0x28);
                                DAT_104bf888 = i + 1;
                            gimmickPicked:
                                DAT_104c0098 = 2;
                                break;
                            }
                        chestDone:
                            if (DAT_1032d69c != 0)
                            {
                                if (DAT_104c00d0 == 0 && DAT_104bfdf0 == 0 && gimmick.type == GIMMICK_TYPE_LEVER && gimmick.node != 0)
                                {
                                    reach = playerAt.Distance(gimmick.node->worldMatrix.GetTranslation());
                                    if (reach < GIMMICK_PICK_REACH)
                                    {
                                        DAT_10802ac8.Copy(gimmick.node->worldMatrix.GetTranslation());
                                        enemyClose = 1;
                                        if (DAT_104bfd80 == 0 && steerX == 0.0f && steerY == 0.0f)
                                            DAT_104bfd80 = GIMMICK_PICK_MOTION;
                                        gimmickFound = 1;
                                        DAT_104bf888 = i + 1;
                                        gimmick.node->M_1019b050(0x28);
                                        goto gimmickPicked;
                                    }
                                    if (reach < GIMMICK_LOOK_REACH && reach < closestEnemy)
                                    {
                                        closestEnemy = reach;
                                        DAT_10802ac8.Copy(gimmick.node->worldMatrix.GetTranslation());
                                        enemyClose = 1;
                                    }
                                }
                                if (DAT_1032d69c != 0 && DAT_104c00d0 == 0 && gimmick.type == GIMMICK_TYPE_MARKER && gimmick.active != 0
                                    && playerAt.Distance(gimmick.position) < GIMMICK_PICK_REACH)
                                {
                                    DAT_104bf888 = i + GIMMICK_MARKER_BASE;
                                    gimmickFound = 1;
                                    goto gimmickPicked;
                                }
                            }
                        }
                    }
                }
            }
            DAT_11b07118.Copy(DAT_10802ac8);
            if (enemyClose != 0 && DAT_104bfe30 == 0 && DAT_104c0300 == 0 && DAT_104bfe4c == 0 && DAT_104c00e0 == 0)
            {
                if (DAT_104bfe6c == 0)
                    FUN_100165e0(1);
            }
            else if (DAT_104bfe6c == 0)
                FUN_100165e0(0);
            if (DAT_104bfd10 == 0 && gimmickFound == 0)
            {
                DAT_104bfd80 = 0;
                if (steerX == 0.0f && steerY == 0.0f)
                {
                    if (DAT_104bfd74 == 0 && DAT_104bfdac != 0)
                    {
                        if (DAT_104bfdb0 < IDLE_COUNT_MAX)
                            DAT_104bfdb0++;
                        if (DAT_104bfdb0 > IDLE_COUNT_RESET)
                            DAT_104bfdb0 = 0;
                    }
                }
                else
                {
                    if (DAT_104bfdb0 < IDLE_COUNT_MAX)
                        DAT_104bfdb0++;
                    if (DAT_104bfd74 != 4 || (DAT_104bfdb0 > WALK_COUNT_RESET && DAT_104bfdac != 0))
                        DAT_104bfdb0 = 0;
                }
            }
            if (DAT_104bfd60 == 0)
                DAT_104bfd74 = 2;
            playerAt.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
            if (DAT_104bfe1c == 0 && DAT_104c0028 == 0 && DAT_104bfdf0 == 0 && DAT_104bfe30 == 0 && DAT_104c0300 == 0 && DAT_104bfe4c == 0
                && DAT_104bfe6c == 0 && DAT_104bfeba == 0)
            {
                if (DAT_104bfd60 == 0 && DAT_104c0091 == 0)
                {
                    // the stand-by mode (4) starts when the ground ahead is higher than the player's feet or when nothing else is to do
                    if (DAT_1032d69c == 0 || DAT_104c00d0 != 0 || DAT_104c00f8 == 0)
                        goto notAtLedge;
                    {
                        float feet = playerAt.z - LEDGE_FEET_DROP;
                        if (FUN_10016700((int)(cos(-DAT_10db6ee8[DAT_104bf8b4].targetHeading - PICKUP_HEADING_SHIFT) * LEDGE_AHEAD
                                               + (playerAt.x - DAT_10edbd38.x * 2.0f)),
                                         (int)(sin(-DAT_10db6ee8[DAT_104bf8b4].targetHeading - PICKUP_HEADING_SHIFT) * LEDGE_AHEAD
                                               + (playerAt.y - DAT_10edbd38.y * 2.0f)))
                            <= feet)
                            goto notAtLedge;
                    }
                    DAT_104c00bc = 1;
                    goto startStandBy;
                notAtLedge:
                    if (DAT_102024ac > 0 || DAT_104c0190 != 0 || DAT_104c00e0 != 0 || DAT_1032d69c == 0 || DAT_104c00d0 != 0
                        || (DAT_104bf888 == 0 && (DAT_104c00ac == 0 || DAT_104c00b7 != 0)))
                        goto frameCountersDone;
                startStandBy:
                    if (DAT_104bfd74 != 4)
                    {
                        DAT_104c009b = 1;
                        DAT_10223a50 = (char)DAT_104bfd74;
                        DAT_1030b868 = 1;
                        DAT_104bfd74 = 4;
                    }
                }
                else if (DAT_104c0091 != 0)
                    DAT_104c0091--;
            }
        }
    frameCountersDone:
        if (DAT_104c023d != 0)
            DAT_104c023d--;
        if (DAT_10db6ee8[0].m_238c == 0)
        {
            DAT_104c01cc = 0;
            DAT_104c01c8 = 0;
            if (DAT_104c01d8 != 0)
                DAT_104c01d8--;
            DAT_10db6ee8[0].m_0074 = 0;
        }
        else
        {
            if (DAT_104bfe1c == 0 && DAT_104c025d != 0)
                DAT_104c025d--;
            if (DAT_104c01cc != 0)
                DAT_104c01cc--;
            if (DAT_104c01c8 != 0)
                DAT_104c01c8--;
            if (DAT_104c01d0 != 0)
                DAT_104c01d0--;
            if (DAT_104c01d4 != 0)
                DAT_104c01d4--;
            if (DAT_104c01d8 != 0)
            {
                DAT_104c01d8--;
                DAT_10db6ee8[0].m_0074 = 1;
                if (DAT_104c01d8 == 0 || DAT_10c635f8 == 1)
                    DAT_10db6ee8[0].m_0074 = 0;
            }
        }
        DAT_104b7200.MoveCursor();
        DAT_104b7200.UpdateWindows();
        if (DAT_104c0135 == 0)
        {
            DAT_104bf8a4 = DAT_104bf8a4 + 1;
            DAT_104bf8a8 = DAT_104bf8a8 + 1;
        }
        DAT_10db6ee8[0].matrix.GetTranslation();
        if (DAT_104bf8c0 == 1)
        {
            // the characters in view, sorted by their distance to the camera
            if (DAT_10255f08 != 0)
                DAT_10255f08->UpdateVisibleCells();
            float distances[CHAR_SLOT_COUNT];
            int visible = 0;
            for (i = 0; i < CHAR_SLOT_COUNT; i++)
            {
                if (DAT_10db6ee8[i].root != 0 && -10000.0f < DAT_10db6ee8[i].matrix.m[MATRIX_TRANSLATION_X])
                {
                    if (DAT_104bfe6c == 0)
                    {
                        if (i == 0)
                        {
                            if (DAT_10db6ee8[0].matrix.GetTranslation().DistanceSquared(DAT_118e6cb8) <= VIEW_FAR_SQUARED
                                && DAT_10db6ee8[0].matrix.GetTranslation().DistanceSquared(DAT_118e69e8) >= VIEW_NEAR_PLAYER_SQUARED
                                && FUN_10015710(0) != 0)
                                DAT_10db6ee8[0].m_2378 = 0;
                            else
                                DAT_10db6ee8[0].m_2378 = 1;
                        }
                        else
                        {
                            if (DAT_10db6ee8[i].matrix.GetTranslation().DistanceSquared(DAT_118e6cb8) > VIEW_FAR_SQUARED
                                || DAT_10db6ee8[i].matrix.GetTranslation().DistanceSquared(DAT_118e69e8) < VIEW_NEAR_SQUARED || FUN_10015710(i) == 0)
                                DAT_10db6ee8[i].m_2378 = 1;
                            else
                                DAT_10db6ee8[i].m_2378 = 0;
                        }
                    }
                    else
                        DAT_10db6ee8[i].m_2378 = 0;
                    DAT_10315140[visible] = i;
                    distances[visible] = DAT_118e69e8.DistanceSquared(DAT_10db6ee8[i].matrix.GetTranslation());
                    visible++;
                }
            }
            DAT_104c0054 = visible;
            FUN_10165020(distances, 0, 0, DAT_10315140, visible);
        }
        else
            DAT_104c0054 = 0;
        // scenery models near the camera that are hit by the view segment are marked
        for (i = SCENERY_FIRST_MODEL; i < SCENE_DOUBLE_MODEL; i++)
        {
            char sceneryKind = DAT_1021f3c0[i];
            if (sceneryKind == 0x13 || sceneryKind == 0x14 || sceneryKind == 'Q' || sceneryKind == 'R' || sceneryKind == 'S' || sceneryKind == '_'
                || sceneryKind == 5 || sceneryKind == 6)
            {
                if (DAT_10db6ee8[i].matrix.GetTranslation().DistanceSquared(DAT_118e6cb8) < SCENERY_NEAR_SQUARED
                    && DAT_10db6ee8[i].HitsSegment(&DAT_118e69e8, &DAT_118e6cb8) != 0)
                    DAT_10db6ee8[i].m_2378 = 2;
            }
        }
    }
prepareFrame:
    DAT_10804bf8.PrepareFrame(&DAT_10836ec8, &DAT_108392d8, DAT_1032d67c[0], 1);
    if (DAT_10467798 > LENS_FLARE_MIN_COUNT)
        DAT_10804bf8.AddFlarePoint(DAT_10864ce8, &DAT_108392d8);
    if (DAT_104bf890 != 0)
        Scene_DrawFrame(0);
    return 1;
}
