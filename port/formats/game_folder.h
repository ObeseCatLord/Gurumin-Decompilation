// Finding files in the user's own installed copy of the game. Windows file names are case-insensitive and the game
// spells them in mixed case ("bin\\map.itm", "3ddata"), so on Linux each path component is matched ignoring case.
// Nothing here writes to the game folder.
#ifndef GURUMIN_PORT_GAME_FOLDER_H
#define GURUMIN_PORT_GAME_FOLDER_H

#include <cstdint>
#include <map>
#include <string>

namespace gurumin
{

// Resolves `relative` ('/' or '\\' separated) under `root`, matching each component case-insensitively.
// Returns the real path, or an empty string when some component does not exist.
std::string ResolveGamePath(const std::string &root, const std::string &relative);

// True when `path` is an existing directory.
bool IsDirectory(const std::string &path);

// Summary of one data folder: number of regular files and their total size, per lower-case extension.
struct FolderSummary
{
    uint64_t files = 0;
    uint64_t bytes = 0;
    std::map<std::string, uint64_t> filesByExtension;
};

// Summarises the regular files directly inside `path` (not recursive). Returns false if it is not a directory.
bool SummariseFolder(const std::string &path, FolderSummary &summary);

}

#endif
