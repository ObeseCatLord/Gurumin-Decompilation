// MotionList: declared here once (first used by src/containers.cpp).
#ifndef GURUMIN_MOTION_LIST_H
#define GURUMIN_MOTION_LIST_H

// List of up to 0x7f motion ids plus up to 64 named entries (id, value, extra).
struct MotionList
{
    int m_0000;
    int m_0004;
    int count;
    int items[0x7f];
    int m_0208;
    int entryCount;
    char names[0x40][0x20];
    int ids[0x40];
    int values[0x40];
    int extras[0x40];
    int Insert(int position, int value, int tag);
    int AddEntry(int id, int value, const char *name, int extra);
    int Clear();
    int InsertByName(int position, const char *name, int tag);
    int FrameAt(int time, int *outItem, int *outEntry);
};

#endif
