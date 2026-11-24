// The motions an animation plug plays (EN build). Only the methods used so far are declared; the data layout is
// not recovered yet, so sizeof(MotionList) is not the real size (it only ever appears at offset 4 of a Channel).
// This is the one declaration of MotionList and Channel in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_MOTION_LIST_H
#define GURUMIN_EN_MOTION_LIST_H

struct MotionList
{
    int Clear();                                                       // EN 0x7722c0 (JP MotionList::Clear)
    int InsertByName(int position, const char *name, int tag);         // EN 0x76fad0 (JP MotionList::InsertByName)
};

// The block behind one animation plug of a node (Node::plugs[k]).
struct Channel
{
    int size;          // 0x00: size of the whole channel block in bytes
    MotionList keys;   // 0x04: the motions the plug plays (plugs 6 and 7 give the frame)
};

#endif
