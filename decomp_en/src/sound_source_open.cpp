// Opening of a sound source by the end of its file name (EN build; JP twin: sound3d.dll 10004860, not matched).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <new>

#include "sound_ogg.h"
#include "sound_voice.h"

// Bytes of the path buffers used for names that end with the wildcard character, and the characters that decide how a
// file is read (the last character of its name; digits and the upper and lower case letter share an entry).
const int SOUND_OPEN_PATH_BYTES = 0x200;
const char SOUND_SUFFIX_WILDCARD = '?';
const char SOUND_SUFFIX_KEYED = '2';
const char SOUND_SUFFIX_WAVE_LOWER = 'v';
const char SOUND_SUFFIX_WAVE_UPPER = 'V';
const char SOUND_SUFFIX_COMPRESSED_LOWER = 'c';
const char SOUND_SUFFIX_COMPRESSED_UPPER = 'C';
const char SOUND_SUFFIX_ACM = '3';
const char SOUND_SUFFIX_OGG_LOWER = 'g';
const char SOUND_SUFFIX_OGG_UPPER = 'G';
// Mode string the sound files are opened with, the two names a wildcard path is tried as, and the factor between the
// size of a plain file and the decoded size.
static const char SOUND_OPEN_MODE_READ[] = "rb";
static const char SOUND_WILDCARD_WAVE_FORMAT[] = "%swav";
static const char SOUND_WILDCARD_COMPRESSED_FORMAT[] = "%sdec";
const int SOUND_RAW_SIZE_FACTOR = 11;

// Opens the sound `path` and decides how it is read from the last character of its name: '?' tries the name with
// "wav" and then with "dec" instead, '2' a scrambled block, 'v' a wave file, 'c' the compressed layout, '3' an ACM
// source (kind only), 'g' an Ogg Vorbis file; any other name is read as a plain file whose size gives the decoded size.
// Returns 1 when the source could be opened, else 0. Called when a voice starts a sound.
int SoundSource::OpenSource(const char *path)
{
    isFileSource = 0;
    sourceKind = 0;
    isCompressed = 0;
    if (path == 0)
        return 0;
    if (path[strlen(path) - 1] == SOUND_SUFFIX_WILDCARD)
    {
        char base[SOUND_OPEN_PATH_BYTES];
        char name[SOUND_OPEN_PATH_BYTES];
        strcpy(base, path);
        base[strlen(base) - 1] = 0;
        sprintf(name, SOUND_WILDCARD_WAVE_FORMAT, base);
        if (OpenWave(name) != 0)
            return 1;
        sprintf(name, SOUND_WILDCARD_COMPRESSED_FORMAT, base);
        return OpenCompressed(name) != 0;
    }
    if (path[strlen(path) - 1] == SOUND_SUFFIX_KEYED || path[strlen(path) - 1] == SOUND_SUFFIX_KEYED)
        return OpenKeyed(path);
    if (path[strlen(path) - 1] == SOUND_SUFFIX_WAVE_LOWER || path[strlen(path) - 1] == SOUND_SUFFIX_WAVE_UPPER)
        return OpenWave(path);
    if (path[strlen(path) - 1] == SOUND_SUFFIX_COMPRESSED_LOWER || path[strlen(path) - 1] == SOUND_SUFFIX_COMPRESSED_UPPER)
        return OpenCompressed(path);
    if (path[strlen(path) - 1] == SOUND_SUFFIX_ACM || path[strlen(path) - 1] == SOUND_SUFFIX_ACM)
    {
        sourceKind = SOUND_SOURCE_ACM;
        return 0;
    }
    if (path[strlen(path) - 1] == SOUND_SUFFIX_OGG_LOWER || path[strlen(path) - 1] == SOUND_SUFFIX_OGG_UPPER)
    {
        sourceKind = SOUND_SOURCE_OGG;
        oggReader = new (malloc(sizeof(OggReader))) OggReader;
        if (oggReader->Open(path) == 0)
            return 0;
        dataSize = oggReader->totalBytes;
        m_0174 = 1;
        return 1;
    }
    m_0174 = 0;
    if (file.handle != 0)
        return 0;
    if (file.Open(path, SOUND_OPEN_MODE_READ) == 0)
        return 0;
    decodedPosition = 0;
    file.Seek(WAVE_HEADER_BYTES, SEEK_SET);
    file.Seek(0, SEEK_END);
    int length = file.Tell();
    m_0174 = 1;
    m_0188 = 1;
    dataSize = length * SOUND_RAW_SIZE_FACTOR;
    m_0184 = 0;
    return 1;
}
