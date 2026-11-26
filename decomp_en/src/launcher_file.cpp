// EN-only launcher: file attribute helper written against MFC's CFile (see NOTES.md "Launcher (MFC)").
#include "launcher_dialog.h"

// Makes the file `path` writable: reads its status, clears the read-only bit and writes the status back. Does nothing
// when the file cannot be found. Called by the launcher before it saves a settings file.
extern "C" void Launcher_ClearReadOnly(const char *path)
{
    CFileStatus status;
    if (CFile::GetStatus(path, status) != 0)
    {
        status.m_attribute &= ~CFile::readOnly;
        CFile::SetStatus(path, status);
    }
}
