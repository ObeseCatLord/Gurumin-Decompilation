// The four command strings of the scene script reader (EN build): short strings of letter commands that the world
// update reads character by character. Declared here once.
#ifndef GURUMIN_EN_COMMAND_STRINGS_H
#define GURUMIN_EN_COMMAND_STRINGS_H

// Number of command strings and capacity of each (including the terminator).
const int COMMAND_STRING_COUNT = 4;
const int COMMAND_STRING_SIZE = 0x100;

// The command strings. EN 0x01664bc0.
extern "C" char g_commandStrings[COMMAND_STRING_COUNT][COMMAND_STRING_SIZE];
// Read position inside each command string (0 = start). EN 0x01664fc0.
extern "C" int g_commandStringCursors[COMMAND_STRING_COUNT];

// Replaces command string `index` (clamped to 0..3) with `text` and rewinds its read position. Returns 0. EN 0x746c10
// (command_strings.cpp). Called by the script frame update (EN 0x70c120).
extern "C" int CommandString_Set(int index, const char *text);

#endif
