// FlagPair, FlagOwner: declared here once (first used by src/flag_list.cpp).
#ifndef GURUMIN_FLAG_LIST_H
#define GURUMIN_FLAG_LIST_H

struct FlagPair
{
    int id;
    int flag;
};
struct FlagOwner
{
    char pad_0000[0x3eda08];
    int marked[0x100];
    FlagPair pairs[0x100];
    int M_100020d0(int id);
};

#endif
