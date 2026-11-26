// The Steam Remote Storage interface as the game uses it (EN build): the system save file is mirrored in the cloud
// under its file name. Only the three calls the game makes are named. This is the one declaration in decomp_en.
#ifndef GURUMIN_EN_STEAM_STORAGE_H
#define GURUMIN_EN_STEAM_STORAGE_H

// ISteamRemoteStorage: one object of the Steam client, reached through SteamRemoteStorage().
class ISteamRemoteStorage
{
public:
    // Writes `size` bytes of `data` as cloud file `name`; vtable slot 0.
    virtual bool FileWrite(const char *name, const void *data, int size) = 0;
    // Reads up to `size` bytes of cloud file `name` into `data`; vtable slot 1.
    virtual int FileRead(const char *name, void *data, int size) = 0;
    // Slots 2 to 9 are not used by the game.
    virtual void Slot2() = 0;
    virtual void Slot3() = 0;
    virtual void Slot4() = 0;
    virtual void Slot5() = 0;
    virtual void Slot6() = 0;
    virtual void Slot7() = 0;
    virtual void Slot8() = 0;
    virtual void Slot9() = 0;
    // True when cloud file `name` exists; vtable slot 10 (offset 0x28).
    virtual bool FileExists(const char *name) = 0;
    virtual void Slot11() = 0;
    // Size in bytes of cloud file `name`; vtable slot 12 (offset 0x30).
    virtual int GetFileSize(const char *name) = 0;
};

// Returns the Steam client's remote storage object (steam_api.dll export, imported by name; EN import 0x847d38).
extern "C" __declspec(dllimport) ISteamRemoteStorage *__cdecl SteamRemoteStorage();

#endif
