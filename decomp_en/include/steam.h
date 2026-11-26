// Steamworks interfaces used by the EN Steam build (steam_api.dll imports) and the small wrappers around them.
// The Steamworks SDK headers are not part of this repository, so only the interface slots the game calls are declared;
// every other slot is a placeholder that keeps the virtual-table index right. Slot names that are not confirmed
// against the SDK carry the index, taken from the game's own call sites.
#ifndef GURUMIN_EN_STEAM_H
#define GURUMIN_EN_STEAM_H

// Opaque Steam pipe handle returned by GetHSteamPipe.
typedef int HSteamPipe;

// Steam account id (SDK type CSteamID): one 64-bit value, returned through a hidden pointer.
struct CSteamID {
    // The packed account id, universe, type and instance.
    unsigned __int64 steamId64;
};

// Remote storage (cloud save) interface; steam_api.dll export SteamRemoteStorage.
class ISteamRemoteStorage {
public:
    // Slot 0: writes a named cloud file; returns true on success.
    virtual bool FileWrite(const char *name, const void *data, int size) = 0;
    // Slot 1: reads a named cloud file into buffer; returns the bytes read.
    virtual int FileRead(const char *name, void *buffer, int size) = 0;
    virtual void Slot02() = 0;
    virtual void Slot03() = 0;
    virtual void Slot04() = 0;
    virtual void Slot05() = 0;
    virtual void Slot06() = 0;
    virtual void Slot07() = 0;
    virtual void Slot08() = 0;
    virtual void Slot09() = 0;
    // Slot 10: true when the named cloud file exists.
    virtual bool FileExists(const char *name) = 0;
    virtual void Slot11() = 0;
    // Slot 12: size in bytes of the named cloud file.
    virtual int GetFileSize(const char *name) = 0;
};

// User interface; steam_api.dll export SteamUser.
class ISteamUser {
public:
    virtual void GetHSteamUser() = 0;
    virtual void BLoggedOn() = 0;
    // Slot 2: writes the account id of the logged-in user into *id (the SDK returns it by value, through this hidden pointer).
    virtual void GetSteamID(CSteamID *id) = 0;
};

// Stats and achievements interface; steam_api.dll export SteamUserStats.
class ISteamUserStats {
public:
    virtual void Slot00() = 0;
    virtual void Slot01() = 0;
    virtual void Slot02() = 0;
    virtual void Slot03() = 0;
    virtual void Slot04() = 0;
    virtual void Slot05() = 0;
    virtual void Slot06() = 0;
    // Slot 7: unlocks the named achievement.
    virtual bool SetAchievement(const char *name) = 0;
    virtual void Slot08() = 0;
    virtual void Slot09() = 0;
    // Slot 10: sends the pending stats and achievements to Steam.
    virtual bool StoreStats() = 0;
};

// Utility interface obtained from the Steam client.
class ISteamUtils {
public:
    virtual void Slot00() = 0;
    virtual void Slot01() = 0;
    virtual void Slot02() = 0;
    virtual void Slot03() = 0;
    virtual void Slot04() = 0;
    virtual void Slot05() = 0;
    virtual void Slot06() = 0;
    virtual void Slot07() = 0;
    virtual void Slot08() = 0;
    virtual void Slot09() = 0;
    virtual void Slot10() = 0;
    virtual void Slot11() = 0;
    virtual void Slot12() = 0;
    virtual void Slot13() = 0;
    virtual void Slot14() = 0;
    virtual void Slot15() = 0;
    virtual void Slot16() = 0;
    virtual void Slot17() = 0;
    virtual void Slot18() = 0;
    virtual void Slot19() = 0;
    // Slot 20: opens the on-screen (Big Picture) text input; true when it was shown.
    virtual bool ShowGamepadTextInput(int inputMode, int lineInputMode, const char *description, unsigned maxCharacters) = 0;
    // Slot 21: length of the text entered in the on-screen input.
    virtual unsigned GetEnteredGamepadTextLength() = 0;
    // Slot 22: copies the entered text into the buffer; true on success.
    virtual bool GetEnteredGamepadTextInput(char *text, unsigned textSize) = 0;
};

// Steam client interface; slot 9 returns the utility interface for a pipe and version string.
class ISteamClient {
public:
    virtual void Slot00() = 0;
    virtual void Slot01() = 0;
    virtual void Slot02() = 0;
    virtual void Slot03() = 0;
    virtual void Slot04() = 0;
    virtual void Slot05() = 0;
    virtual void Slot06() = 0;
    virtual void Slot07() = 0;
    virtual void Slot08() = 0;
    // Slot 9: returns the utility interface of the given pipe for an interface version string.
    virtual ISteamUtils *GetISteamUtils(HSteamPipe pipe, const char *version) = 0;
};

// steam_api.dll accessors (imports in the game).
extern "C" __declspec(dllimport) ISteamRemoteStorage *__cdecl SteamRemoteStorage();
extern "C" __declspec(dllimport) ISteamUserStats *__cdecl SteamUserStats();
extern "C" __declspec(dllimport) ISteamUser *__cdecl SteamUser();
extern "C" __declspec(dllimport) ISteamClient *__cdecl SteamClient();
extern "C" __declspec(dllimport) HSteamPipe __cdecl GetHSteamPipe();
extern "C" __declspec(dllimport) void __cdecl SteamAPI_RunCallbacks();   // dispatches the pending Steam callbacks

// Wrappers the game calls (EN-only, no JP twin).
// Cloud file size and existence queries; the game ignores the results. EN 0x005ec220 and 0x005e7b30.
extern "C" void Steam_GetCloudFileSize(const char *name);
extern "C" bool Steam_CloudFileExists(const char *name);
// Reads a cloud file; true when any bytes came back. EN 0x005e7b50.
extern "C" bool Steam_CloudFileRead(const char *name, void *buffer, int size);
// Writes a cloud file and returns the Steam result. EN 0x005e7b80.
extern "C" bool Steam_CloudFileWrite(const char *name, const void *data, int size);
// Stores the logged-in user's account id into *id and returns id. EN 0x005ee4b0.
extern "C" CSteamID *Steam_GetUserId(CSteamID *id);
// Unlocks an achievement and sends the stats to Steam. EN 0x00642ca0.
extern "C" bool Steam_UnlockAchievement(const char *name);
// Reads the on-screen text input into a buffer. EN 0x005ec1e0.
extern "C" bool Steam_GetEnteredText(char *text, unsigned size);
// Opens the on-screen text input and remembers that it is open. EN 0x00646a90.
extern "C" void Steam_ShowTextInput(const char *description, unsigned maxCharacters);
// Steam callback payload GamepadTextInputDismissed_t (callback id 714, 8 bytes).
struct GamepadTextInputDismissed {
    bool submitted;                // 0x00: true when the player confirmed the text
    unsigned int submittedLength;  // 0x04: length of the submitted text
};

// Receiver of the on-screen text input callback (the game's CCallback target).
struct TextInputReceiver {
    char *submittedOut;            // 0x00: where the "text was submitted" flag is stored
    // EN 0x7a9d00: copies the flag out and marks the input as dismissed (g_steamTextInputShown = 2).
    void OnDismissed(const GamepadTextInputDismissed *result);
};
// Value of g_steamTextInputShown once the on-screen input was dismissed.
const int STEAM_TEXT_INPUT_DISMISSED = 2;

// Set once the on-screen text input has been shown. EN 0x00924d50.
extern int g_steamTextInputShown;

#endif
