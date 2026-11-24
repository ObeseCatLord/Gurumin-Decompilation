// EN-only Steam wrappers (cloud save, achievements, on-screen text input) around the steam_api.dll interfaces.
#include "steam.h"

// Interface version of the Steam utility interface the game requests.
static const char STEAM_UTILS_VERSION[] = "SteamUtils006";

// Set once the on-screen text input has been shown; EN 0x00924d50.
int g_steamTextInputShown;

// Asks the cloud for the size of a file; the game ignores the answer. Called by the save code.
extern "C" void Steam_GetCloudFileSize(const char *name)
{
    SteamRemoteStorage()->GetFileSize(name);
}

// Asks the cloud whether a file exists; the game ignores the answer. Called by the save code.
extern "C" void Steam_CloudFileExists(const char *name)
{
    SteamRemoteStorage()->FileExists(name);
}

// Reads a cloud file into buffer; true when any bytes came back.
extern "C" bool Steam_CloudFileRead(const char *name, void *buffer, int size)
{
    return SteamRemoteStorage()->FileRead(name, buffer, size) != 0;
}

// Writes a cloud file.
extern "C" bool Steam_CloudFileWrite(const char *name, const void *data, int size)
{
    return SteamRemoteStorage()->FileWrite(name, data, size);
}

// Writes the logged-in user's Steam id into *id and returns id.
extern "C" CSteamID *Steam_GetUserId(CSteamID *id)
{
    SteamUser()->GetSteamID(id);
    return id;
}

// Unlocks an achievement and sends the stats to Steam.
extern "C" void Steam_UnlockAchievement(const char *name)
{
    SteamUserStats()->SetAchievement(name);
    SteamUserStats()->StoreStats();
}

// Reads the text typed into the Steam on-screen input.
extern "C" void Steam_GetEnteredText(char *text, unsigned size)
{
    SteamClient()->GetISteamUtils(GetHSteamPipe(), STEAM_UTILS_VERSION)->GetEnteredGamepadTextInput(text, size);
}

// Opens the Steam on-screen text input and remembers that it is open.
extern "C" void Steam_ShowTextInput(const char *description, unsigned maxCharacters)
{
    if (SteamClient()->GetISteamUtils(GetHSteamPipe(), STEAM_UTILS_VERSION)->ShowGamepadTextInput(0, 0, description, maxCharacters)) {
        g_steamTextInputShown = 1;
    }
}
