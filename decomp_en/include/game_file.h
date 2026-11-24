// An open game data file (EN build; JP: decomp/include/game_file.h) and the path helpers the loaders use. This is the
// one declaration of GameFile in decomp_en: include this header, never redeclare it.
#ifndef GURUMIN_EN_GAME_FILE_H
#define GURUMIN_EN_GAME_FILE_H

#include <stdio.h>

// Characters of a file's base name kept in GameFile::name (including the terminator).
const int GAME_FILE_NAME_SIZE = 0x24;

struct GameFile
{
    FILE *handle;                        // 0x00: the open stdio file, 0 when closed
    char name[GAME_FILE_NAME_SIZE];      // 0x04: base name of the open file

    GameFile();                          // EN 0x7a8900 (JP 0x101ad9e0): no file open
    ~GameFile();                         // EN 0x7a8910 (JP 0x101b2570): closes the file when it is still open
    FILE *Open(const char *path, const char *mode);   // EN 0x7a8ac0 (JP 0x101b2310): the stdio handle, 0 on failure
    void Close();                        // EN 0x7a8aa0 (JP 0x101b22f0)
    int Seek(int offset, int origin);    // EN 0x7a8cb0: fseek on the open file
    int Size();                          // EN 0x7a8cd0: current file position (the length after a seek to the end)
    size_t Read(void *buffer, size_t size, size_t count);   // EN 0x7a8b20: fread from the open file
};

// Copies the file name of `path` without directory and extension into `out`. EN 0x7a8930 (JP 0x10161120).
extern "C" int Path_GetBaseName(char *out, const char *path);

// Error report of the loaders (empty in the release build): `title`, `detail` and a number. EN 0x77bf60 (JP 0x1013edb0).
extern "C" int ErrorReport(const char *title, const char *detail, int value);

#endif
