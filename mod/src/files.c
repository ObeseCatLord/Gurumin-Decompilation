// Replacement files for Gurumin (JP PC, bin\APP.dll): every file the game reads can be replaced by a file of the same
// name anywhere under the mod folders (default: mod\ next to game.exe).
//
// What the game does: every loader (textures, models, movies, scripts, tables) opens its files through the CRT
// fopen inside APP.dll (0x101b3304, `_fsopen(path, mode, 0x40)`), mostly via File_Open 0x101b2310. No two game files
// share a name (4,379 files in the JP release), so a file name identifies a game file on its own.
//
// What this file does:
//   - at startup it indexes every file under the mod folders by name (sub-folders are only for sorting);
//   - fopen for reading opens the replacement instead when one has the same name;
//   - a PNG (or JPEG/BMP) named <texture>.png or <texture>.iti.png replaces <texture>.iti: it is converted once into
//     a raw .iti (type 0 RGB, type 1 RGB + alpha) under <mod folder>\.cache\, at its own size (the game takes
//     textures up to 2048x2048);
//   - [files] log=1 writes every file the game opens to gurumin_files.log, to find the names to replace.
#define COBJMACROS
#include <windows.h>
#include <wincodec.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "display.h"

#define VA_FOPEN 0x101b3304u
#define MAX_TEXTURE_SIZE 2048

typedef FILE *(__cdecl *fopen_fn)(const char *path, const char *mode);
static fopen_fn real_fopen;

static void (*log_line)(const char *format, ...);
static FILE *open_log;
static CRITICAL_SECTION lock;

typedef struct
{
    char *name;      // lower-case game file name ("pc_01.iti")
    char *path;      // the replacement on disk
    char *root;      // the mod folder it was found in (for the PNG cache)
    int image;       // 1 = an image to convert into an .iti
} Replacement;

static Replacement *replacements;
static int replacement_count, replacement_capacity;

static char *copy_string(const char *text)
{
    char *copy = malloc(strlen(text) + 1);
    if (copy)
        strcpy(copy, text);
    return copy;
}

static void lower_case(char *text)
{
    for (; *text; text++)
        if (*text >= 'A' && *text <= 'Z')
            *text += 'a' - 'A';
}

static int ends_with(const char *text, const char *end)
{
    size_t a = strlen(text), b = strlen(end);
    return a >= b && _stricmp(text + a - b, end) == 0;
}

static Replacement *find_replacement(const char *name)
{
    for (int i = 0; i < replacement_count; i++)
        if (strcmp(replacements[i].name, name) == 0)
            return &replacements[i];
    return NULL;
}

// Adds `name` -> `path`. Earlier folders win; a game-format file wins over an image for the same texture.
static void add_replacement(const char *name, const char *path, const char *root, int image)
{
    char key[MAX_PATH];
    snprintf(key, sizeof key, "%s", name);
    lower_case(key);
    Replacement *existing = find_replacement(key);
    if (existing)
    {
        if (existing->image && !image && strcmp(existing->root, root) == 0)
        {
            free(existing->path);
            existing->path = copy_string(path);
            existing->image = 0;
        }
        else if (!(existing->image && image))
            log_line("files: %s found twice, using %s", key, existing->path);
        return;
    }
    if (replacement_count == replacement_capacity)
    {
        int capacity = replacement_capacity ? replacement_capacity * 2 : 256;
        Replacement *grown = realloc(replacements, capacity * sizeof *grown);
        if (!grown)
            return;
        replacements = grown;
        replacement_capacity = capacity;
    }
    Replacement *entry = &replacements[replacement_count++];
    entry->name = copy_string(key);
    entry->path = copy_string(path);
    entry->root = copy_string(root);
    entry->image = image;
}

static int is_image_name(const char *name)
{
    return ends_with(name, ".png") || ends_with(name, ".jpg") || ends_with(name, ".jpeg") || ends_with(name, ".bmp");
}

// "pc_01.iti.png" and "pc_01.png" both replace "pc_01.iti".
static void texture_name_for_image(const char *image_name, char *out, size_t size)
{
    snprintf(out, size, "%s", image_name);
    char *dot = strrchr(out, '.');
    if (dot)
        *dot = 0;
    if (!ends_with(out, ".iti"))
        strncat(out, ".iti", size - strlen(out) - 1);
}

static void index_folder(const char *folder, const char *root)
{
    char pattern[MAX_PATH];
    snprintf(pattern, sizeof pattern, "%s\\*", folder);
    WIN32_FIND_DATAA found;
    HANDLE search = FindFirstFileA(pattern, &found);
    if (search == INVALID_HANDLE_VALUE)
        return;
    do
    {
        const char *name = found.cFileName;
        if (name[0] == '.')
            continue;    // ".", "..", ".cache" and other hidden folders
        char path[MAX_PATH];
        snprintf(path, sizeof path, "%s\\%s", folder, name);
        if (found.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            index_folder(path, root);
        else if (ends_with(name, ".txt") || ends_with(name, ".ini") || ends_with(name, ".md"))
            continue;    // notes and readmes that come with a mod
        else if (is_image_name(name) && !ends_with(name, ".bmp"))
        {
            char texture[MAX_PATH];
            texture_name_for_image(name, texture, sizeof texture);
            add_replacement(texture, path, root, 1);
        }
        else
            add_replacement(name, path, root, 0);
    } while (FindNextFileA(search, &found));
    FindClose(search);
}

static void game_folder(char *out)
{
    GetModuleFileNameA(NULL, out, MAX_PATH);
    char *slash = strrchr(out, '\\');
    if (slash)
        *slash = 0;
}

// ---- images -> .iti ---------------------------------------------------------------------------------------------

static IWICImagingFactory *wic_factory(void)
{
    static IWICImagingFactory *factory;
    if (!factory)
    {
        CoInitializeEx(NULL, COINIT_MULTITHREADED);    // fails harmlessly when the thread already has COM
        if (FAILED(CoCreateInstance(&CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, &IID_IWICImagingFactory,
                                    (void **)&factory)))
            factory = NULL;
    }
    return factory;
}

// Decodes an image to 8-bit RGBA, top-down. Returns malloc'd pixels or NULL.
static BYTE *load_image_rgba(const char *path, UINT *width, UINT *height)
{
    IWICImagingFactory *factory = wic_factory();
    if (!factory)
        return NULL;
    WCHAR wide[MAX_PATH];
    MultiByteToWideChar(CP_ACP, 0, path, -1, wide, MAX_PATH);
    IWICBitmapDecoder *decoder = NULL;
    IWICBitmapFrameDecode *frame = NULL;
    IWICFormatConverter *converter = NULL;
    BYTE *pixels = NULL;
    if (SUCCEEDED(IWICImagingFactory_CreateDecoderFromFilename(factory, wide, NULL, GENERIC_READ,
                                                               WICDecodeMetadataCacheOnDemand, &decoder)) &&
        SUCCEEDED(IWICBitmapDecoder_GetFrame(decoder, 0, &frame)) &&
        SUCCEEDED(IWICImagingFactory_CreateFormatConverter(factory, &converter)) &&
        SUCCEEDED(IWICFormatConverter_Initialize(converter, (IWICBitmapSource *)frame, &GUID_WICPixelFormat32bppRGBA,
                                                 WICBitmapDitherTypeNone, NULL, 0.0, WICBitmapPaletteTypeCustom)) &&
        SUCCEEDED(IWICFormatConverter_GetSize(converter, width, height)) && *width && *height)
    {
        pixels = malloc((size_t)*width * *height * 4);
        if (pixels && FAILED(IWICFormatConverter_CopyPixels(converter, NULL, *width * 4, *width * *height * 4, pixels)))
        {
            free(pixels);
            pixels = NULL;
        }
    }
    if (converter)
        IWICFormatConverter_Release(converter);
    if (frame)
        IWICBitmapFrameDecode_Release(frame);
    if (decoder)
        IWICBitmapDecoder_Release(decoder);
    return pixels;
}

// Writes `rgba` as a raw .iti: type 0 (RGB) when fully opaque, else type 1 (RGB, then alpha as grey RGB).
static int write_iti(const char *path, const BYTE *rgba, UINT width, UINT height)
{
    size_t count = (size_t)width * height;
    int has_alpha = 0;
    for (size_t i = 0; i < count && !has_alpha; i++)
        has_alpha = rgba[i * 4 + 3] != 255;
    FILE *file = fopen(path, "wb");    // this DLL's own C runtime (a FILE* never crosses runtimes)
    if (!file)
        return 0;
    UINT32 header[3] = {has_alpha ? 1u : 0u, width, height};
    fwrite(header, 4, 3, file);
    BYTE *row = malloc((size_t)width * 3);
    for (int pass = 0; pass < (has_alpha ? 2 : 1); pass++)
        for (UINT y = 0; y < height; y++)
        {
            const BYTE *in = rgba + (size_t)y * width * 4;
            for (UINT x = 0; x < width; x++)
            {
                BYTE *out = row + x * 3;
                if (pass == 0)
                    out[0] = in[x * 4], out[1] = in[x * 4 + 1], out[2] = in[x * 4 + 2];
                else
                    out[0] = out[1] = out[2] = in[x * 4 + 3];
            }
            fwrite(row, 3, width, file);
        }
    free(row);
    return fclose(file) == 0;
}

static int newer_than(const char *a, const char *b)
{
    WIN32_FILE_ATTRIBUTE_DATA da, db;
    if (!GetFileAttributesExA(b, GetFileExInfoStandard, &db))
        return 1;
    if (!GetFileAttributesExA(a, GetFileExInfoStandard, &da))
        return 0;
    return CompareFileTime(&da.ftLastWriteTime, &db.ftLastWriteTime) > 0;
}

// The .iti made from an image replacement (converted again when the image is newer). NULL when it cannot be made.
static const char *converted_texture(Replacement *entry, char *out)
{
    char cache[MAX_PATH];
    snprintf(cache, MAX_PATH, "%s\\.cache", entry->root);
    snprintf(out, MAX_PATH, "%s\\%s", cache, entry->name);
    if (!newer_than(entry->path, out))
        return out;
    UINT width = 0, height = 0;
    BYTE *rgba = load_image_rgba(entry->path, &width, &height);
    if (!rgba)
    {
        log_line("files: cannot read the image %s", entry->path);
        return NULL;
    }
    if (width > MAX_TEXTURE_SIZE || height > MAX_TEXTURE_SIZE)
    {
        log_line("files: %s is %ux%u, the game takes textures up to %dx%d: not used", entry->path, width, height,
                 MAX_TEXTURE_SIZE, MAX_TEXTURE_SIZE);
        free(rgba);
        return NULL;
    }
    CreateDirectoryA(cache, NULL);
    SetFileAttributesA(cache, FILE_ATTRIBUTE_HIDDEN);
    int ok = write_iti(out, rgba, width, height);
    free(rgba);
    log_line("files: %s -> %s (%ux%u)%s", entry->path, out, width, height, ok ? "" : " FAILED");
    return ok ? out : NULL;
}

// ---- fopen ------------------------------------------------------------------------------------------------------

static int reading_only(const char *mode)
{
    return mode && strchr(mode, 'r') && !strchr(mode, 'w') && !strchr(mode, 'a') && !strchr(mode, '+');
}

// Game_Startup checksums app.dll, app8.dll, bin\map.itm, sound3d.dll, 3ddata\test.dat and game.exe with File_Checksum
// (0x101602d0) against values stored in soundarc.dll and closes the game on a mismatch. That function always gets
// the game's own file, so a replaced map.itm or test.dat passes the check while the game loads the replacement.
#define VA_FILE_CHECKSUM_START 0x101602d0
#define VA_FILE_CHECKSUM_END   0x10160360

static int called_from_file_checksum(void *return_address)
{
    DWORD va = (DWORD)(UINT_PTR)return_address - (DWORD)(UINT_PTR)app_address(0x10000000) + 0x10000000;
    return va >= VA_FILE_CHECKSUM_START && va < VA_FILE_CHECKSUM_END;
}

static FILE *__cdecl hook_fopen(const char *path, const char *mode)
{
    if (!path || !reading_only(mode) || called_from_file_checksum(__builtin_return_address(0)))
        return real_fopen(path, mode);
    const char *name = path;
    for (const char *p = path; *p; p++)
        if (*p == '\\' || *p == '/')
            name = p + 1;
    char key[MAX_PATH];
    snprintf(key, sizeof key, "%s", name);
    lower_case(key);
    const char *use = path;
    char converted[MAX_PATH];
    EnterCriticalSection(&lock);
    Replacement *entry = find_replacement(key);
    if (entry)
    {
        const char *replacement = entry->image ? converted_texture(entry, converted) : entry->path;
        if (replacement)
            use = replacement;
    }
    FILE *file = real_fopen(use, mode);
    if (open_log && file)
    {
        DWORD caller = (DWORD)(UINT_PTR)__builtin_return_address(0) - (DWORD)(UINT_PTR)app_address(0x10000000)
                     + 0x10000000;
        if (use != path)
            fprintf(open_log, "%s\t<- %s\tcalled from 0x%08lx\n", path, use, (unsigned long)caller);
        else
            fprintf(open_log, "%s\tcalled from 0x%08lx\n", path, (unsigned long)caller);
        fflush(open_log);
    }
    LeaveCriticalSection(&lock);
    if (!file && use != path)
        file = real_fopen(path, mode);    // replacement unreadable: fall back to the game's file
    return file;
}

void files_install(void (*log)(const char *format, ...))
{
    static const short fopen_code[] = {0x6a, 0x40, 0xff, 0x74, 0x24, 0x0c};    // push 0x40; push [esp+0xc]
    log_line = log;
    char game[MAX_PATH], ini[MAX_PATH], folders[1024];
    game_folder(game);
    snprintf(ini, sizeof ini, "%s\\gurumin_mod.ini", game);
    if (!GetPrivateProfileIntA("files", "enabled", 1, ini))
        return;
    GetPrivateProfileStringA("files", "folders", "mod", folders, sizeof folders, ini);
    for (char *folder = strtok(folders, ",;"); folder; folder = strtok(NULL, ",;"))
    {
        while (*folder == ' ')
            folder++;
        size_t length = strlen(folder);
        while (length && (folder[length - 1] == ' ' || folder[length - 1] == '\\' || folder[length - 1] == '/'))
            folder[--length] = 0;
        if (!length)
            continue;
        char root[MAX_PATH];
        if (folder[0] == '\\' || (folder[0] && folder[1] == ':'))
            snprintf(root, sizeof root, "%s", folder);
        else
            snprintf(root, sizeof root, "%s\\%s", game, folder);
        int before = replacement_count;
        index_folder(root, root);
        log_line("files: %d replacement file(s) in %s", replacement_count - before, root);
    }
    if (GetPrivateProfileIntA("files", "log", 0, ini))
    {
        char log_path[MAX_PATH];
        snprintf(log_path, sizeof log_path, "%s\\gurumin_files.log", game);
        open_log = fopen(log_path, "w");
    }
    if (!replacement_count && !open_log)
        return;
    if (!code_matches(VA_FOPEN, fopen_code, 6))
        return;
    InitializeCriticalSection(&lock);
    real_fopen = (fopen_fn)detour(VA_FOPEN, 6, (void *)hook_fopen);
    log_line("files: replacement loader %s", real_fopen ? "on" : "FAILED");
}
