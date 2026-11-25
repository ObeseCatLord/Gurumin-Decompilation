// gurumin_port: the native shell of the port scaffold. It is not the game. It opens an SDL2 window, looks at the
// user's own installed copy of Gurumin (read-only), runs a self-test of the decomp code compiled into gurumin_core,
// and shows the result as a status screen until Escape is pressed.
//
//   gurumin_port --help
//   gurumin_port --probe [--game <folder>]   headless: self-test and game folder report, no window
//   gurumin_port --game <folder>             window with the status screen (Escape or closing it exits)
#define SDL_MAIN_HANDLED
#include <SDL.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "game_folder.h"
#include "itm.h"

#include "matrix4.h"
#include "vec4.h"

// Matched decomp functions used by the shell (definitions in decomp/src, compiled into gurumin_core).
extern "C" int FUN_101602d0(const char *path, int *sum, unsigned int *check);                         // file_check.cpp
extern "C" int FUN_101612b0(unsigned char *out, int size, const unsigned char *packed, int packedSize);  // rle_decode.cpp
extern "C" int FUN_1015d520(char *text);                                                               // text_fullwidth.cpp
Vec4 Cross(const Vec4 &a, const Vec4 &b);                                                              // vec_math.cpp

namespace
{

struct StatusLine
{
    std::string text;
    bool ok;
};

struct Report
{
    std::vector<StatusLine> lines;
    bool selfTestPassed = true;
    bool gameFound = false;

    void Add(bool ok, const std::string &text)
    {
        lines.push_back({text, ok});
        std::printf("  [%s] %s\n", ok ? " ok " : "FAIL", text.c_str());
    }
};

const char kUsage[] =
    "gurumin_port: native port scaffold for the Gurumin decompilation (not playable yet)\n"
    "\n"
    "usage:\n"
    "  gurumin_port --game <folder>          open the status window for your installed game\n"
    "  gurumin_port --probe [--game <folder>] headless self-test (and game folder report)\n"
    "  gurumin_port --help                   this text\n"
    "\n"
    "<folder> is your own installed copy of Gurumin (the folder holding bin/ and 3ddata/).\n"
    "It is only read, never written. In the window, press Escape to quit.\n";

// Runs a few matched decomp functions on known inputs. This is the shell's proof that gurumin_core runs natively;
// the full checks are in port/tests.
void SelfTest(Report &report)
{
    std::printf("self-test of decomp code compiled natively (%zu-bit):\n", sizeof(void *) * 8);

    Matrix4 matrix;
    matrix.SetIdentity();
    matrix.Translate(1, 2, 3);
    Vec4 moved = TransformCoord(Vec4(1, 1, 1, 1), matrix);
    bool matrixOk = moved.x == 2 && moved.y == 3 && moved.z == 4 && moved.w == 1;
    report.Add(matrixOk, "Matrix4::SetIdentity/Translate, TransformCoord (vec4.cpp, containers.cpp, vec_math.cpp)");

    Vec4 axis = Cross(Vec4(1, 0, 0, 0), Vec4(0, 1, 0, 0));
    Vec4 direction(3, 4, 0, 0);
    bool vectorOk = axis.z == 1 && std::fabs(direction.Length() - 5.0f) < 1e-6f;
    report.Add(vectorOk, "Cross, Vec4::Length (vec_math.cpp, vec4.cpp)");

    const unsigned char packed[] = {0x02, 'G', 'U', 'R', 0x41};
    unsigned char unpacked[5] = {};
    bool rleOk = FUN_101612b0(unpacked, 5, packed, 5) == 1 && std::memcmp(unpacked, "GUR\0\0", 5) == 0;
    report.Add(rleOk, "Rle_Decode (rle_decode.cpp)");

    char text[16] = "OK";
    FUN_1015d520(text);
    bool textOk = std::memcmp(text, "\x82\x6e\x82\x6a", 5) == 0;
    report.Add(textOk, "Text_ToFullWidth (text_fullwidth.cpp)");

    report.selfTestPassed = matrixOk && vectorOk && rleOk && textOk;
}

// Reports what is in the user's game folder, using the port's format readers and the decomp's File_Checksum.
void ProbeGame(Report &report, const std::string &folder)
{
    std::printf("game folder: %s\n", folder.c_str());
    if (!gurumin::IsDirectory(folder))
    {
        report.Add(false, "game folder not found: " + folder);
        return;
    }
    std::string mapItm = gurumin::ResolveGamePath(folder, "bin/map.itm");
    if (mapItm.empty())
    {
        report.Add(false, "bin/map.itm not found (is this the game folder?)");
        return;
    }
    report.gameFound = true;

    gurumin::ItmTrailer trailer;
    std::string error;
    if (gurumin::ReadItmTrailer(mapItm, trailer, error))
    {
        char line[256];
        std::snprintf(line, sizeof(line), "bin/map.itm: %llu bytes, content type %d (%s), plain sum %08x, xor %08x",
                      (unsigned long long)trailer.fileSize, (int)trailer.contentType,
                      gurumin::ItmContentTypeName(trailer.contentType), (unsigned)trailer.plainSum,
                      (unsigned)trailer.plainXor);
        report.Add(trailer.contentType == gurumin::ITM_CONTENT_TEXT, line);
    }
    else
    {
        report.Add(false, error);
    }

    int sum = 0;
    unsigned int check = 0;
    if (FUN_101602d0(mapItm.c_str(), &sum, &check))
    {
        char line[160];
        std::snprintf(line, sizeof(line), "File_Checksum (decomp file_check.cpp) of bin/map.itm: sum %08x, check %08x",
                      (unsigned)sum, check);
        report.Add(true, line);
    }
    else
    {
        report.Add(false, "File_Checksum could not open bin/map.itm");
    }

    const char *folders[] = {"3ddata", "wav", "bgm", "bin"};
    for (const char *name : folders)
    {
        gurumin::FolderSummary summary;
        std::string path = gurumin::ResolveGamePath(folder, name);
        if (!gurumin::SummariseFolder(path, summary))
        {
            report.Add(false, std::string(name) + "/ not found");
            continue;
        }
        std::string line = std::string(name) + "/: " + std::to_string(summary.files) + " files, " +
                           std::to_string(summary.bytes / 1024) + " KiB (";
        bool first = true;
        for (const auto &entry : summary.filesByExtension)
        {
            line += (first ? "" : ", ") + std::to_string(entry.second) + " " + entry.first;
            first = false;
        }
        report.Add(summary.files > 0, line + ")");
    }

    bool jp = !gurumin::ResolveGamePath(folder, "bin/APP.dll").empty();
    bool en = !gurumin::ResolveGamePath(folder, "game_en_steam.exe").empty() ||
              !gurumin::ResolveGamePath(folder, "gurumin.exe").empty();
    report.Add(jp || en, std::string("release: ") + (jp ? "JP (bin/APP.dll)" : en ? "EN (Steam)" : "unknown"));
}

// Draws the status screen: one bar per line (green = ok, red = failed). SDL2 has no text rendering, so the
// details go to stdout and the window title.
void Draw(SDL_Renderer *renderer, const Report &report, int width, int height)
{
    SDL_SetRenderDrawColor(renderer, 18, 22, 30, 255);
    SDL_RenderClear(renderer);
    int count = (int)report.lines.size();
    int barHeight = count > 0 ? std::min(36, (height - 80) / count) : 0;
    for (int i = 0; i < count; i++)
    {
        SDL_Rect bar = {40, 40 + i * barHeight, width - 80, barHeight - 6};
        if (report.lines[i].ok)
            SDL_SetRenderDrawColor(renderer, 60, 170, 90, 255);
        else
            SDL_SetRenderDrawColor(renderer, 190, 60, 60, 255);
        SDL_RenderFillRect(renderer, &bar);
    }
    SDL_RenderPresent(renderer);
}

int RunWindow(const Report &report)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    std::string title = std::string("Gurumin native port scaffold - self-test ") +
                        (report.selfTestPassed ? "passed" : "FAILED") + ", game " +
                        (report.gameFound ? "found" : "not found") + " - Esc to quit";
    const int width = 800;
    const int height = 480;
    SDL_Window *window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width,
                                          height, SDL_WINDOW_SHOWN);
    if (window == nullptr)
    {
        std::fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
    if (renderer == nullptr)
    {
        std::fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    bool running = true;
    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                running = false;
            else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
                running = false;
        }
        Draw(renderer, report, width, height);
        SDL_Delay(16);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

}

int main(int argc, char **argv)
{
    bool probe = false;
    std::string game;
    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h")
        {
            std::fputs(kUsage, stdout);
            return 0;
        }
        else if (arg == "--probe")
        {
            probe = true;
        }
        else if (arg == "--game" && i + 1 < argc)
        {
            game = argv[++i];
        }
        else
        {
            std::fprintf(stderr, "unknown argument: %s\n\n%s", arg.c_str(), kUsage);
            return 2;
        }
    }

    Report report;
    SelfTest(report);
    if (!game.empty())
        ProbeGame(report, game);
    else
        std::printf("no --game folder given: only the self-test ran\n");

    if (probe)
    {
        // Headless: the exit code is the self-test result. A missing or incomplete game folder is reported, not
        // treated as a failure, so CI can run this without game files.
        return report.selfTestPassed ? 0 : 1;
    }
    return RunWindow(report);
}
