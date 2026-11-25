#include "itm.h"

#include <cstdio>

namespace gurumin
{

namespace
{
uint32_t ReadLe32(const unsigned char *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}
}

bool ParseItmTrailer(const unsigned char *data, uint64_t size, ItmTrailer &trailer)
{
    if (size < 12)
        return false;
    const unsigned char *tail = data + size - 12;
    trailer.fileSize = size;
    trailer.bodySize = size - 12;
    trailer.contentType = (int32_t)ReadLe32(tail);
    trailer.plainSum = ReadLe32(tail + 4);
    trailer.plainXor = ReadLe32(tail + 8);
    return true;
}

bool ReadItmTrailer(const std::string &path, ItmTrailer &trailer, std::string &error)
{
    FILE *file = std::fopen(path.c_str(), "rb");
    if (file == nullptr)
    {
        error = "cannot open " + path;
        return false;
    }
    bool ok = false;
    if (std::fseek(file, 0, SEEK_END) == 0)
    {
        long size = std::ftell(file);
        unsigned char tail[12];
        if (size >= 12 && std::fseek(file, size - 12, SEEK_SET) == 0 && std::fread(tail, 1, 12, file) == 12)
        {
            ParseItmTrailer(tail, 12, trailer);
            trailer.fileSize = (uint64_t)size;
            trailer.bodySize = (uint64_t)size - 12;
            ok = true;
        }
    }
    std::fclose(file);
    if (!ok)
        error = path + " is shorter than an .itm trailer (12 bytes)";
    return ok;
}

const char *ItmContentTypeName(int32_t type)
{
    switch (type)
    {
    case ITM_CONTENT_BMP:
        return "bmp";
    case ITM_CONTENT_TEXT:
        return "text";
    case ITM_CONTENT_WAV:
        return "wav";
    default:
        return "unknown";
    }
}

}
