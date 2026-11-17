// Model files (.it3): the pre-pass that reads the INFO/MAN2 chunks of a model file before loading it.

#include <stdio.h>

// File wrapper from the engine library (save_slot.cpp): constructor 0x101ad9e0, destructor 0x101b2570 closes it.
struct GameFile
{
    FILE *handle;
    char name[0x24];
    GameFile();
    ~GameFile();
    int Open(const char *path, const char *mode);  // 0x101b2310
    void Close();                                  // 0x101b22f0
};

struct Node
{
    int ScanChunks(GameFile *file, int arg);  // 0x101a4a60: It3_ScanChunks
    int PreScanFile(const char *path, int arg);
};

extern "C" int FUN_1013edb0(const char *format, ...);  // error report (empty in the release build)

// Opens model file `path` and runs the chunk pre-pass (Node::ScanChunks) on it. Returns 1, or 0 after reporting
// "3Dデータがない" ("no 3D data") when the file cannot be opened.
int Node::PreScanFile(const char *path, int arg)
{
    GameFile file;
    if (file.Open(path, "rb") != 0)
    {
        ScanChunks(&file, arg);
        file.Close();
        return 1;
    }
    FUN_1013edb0("3D\x83" "f\x81[\x83^\x82\xaa\x82\xc8\x82\xa2", path, 0);
    return 0;
}
