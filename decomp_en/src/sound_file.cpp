// File access of the sound engine (EN build; JP twin: sound3d.dll, not matched yet). The small MmioFile methods are
// inline in sound_file.h because the original inlines them into the sound code. The original also has a standalone
// copy of Close (0x7bf150), Read (0x7bf540), Seek (0x7bf5d0) and Tell (0x7bf630), emitted where some sound function
// calls them out of line; they are compared once those callers are written.
// No address-taking table here: data the original does not have would only exist to steer the comparison.
#include <stdio.h>
#include <string.h>

#include "sound_file.h"

// Reverses the scrambling of an .itm sound data block in place (docs/ITM_FORMAT.md): the key mixes a fixed string, a
// fixed table and the upper-cased file name (written into the table at offset 8). Returns 1 when the byte sum and
// the xor of the plain bytes equal the expected values. Called when a sound file is loaded from an archive.
extern "C" int ItmDescramble(unsigned char *data, int size, char *name, int expectedSum, unsigned int expectedCheck)
{
    int total = 0;
    unsigned int mix = ITM_CHECK_SEED;
    char table[ITM_TABLE_BYTES] = "!~_Q#xDRgd-d&yfg&'(8)(5(594er2f4asf5f6e5f4s5fvwyjk%fgqTUCFD4568r";
    char key[ITM_KEY_BYTES] = "GURUguruMinmIN_";
    _strupr(name);
    strncpy(table + ITM_TABLE_NAME_OFFSET, name, ITM_TABLE_NAME_LENGTH);
    for (int i = 0; i < size; i++)
    {
        unsigned char c = (key[i % ITM_KEY_LENGTH] + (i & 7)) * table[i & ITM_TABLE_MASK] ^ ~data[i];
        data[i] = c;
        total += c;
        mix ^= c;
    }
    if (expectedSum == total && expectedCheck == mix)
        return 1;
    return 0;
}

// Copies the file name of `path` without directory and extension into `name` (zero filled first, so a short name
// ends up terminated). Called when a sound file is looked up in an archive.
extern "C" void SoundFile_BaseName(ArchiveName *name, const char *path)
{
    char copy[SOUND_PATH_BYTES];
    strcpy(copy, path);
    name->word[0] = 0;
    name->word[1] = 0;
    int start = 0;
    int i = 0;
    int dot = 0;
    if (copy[0] != 0)
    {
        do
        {
            if (copy[i] == PATH_SEPARATOR)
                start = i + 1;
            if (copy[i] == '.')
                dot = i;
            i++;
        } while (copy[i] != 0);
    }
    if (dot == 0)
        dot = strlen(copy);
    copy[dot] = 0;
    strcpy(name->text, copy + start);
}

// Turns an upper case letter into lower case (other characters stay).
static void ToLowerLetter(char &letter)
{
    if (letter >= 'A' && letter <= 'Z')
        letter += CASE_OFFSET;
}

// Opens a sound file. Writing and appending open the plain file. Reading looks the file up first in the archive
// "<folder>.arc", then in "<folder>.dat" (both hold name tables grouped by extension), and last as a plain file;
// a section of an archive gets its size and offset, a plain file its size from the end position. Returns whether the
// file was opened. Called by every sound loader.
int MmioFile::Open(const char *path, const char *mode)
{
    if (handle != 0)
        return 0;
    if (mode[0] == 'w' || mode[0] == 'W' || mode[1] == 'w' || mode[1] == 'W')
    {
        handle = mmioOpen((LPSTR)path, 0, MMIO_WRITE);
        if (handle == 0)
            handle = mmioOpen((LPSTR)path, 0, MMIO_CREATE | MMIO_WRITE);
    }
    else if (mode[0] == 'a' || mode[0] == 'A' || mode[1] == 'a' || mode[1] == 'A')
    {
        handle = mmioOpen((LPSTR)path, 0, MMIO_READWRITE);
        mmioSeek(handle, 0, SEEK_END);
    }
    else
    {
        LONG position;
        int tag;
        unsigned int entryCount;
        int entrySize;
        ArchiveKey key;
        char nameText[ARCHIVE_NAME_BUFFER];
        ArchiveName entryName;
        char folder[SOUND_PATH_BYTES];
        char arcPath[SOUND_PATH_BYTES];
        char plainPath[SOUND_PATH_BYTES];
        char datPath[SOUND_PATH_BYTES];
        strcpy(folder, path);
        int length = strlen(folder);
        int split = length;
        while (split > 0 && folder[split] != PATH_SEPARATOR)
        {
            split--;
            if (folder[split] == '.')
                folder[split] = ' ';
        }
        folder[split] = 0;
        ExtensionTag extension;
        extension.word = 0;
        memset(nameText, 0, sizeof(nameText));
        if (length - split > ARCHIVE_NAME_LIMIT)
            return 0;
        sprintf(datPath, "%s.dat", folder);
        sscanf(folder + split + 1, "%s %s", nameText, extension.text);
        sprintf(arcPath, "%s.arc", folder);
        int nameLength = strlen(nameText);
        if (nameLength > ARCHIVE_NAME_CHARS)
            nameLength = ARCHIVE_NAME_CHARS;
        for (int i = 0; i < nameLength; i++)
            ToLowerLetter(nameText[i]);
        ToLowerLetter(extension.text[0]);
        ToLowerLetter(extension.text[1]);
        ToLowerLetter(extension.text[2]);
        ToLowerLetter(extension.text[3]);
        key.extension = extension.word;
        key.name.word[0] = ((ArchiveName *)nameText)->word[0];
        key.name.word[1] = ((ArchiveName *)nameText)->word[1];
        handle = mmioOpen(arcPath, 0, MMIO_PARSE);
        handle = mmioOpen(arcPath, 0, MMIO_READ);
        if (handle == 0)
        {
            handle = mmioOpen(datPath, 0, MMIO_PARSE);
            handle = mmioOpen(datPath, 0, MMIO_READ);
            if (handle == 0)
            {
                strcpy(plainPath, path);
                handle = mmioOpen(plainPath, 0, MMIO_PARSE);
                handle = mmioOpen(plainPath, 0, MMIO_READ);
                if (handle == 0)
                    return 0;
                isPlain = -1;
                mmioSeek(handle, 0, SEEK_END);
                size = Tell();
                mmioSeek(handle, 0, SEEK_SET);
                return 1;
            }
        }
        isPlain = 0;
        int groupCount;
        mmioRead(handle, (HPSTR)&tag, ARCHIVE_FIELD_BYTES);
        mmioRead(handle, (HPSTR)&groupCount, ARCHIVE_FIELD_BYTES);
        int group = 0;
        if (groupCount > 0)
        {
            do
            {
                mmioRead(handle, (HPSTR)&tag, ARCHIVE_FIELD_BYTES);
                mmioRead(handle, (HPSTR)&position, ARCHIVE_FIELD_BYTES);
                mmioRead(handle, (HPSTR)&entryCount, ARCHIVE_FIELD_BYTES);
                if (tag == key.extension)
                    break;
                group++;
            } while (group < groupCount);
        }
        if (group != groupCount)
        {
            mmioSeek(handle, position, SEEK_SET);
            unsigned int entry = 0;
            if (entryCount != 0)
            {
                do
                {
                    mmioRead(handle, (HPSTR)&entryName, sizeof(entryName));
                    mmioRead(handle, (HPSTR)&entrySize, ARCHIVE_FIELD_BYTES);
                    mmioRead(handle, (HPSTR)&position, ARCHIVE_FIELD_BYTES);
                    if (entryName.word[0] == key.name.word[0] && entryName.word[1] == key.name.word[1])
                        break;
                    entry++;
                } while (entry < entryCount);
            }
            if (entry != entryCount)
            {
                size = entrySize;
                offset = position;
                mmioSeek(handle, position, SEEK_SET);
                return 1;
            }
        }
        mmioClose(handle, 0);
        handle = 0;
        return 0;
    }
    isPlain = -1;
    return handle != 0;
}
