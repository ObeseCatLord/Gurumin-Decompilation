// The motions an animation plug plays (EN build). Only the methods used so far are declared; the data layout is
// not recovered yet, so sizeof(MotionList) is not the real size (it only ever appears at offset 4 of a Channel).
// This is the one declaration of MotionList and Channel in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_MOTION_LIST_H
#define GURUMIN_EN_MOTION_LIST_H

// Motion entries a MotionList holds.
const int MOTION_ENTRY_COUNT = 0x40;
// Bytes of one entry name, including the terminator.
const int MOTION_NAME_SIZE = 0x20;
// Items a MotionList holds (the timeline).
const int MOTION_ITEM_COUNT = 0x7f;

// Bit of MotionList::m_0004 that makes the list play its items in turn, and the entry extra bits that play an item
// backwards and that hold its last frame once the list has run out.
const int MOTION_LIST_PLAYS_ITEMS = 1;
const int MOTION_EXTRA_BACKWARDS = 1;
const int MOTION_EXTRA_HOLD_LAST = 2;

struct MotionList
{
    int m_0000;                                   // 0x000: not used by EN code yet
    int m_0004;                                   // 0x004: not used by EN code yet
    int count;                                    // 0x008: items in use
    int items[MOTION_ITEM_COUNT];                 // 0x00c: the timeline items
    int m_0208;                                   // 0x208: not used by EN code yet
    int entryCount;                               // 0x20c: entries in use
    char names[MOTION_ENTRY_COUNT][MOTION_NAME_SIZE];  // 0x210: entry names
    int ids[MOTION_ENTRY_COUNT];                  // 0xa10: entry ids
    int values[MOTION_ENTRY_COUNT];               // 0xb10: entry values
    int extras[MOTION_ENTRY_COUNT];               // 0xc10: entry extra values
    int AddEntry(int id, int value, const char *name, int extra);       // EN 0x76f9d0 (JP 0x101ad6a0): appends an entry; 0 when all are used
    int Clear();                                                       // EN 0x7722c0 (JP MotionList::Clear)
    int InsertByName(int position, const char *name, int tag);         // EN 0x76fad0 (JP MotionList::InsertByName)
    int FrameAt(int time, int *outItem, int *outEntry);                // EN 0x76fba0 (JP 0x101ad350): frame shown `time` frames after m_0208
};

// The block behind one animation plug of a node (Node::plugs[k]).
struct Channel
{
    int size;          // 0x00: size of the whole channel block in bytes
    MotionList keys;   // 0x04: the motions the plug plays (plugs 6 and 7 give the frame)
};

#endif
