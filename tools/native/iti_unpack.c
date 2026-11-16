/* iti_unpack.exe <packs.dll> <in.iti> <out.raw>
   Decodes the wavelet blocks of .iti type 666 (RGB) / 667 (RGB + alpha) with the game's own packs.dll BMPUnpack and
   writes width, height, has_alpha as three uint32 followed by RGB (w*h*3) and, for 667, alpha (w*h*3). */
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

typedef int (__cdecl *BmpUnpackProc)(void *packed, int size, void *out, int *a, int *b);

static int unpack_block(FILE *in, BmpUnpackProc unpack, unsigned char *out, int bytes)
{
    unsigned int size;
    unsigned char *packed;
    int a = 0, b = 0;
    if (fread(&size, 4, 1, in) != 1 || size == 0 || size > 0x4000000)
        return 0;
    packed = (unsigned char *)malloc(size + 16);
    fread(packed, 1, size, in);
    unpack(packed, size, out, &a, &b);
    free(packed);
    return 1;
}

int main(int argc, char **argv)
{
    HMODULE packs;
    BmpUnpackProc unpack;
    FILE *in, *out;
    unsigned int header[3];
    unsigned char *buffer;
    int bytes, alpha;
    if (argc < 4)
    {
        fprintf(stderr, "usage: iti_unpack <packs.dll> <in.iti> <out.raw>\n");
        return 2;
    }
    packs = LoadLibraryA(argv[1]);
    if (packs == 0)
    {
        fprintf(stderr, "cannot load %s\n", argv[1]);
        return 1;
    }
    unpack = (BmpUnpackProc)GetProcAddress(packs, "BMPUnpack");
    in = fopen(argv[2], "rb");
    if (unpack == 0 || in == 0 || fread(header, 4, 3, in) != 3 || (header[0] != 666 && header[0] != 667))
    {
        fprintf(stderr, "not a type 666/667 .iti\n");
        return 1;
    }
    bytes = header[1] * header[2] * 3;
    alpha = header[0] == 667;
    buffer = (unsigned char *)calloc(1, 0x1000000);
    out = fopen(argv[3], "wb");
    fwrite(&header[1], 4, 2, out);
    fwrite(&alpha, 4, 1, out);
    if (!unpack_block(in, unpack, buffer, bytes))
        return 1;
    fwrite(buffer, 1, bytes, out);
    if (alpha)
    {
        memset(buffer, 0, bytes);
        if (!unpack_block(in, unpack, buffer, bytes))
            return 1;
        fwrite(buffer, 1, bytes, out);
    }
    fclose(out);
    fclose(in);
    return 0;
}
