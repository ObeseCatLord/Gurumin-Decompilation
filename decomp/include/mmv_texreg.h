// ClipRect, MovieFile: declared here once (first used by src/mmv_texreg.cpp).
#ifndef GURUMIN_MMV_TEXREG_H
#define GURUMIN_MMV_TEXREG_H

struct ClipRect
{
    int left;
    int top;
    int right;
    int bottom;
};
struct MovieFile
{
    void *handle;
    char name[0x20];
    MovieFile();
    ~MovieFile();
    void Close();
    int Read(void *buffer, int size, int count);   // 0x101b23a0
    int Seek(int offset, int origin);              // 0x101b2380
    int Open(const char *path, const char *mode);  // 0x101b2310: non-zero when the file was opened
    int Size();                                    // 0x101b2370
};

#endif
