// Face (expression) animation tables of the characters (EN build). Each character has 0x40 face frames (texture names) and a
// face set that gives, for each frame, how long it is shown and which frame follows.
#ifndef GURUMIN_EN_FACE_SET_H
#define GURUMIN_EN_FACE_SET_H

#include "char.h"

// Face frames of a character.
const int FACE_FRAME_COUNT = 0x40;
// Characters in a face texture name.
const int FACE_NAME_SIZE = 0x10;
// Costume number (g_guardedValue9) of the player whose faces come from the costume's own textures.
const int FACE_COSTUME_PLAYER = 6;
// Where the costume's face texture names differ from the plain ones: the two digits at these name positions become "14".
const int FACE_COSTUME_DIGIT_FIRST = 2;
const int FACE_COSTUME_DIGIT_SECOND = 3;

// A face set (0x700 bytes).
struct FaceSet
{
    int delay[FACE_FRAME_COUNT];                   // 0x000: frames to show each face before switching
    int next[FACE_FRAME_COUNT];                    // 0x100: the face that follows (the same face = stay)
    char m_0200[0x100];                            // 0x200: not used by EN code yet
    char names[FACE_FRAME_COUNT][FACE_NAME_SIZE];  // 0x300: face texture names (indexed by character, see Char_SetFace)
};

extern "C" FaceSet g_faceSets[];                          // EN 0x00f463f4: face sets (names are used by character number, delays and next faces by CharRecord::faceSet)
extern "C" short g_lastFaceFrame;                         // EN 0x01201268 (JP DAT_10b314c0): the face frame most recently shown
extern "C" int g_faceDelay[CHAR_SLOT_COUNT];              // EN 0x0091eb70 (JP DAT_1030c278): frames until a character's next face frame (0 = none)
extern "C" int g_faceNext[CHAR_SLOT_COUNT];               // EN 0x0091f010 (JP DAT_10447dd8): the face frame a character switches to next

// Shows face frame `frame` on character `index` (the player in costume FACE_COSTUME_PLAYER uses the costume's own face textures: character 0's names with "14" in positions 2 and 3; does nothing for a character without a face set). Returns 0. EN 0x5e7a20 (face_anim.cpp). Called by the script
// commands (EN 0x631890, 0x60cb10).
extern "C" int Char_SetFace(int index, int frame);

// Counts down the face delay of every character slot and, when one runs out, shows the character's next face frame.
// Returns 0. EN 0x5aa6b0 (face_anim.cpp). Called by the script commands that change faces (EN 0x631890, 0x60cb10).
extern "C" int Faces_Update(void);

#endif
