// Tests for the port's own format readers (port/formats): .itm trailers and case-insensitive game paths.
// They use synthetic files only; no game data is needed.
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>

#include "game_folder.h"
#include "itm.h"

namespace
{
int g_checks = 0;
int g_failures = 0;

void Check(bool ok, const char *what, int line)
{
    g_checks++;
    if (!ok)
    {
        g_failures++;
        std::printf("  FAIL line %d: %s\n", line, what);
    }
}
#define CHECK(cond) Check((cond), #cond, __LINE__)
}

int main()
{
    namespace fs = std::filesystem;
    using namespace gurumin;

    // Trailer parsing from memory: body "abc", type 1 (text), sum 0x12345678, xor 0x0000de2f.
    const unsigned char data[] = {'a', 'b', 'c', 1, 0, 0, 0, 0x78, 0x56, 0x34, 0x12, 0x2f, 0xde, 0, 0};
    ItmTrailer trailer;
    CHECK(ParseItmTrailer(data, sizeof(data), trailer));
    CHECK(trailer.bodySize == 3 && trailer.contentType == ITM_CONTENT_TEXT);
    CHECK(trailer.plainSum == 0x12345678u && trailer.plainXor == 0xde2fu);
    CHECK(std::string(ItmContentTypeName(trailer.contentType)) == "text");
    CHECK(std::string(ItmContentTypeName(13309)) == "unknown");
    CHECK(!ParseItmTrailer(data, 11, trailer));

    // A fake game folder with mixed-case names, as on a Windows install.
    std::error_code ec;
    fs::path root = fs::temp_directory_path(ec) / "gurumin_formats_tests";
    fs::remove_all(root, ec);
    fs::create_directories(root / "Bin", ec);
    fs::create_directories(root / "3DDATA", ec);
    {
        FILE *file = std::fopen((root / "Bin" / "Map.ITM").string().c_str(), "wb");
        CHECK(file != nullptr);
        if (file != nullptr)
        {
            std::fwrite(data, 1, sizeof(data), file);
            std::fclose(file);
        }
        FILE *other = std::fopen((root / "3DDATA" / "a.it3").string().c_str(), "wb");
        if (other != nullptr)
            std::fclose(other);
    }
    std::string resolved = ResolveGamePath(root.string(), "bin\\map.itm");
    CHECK(!resolved.empty());
    std::string error;
    CHECK(ReadItmTrailer(resolved, trailer, error));
    CHECK(trailer.fileSize == sizeof(data) && trailer.contentType == ITM_CONTENT_TEXT);
    CHECK(ResolveGamePath(root.string(), "bin/missing.itm").empty());
    CHECK(IsDirectory(ResolveGamePath(root.string(), "3ddata")));
    FolderSummary summary;
    CHECK(SummariseFolder(ResolveGamePath(root.string(), "3ddata"), summary));
    CHECK(summary.files == 1 && summary.filesByExtension[".it3"] == 1);
    fs::remove_all(root, ec);

    std::printf("formats tests: %d checks, %d failed\n", g_checks, g_failures);
    return g_failures == 0 ? 0 : 1;
}
