// FaceSet: declared here once (first used by src/face_anim.cpp).
#ifndef GURUMIN_FACE_SET_H
#define GURUMIN_FACE_SET_H

struct FaceSet
{
    int delay[0x40];          // frames to show each face before switching
    int next[0x40];           // following face (the same face = stay)
    char pad_0200[0x100];
    char names[0x40][0x10];   // 0x300: face texture names (indexed by character, see below)
};

#endif
