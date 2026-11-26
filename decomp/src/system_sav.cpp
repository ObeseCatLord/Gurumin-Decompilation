// system.sav, the game's machine-bound record file (docs/SAVE_FORMAT.md): 0x400 dwords scrambled with the file's
// last-write time (dword i is XORed with byte i % 8 of the time plus 13 * i), a sum and xor of the plain dwords and
// of the inverted computer name at 0xff8, and the original file times put back after every write so that the
// scrambling key stays valid.

#include <windows.h>
#include <stdlib.h>
#include <string.h>

// 0x1015fea0 (SystemSav_WriteRecord). Writes `value` as record `index` (dword index) of `path`: the file is
// descrambled with its last-write time, the record stored, the check values at 0xff8 recomputed and the file
// scrambled again with the time it had before this write, which is then restored. Returns 1 when the record was
// written, 0 when the file cannot be opened.
extern "C" int FUN_1015fea0(const char *path, int index, unsigned int value)
{
    DWORD done;
    DWORD written = 0;
    unsigned int header = 0;
    FILETIME times[4];                               // creation, last access, last write (the 4th is not used)
    HANDLE file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (file == INVALID_HANDLE_VALUE)
        return 0;
    GetFileTime(file, &times[0], &times[1], &times[2]);
    if (file != 0)
        CloseHandle(file);
    // rewrite the first dword so the file gets a new write time
    file = CreateFileA(path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (file == INVALID_HANDLE_VALUE)
        return 0;
    SetFilePointer(file, 0, 0, FILE_BEGIN);
    ReadFile(file, &header, 4, &done, 0);
    SetFilePointer(file, 0, 0, FILE_BEGIN);
    WriteFile(file, &header, 4, &done, 0);
    SetFilePointer(file, 0x1000, 0, FILE_BEGIN);
    if (file != 0)
        CloseHandle(file);
    FILETIME newTimes[4];                            // the same after rewriting the first dword
    file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (file == INVALID_HANDLE_VALUE)
        return 0;
    GetFileTime(file, &newTimes[0], &newTimes[1], &newTimes[2]);
    if (file != 0)
        CloseHandle(file);
    file = CreateFileA(path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (file == INVALID_HANDLE_VALUE)
        return 0;
    // descramble with the old write time
    FILETIME key = times[2];
    unsigned int *data = (unsigned int *)operator new(0x4000);
    SetFilePointer(file, 0, 0, FILE_BEGIN);
    int i;
    for (i = 0; i < 0x400; i++)
    {
        ReadFile(file, &data[i], 4, &done, 0);
        data[i] ^= ((unsigned char *)&key)[i % 8] + i * 13;
    }
    SetFilePointer(file, 0, 0, FILE_BEGIN);
    for (i = 0; i < 0x400; i++)
        WriteFile(file, &data[i], 4, &done, 0);
    SetFilePointer(file, index * 4, 0, FILE_BEGIN);
    WriteFile(file, &value, 4, &written, 0);
    // check values: the plain dwords and the inverted computer name
    SetFilePointer(file, 0, 0, FILE_BEGIN);
    char name[0x100];
    memset(name, 0, sizeof(name));
    DWORD nameSize = 0x100;
    GetComputerNameA(name, &nameSize);
    for (i = 0; i < 0x100; i++)
        name[i] = ~name[i];
    SetFilePointer(file, 0, 0, FILE_BEGIN);
    unsigned int check = 0x87e85;
    int sum = 0;
    for (i = 0; i < 0x3fe; i++)
    {
        unsigned int word;
        ReadFile(file, &word, 4, &done, 0);
        check ^= word;
        sum += word;
    }
    for (i = 0; i < 0x100; i++)
    {
        int c = name[i];
        check ^= c;
        sum += c;
    }
    SetFilePointer(file, 0xff8, 0, FILE_BEGIN);
    WriteFile(file, &check, 4, &done, 0);
    WriteFile(file, &sum, 4, &done, 0);
    // scramble again with the write time the file will get back
    key = newTimes[2];
    SetFilePointer(file, 0, 0, FILE_BEGIN);
    for (i = 0; i < 0x400; i++)
    {
        ReadFile(file, &data[i], 4, &done, 0);
        data[i] ^= ((unsigned char *)&key)[i % 8] + i * 13;
    }
    SetFilePointer(file, 0, 0, FILE_BEGIN);
    for (i = 0; i < 0x400; i++)
        WriteFile(file, &data[i], 4, &done, 0);
    free(data);
    SetFilePointer(file, 0x1000, 0, FILE_BEGIN);
    SetFileTime(file, &newTimes[0], &newTimes[1], &newTimes[2]);
    if (file != 0)
        CloseHandle(file);
    return written == 4;
}

// 0x101603c0 (SystemSav_ReadRecord). Reads record `index` of `path` into `value`, creating an empty (zero) file when
// there is none. The file is descrambled with its last-write time; its first 0x20 bytes must match the inverted
// computer name (a mismatch puts the original times back and marks the file as copied from another machine, a file
// without a name gets this machine's name and its records cleared), the record is read, the check values at 0xff8
// are verified, and the file is scrambled again and its times restored. Returns 1 when the record passes the
// check, -1 for a file from another machine and 0 otherwise (value is 0 then).
extern "C" int FUN_101603c0(const char *path, int index, unsigned int *value)
{
    DWORD done;
    int fromOtherMachine = 0;
    int created = 0;
    *value = 0;
    DWORD read = 0;
    HANDLE file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (file == INVALID_HANDLE_VALUE)
    {
        file = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
        if (file == INVALID_HANDLE_VALUE)
            return 0;
        unsigned int zero = 0;
        for (int i = 0; i < 0x400; i++)
            WriteFile(file, &zero, 4, &read, 0);
        created = 1;
    }
    FILETIME times[4];                               // creation, last access, last write (the 4th is not used)
    GetFileTime(file, &times[0], &times[1], &times[2]);
    if (file != 0)
        CloseHandle(file);
    file = CreateFileA(path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (file == INVALID_HANDLE_VALUE)
        return 0;
    char name[0x100];
    char storedName[0x100];
    memset(name, 0, sizeof(name));
    memset(storedName, 0, sizeof(storedName));
    DWORD nameSize = 0x100;
    GetComputerNameA(name, &nameSize);
    int i;
    for (i = 0; i < 0x100; i++)
        name[i] = ~name[i];
    FILETIME key = times[2];
    unsigned int *data = (unsigned int *)operator new(0x4000);
    SetFilePointer(file, 0, 0, FILE_BEGIN);
    if (created == 0)
    {
        for (i = 0; i < 0x400; i++)
        {
            ReadFile(file, &data[i], 4, &done, 0);
            data[i] ^= ((unsigned char *)&key)[i % 8] + i * 13;
        }
        SetFilePointer(file, 0, 0, FILE_BEGIN);
        for (i = 0; i < 0x400; i++)
            WriteFile(file, &data[i], 4, &done, 0);
    }
    // the machine name in the first 0x20 bytes
    SetFilePointer(file, 0, 0, FILE_BEGIN);
    ReadFile(file, storedName, 0x20, &read, 0);
    int noName = 0;
    for (i = 0; i < 0x20; i++)
    {
        if (storedName[i] != 0 && name[i] != storedName[i])
        {
            SetFilePointer(file, 0x1000, 0, FILE_BEGIN);
            SetFileTime(file, &times[0], &times[1], &times[2]);
            fromOtherMachine = 1;
        }
        if (storedName[i] == 0 && name[i] != 0)
            noName = 1;
    }
    SetFilePointer(file, 0, 0, FILE_BEGIN);
    WriteFile(file, name, 0x20, &read, 0);
    if (noName != 0)
    {
        unsigned int zero = 0;
        for (i = 0; i < 0x3f8; i++)
            WriteFile(file, &zero, 4, &read, 0);
    }
    SetFilePointer(file, index * 4, 0, FILE_BEGIN);
    ReadFile(file, value, 4, &read, 0);
    // check values
    SetFilePointer(file, 0, 0, FILE_BEGIN);
    unsigned int check = 0x87e85;
    int sum = 0;
    for (i = 0; i < 0x3fe; i++)
    {
        unsigned int word;
        ReadFile(file, &word, 4, &done, 0);
        check ^= word;
        sum += word;
    }
    for (i = 0; i < 0x100; i++)
    {
        int c = name[i];
        check ^= c;
        sum += c;
    }
    SetFilePointer(file, 0xff8, 0, FILE_BEGIN);
    int storedSum;
    unsigned int storedCheck;
    ReadFile(file, &storedCheck, 4, &done, 0);
    ReadFile(file, &storedSum, 4, &done, 0);
    // scramble again
    SetFilePointer(file, 0, 0, FILE_BEGIN);
    for (i = 0; i < 0x400; i++)
    {
        ReadFile(file, &data[i], 4, &done, 0);
        data[i] ^= ((unsigned char *)&key)[i % 8] + i * 13;
    }
    SetFilePointer(file, 0, 0, FILE_BEGIN);
    for (i = 0; i < 0x400; i++)
        WriteFile(file, &data[i], 4, &done, 0);
    free(data);
    SetFilePointer(file, 0x1000, 0, FILE_BEGIN);
    SetFileTime(file, &times[0], &times[1], &times[2]);
    if (file != 0)
        CloseHandle(file);
    if (fromOtherMachine == 0 && storedCheck == check && storedSum == sum && read == 4)
        return 1;
    if (fromOtherMachine != 0)
    {
        *value = 0;
        return -1;
    }
    *value = 0;
    return 0;
}
