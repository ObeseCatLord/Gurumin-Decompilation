// Reader for the 12-byte trailer of the game's .itm files (docs/ITM_FORMAT.md). This is port code written from the
// format notes, not decompiled code. It does not descramble the body: the descrambling table lives in APP.dll and
// is not shipped with this repository.
#ifndef GURUMIN_PORT_ITM_H
#define GURUMIN_PORT_ITM_H

#include <cstdint>
#include <string>

namespace gurumin
{

// Content types stored in the trailer.
enum ItmContentType : int32_t
{
    ITM_CONTENT_BMP = 0,   // a BMP image (3ddata)
    ITM_CONTENT_TEXT = 1,  // scrambled text (bin/map.itm, some 3ddata tables)
    ITM_CONTENT_WAV = 3,   // a WAV sound (wav/)
};

struct ItmTrailer
{
    uint64_t fileSize = 0;     // whole file, trailer included
    uint64_t bodySize = 0;     // scrambled body (fileSize - 12)
    int32_t contentType = 0;   // ItmContentType (sound.itm is a different format and shows 13309 here)
    uint32_t plainSum = 0;     // sum of all plain (descrambled) bytes, mod 2^32
    uint32_t plainXor = 0;     // 0xDE2F xor every plain byte
};

// Reads the trailer of `path`. Returns false (with `error` set) when the file cannot be read or is too short.
bool ReadItmTrailer(const std::string &path, ItmTrailer &trailer, std::string &error);

// Parses a trailer from the last 12 bytes of a file already in memory (little-endian words).
bool ParseItmTrailer(const unsigned char *data, uint64_t size, ItmTrailer &trailer);

// Name of a content type for display ("text", "bmp", "wav" or "unknown").
const char *ItmContentTypeName(int32_t type);

}

#endif
