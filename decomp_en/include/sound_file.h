// File access of the sound engine (EN build; JP: sound3d.dll): a multimedia file handle that is either a plain file
// or a section of an archive. Declared once here.
#ifndef GURUMIN_EN_SOUND_FILE_H
#define GURUMIN_EN_SOUND_FILE_H

#include <windows.h>
#include <mmsystem.h>

// Largest block read from an archive at once; the reader sleeps between blocks so the game keeps running.
const int SOUND_FILE_READ_BLOCK = 0x80000;
// Milliseconds slept between two blocks.
const int SOUND_FILE_READ_SLEEP_MS = 2;

// A sound file opened through the multimedia I/O functions.
struct MmioFile
{
    HMMIO handle;       // 0x00: open file (0 when closed)
    int size;           // 0x04: size of the section inside an archive
    int isPlain;        // 0x08: non-zero for a plain file, 0 for a section of an archive
    int offset;         // 0x0c: start of the section inside the archive file

    // Destructor: closes the file when open. EN 0x7be960.
    ~MmioFile();
    // Closes the file when open. Returns 0. EN 0x7bf150.
    int Close();
    // Reads size * count bytes; from an archive in blocks with a short sleep in between. EN 0x7bf540.
    void Read(char *buffer, int size, int count);
    // Seeks within the file or section (origin 0 start, 1 current, other end). EN 0x7bf5d0.
    LONG Seek(LONG position, int origin);
    // Returns the current position, relative to the start of the section. EN 0x7bf630.
    int Tell();
};


// Destructor: closes the file when it is open. Called when a sound object is destroyed.
inline MmioFile::~MmioFile()
{
    if (handle != 0)
        mmioClose(handle, 0);
    handle = 0;
}

// Closes the file when it is open. Returns 0. Called when a sound is released.
inline int MmioFile::Close()
{
    if (handle != 0)
    {
        mmioClose(handle, 0);
        handle = 0;
    }
    return 0;
}

// Reads size * count bytes into `buffer`. A plain file is read at once; a section of an archive is read in blocks of
// SOUND_FILE_READ_BLOCK with a short sleep between them. Called while sounds load.
inline void MmioFile::Read(char *buffer, int size, int count)
{
    if (isPlain != 0)
    {
        mmioRead(handle, buffer, size * count);
        return;
    }
    int bytes = size * count;
    int done = 0;
    if (bytes > SOUND_FILE_READ_BLOCK)
    {
        unsigned blocks = ((unsigned)(bytes - (SOUND_FILE_READ_BLOCK + 1)) >> 19) + 1;
        bytes -= blocks * SOUND_FILE_READ_BLOCK;
        do
        {
            mmioRead(handle, buffer + done, SOUND_FILE_READ_BLOCK);
            done += SOUND_FILE_READ_BLOCK;
            Sleep(SOUND_FILE_READ_SLEEP_MS);
            blocks--;
        } while (blocks != 0);
    }
    mmioRead(handle, buffer + done, bytes);
}

// Seeks to `position` from the start (origin 0), the current position (1) or the end of the section. Plain files
// return the new position; archive sections return -1. Called while sounds load and loop.
inline LONG MmioFile::Seek(LONG position, int origin)
{
    if (isPlain != 0)
        return mmioSeek(handle, position, origin);
    int base;
    if (origin == 0)
        base = offset;
    else
    {
        if (origin == 1)
        {
            mmioSeek(handle, position, 1);
            return -1;
        }
        base = offset + size;
    }
    mmioSeek(handle, base + position, 0);
    return -1;
}

// Returns the current position, relative to the start of the section for an archive file. Called by the loaders.
inline int MmioFile::Tell()
{
    if (handle == 0)
        return 0;
    MMIOINFO info;
    mmioGetInfo(handle, &info, 0);
    if (isPlain == 0)
        info.lBufOffset -= offset;
    return info.lBufOffset;
}


#endif
