// Scene command ids and registers of Scene_RunCommand (scene_command.cpp, 0x10083eb0). The script built-ins pass
// one of these ids with a number and a node or file name; the registers are the shared scratch values the
// position-reading commands fill and the placing commands read.
#ifndef GURUMIN_SCENE_COMMAND_H
#define GURUMIN_SCENE_COMMAND_H

// Command ids (Scene_RunCommand's first argument).
#define SCENE_CMD_SET_PANEL_LINE 0x2717            // panel line `arg` = the name text
#define SCENE_CMD_SET_PANEL_LINE_SUBSTITUTED 0x2718  // the same, with the %-substitutions of the message boxes
#define SCENE_CMD_LOAD_PICTURE 0x2719              // picture `arg` = the .iti or .bmp file `name`
#define SCENE_CMD_RELEASE_PICTURE 0x271a           // frees the texture of file `name` and the pictures using it
#define SCENE_CMD_SHOW_MESSAGE 0x4e20              // message window with the text `name` for `arg` frames
#define SCENE_CMD_OPEN_MESSAGE 0x4e21              // message box `arg` with the line `name`
#define SCENE_CMD_SHOW_MESSAGE_ALT 0x4e22          // message window of the alternative style, 30 frames
#define SCENE_CMD_LOAD_MAIN_MODEL 0x4e23           // loads model file `name` into model slot 0, remembers `arg`
#define SCENE_CMD_FOLLOW_FRAME_COUNTER 0x4e24      // node `name` shows the frame given by the frame counter
#define SCENE_CMD_GROUP_VISIBILITY 0x7533          // `arg` 0..3: flag bit 4 on/off for the nodes of the group register
#define SCENE_CMD_START_EFFECT_AT_NODE 0x7539      // effect `arg` at the position of node `name`
#define SCENE_CMD_SHOW_NODE 0x753a                 // node `name`: set flag 4, clear flag 0x20
#define SCENE_CMD_HIDE_NODE 0x753b                 // node `name`: clear flag 4, set flag 0x20
#define SCENE_CMD_COUNT_NODES 0x753c               // group register = number of nodes called `name`
#define SCENE_CMD_SET_NODE_GROUP 0x753d            // node `name` joins group `arg`
#define SCENE_CMD_SET_NODE_FLAGS 0x753e            // `arg` 0..3: flag 0x20 set/clear, flag 4 set/clear
#define SCENE_CMD_SELECT_NODE_INDEX 0x753f         // the `arg`-th node called `name`: set 0x20, clear 4
#define SCENE_CMD_SET_TITLE_CAPTION 0x7540         // title caption text
#define SCENE_CMD_SET_CARRIED_NODE 0x7541          // the carried object's node = node `name` (`arg` 0 = none)
#define SCENE_CMD_FOLLOW_NODE 0x7542               // the camera follows node `name` (`arg` 0 = stops)
#define SCENE_CMD_PLACE_EFFECT_MODEL_AT_PANEL_NODE 0x7543  // effect model `name` at the node named in panel line 0
#define SCENE_CMD_ADD_TEST_NODE 0x7544             // adds a model node `name` at the register position, turned
#define SCENE_CMD_RENAME_COLLISION_NODES 0x7545    // nodes called `name` become collision nodes ("LDC...")
#define SCENE_CMD_PLACE_EFFECT_MODEL 0x7546        // effect model `name` at the register position
#define SCENE_CMD_SET_ATTACHED_NODE 0x7547         // attached node `group register` of slot `arg` = node `name`
#define SCENE_CMD_START_EFFECT_AT_MODEL 0x7548     // effect `name` at the node of model `arg` (or its position)
#define SCENE_CMD_REMOVE_EFFECT_SLOT_NEAR_NODE 0x7549  // moves the type-14 effect slot at node `name` out of sight
#define SCENE_CMD_READ_NODE_POSITION 0x754a        // registers = position of node `name` (`arg` != 0: near flag)
#define SCENE_CMD_START_EFFECT_BEHIND_MODEL 0x9c40  // effect from `name` (a number) at the model `arg`, heading turned

// Model slots that hold map objects (CharSlot::kind 0x23 = ...).
#define SCENE_OBJECT_SLOT_FIRST 0x5c
#define SCENE_OBJECT_SLOT_END 0x7c

// Scratch registers shared by the commands: the group of a node, the integer position and the two turn angles.
extern "C" int DAT_10c63578;   // third coordinate register (and the camera follow point's z)
extern "C" int DAT_10c6357c;   // turn angle about z in degrees (SCENE_CMD_ADD_TEST_NODE)
extern "C" int DAT_10c63580;   // 1 while the last started effect of SCENE_CMD_START_EFFECT_AT_NODE exists
extern "C" int DAT_10db717c;   // set to 1 when the main model was loaded by a command
extern "C" int DAT_10db9250;   // cleared when the main model is loaded by a command

int Scene_RunCommand(int command, int arg, const char *name);   // 0x10083eb0

#endif
