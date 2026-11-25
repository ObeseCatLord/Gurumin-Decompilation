// Conversion of compressed sound files to wave files (EN build). The JP source file name (file_check.cpp) belongs to a
// different function; the EN bytes at this address are the ACM converter.
#include "sound_voice.h"
#include <stdio.h>

// Bytes of the compressed layout's file header (a wave header).
const int CONVERT_HEADER_SIZE = sizeof(WaveFileHeader);
// Compressed bytes per progress step of the file length.
const int CONVERT_STEP_BYTES = 0x5000;
// Progress counter increment per converted block (percent = counter / steps).
const int CONVERT_PROGRESS_UNIT = 100;

// SoundSource::ConvertFile: converts compressed sound file `path` to the wave file `outPath`: copies the 0x2c-byte
// header, then reads each block (compressed size, decoded size, data), runs it through the ACM stream and writes the
// decoded data. `progress` is told about every step. Returns 1, or 0 when a file cannot be opened. Called by the
// sound conversion tool.
int SoundSource::ConvertFile(const char *path, const char *outPath, SoundConvertProgress progress)
{
    progressCallback = progress;
    FILE *source = fopen(path, "rb");
    if (source == 0)
        return 0;
    FILE *target = fopen(outPath, "wb");
    if (target == 0)
        return 0;
    fseek(source, 0, SEEK_END);
    unsigned length = ftell(source);
    int steps = (length - CONVERT_HEADER_SIZE) / CONVERT_STEP_BYTES;
    fseek(source, 0, SEEK_SET);
    if (progressCallback)
        progressCallback(path, CONVERT_HEADER_SIZE, 0);
    WaveFileHeader header;
    fread(&header, 1, CONVERT_HEADER_SIZE, source);
    fwrite(&header, 1, CONVERT_HEADER_SIZE, target);
    int blockSize;
    if (fread(&blockSize, 1, 4, source) != 0)
    {
        int counter = 0;
        do
        {
            int decodedSize;
            if (fread(&decodedSize, 1, 4, source) == 0 || fread(acmSourceBuffer, 1, blockSize, source) == 0)
                break;
            acmStreamConvert(acmStreamA, &headerA, 0);
            fwrite(acmDestBuffer, 1, decodedSize, target);
            if (progressCallback)
                progressCallback(path, blockSize, counter / steps);
            counter += CONVERT_PROGRESS_UNIT;
        } while (fread(&blockSize, 1, 4, source) != 0);
    }
    fclose(source);
    fclose(target);
    return 1;
}
