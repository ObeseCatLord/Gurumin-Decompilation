// GameFile: the engine library's file object (0x101b2xxx), 0x28 bytes: the stdio handle and the base name of the open
// file. The constructor 0x101ad9e0 clears the handle (game_file.cpp), the destructor 0x101b2570 closes it.
#ifndef GURUMIN_GAME_FILE_H
#define GURUMIN_GAME_FILE_H

#include <stdio.h>

struct GameFile
{
    FILE *handle;                                    // 0x00: the open stdio file, 0 when closed
    char name[0x24];                                 // 0x04: base name of the open file (Path_GetBaseName 0x10161120)
    GameFile();                                      // 0x101ad9e0
    ~GameFile();                                     // 0x101b2570
    FILE *Open(const char *path, const char *mode);  // 0x101b2310: the stdio handle, 0 on failure
    void Close();                                    // 0x101b22f0
    int Size();                                      // 0x101b2370: the file size in bytes
    int Seek(int offset, int origin);                // 0x101b2380: fseek
    int Read(void *buffer, int size, int count);     // 0x101b23a0: fread
    int ReadPacked(void *buffer, int size, int count);    // 0x101b2450: u32 packed size, then ExecUnpack into `buffer`
    int ReadPackedImage(void *buffer, int width, int height);   // 0x101b24e0: u32 packed size, then the wavelet unpack of a width x height RGB image
    int AtEnd();                                     // 0x101b2360: the stdio end-of-file flag
    int Write(const void *buffer, int size, int count);   // 0x101b2420: fwrite
};

#endif
