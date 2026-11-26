#include "game_folder.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <system_error>
#include <vector>

namespace fs = std::filesystem;

namespace gurumin
{

namespace
{
std::string Lower(std::string text)
{
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return (char)std::tolower(c); });
    return text;
}

std::vector<std::string> SplitComponents(const std::string &relative)
{
    std::vector<std::string> parts;
    std::string current;
    for (char c : relative)
    {
        if (c == '/' || c == '\\')
        {
            if (!current.empty())
                parts.push_back(current);
            current.clear();
        }
        else
        {
            current += c;
        }
    }
    if (!current.empty())
        parts.push_back(current);
    return parts;
}
}

std::string ResolveGamePath(const std::string &root, const std::string &relative)
{
    std::error_code ec;
    fs::path current(root);
    if (!fs::is_directory(current, ec))
        return std::string();
    for (const std::string &part : SplitComponents(relative))
    {
        fs::path exact = current / part;
        if (fs::exists(exact, ec))
        {
            current = exact;
            continue;
        }
        const std::string wanted = Lower(part);
        bool found = false;
        for (fs::directory_iterator it(current, ec), end; !ec && it != end; it.increment(ec))
        {
            if (Lower(it->path().filename().string()) == wanted)
            {
                current = it->path();
                found = true;
                break;
            }
        }
        if (!found)
            return std::string();
    }
    return current.string();
}

bool IsDirectory(const std::string &path)
{
    std::error_code ec;
    return !path.empty() && fs::is_directory(fs::path(path), ec);
}

bool SummariseFolder(const std::string &path, FolderSummary &summary)
{
    std::error_code ec;
    if (!IsDirectory(path))
        return false;
    for (fs::directory_iterator it(fs::path(path), ec), end; !ec && it != end; it.increment(ec))
    {
        std::error_code fileEc;
        if (!it->is_regular_file(fileEc))
            continue;
        summary.files++;
        uint64_t size = it->file_size(fileEc);
        if (!fileEc)
            summary.bytes += size;
        std::string extension = Lower(it->path().extension().string());
        summary.filesByExtension[extension.empty() ? "(none)" : extension]++;
    }
    return true;
}

}
