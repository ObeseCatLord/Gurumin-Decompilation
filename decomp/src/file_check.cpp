#include <stdio.h>

extern "C" int FUN_101602d0(const char *path, int *sum, unsigned int *check)
{
    FILE *file = fopen(path, "rb");
    if (file == 0)
        return 0;
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    int total = 0;
    unsigned int mixed = 0xcc82;
    for (int i = 0; i < size; i += 4)
    {
        unsigned int word;
        fread(&word, 1, 4, file);
        total += word;
        mixed ^= word;
    }
    fclose(file);
    *sum = total;
    *check = mixed;
    return 1;
}

extern "C" int FUN_10160360(const char *path, int base, int index, void *first, void *second)
{
    FILE *file = fopen(path, "rb");
    if (file == 0)
        return 0;
    fseek(file, base + index * 8, SEEK_SET);
    fread(first, 1, 4, file);
    fread(second, 1, 4, file);
    fclose(file);
    return 1;
}
